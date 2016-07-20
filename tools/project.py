import sys
from os.path import join, abspath, dirname, exists, basename
ROOT = abspath(join(dirname(__file__), ".."))
sys.path.insert(0, ROOT)

from shutil import move, rmtree
from argparse import ArgumentParser
from os.path import normpath


from tools.paths import EXPORT_DIR, MBED_BASE, MBED_LIBRARIES, EXPORT_WORKSPACE
from tools.export import EXPORTERS, mcu_ide_matrix
from tools.tests import TESTS, TEST_MAP
from tools.tests import test_known, test_name_known, Test
from tools.targets import TARGET_NAMES, TARGET_MAP
from utils import argparse_filestring_type, argparse_many
from utils import argparse_force_lowercase_type, argparse_force_uppercase_type
from tools.libraries import LIBRARIES
from project_api import export_project, get_exporter_toolchain, get_src_paths, copy_resources
from tools.build_api import scan_resources, prepare_toolchain

def get_lib_symbols(macros, src, program):
    # Some libraries have extra macros (called by exporter symbols) to we need to pass
    # them to maintain compilation macros integrity between compiled library and
    # header files we might use with it
    lib_symbols = []
    if macros:
        lib_symbols += macros
    if src:
        return lib_symbols
    test = Test(program)
    for lib in LIBRARIES:
        if lib['build_dir'] in test.dependencies:
            lib_macros = lib.get('macros', None)
            if lib_macros is not None:
                lib_symbols.extend(lib_macros)
    return lib_symbols


def setup_project(ide, target, program=None, source_dir=None, build=None, macros=None, clean=False):
    # Some libraries have extra macros (called by exporter symbols) to we need to pass
    # them to maintain compilation macros integrity between compiled library and
    # header files we might use with it
    lib_symbols = get_lib_symbols(macros, source_dir, program)
    if source_dir:
        # --source is used to generate IDE files to toolchain directly in the source tree and doesn't generate zip file
        project_dir = source_dir
        project_name = TESTS[program] if program else basename(normpath(source_dir))
        test = None
        src_paths = source_dir
        lib_paths=None
    else:
        test = Test(program)
        if not build:
            # Substitute the library builds with the sources
            # TODO: Substitute also the other library build paths
            if MBED_LIBRARIES in test.dependencies:
                test.dependencies.remove(MBED_LIBRARIES)
                test.dependencies.append(MBED_BASE)

        # Build the project with the same directory structure of the mbed online IDE
        src_paths = test.source_dir
        lib_paths = test.dependencies
        get_src_paths(src_paths, lib_paths)
        project_name = test.id
        project_dir = join(EXPORT_WORKSPACE, project_name)

    return project_dir, project_name, [src_paths]+lib_paths


def perform_export(target, ide, build=None, src=None,
                    macros=None, project_id=None, clean=False, zip=False):
    export, name, paths = setup_project(ide, target, program=project_id, source_dir=src, build=build, macros=macros, clean=clean)


    export_project(paths, export, target, ide,
                   clean=clean, name=name, macros=macros, build=build)
    return None, None

if __name__ == '__main__':
    # Parse Options
    parser = ArgumentParser()

    targetnames = TARGET_NAMES
    targetnames.sort()
    toolchainlist = EXPORTERS.keys()
    toolchainlist.sort()

    parser.add_argument("-m", "--mcu",
                      metavar="MCU",
                      default='LPC1768',
                      type=argparse_many(argparse_force_uppercase_type(targetnames, "MCU")),
                      help="generate project for the given MCU (%s)"% ', '.join(targetnames))

    parser.add_argument("-i",
                      dest="ide",
                      default='uvision',
                      type=argparse_force_lowercase_type(toolchainlist, "toolchain"),
                      help="The target IDE: %s"% str(toolchainlist))

    parser.add_argument("-c", "--clean",
                      action="store_true",
                      default=False,
                      help="clean the export directory")

    group = parser.add_mutually_exclusive_group(required=False)
    group.add_argument("-p",
                      type=test_known,
                      dest="program",
                      help="The index of the desired test program: [0-%d]"% (len(TESTS)-1))

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
        print '\n'.join(map(str, sorted(TEST_MAP.values())))
        sys.exit()

    # Only prints matrix of supported IDEs
    if options.supported_ides:
        print mcu_ide_matrix()
        exit(0)

    # Only prints matrix of supported IDEs
    if options.supported_ides_html:
        html = mcu_ide_matrix(verbose_html=True)
        try:
            with open("./export/README.md","w") as f:
                f.write("Exporter IDE/Platform Support\n")
                f.write("-----------------------------------\n")
                f.write("\n")
                f.write(html)
        except IOError as e:
            print "I/O error({0}): {1}".format(e.errno, e.strerror)
        except:
            print "Unexpected error:", sys.exc_info()[0]
            raise
        exit(0)

    # Clean Export Directory
    if options.clean:
        if exists(EXPORT_DIR):
            rmtree(EXPORT_DIR)

    # Export results
    successes = []
    failures = []

    # source_dir = use relative paths, otherwise sources are copied
    sources_relative = True if options.source_dir else False

    for mcu in options.mcu:
        # Program Number or name
        p, src, ide = options.program, options.source_dir, options.ide

        zip = src is not []  # create zip when no src_dir provided
        clean = src is not []  # don't clean when source is provided, use acrual source tree for IDE files

        # Export to selected toolchain
        #lib_symbols = get_lib_symbols(options.macros, src, p)
        tmp_path, report = perform_export(mcu,ide,build=options.build,src=src, macros=options.macros, project_id=p, clean=clean)
        print report
