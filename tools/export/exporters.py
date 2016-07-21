"""Just a template for subclassing"""
import uuid, shutil, os, logging, fnmatch
from os import walk, remove
from os.path import join, dirname, isdir, split
from copy import copy
from jinja2 import Template, FileSystemLoader
from jinja2.environment import Environment
from contextlib import closing
from zipfile import ZipFile, ZIP_DEFLATED
from operator import add
from itertools import groupby

from tools.utils import mkdir
from tools.toolchains import TOOLCHAIN_CLASSES
from tools.targets import TARGET_MAP

from project_generator.generate import Generator
from project_generator.project import Project, ProjectTemplateInternal
from project_generator.settings import ProjectSettings

from tools.config import Config

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

    def __init__(self, target, inputDir, program_name, toolchain, extra_symbols=[],
                 sources_relative=True, resources=None):
        self.inputDir = inputDir
        self.target = target
        self.program_name = program_name
        self.toolchain = toolchain
        jinja_loader = FileSystemLoader(os.path.dirname(os.path.abspath(__file__)))
        self.jinja_environment = Environment(loader=jinja_loader)
        self.config_macros = self.toolchain.config.get_config_data_macros()
        self.sources_relative = sources_relative
        self.config_header = self.toolchain.MBED_CONFIG_FILE_NAME
        self.resources = resources
        self.symbols = self.toolchain.get_symbols()
        if self.config_macros:
            self.symbols += self.config_macros
        if extra_symbols:
            self.symbols += extra_symbols


    def get_toolchain(self):
        return self.TOOLCHAIN


    @property
    def progen_flags(self):
        print self.toolchain.flags
        if not hasattr(self, "_progen_flag_cache") :
            self._progen_flag_cache = dict([(key + "_flags", value) for key,value in self.toolchain.flags.iteritems()])
            if self.config_header:
                self._progen_flag_cache['c_flags'] += self.toolchain.get_config_option(self.config_header)
                self._progen_flag_cache['cxx_flags'] += self.toolchain.get_config_option(self.config_header)
        return self._progen_flag_cache


    def get_source_paths(self):
        source_keys = ['s_sources', 'c_sources', 'cpp_sources', 'hex_files', 'objects', 'libraries']
        source_files = []
        [source_files.extend(getattr(self.resources,key)) for key in source_keys]
        return list(set([os.path.dirname(src) for src in source_files]))


    def progen_get_project_data(self):
        """ Get ProGen project data  """
        # provide default data, some tools don't require any additional
        # tool specific settings

        def grouped(sources):
            data = sorted(sources, key=os.path.dirname)
            return {k:list(g) for k,g in groupby(data,os.path.dirname)}

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
        project_data['output_dir']['path'] = self.inputDir
        project_data['linker_file'] = self.resources.linker_script
        project_data['macros'] = self.symbols
        project_data['build_dir'] = '.build'
        project_data['template'] = None
        project_data['name'] = self.program_name
        project_data['output_type'] = 'exe'
        project_data['debugger'] = None
        return project_data

    def progen_gen_file(self, tool_name, project_data, progen_build=False):
        """ Generate project using ProGen Project API """
        settings = ProjectSettings()
        s = {"root":[os.path.dirname(os.getcwd())]}
        settings.update(s)
        project = Project(self.program_name, [project_data], settings)
        project.project['export'] = project_data.copy()
        project.generate(tool_name, copied=not self.sources_relative)
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
        template_path = join(Exporter.TEMPLATE_DIR, template_file)
        template = self.jinja_environment.get_template(template_file)
        target_text = template.render(data)

        target_path = join(self.inputDir, target_file)
        logging.debug("Generating: %s" % target_path)
        open(target_path, "w").write(target_text)
        self.generated_files += target_path
