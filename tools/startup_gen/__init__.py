"""Generate linker files and startup files"""
import os
import jinja2
from copy import copy, deepcopy
from collections import namedtuple
from jinja2 import FileSystemLoader
from jinja2.environment import Environment
from lpc1768_descriptor import SECTION_LIST, RAM_LIST, ROM_LIST, VECTOR_TABLE

IRQ = namedtuple("IRQ", "name desc enabled_on")

CORTEX_M_ALL = ["cortex-m" + num for num in ["0", "3", "4", "7"]]
CORTEX_M_NOT0 = copy(CORTEX_M_ALL)
CORTEX_M_NOT0.remove("cortex-m0")

CORE_IRQ_LIST = [IRQ(*i) for i in [
    ("__stack", "Top of Stack", CORTEX_M_ALL),
    ("Reset", "Reset Handler", CORTEX_M_ALL),
    ("NMI", "NMI Handler", CORTEX_M_ALL),
    ("HardFault", "Hard Fault Handler", CORTEX_M_ALL),
    ("MemManage", "MPU Fault Handler", CORTEX_M_NOT0),
    ("BusFault", "Bus Fault Handler", CORTEX_M_NOT0),
    ("UsageFault", "Usage Fault Handler", CORTEX_M_NOT0),
    (None, "Reserved", CORTEX_M_ALL),
    (None, "Reserved", CORTEX_M_ALL),
    (None, "Reserved", CORTEX_M_ALL),
    (None, "Reserved", CORTEX_M_ALL),
    ("SVC", "SVCall Handler", CORTEX_M_ALL),
    ("DebugMon", "Debug Monitor Handler", CORTEX_M_NOT0),
    (None, "Reserved", CORTEX_M_ALL),
    ("PendSV", "PendSV Handler", CORTEX_M_ALL),
    ("SysTick", "SysTick Handler", CORTEX_M_ALL),
]]

def _jinja_render(template_file, output, context):
    jinja_loader = FileSystemLoader(os.path.dirname(os.path.abspath(__file__)))
    jinja_environment = Environment(loader=jinja_loader)
    template = jinja_environment.get_template(template_file)
    open(output, "w").write(template.render(context))

def irq_section_bytes(irq_list):
    return len(irq_list) * 4

def gen_startup(core_name, extra_irqs):
    irq_list = deepcopy(CORE_IRQ_LIST)
    irq_list.extend(IRQ(**(i + {"enabled_on":core_name})) for i in extra_irqs)
    _jinja_render("startup_c.tmpl", "test_startup.c",
                  {"core_irq_list": irq_list})
    return irq_section_bytes(irq_list)

# Section list
SECTION_LIST_COPY = deepcopy(SECTION_LIST)
SECTION_LIST_COPY.append({
    "name": ".vector_rom",# or isr_vector?
    "start": ROM_LIST[0]["start"],
    "size": vector_table_size,
    })
SECTION_LIST_COPY.append({
    "name": ".vector_ram",
    "start": RAM_LIST[0]["start"],
    "size": vector_table_size,
    })

# Create RAM list - check for regions
RAM_LIST_COPY = deepcopy(RAM_LIST)
ROM_LIST_COPY = deepcopy(ROM_LIST)
region_list = []
region_list.extend(RAM_LIST_COPY)
region_list.extend(ROM_LIST_COPY)
for region in region_list:
    for section in SECTION_LIST_COPY:
        region_start = region["start"]
        region_end = region["start"] + region["size"]
        section_start = section["start"]
        section_end = section["start"] + section["size"]
        if (section_start >= region_end) or (section_end <= region_start):
            # Section comes completely before or after region
            continue

        if section_end >= region_end:
            # Mark that the region should be removed
            region["size"] = None
            break
        else:
            # Move region to come after section
            diff = section_end - region_start
            region["start"] += diff
            region["size"] -= diff

template_vars = {}
template_vars["section_list"] = SECTION_LIST_COPY
template_vars["ram_list"] = RAM_LIST_COPY
template_vars["rom_list"] = ROM_LIST_COPY
template_vars["msp_stack"] = RAM_LIST[0]["start"] + RAM_LIST[0]["size"]
template_vars["irq_list"] = irq_list
template_vars["core_irq_list"] = core_irq_list

output_dir = "output"
if not os.path.exists(output_dir):
    os.mkdir(output_dir)

template_list = [
    ("template.sct", "output.sct"),
    ("template.ld", "output.ld"),
    ("template.icf", "output.icf"),
    ("template_arm_startup.s", "output.s"),
    ("template_gcc_arm_startup.c", "gcc_arm_output.c")
]

env = jinja2.Environment(trim_blocks=True, lstrip_blocks=True)
for template_name, output_name in template_list:
    with open(template_name, "rb") as file_handle:
        data = file_handle.read()
    t = env.from_string(data)
    output = t.render(template_vars)
    with open(os.path.join(output_dir, output_name), "wb") as file_handle:
        file_handle.write(output)

