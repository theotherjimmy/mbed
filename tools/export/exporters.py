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

from tools.utils import mkdir
from tools.toolchains import TOOLCHAIN_CLASSES
from tools.targets import TARGET_MAP

from project_generator.generate import Generator
from project_generator.project import Project
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

    def __init__(self, target, inputDir, program_name, extra_symbols=None,
                 sources_relative=True, resources=None):
        self.inputDir = inputDir
        self.target = target
        self.program_name = program_name
        self.toolchain = TOOLCHAIN_CLASSES[self.get_toolchain()](TARGET_MAP[target])
        jinja_loader = FileSystemLoader(os.path.dirname(os.path.abspath(__file__)))
        self.jinja_environment = Environment(loader=jinja_loader)
        self.extra_symbols = extra_symbols
        self.config_macros = []
        self.sources_relative = sources_relative
        self.config_header = None
        self.resources = resources
        self.generated_files = []

    def get_toolchain(self):
        return self.TOOLCHAIN

    @property
    def flags(self):
        return self.toolchain.flags

    @property
    def progen_flags(self):
        if not hasattr(self, "_progen_flag_cache") :
            self._progen_flag_cache = dict([(key + "_flags", value) for key,value in self.flags.iteritems()])
            if self.config_header:
                self._progen_flag_cache['c_flags'] += self.toolchain.get_config_option(self.config_header)
                self._progen_flag_cache['cxx_flags'] += self.toolchain.get_config_option(self.config_header)
        return self._progen_flag_cache

    @staticmethod
    def _get_dir_grouped_files(files):
        """ Get grouped files based on the dirname """
        files_grouped = {}
        for file in files:
            dir_path = os.path.dirname(file)
            if dir_path == '':
                # all files within the current dir go into Source_Files
                dir_path = 'Source_Files'
            if not dir_path in files_grouped.keys():
                files_grouped[dir_path] = []
            files_grouped[dir_path].append(file)
        return files_grouped

    def progen_get_project_data(self):
        """ Get ProGen project data  """
        # provide default data, some tools don't require any additional
        # tool specific settings
        code_files = []
        for r_type in ['c_sources', 'cpp_sources', 's_sources']:
            for file in getattr(self.resources, r_type):
                code_files.append(file)

        sources_files = code_files + self.resources.hex_files + self.resources.objects + \
            self.resources.libraries
        sources_grouped = Exporter._get_dir_grouped_files(sources_files)
        headers_grouped = Exporter._get_dir_grouped_files(self.resources.headers)

        project_data = {
            'common': {
                'sources': sources_grouped,
                'includes': headers_grouped,
                'build_dir':'.build',
                'target': [TARGET_MAP[self.target].progen['target']],
                'macros': self.get_symbols(),
                'export_dir': [self.inputDir],
                'linker_file': [self.resources.linker_script],
            }
        }
        return project_data

    def progen_gen_file(self, tool_name, project_data, progen_build=False):
        """ Generate project using ProGen Project API """
        settings = ProjectSettings()
        s = {"root":[os.path.dirname(os.getcwd())]}
        settings.update(s)
        project = Project(self.program_name, [project_data], settings)
        # TODO: Fix this, the inc_dirs are not valid (our scripts copy files), therefore progen
        # thinks it is not dict but a file, and adds them to workspace.
        project.project['common']['include_paths'] = self.resources.inc_dirs
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

    def get_symbols(self, add_extra_symbols=True):
        """ This function returns symbols which must be exported.
            Please add / overwrite symbols in each exporter separately
        """
        symbols = self.toolchain.get_symbols() + self.config_macros
        # We have extra symbols from e.g. libraries, we want to have them also added to export
        if add_extra_symbols:
            if self.extra_symbols is not None:
                symbols.extend(self.extra_symbols)
        return symbols
