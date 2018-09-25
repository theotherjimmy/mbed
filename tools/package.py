from __future__ import print_function

import argparse
import socket
import sys
from datetime import datetime
from os.path import join, abspath, dirname, exists
from jinja2 import FileSystemLoader, StrictUndefined
from jinja2.environment import Environment
from zipfile import ZipFile
from xml.etree import ElementTree
try:
    from http.server import HTTPServer, SimpleHTTPRequestHandler
except ImportError:
    from BaseHTTPServer import HTTPServer
    from SimpleHTTPServer import SimpleHTTPRequestHandler



ROOT = abspath(join(dirname(__file__), ".."))
sys.path.insert(0, ROOT)

from tools.resources import Resources, FileType, FileRef
from tools.notifier.term import TerminalNotifier
from tools.notifier.mock import MockNotifier
from tools.targets import TARGETS
from tools.build_api import prepare_toolchain


def get_ip():
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        s.connect(('10.255.255.255', 1))
        return s.getsockname()[0]
    except:
        return '127.0.0.1'
    finally:
        s.close()


def gen_file(template_file, target_file, version, data):
    """Create a PDSC file"""
    data["version"] = version
    old_releases = ""
    if exists(target_file):
        tree = ElementTree.parse(target_file).getroot()
        old_release_strings = []
        for release in tree.find("releases") or []:
            if release.attrib["version"] != version:
                old_release_strings.append(ElementTree.tostring(release).strip())
        old_releases = "\n".join(old_release_strings)
    data["old_releases"] = old_releases
    data["now"] = str(datetime.utcnow())
    data["now_str"] = "generated by `mbed package cmsis {}` at {}".format(version, datetime.utcnow())

    jinja_loader = FileSystemLoader(dirname(abspath(__file__)))
    jinja_environment = Environment(loader=jinja_loader, undefined=StrictUndefined)
    template = jinja_environment.get_template(template_file)
    target_text = template.render(data)
    with open(target_file, "w") as outfd:
        outfd.write(target_text)


TYPE_MAPPING = {
    FileType.C_SRC: "sourceC",
    FileType.CPP_SRC: "sourceC++",
    FileType.ASM_SRC: "sourceAsm",
    FileType.INC_DIR: "include",
    FileType.OBJECT: "object",
    FileType.LD_SCRIPT: "linkerScript",
    FileType.HEADER: "header",
}


def get_files(res, target=None):
    all_file_refs = []
    all_file_names = []
    for typ, typ_name in TYPE_MAPPING.items():
        for name in res.get_file_names(typ):
            if name.startswith("./"):
                name = name[2:]
            all_file_names.append((typ_name, name, None))
        all_file_refs.extend(res.get_file_refs(typ))
    if target != None:
        for tc in ["GCC_ARM", "ARM", "IAR"]:
            if tc not in target.supported_toolchains:
                continue
            cur_tc = prepare_toolchain([""], "BUILD", target, tc)
            tc_res = res.new_res_from_toolchain(cur_tc)
            for typ, typ_name in TYPE_MAPPING.items():
                for name in tc_res.get_file_names(typ):
                    if name.startswith("./"):
                        name = name[2:]
                    all_file_names.append((typ_name, name, tc))
                all_file_refs.extend(tc_res.get_file_refs(typ))
    return sorted(all_file_refs), sorted(all_file_names)


def zip_files(zip_file, refs_list):
    with ZipFile(zip_file, mode="w") as zipfd:
        for name, path in refs_list:
            zipfd.write(path, arcname=name)


def progress(notifier, action, filename, num_completed, total_things):
    percent = 100. * float(num_completed) / float(total_things)
    notifier.progress(action, filename, percent)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("standard")
    parser.add_argument("version")
    parser.add_argument("--server", action="store_true")
    options = parser.parse_args()

    core_res = Resources(MockNotifier())
    core_res.add_directory(".")
    core_refs, core_files = get_files(core_res)
    vendor = "Mbed_OS"
    core_group = "Core"

    notifier = TerminalNotifier(color=True)
    total_things = (len(list(
        tgt for tgt in TARGETS if
        ("5" in getattr(tgt, "release_versions", []) and
         getattr(tgt, "device_name", None)))) + 1)
    done_things = 0
    ip = get_ip()
    cur_url = "http://{}:8000/".format(ip)

    core_pdsc_name = "{}.{}.pdsc".format(vendor, core_group)
    gen_file(
        "cmsis-pdsc.tmpl", 
        core_pdsc_name,
        options.version,
        {
            "cur_url": cur_url,
            "vendor": vendor,
            "files": core_files,
            "CGroup": core_group,
            "pack_name": core_group,
            "device_name": False,
        },
    )
    core_refs.append(FileRef(core_pdsc_name, core_pdsc_name))
    pack_file_name = "{}.{}.{}.pack".format(vendor, core_group, options.version)
    zip_files(pack_file_name, core_refs)
    done_things += 1
    progress(notifier, "packaging", pack_file_name, done_things, total_things)
    
    for tgt in TARGETS:
        if ("5" in getattr(tgt, "release_versions", []) and
            getattr(tgt, "device_name", None)):
            tgt_res = core_res.new_res_from_target(tgt)
            tgt_refs, files = get_files(tgt_res, tgt)

            tgt_pdsc_name = "{}.{}.pdsc".format(vendor, tgt.name)
            gen_file(
                "cmsis-pdsc.tmpl",
                tgt_pdsc_name,
                options.version,
                {
                    "cur_url": cur_url,
                    "vendor": vendor,
                    "files": files,
                    "CGroup": "HAL",
                    "pack_name": tgt.name,
                    "device_name": tgt.device_name,
                },
            )
            tgt_refs.append(FileRef(tgt_pdsc_name, tgt_pdsc_name))
            pack_file_name = "{}.{}.{}.pack".format(vendor, tgt.name, options.version)
            zip_files(pack_file_name, tgt_refs)
            done_things += 1
            progress(notifier, "packaging", pack_file_name, done_things, total_things)
    gen_file(
        "cmsis-pidx.tmpl",
        "{}.pidx".format(vendor),
        options.version,
        {
            "vendor": vendor,
            "curdir": cur_url,
            "packs": [tgt.name for tgt in TARGETS],
        }
    )

    if options.server:
        print("CMSIS Pack testing server running at {}:8000".format(ip))
        print("Use the index at http://{}:8000/{}.pidx".format(ip, vendor))
        server_address = ('', 8000)
        httpd = HTTPServer(server_address, SimpleHTTPRequestHandler)
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            pass


if __name__ == "__main__":
    main()
