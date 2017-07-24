"""
mbed SDK
Copyright (c) 2011-2016 ARM Limited

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
"""
from os.path import splitext, basename, relpath, join, abspath, dirname,\
    exists
from os import remove
from datetime import datetime
import sys
from subprocess import check_output, CalledProcessError, Popen, PIPE
import shutil
from jinja2.exceptions import TemplateNotFound
from tools.export.exporters import Exporter, filter_supported
from tools.export.cmsis import DeviceCMSIS
from tools.utils import NotSupportedException
from tools.targets import TARGET_MAP


class CMSISPack(Exporter):
    """Generic Makefile template that mimics the behavior of the python build
    system
    """

    DOT_IN_RELATIVE_PATH = False

    MBED_CONFIG_HEADER_SUPPORTED = True

    POST_BINARY_WHITELIST = set([
        "MCU_NRF51Code.binary_hook",
        "TEENSY3_1Code.binary_hook",
        "LPCTargetCode.lpc_patch",
        "LPC4088Code.binary_hook"
    ])

    @staticmethod
    def symbol_parse(sym):
        if "=" in sym:
            name, value = sym.split("=")
            return name, value
        else:
            return sym, None

    def generate(self):
        """Generate the pdsc file

        Note: subclasses should not need to override this method
        """
        if not self.resources.linker_script:
            raise NotSupportedException("No linker script found.")

        self.resources.win_to_unix()

        dev = DeviceCMSIS(self.target)

        symbols = map(self.symbol_parse, self.toolchain.get_symbols())

        ctx = {
            'syms': symbols,
            'linker_script': self.resources.linker_script,
            'include_paths': list(set(join(p, "") for p in self.resources.inc_dirs if p)),
            'object_files': self.resources.objects,
            'c_files' : self.resources.c_sources,
            's_files' : self.resources.s_sources,
            'cpp_files' : self.resources.cpp_sources,
            'headers' : self.resources.headers,
            'hex_files': self.resources.hex_files,
            'libraries': self.resources.libraries,
            'preinclude': self.toolchain.get_config_header(),
            'device_name': dev.dname,
            'dependent_pack': dev.pack_url,
            'target': self.target,
            'toolchain': self.TOOLCHAIN,
            'time': datetime.now().isoformat(),
            'compreq': self.COMPILER_REQUIREMENT,
            'features': {}
        }
        for feature, res in self.resources.features.iteritems():
            ctx['features'][feature] = {
                'object_files': res.objects,
                'include_paths': list(set(join(p, "") for p in res.inc_dirs if p)),
                'hex_files': res.hex_files,
                'object_files': res.objects,
                'c_files' : res.c_sources,
                's_files' : res.s_sources,
                'cpp_files' : res.cpp_sources,
                'headers' : res.headers,
                'libraries': res.libraries,
                'linker_script': res.linker_script
            }

        self.gen_file("cmsis_pack/pdsc.tmpl", ctx, 'MBED.%s_%s.pdsc' % (self.target.upper(), self.TOOLCHAIN.upper()))


class GccArm(CMSISPack):
    """GCC ARM specific makefile target"""
    TARGETS = filter_supported("GCC_ARM", CMSISPack.POST_BINARY_WHITELIST)
    NAME = 'CMSIS-Pack-GCC-ARM'
    TOOLCHAIN = "GCC_ARM"
    COMPILER_REQUIREMENT = "GCC"

class Armc5(CMSISPack):
    """ARM Compiler 5 specific makefile target"""
    TARGETS = filter_supported("ARM", CMSISPack.POST_BINARY_WHITELIST)
    NAME = 'CMSIS-Pack-ARMc5'
    TOOLCHAIN = "ARM"
    COMPILER_REQUIREMENT = "ARMCC"

class IAR(CMSISPack):
    """IAR specific makefile target"""
    TARGETS = filter_supported("IAR", CMSISPack.POST_BINARY_WHITELIST)
    NAME = 'CMSIS-Pack-IAR'
    TOOLCHAIN = "IAR"
    COMPILER_REQUIREMENT = "IAR"
