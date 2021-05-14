import argparse
import subprocess
import os
import sys
import shutil
import json
import defines
import helpers
import time
import re
from CircuitGraph import CircuitGraph
from SafeGraph import SafeGraph


LABEL_FILE_PATH = defines.TMP_DIR + "/labels.txt"
JSON_FILE_PATH = defines.TMP_DIR + "/circuit.json"
NETLIST_FILE_PATH = defines.TMP_DIR + "/circuit.v"
SYNTH_FILE_PATH = defines.TMP_DIR + "/yosys_synth.ys"
TEMPLATE_FILE_PATH = defines.TEMPLATE_DIR + "/yosys_synth_template.txt"
DEFAULT_FILE_PATHS = (LABEL_FILE_PATH, JSON_FILE_PATH, NETLIST_FILE_PATH)


def parse_arguments():
    parser = argparse.ArgumentParser(description="Parse", fromfile_prefix_chars="@")
    
    group = parser.add_mutually_exclusive_group(required=True)

    # Either specify source
    group.add_argument("-s", "--source", dest="verilog_file_paths",
                       nargs="+", type=helpers.ap_check_file_exists,
                       help="File path(s) to the (Verilog or System Verilo) source file(s)", metavar="VERILOG_FILES")
    
    # OR synthesis script, but not both
    group.add_argument("-sy", "--synthesis-file", dest="synthesis_file_path",
                       type=helpers.ap_check_file_exists,
                       help="Path of the yosys synthesis script")

    # Required: top module
    parser.add_argument("-t", "--top-module", dest="top_module",
                        required=True, type=str,
                        help="Name of the top module")

    # Optional arguments
    parser.add_argument("-l", "--label", dest="label_file_path",
                        required=False, default=LABEL_FILE_PATH, type=helpers.ap_check_dir_exists,
                        help="Path of output label file (default: %(default)s)")
    parser.add_argument("-j", "--json", dest="json_file_path",
                        required=False, default=JSON_FILE_PATH, type=helpers.ap_check_dir_exists,
                        help="Path of output JSON file (default: %(default)s)")
    parser.add_argument("-n", "--netlist", dest="netlist_file_path",
                        required=False, default=NETLIST_FILE_PATH, type=helpers.ap_check_dir_exists,
                        help="Path of output verilog netlist file (default: %(default)s)")
    parser.add_argument("-y", "--yosys", dest="yosys_bin_path",
                        required=False, type=helpers.ap_check_file_exists,
                        help="Path to a custom yosys binary file (default: %(default)s)")
    parser.add_argument("--log-yosys", dest="log_yosys", action="store_true", default=False, required=False,
                        help="Print output of Yosys synthesis process to logfile (default: %(default)s)")

    args, _ = parser.parse_known_args()

    arg_paths = (args.label_file_path, args.json_file_path, args.netlist_file_path)

    for file_path, default_file_path in zip(arg_paths, DEFAULT_FILE_PATHS):
        if file_path != default_file_path and os.path.isfile(file_path):
            res = input("File %s already exists, do you want to overwrite it? (y/n)  " % file_path)
            while res.lower() not in ("y", "n"):
                res = input("Please answer with 'y' or 'n':  ")
            if res.lower() == "n":
                sys.exit(0)

    return args


def create_yosys_script(args):
    yosys_script = ""
    with open(TEMPLATE_FILE_PATH) as template_file:
        yosys_script += template_file.read()
    assert("{READ_FILES}" in yosys_script)
    read_verilog_commands = "\n".join(["read_verilog %s;" % os.path.abspath(f) for f in args.verilog_file_paths]) + "\n"
    yosys_script = yosys_script.replace("{READ_FILES}", read_verilog_commands)
    assert("{TOP_MODULE}" in yosys_script)
    yosys_script = yosys_script.replace("{TOP_MODULE}", args.top_module)
    assert("{JSON_FILE_PATH}" in yosys_script)
    yosys_script = yosys_script.replace("{JSON_FILE_PATH}", args.json_file_path)
    assert("{NETLIST_FILE_PATH}" in yosys_script)
    yosys_script = yosys_script.replace("{NETLIST_FILE_PATH}", args.netlist_file_path)
    with open(SYNTH_FILE_PATH, "w") as f:
        f.write(yosys_script)


