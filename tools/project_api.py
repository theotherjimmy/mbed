import sys
from os.path import join, abspath, dirname, exists
from os.path import basename, relpath, normpath
from os import mkdir
ROOT = abspath(join(dirname(__file__), ".."))
sys.path.insert(0, ROOT)

from tools.build_api import prepare_toolchain, prep_report, create_result
from tools.build_api import prep_properties, scan_resources
from tools.build_api import add_result_to_report
from tools.export import EXPORTERS
from time import time
from shutil import rmtree
import zipfile
import copy
import os


def get_exporter_toolchain(ide):
    """ Return the exporter class and the toolchain string as a tuple

    Positional arguments:
    ide - the ide name of an exporter
    """
    return EXPORTERS[ide], EXPORTERS[ide].TOOLCHAIN

def new_basepath(val, resources, export_path):
    new_f = relpath(val, resources.file_basepath[val])
    resources.file_basepath[join(export_path, new_f)] = export_path
    return new_f


def subtract_basepath(resources, export_path):
    keys = ['s_sources', 'c_sources', 'cpp_sources', 'hex_files',
            'objects', 'libraries', 'inc_dirs', 'headers', 'linker_script']
    for key in keys:
        vals = getattr(resources, key)
        if type(vals) is list:
            new_vals = []
            for val in vals:
                new_vals.append(new_basepath(val, resources, export_path))
            setattr(resources, key, new_vals)
        else:
            setattr(resources, key, new_basepath(vals, resources, export_path))


def export_project(src_paths, export_path, target, ide,
                   libraries_paths=None, options=None, linker_script=None,
                   clean=False, notify=None, verbose=False, name=None,
                   inc_dirs=None, jobs=1, silent=False,
                   report=None, properties=None, project_id=None,
                   project_description=None, extra_verbose=False, config=None,
                   build=False, macros=[]):
    """ This function builds a project. Project can be for example one test / UT
    """

    # Convert src_path to a list if needed
    if type(src_paths) != type([]):
        src_paths = [src_paths]
    # Extend src_paths wiht libraries_paths
    if libraries_paths is not None:
        src_paths.extend(libraries_paths)

    # Export Directory
    if clean:
        if exists(export_path):
            rmtree(export_path)
    mkdir(export_path)

    exporter_cls, toolchain_name = get_exporter_toolchain(ide)

    # Pass all params to the unified prepare_resources()
    toolchain = prepare_toolchain(src_paths, export_path, target,
                                  toolchain_name, macros=macros,
                                  options=options, clean=clean, jobs=jobs,
                                  notify=notify, silent=silent, verbose=verbose,
                                  extra_verbose=extra_verbose, config=config)


    # The first path will give the name to the library
    if name is None:
        name = basename(normpath(abspath(src_paths[0])))

    # Initialize reporting
    if report != None:
        start = time()
        # If project_id is specified, use that over the default name
        id_name = project_id.upper() if project_id else name.upper()
        description = project_description if project_description else name
        vendor_label = target.extra_labels[0]
        prep_report(report, target.name, toolchain_name, id_name)
        cur_result = create_result(target.name, toolchain_name, id_name,
                                   description)
        if properties != None:
            prep_properties(properties, target.name, toolchain_name,
                            vendor_label)

    try:
        # Call unified scan_resources
        resources = scan_resources(src_paths, toolchain, inc_dirs=inc_dirs)
        toolchain.build_dir = export_path
        config_header = toolchain.get_config_header()
        resources.headers.append(config_header)
        resources.file_basepath[config_header] = dirname(config_header)

        # Change linker script if specified
        if linker_script is not None:
            resources.linker_script = linker_script


        temp = copy.deepcopy(resources)
        subtract_basepath(resources,export_path)
        exporter = exporter_cls(target, export_path, name, toolchain,
                                extra_symbols=macros, resources=resources)
        exporter.generate()
        files = exporter.generated_files

        if report != None:
            end = time()
            cur_result["elapsed_time"] = end - start
            cur_result["output"] = toolchain.get_output()
            cur_result["result"] = "OK"
            cur_result["memory_usage"] = toolchain.map_outputs

            add_result_to_report(report, cur_result)

        return files, temp

    except Exception:
        if report != None:
            end = time()

            cur_result["result"] = "FAIL"

            cur_result["elapsed_time"] = end - start

            toolchain_output = toolchain.get_output()
            if toolchain_output:
                cur_result["output"] += toolchain_output

            add_result_to_report(report, cur_result)

        # Let Exception propagate
        raise

def zip_export(file_name, prefix, resources, project_files):
    """Create a zip file from an exported project.

    Positional Parameters:
    file_name - the file name of the resulting zip file
    prefix - a directory name that will prefix the entire zip file's contents
    resources - a resources object with files that must be included in the zip
    project_files - a list of extra files to be added to the root of the prefix
      directory
    """
    with zipfile.ZipFile(file_name, "w") as zip_file:
        for prj_file in project_files:
            zip_file.write(prj_file, join(prefix, basename(prj_file)))
        for source in resources.headers + resources.s_sources + \
            resources.c_sources + resources.cpp_sources + \
            resources.libraries + resources.hex_files + \
            [resources.linker_script] + resources.bin_files \
            + resources.objects + resources.json_files:
            zip_file.write(source,
                      join(prefix, relpath(source,
                                           resources.file_basepath[source])))

def print_results(successes, failures, skips=None):
    """ Print out the results of an export process

    Positional arguments:
    successes - The list of exports that succeeded
    failures - The list of exports that failed

    Keyword arguments:
    skips - The list of exports that were skipped
    """
    print
    if successes:
        print "Successful: "
        for success in successes:
            print "  * %s" % success
    if failures:
        print "Failed: "
        for failure in failures:
            print "  * %s" % failure
    if skips:
        print "Skipped: "
        for skip in skips:
            print "  * %s" % skip

