"""
mbed SDK
Copyright (c) 2011-2013 ARM Limited

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
import os, tempfile
from os.path import join, exists, basename
from shutil import copytree, rmtree, copy
import yaml

from tools.utils import mkdir
from tools.export import uvision4, uvision5, codered, gccarm, ds5_5, iar, emblocks, coide, kds, zip, simplicityv3, atmelstudio, sw4stm32, e2studio
from tools.export.exporters import OldLibrariesException, FailedBuildException
from tools.targets import TARGET_NAMES, EXPORT_MAP, TARGET_MAP

from project_generator_definitions.definitions import ProGenDef

EXPORTERS = {
    'uvision': uvision4.Uvision4,
    'uvision4': uvision4.Uvision4,
    'uvision5': uvision5.Uvision5,
    'lpcxpresso': codered.CodeRed,
    'gcc_arm': gccarm.GccArm,
    'ds5_5': ds5_5.DS5_5,
    'iar': iar.IAREmbeddedWorkbench,
    'emblocks' : emblocks.IntermediateFile,
    'coide' : coide.CoIDE,
    'kds' : kds.KDS,
    'simplicityv3' : simplicityv3.SimplicityV3,
    'atmelstudio' : atmelstudio.AtmelStudio,
    'sw4stm32'    : sw4stm32.Sw4STM32,
    'e2studio' : e2studio.E2Studio,
}

ERROR_MESSAGE_UNSUPPORTED_TOOLCHAIN = """
Sorry, the target %s is not currently supported on the %s toolchain.
Please refer to <a href='/handbook/Exporting-to-offline-toolchains' target='_blank'>Exporting to offline toolchains</a> for more information.
"""

ERROR_MESSAGE_NOT_EXPORT_LIBS = """
To export this project please <a href='http://mbed.org/compiler/?import=http://mbed.org/users/mbed_official/code/mbed-export/k&mode=lib' target='_blank'>import the export version of the mbed library</a>.
"""

def mcu_ide_matrix(verbose_html=False, platform_filter=None):
    """  Shows target map using prettytable """
    supported_ides = []
    for key in EXPORTERS.iterkeys():
        supported_ides.append(key)
    supported_ides.sort()
    from prettytable import PrettyTable, ALL # Only use it in this function so building works without extra modules

    # All tests status table print
    columns = ["Platform"] + supported_ides
    pt = PrettyTable(columns)
    # Align table
    for col in columns:
        pt.align[col] = "c"
    pt.align["Platform"] = "l"

    perm_counter = 0
    target_counter = 0
    for target in sorted(TARGET_NAMES):
        target_counter += 1

        row = [target]  # First column is platform name
        for ide in supported_ides:
            text = "-"
            if target in EXPORTERS[ide].TARGETS:
                if verbose_html:
                    text = "&#10003;"
                else:
                    text = "x"
                perm_counter += 1
            row.append(text)
        pt.add_row(row)

    pt.border = True
    pt.vrules = ALL
    pt.hrules = ALL
    # creates a html page suitable for a browser
    # result = pt.get_html_string(format=True) if verbose_html else pt.get_string()
    # creates a html page in a shorter format suitable for readme.md
    result = pt.get_html_string() if verbose_html else pt.get_string()
    result += "\n"
    result += "Total IDEs: %d\n"% (len(supported_ides))
    if verbose_html: result += "<br>"
    result += "Total platforms: %d\n"% (target_counter)
    if verbose_html: result += "<br>"
    result += "Total permutations: %d"% (perm_counter)
    if verbose_html: result = result.replace("&amp;", "&")
    return result
