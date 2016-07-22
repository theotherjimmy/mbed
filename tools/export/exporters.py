"""Just a template for subclassing"""
import os
import logging
from os.path import join, dirname, relpath
from itertools import groupby
from jinja2 import FileSystemLoader
from jinja2.environment import Environment

from tools.targets import TARGET_MAP
from project_generator.project import Project, ProjectTemplateInternal
from project_generator.settings import ProjectSettings


class OldLibrariesException(Exception): pass

class FailedBuildException(Exception) : pass

# Exporter descriptor for TARGETS
# TARGETS as class attribute for backward compatibility (allows: if in Exporter.TARGETS)
class ExporterTargetsProperty(object):
    def __init__(self, func):
        self.func = func
    def __get__(self, inst, cls):
        return self.func(cls)

class Exporter(object):
    TEMPLATE_DIR = dirname(__file__)
    DOT_IN_RELATIVE_PATH = False

    def __init__(self, target, export_dir, project_name, toolchain, extra_symbols=[],
                 resources=None):
        """Initialize an instance of class exporter
        Positional arguments:
        target        - the target mcu/board for this project
        export_dir    - the directory of the exported project files
        project_name  - the name of the project
        toolchain     - an instance of class toolchain
        extra_symbols - a list of extra macros for the toolchain

        Keyword arguments:
        resources     - an instance of class Resources
        """
        self.export_dir = export_dir
        self.target = target
        self.project_name = project_name
        self.toolchain = toolchain
        self.resources = resources
        self.symbols = self.toolchain.get_symbols()
        self.generated_files = []

        # Add extra symbols and config file symbols to the Exporter's list of
        # symbols.
        config_macros = self.toolchain.config.get_config_data_macros()
        if config_macros:
            self.symbols.extend(config_macros)
        if extra_symbols:
            self.symbols.extend(extra_symbols)

    def get_toolchain(self):
        return self.TOOLCHAIN

    @property
    def flags(self):
        """Returns a dictionary of toolchain flags.
        Keys of the dictionary are:
        cxx_flags    - c++ flags
        c_flags      - c flags
        ld_flags     - linker flags
        asm_flags    - assembler flags
        common_flags - common options
        """
        config_header = self.toolchain.get_config_header()
        config_header = relpath(config_header, self.resources.file_basepath[config_header])
        flags = dict([(key + "_flags", value) for key,value in self.toolchain.flags.iteritems()])
        if config_header:
            flags['c_flags'] += self.toolchain.get_config_option(config_header)
            flags['cxx_flags'] += self.toolchain.get_config_option(config_header)
        return flags

    def get_source_paths(self):
        """Returns a list of the directories where source files are contained"""
        source_keys = ['s_sources', 'c_sources', 'cpp_sources', 'hex_files', 'objects', 'libraries']
        source_files = []
        [source_files.extend(getattr(self.resources,key)) for key in source_keys]
        return list(set([os.path.dirname(src) for src in source_files]))

    def progen_get_project_data(self):
        """ Get ProGen project data  """
        # provide default data, some tools don't require any additional
        # tool specific settings

        def make_key(src):
            key = os.path.basename(os.path.dirname(src))
            if not key:
                key = os.path.basename(self.export_dir)
            return key

        def grouped(sources):
            # Group the source files by their encompassing directory
            data = sorted(sources, key=make_key)
            return {k:list(g) for k,g in groupby(data,make_key)}

        project_data = ProjectTemplateInternal._get_project_template()

        project_data['target'] = TARGET_MAP[self.target].progen['target']
        project_data['source_paths'] = self.get_source_paths()
        project_data['include_paths'] = self.resources.inc_dirs
        project_data['include_files'] = grouped(self.resources.headers)
        project_data['source_files_s'] = grouped(self.resources.s_sources)
        project_data['source_files_c'] = grouped(self.resources.c_sources)
        project_data['source_files_cpp'] = grouped(self.resources.cpp_sources)
        project_data['source_files_obj'] = grouped(self.resources.objects)
        project_data['source_files_lib'] = grouped(self.resources.libraries)
        project_data['output_dir']['path'] = self.export_dir
        project_data['linker_file'] = self.resources.linker_script
        project_data['macros'] = self.symbols
        project_data['build_dir'] = '.build'
        project_data['template'] = None
        project_data['name'] = self.project_name
        project_data['output_type'] = 'exe'
        project_data['debugger'] = None
        return project_data

    def progen_gen_file(self, tool_name, project_data, progen_build=False):
        """ Generate project using ProGen Project API
        Positional arguments:
        tool_name    - the tool for which to generate project files
        project_data - a dict whose base key, values are specified in
                       progen_get_project_data, the items will have been
                       modified by Exporter subclasses

        Keyword arguments:
        progen_build - A boolean that determines if the tool will build the
                       project
        """
        settings = ProjectSettings()
        project = Project(self.project_name, [project_data], settings)
        project.project['export'] = project_data.copy()
        project.generate(tool_name, copied=False, fill=False)
        for _, dict in project.generated_files.iteritems():
            for feild, thing in dict.iteritems():
                if feild == "files":
                    for __, filename in thing.iteritems():
                        self.generated_files.append(filename)
        if progen_build:
            print("Project exported, building...")
            result = project.build(tool_name)
            if result == -1:
                raise FailedBuildException("Build Failed")

    def gen_file(self, template_file, data, target_file):
        """Generates a project file from a template using jinja"""
        jinja_loader = FileSystemLoader(os.path.dirname(os.path.abspath(__file__)))
        jinja_environment = Environment(loader=jinja_loader)

        template_path = join(Exporter.TEMPLATE_DIR, template_file)
        template = jinja_environment.get_template(template_file)
        target_text = template.render(data)

        target_path = join(self.export_dir, target_file)
        logging.debug("Generating: %s" % target_path)
        open(target_path, "w").write(target_text)
        self.generated_files += target_path