def yosys_synth(args):
    try:
        if args.yosys_bin_path:
            print("Using custom yosys: %s" % args.yosys_bin_path)
            yosys_bin_path = args.yosys_bin_path
        else:
            yosys_bin_path = get_yosys_bin_path()

        if args.synthesis_file_path:
            print("Using custom yosys synthesis script: %s" % args.synthesis_file_path)
            yosys_synth_file_path = args.synthesis_file_path
        else:
            yosys_synth_file_path = SYNTH_FILE_PATH

        # Provide additional opt arguments for Yosys >= 0.9+3470
        version = subprocess.check_output([yosys_bin_path, "-V"], stderr=subprocess.PIPE)
        version = re.search(r"Yosys ([0-9]\.[0-9])\+([0-9]+)", str(version))
        major = version.group(1)
        minor = version.group(2)
        if major > "0.9" or (major == "0.9" and minor >= "3470"):
            yosys_script_patched = ""
            with open(yosys_synth_file_path) as yosys_script:
                yosys_script_patched += yosys_script.read()
            yosys_script_patched = yosys_script_patched.replace("opt", "opt -nodffe -nosdff")
            with open(yosys_synth_file_path, "w") as yosys_script:
                yosys_script.write(yosys_script_patched)

        print("Starting yosys synthesis...")
        if args.log_yosys:
            subprocess.check_output([yosys_bin_path, "-l", defines.TMP_DIR + "/yosys_synth_log.txt", yosys_synth_file_path], stderr=subprocess.PIPE)
        else:
            subprocess.check_output([yosys_bin_path, yosys_synth_file_path], stderr=subprocess.PIPE)

    except subprocess.CalledProcessError as p:
        print(p.stderr.decode())
        print("Yosys synthesis failed.")
        sys.exit(1)
    circuit_json_file = open(args.json_file_path, "r")
    circuit_json = json.load(circuit_json_file)
    circuit_json_file.close()

    circuit_json['top_module'] = args.top_module
    circuit_json_file = open(args.json_file_path, "w")
    circuit_json_file.write(json.dumps(circuit_json, indent=True))
    circuit_json_file.close()
    return circuit_json


def create_label_template(circuit_json, label_file_path):
    label_file = open(label_file_path, "w")
    top_module = circuit_json["top_module"]
    module = circuit_json["modules"][top_module]
    name_lookup, pos_lookup = helpers.bit_to_net(module)
    
    label_file.write("# inputs:\n")
    for port in module['ports']:
        if module["ports"][port]["direction"] == "output": continue
        port_bits = module["ports"][port]["bits"]
        for bit, bit_pos in zip(port_bits, range(len(port_bits))):
            label_file.write('%s:%d:%d: %s\n' % (port, bit_pos, bit, defines.LABEL_OTHER))
    
    label_file.write("# registers:\n")
    for cell in module["cells"]:
        cell_data = module["cells"][cell]
        cell_type_str = cell_data["type"].split("_")[1].lower()
        cell_type = defines.cell_enum[cell_type_str]
        if cell_type not in defines.REGISTER_TYPES: continue
        cell_directions = cell_data["port_directions"]
        cell_outputs = [x for x in cell_directions if cell_directions[x] == "output"]
        assert(len(cell_outputs) == 1)
        bits = cell_data["connections"][cell_outputs[0]]
        assert(len(bits) == 1)
        name, pos, bit = name_lookup[bits[0]], pos_lookup[bits[0]], bits[0]
        label_file.write("%s:%d:%d: %s\n" % (name, pos, bit, defines.LABEL_OTHER))

    label_file.close()


def get_yosys_bin_path():
    bin = shutil.which("yosys")
    if bin is not None: 
        return bin
    bin = defines.ROOT_DIR + "/yosys/yosys"
    if os.path.isfile(bin): 
        return bin
    print("ERROR yosys executable not found")
    sys.exit(2)


def main():
    tstp_begin = time.time()
    args = parse_arguments()
    if not os.path.exists(defines.TMP_DIR):
        os.makedirs(defines.TMP_DIR)
    else:
        shutil.rmtree(defines.TMP_DIR)
        os.makedirs(defines.TMP_DIR)

    if not(args.synthesis_file_path):
        create_yosys_script(args)
    circuit_json = yosys_synth(args)

    create_label_template(circuit_json, args.label_file_path)

    circuit_graph = CircuitGraph(circuit_json)
    circuit_graph.write_pickle()
    safe_graph = SafeGraph(circuit_graph.graph)
    safe_graph.write_pickle()
    
    tstp_end = time.time()
    print("parse.py successful (%.2fs)"%(tstp_end-tstp_begin))


if __name__ == "__main__": 
    main()
