""" The CLI entry point for exporting projects from the mbed tools to any of the
supported IDEs or project structures.
"""
import sys
from os.path import join, abspath, dirname, exists, basename
ROOT = abspath(join(dirname(__file__), ".."))
sys.path.insert(0, ROOT)
from shutil import rmtree
from argparse import ArgumentParser
from os.path import normpath
import copy

from tools.paths import EXPORT_DIR, MBED_BASE, MBED_LIBRARIES
from tools.export import EXPORTERS, mcu_ide_matrix
from tools.tests import TESTS, TEST_MAP
from tools.tests import test_known, test_name_known, Test
from tools.targets import TARGET_NAMES
from tools.utils import argparse_filestring_type, argparse_many
from tools.utils import argparse_force_lowercase_type
from tools.utils import argparse_force_uppercase_type
from tools.project_api import export_project, zip_export, prepare_project
from tools.project_api import subtract_basepath


def setup_project(ide, target, program=None, source_dir=None, build=None):
    """Generate a name, if not provided, and find dependencies

    Positional arguments:
    ide - IDE or project structure that will soon be exported to
    target - MCU that the project will build for

    Keyword arguments:
    program - the index of a test program
    source_dir - the directory, or directories that contain all of the sources
    build - a directory that will contain the result of the export
    """
    # Some libraries have extra macros (called by exporter symbols) to we need
    # to pass them to maintain compilation macros integrity between compiled
    # library and header files we might use with it
    if source_dir:
        # --source is used to generate IDE files to toolchain directly
        # in the source tree and doesn't generate zip file
        project_dir = source_dir[0]
        if program:
            project_name = TESTS[program]
        else:
            project_name = basename(normpath(source_dir[0]))
        src_paths = source_dir
        lib_paths = None
    else:
        test = Test(program)
        if not build:
            # Substitute the mbed library builds with their sources
            if MBED_LIBRARIES in test.dependencies:
                test.dependencies.remove(MBED_LIBRARIES)
                test.dependencies.append(MBED_BASE)

        src_paths = [test.source_dir]
        lib_paths = test.dependencies
        project_name = "_".join([test.id, ide, target])
        project_dir = join(EXPORT_DIR, project_name)

    return project_dir, project_name, src_paths, lib_paths


def export(target, ide, build=None, src=None, macros=None, project_id=None,
            clean=False, zip_proj=False):
    """Do an export of a project.

    Positional arguments:
    target - MCU that the project will compile for
    ide - the IDE or project structure to export to

    Keyword arguments:
    build - to use the compiled mbed libraries or not
    src - directory or directories that contain the source to export
    macros - extra macros to add to the project
    project_id - the name of the project
    clean - start from a clean state before exporting
    zip_proj - create a zip file or not
    """
    project_dir, name, src, lib = setup_project(ide, target, program=project_id,
                                           source_dir=src, build=build)

    resources, toolchain = prepare_project(src, project_dir, target, ide,
                                           clean=clean, name=name,
                                           macros=macros, libraries_paths=lib)

    temp = copy.deepcopy(resources)
    if zip_proj:
        subtract_basepath(resources, export)
    else:
        resources.relative_to(export)

    files, _ = export_project(resources, export, target, name, toolchain, ide,
                              macros=macros)
    if zip_proj:
        zip_export(export + ".zip", name, temp, files)


def main():
    """Entry point"""
    # Parse Options
    parser = ArgumentParser()

    targetnames = TARGET_NAMES
    targetnames.sort()
    toolchainlist = EXPORTERS.keys()
    toolchainlist.sort()

    parser.add_argument("-m", "--mcu",
                        metavar="MCU",
                        default='LPC1768',
                        type=argparse_many(
                            argparse_force_uppercase_type(targetnames, "MCU")),
                        help="generate project for the given MCU ({})".format(
                            ', '.join(targetnames)))

    parser.add_argument("-i",
                        dest="ide",
                        default='uvision',
                        type=argparse_force_lowercase_type(
                            toolchainlist, "toolchain"),
                        help="The target IDE: %s"% str(toolchainlist))

    parser.add_argument("-c", "--clean",
                      action="store_true",
                      default=False,
                      help="clean the export directory")

    group = parser.add_mutually_exclusive_group(required=False)
    group.add_argument(
        "-p",
        type=test_known,
        dest="program",
        help="The index of the desired test program: [0-%s]"% (len(TESTS)-1))

    group.add_argument("-n",
                      type=test_name_known,
                      dest="program",
                      help="The name of the desired test program")

    parser.add_argument("-b",
                      dest="build",
                      action="store_true",
                      default=False,
                      help="use the mbed library build, instead of the sources")

    group.add_argument("-L", "--list-tests",
                      action="store_true",
                      dest="list_tests",
                      default=False,
                      help="list available programs in order and exit")

    group.add_argument("-S", "--list-matrix",
                      action="store_true",
                      dest="supported_ides",
                      default=False,
                      help="displays supported matrix of MCUs and IDEs")

    parser.add_argument("-E",
                      action="store_true",
                      dest="supported_ides_html",
                      default=False,
                      help="writes tools/export/README.md")

    parser.add_argument("--source",
                      action="append",
                      type=argparse_filestring_type,
                      dest="source_dir",
                      default=[],
                      help="The source (input) directory")

    parser.add_argument("-D",
                      action="append",
                      dest="macros",
                      help="Add a macro definition")

    options = parser.parse_args()

    # Print available tests in order and exit
    if options.list_tests is True:
        print '\n'.join([str(test) for test in  sorted(TEST_MAP.values())])
        sys.exit()

    # Only prints matrix of supported IDEs
    if options.supported_ides:
        print mcu_ide_matrix()
        exit(0)

    # Only prints matrix of supported IDEs
    if options.supported_ides_html:
        html = mcu_ide_matrix(verbose_html=True)
        try:
            with open("./export/README.md", "w") as readme:
                readme.write("Exporter IDE/Platform Support\n")
                readme.write("-----------------------------------\n")
                readme.write("\n")
                readme.write(html)
        except IOError as exc:
            print "I/O error({0}): {1}".format(exc.errno, exc.strerror)
        except:
            print "Unexpected error:", sys.exc_info()[0]
            raise
        exit(0)

    # Clean Export Directory
    if options.clean:
        if exists(EXPORT_DIR):
            rmtree(EXPORT_DIR)

    for mcu in options.mcu:
        zip_proj = options.source_dir is not []

        # Export to selected toolchain
        #lib_symbols = get_lib_symbols(options.macros, src, p)
        export(mcu, options.ide, build=options.build, src=options.source_dir,
               macros=options.macros, project_id=options.project,
               clean=options.clean, zip_proj=zip_proj)


if __name__ == "__main__":
    main()
