""" The CLI entry point for creating releases.
"""
from __future__ import print_function, absolute_import
from builtins import str

import sys
from os.path import (join, abspath, dirname, exists, basename, normpath,
                     realpath, relpath, basename)
ROOT = abspath(join(dirname(__file__), ".."))
sys.path.insert(0, ROOT)

from shutil import move, rmtree
from argparse import ArgumentParser

from tools.export import EXPORTERS, export_project, get_exporter_toolchain
from tools.export.cmsis import CmsisPack
from tools.targets import TARGET_NAMES
from tools.utils import (argparse_filestring_type
                         , argparse_profile_filestring_type
                         , argparse_many
                         , args_error
                         , argparse_force_lowercase_type
                         , argparse_force_uppercase_type
                         , print_large_string
                         , NotSupportedException)
from tools.notifier.term import TerminalNotifier


def main(args):
    EXPORTERS['cmsis-pack'] = CmsisPack
    targetnames = TARGET_NAMES
    targetnames.sort()
    parser = ArgumentParser()
    parser.add_argument(
        "target",
        metavar="MCU",
        help="generate project for the given MCU ({})".format(
            ', '.join(targetnames)))
    opts = parser.parse_args()
    export_project([ROOT], ".",  opts.target, 'cmsis-pack', name="Mbed OS")
    pass

if __name__ == "__main__":
    main(sys.argv)
