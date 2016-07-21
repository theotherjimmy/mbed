import sys
from os.path import join, abspath, dirname, exists, basename, relpath, split, isdir
from os import listdir
from shutil import copyfile
ROOT = abspath(join(dirname(__file__), ".."))
sys.path.insert(0, ROOT)


from tools.libraries import LIBRARIES
from tools.build_api import *
from tools.export.__init__ import EXPORTERS
#from tools.export.exporters import TargetNotSupported
import yaml
import zipfile
import copy


def get_exporter_toolchain(ide):
    return EXPORTERS[ide], EXPORTERS[ide].TOOLCHAIN

def export_project(src_paths, export_path, target, ide,
                   libraries_paths=None, options=None, linker_script=None,
                   clean=False, notify=None, verbose=False, name=None, macros=[],
                   inc_dirs=None, jobs=1, silent=False, report=None, properties=None,
                   project_id=None, project_description=None, extra_verbose=False,
                   config=None, build=False):
    """ This function builds a project. Project can be for example one test / UT
    """

    # Convert src_path to a list if needed
    if type(src_paths) != ListType:
        src_paths = [src_paths]
    # Extend src_paths wiht libraries_paths
    if libraries_paths is not None:
        src_paths.extend(libraries_paths)

    # Export Directory
    if clean:
        if exists(export_path):
            rmtree(export_path)
    mkdir(export_path)

    Exporter, toolchain_name = get_exporter_toolchain(ide)

    # Pass all params to the unified prepare_resources()
    toolchain = prepare_toolchain(src_paths, export_path, target, toolchain_name,
                                  macros=macros, options=options, clean=clean, jobs=jobs,
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
        cur_result = create_result(target.name, toolchain_name, id_name, description)
        if properties != None:
            prep_properties(properties, target.name, toolchain_name, vendor_label)

    try:
        # Call unified scan_resources
        resources = scan_resources(src_paths, toolchain, inc_dirs=inc_dirs)
        temp = copy.deepcopy(resources)
        resources.relative_to(export_path)

        # Change linker script if specified
        if linker_script is not None:
            resources.linker_script = linker_script

        # Export project files
        exporter = Exporter(target, export_path, name, toolchain, extra_symbols=macros, resources=resources)

        res = exporter.generate()
        files = exporter.generated_files

        if report != None:
            end = time()
            cur_result["elapsed_time"] = end - start
            cur_result["output"] = toolchain.get_output()
            cur_result["result"] = "OK"
            cur_result["memory_usage"] = toolchain.map_outputs

            add_result_to_report(report, cur_result)

        return files, temp

    except Exception, e:
        if report != None:
            end = time()

            if isinstance(e, TargetNotSupported):
                cur_result["result"] = "NOT_SUPPORTED"
            else:
                cur_result["result"] = "FAIL"

            cur_result["elapsed_time"] = end - start

            toolchain_output = toolchain.get_output()
            if toolchain_output:
                cur_result["output"] += toolchain_output

            add_result_to_report(report, cur_result)

        # Let Exception propagate
        raise

def zip_export(filename, prefix, resources, project_files):
    with zipfile.ZipFile(filename, "w") as zip:
        for file in project_files:
            zip.write(file, join(prefix, basename(file)))
        for source in resources.headers + resources.s_sources + resources.c_sources + resources.cpp_sources + resources.libraries + resources.hex_files + [resources.linker_script] + resources.bin_files + resources.objects + resources.json_files:
            zip.write(source, join(prefix, relpath(source, resources.file_basepath[source])))

def print_results(successes, failures, skips = []):
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

