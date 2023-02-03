#!/usr/bin/env python3

from CircuitGraph import CircuitGraph
from classes import *
from defines import *
from SafeGraph import SafeGraph
from SatChecker import SatChecker
from VCDStorage import *
import argparse
import helpers
import json
import networkx as nx
import sys
import time

SECURE = 0
INSECURE = -1


def parse_arguments():
    parser = argparse.ArgumentParser(description="Verify", fromfile_prefix_chars="@")
    parser.add_argument("-j", "--json", dest="json_file_path",
                        required=True, type=helpers.ap_check_file_exists,
                        help="Path of circuit in JSON format")
    parser.add_argument("-l", "--label", dest="label_file_path",
                        required=True, type=helpers.ap_check_file_exists,
                        help="Path of label file")
    parser.add_argument("-v", "--vcd", dest="vcd_file_path",
                        required=True, type=helpers.ap_check_file_exists,
                        help="Path of VCD file")
    parser.add_argument("-c", "--cycles", dest="cycles",
                        required=False, type=int, default=-1,
                        help="Number of cycles to verify (default: %(default)s; stop when VCD file ends)")
    parser.add_argument("-f", "--from-cycle", dest="from_cycle",
                        required=False, type=int, default=0,
                        help="Start of verification after reset (default: %(default)s)")
    parser.add_argument("-q", "--order", dest="order",
                        required=False, type=helpers.ap_check_positive, default=1,
                        help="Verification order, i.e. the number of probes (default: %(default)s)")
    parser.add_argument("-m", "--mode", dest="mode",
                        required=False, default=STABLE, choices=[STABLE, TRANSIENT],
                        help="The verification mode (default: %(default)s)")
    parser.add_argument("-g", "--glitch-behavior", dest="glitch_behavior",
                        required=False, default=STRICT, choices=[STRICT, LOOSE],
                        help="Determines behavior of glitches. The 'strict' mode is the worst case, 'loose' is more"
                             "realistic (default: %(default)s)")
    parser.add_argument("-t", "--probing-model", dest="probing_model",
                        required=False, default=TIME_CONSTRAINED, choices=[CLASSIC, TIME_CONSTRAINED],
                        help="Specifies probing model in which checks are performed (default: %(default)s)")
    parser.add_argument("-x", "--trace-stable", action="store_true", dest="trace_stable",
                        help="Should trace signals be assumed stable")
    parser.set_defaults(trace_stable=False)
    parser.add_argument("-ml", "--minimize-leaks", action="store_true", dest="minimize_leaks",
                        help="Tells the solver to find the smallest correlating linear combination")
    parser.set_defaults(minimize_leaks=True)
    parser.add_argument("--checking-mode", dest="checking_mode",
                        required=False, default=PER_SECRET, choices=[PER_SECRET, PER_LOCATION],
                        help="Specifies checking mode. 'per-secret' means one formula is built per secret and the"
                        "solver identifies leaking probing locations. 'per-location' means one formula is built per" 
                        "potentially leaking probing locations and the solver identifies combinations of secrets" 
                        "causing leaks (default: %(default)s).")
    parser.add_argument("-n", "--num-leaks", dest="num_leaks",
                        required=False, type=int, default=1,
                        help="Number of leakage locations to be reported if the circuit is insecure." 
                             " (default: %(default)s)")
    parser.add_argument("-r", "--rst-name", dest="rst_name",
                        required=False, default="rst_i",
                        help="Name of the reset signal (default: %(default)s)")
    parser.add_argument("-s", "--rst-cycles", dest="rst_cycles",
                        required=False, default=2, type=helpers.ap_check_positive,
                        help="Number of cycles where reset signal is triggered (default: %(default)s)")
    parser.add_argument("-p", "--rst-phase", dest="rst_phase",
                        required=False, default="1", choices=BIN_STR,
                        help="Phase of the reset signal that triggers the reset (default: %(default)s)")
    parser.add_argument("-i", "--init-delay", dest="init_delay",
                        required=False, default=0, type=helpers.ap_check_positive,
                        help="Number of initial delay cycles after reset (default: %(default)s)")
    parser.add_argument("-d", "--dbg-output-dir", dest="dbg_output_dir_path",
                        required=False, default=TMP_DIR,
                        help="Directory in which debug traces (dbg-label-trace-?.txt, dbg-circuit-?.dot) "
                             "are written (default: %(default)s)")
    parser.add_argument("-ds", "--dbg-signals", dest="debugs",
                        required=False, default=[], nargs="+", type=str,
                        help="List of debug signals whose values should be printed")
    parser.add_argument("-is", "--ignored-signals", dest="ignored",
                        required=False, default=[], nargs="+", type=str,
                        help="Cells whose names contain these strings (and their logic cone) to be "
                             "are forced to be stable and then ignored during checks")
    parser.add_argument("-hd", "--include-hamming", action="store_true", dest="hamming",
                        help="Include transition leakage in stable mode")
    parser.set_defaults(hamming=False)
    parser.add_argument("--dbg-exact-formula", action="store_true", dest="dbg_exact_formula",
                        help="For each node, print exact formula.")
    parser.set_defaults(dbg_exact_formula=False)
    parser.add_argument("--export-cnf", dest="export_cnf", action="store_true", help="Export CNF which needs to be" 
                        "solved for each secret to dbg_output_dir. This allows to use other solvers than" 
                        "CaDiCaL, e.g. Kissat." )
    parser.set_defaults(export_cnf=False)
    parser.add_argument("--kissat", dest="kissat_bin_path",
                        required=False, type=helpers.ap_check_file_exists,
                        help="Path to a the Kissat binary file. Note that for enabling solving with Kissat," 
                             "you need to set the --export-cnf option.")
    parser.add_argument("--top-module", dest="top_module",
                        required=True, type=str,
                        help="Name of the top module")

    args = parser.parse_args()
    # args, unknown = parser.parse_known_args()
    if args.cycles <= 0:    args.cycles = UINT_MAX
    if args.num_leaks <= 0: args.num_leaks = UINT_MAX
    
    # Unfortunately, ap_check_dir_exists does not work for optional parameters
    helpers.check_dir_exists(args.dbg_output_dir_path) 

    if args.export_cnf == True and args.probing_model == TIME_CONSTRAINED:
        raise argparse.ArgumentTypeError("Cannot export CNF formulas for time-constrained probing model. " 
                                         "Please use the --probing-model classic option.")
    if args.kissat_bin_path != None and args.export_cnf == False:
        raise argparse.ArgumentTypeError("Cannot use Kissat without exporting CNF formulas. "
                                         "Please use the --export-cnf option.")

    return args


def generate_labeling(label_file_path, json_module):
    net_bits, _, _ = helpers.bit_to_net(json_module)
    label_data = ""
    with open(label_file_path, "r") as f:
        label_data = f.read()
    label_data = label_data.strip().split("\n")
    label_dict = {}
    for li_, line in enumerate(label_data):
        li = li_ + 1
        line = line.strip()
        if line[0] == "#": continue
        info = line.split(" = ")
        assert(len(info) == 2), "label format error in line %d" % li
        signal_str, signal_label = info
        signal_name, signal_top, signal_bot = None, 0, 0
        signal_str = signal_str.strip()
        if signal_str in net_bits:
            assert(len(net_bits[signal_str]) == 1), "label size error in line %d" % li
            signal_name = signal_str
        else:
            info = signal_str.split()
            assert(len(info) == 2), "label format error in line %d" % li
            signal_name, rest = info
            assert(signal_name in net_bits), "label error in line %d: %s does not exist" % (li, signal_name)
            assert(rest[0] == "[" and rest[-1] == "]"), "label range %s invalid in line %d" % (rest, li)
            rest = rest[1:-1]
            signal_top, signal_bot = helpers.get_slice(rest, li, len(net_bits[signal_name]))

        signal_label = signal_label.strip().split()
        label_type = signal_label[0]
        assert(label_type in LABEL_TYPES)
        assert((label_type in (LABEL_STATIC_RANDOM, LABEL_VOLATILE_RANDOM, LABEL_OTHER) and len(signal_label) == 1) or
               (label_type == LABEL_SHARE and len(signal_label) == 2))
        if label_type == LABEL_OTHER: continue

        share_top, share_bot = None, None
        if label_type == LABEL_SHARE:
            share_top, share_bot = helpers.get_slice(signal_label[1], li, None)
            assert(share_top - share_bot == signal_top - signal_bot), "label length mismatch in line %d" % li
        for pos in range(signal_top + 1 - signal_bot):
            signal_pos = signal_bot + pos
            share_pos = None if (share_bot is None) else (share_bot + pos)
            bit = net_bits[signal_name][signal_pos]
            assert(bit not in label_dict), "label re-declaration for %s[%d] in line %d" % (signal_name, signal_pos, li)
            label_dict[bit] = Label(bit, label_type, share_pos)
    return label_dict


def generate_ignored(circuit, json_module, ignored_strings):
    net_bits, _, _ = helpers.bit_to_net(json_module)
    ignored = set()
    for fs in ignored_strings:
        for name in net_bits:
            if fs not in name: continue
            for b in net_bits[name]:
                if type(b) is int:
                    ignored.add(b)

    for node_id in circuit.nodes:
        preds = tuple(circuit.predecessors(node_id))
        if len(preds) == 0: continue
        if all(map(lambda p: p in ignored, preds)):
            ignored.add(node_id)

    for node_id in ignored:
        print(circuit.cells[node_id])

    return ignored


def vcd_json_sanity_check(trace, circuit_graph, rst_name):
    assert(rst_name in trace.name_to_id), "Reset signal %s not recognized." % (rst_name)
    for node in circuit_graph.nodes():
        graph_node_name = circuit_graph.nodes()[node]["cell"].name
        assert (("const" in graph_node_name) or (graph_node_name in trace.name_to_id)), "%s not recognized"%(graph_node_name)


def pretty_error(checker, cycle, cell):
    # from SatChecker.py: SatChecker.__dbg_write_label_trace
    cells = [checker.circuit.cells[x] for x in checker.variables]
    initial = ["%s:%s" % (c.name, c.pos) for c in cells]
    # mapping = dict(zip(checker.pretty_names, initial))

    stable = checker.formula.node_vars_stable[cycle]
    trans = checker.formula.node_vars_trans[cycle] if checker.mode == TRANSIENT else None
    hamming = checker.formula.node_vars_diff[cycle] if checker.hamming else None
    model = set(checker.formula.solver.get_model())

    for node_id in checker.circuit.nodes:
        node_cell = checker.circuit.cells[node_id]
        if node_cell != cell: continue

        for mode, mstr in zip((stable, hamming, trans), ("stable ", "hamming", "trans  ")):
            if mode is None or node_id not in mode: continue
            res = checker.formula.model_for_vars(model, mode[node_id])
            # generate mappings for signals that contribute to the leak
            pretty_comp = [n for n, v in zip(checker.pretty_names, res) if v == 1]
            initial_comp = [n for n, v in zip(initial, res) if v == 1]

            print("{} {} {} vars   : {}".format(cycle, mstr, cell, pretty_comp))
            initial_comp = ["{0}[{1}]".format(*x.split(':')) for x in initial_comp]
            print("{} {} {} signals: {}".format(cycle, mstr, cell, " ^ ".join(initial_comp)))


def main():
    args = parse_arguments()

    with open(args.json_file_path, "r") as f:
        circuit_json = json.load(f)
    circuit_graph = CircuitGraph(circuit_json, args.top_module)
    safe_graph = SafeGraph(circuit_graph.graph)

    module = circuit_json["modules"][args.top_module]
    label_dict = generate_labeling(args.label_file_path, module)
    ignored_set = generate_ignored(safe_graph, module, args.ignored)
    trace = VCDStorage(args.vcd_file_path)
    checker = SatChecker(label_dict, ignored_set, trace, safe_graph, args)

    status, locations = checker.check()
    leaks = [l[1] for l in locations]
    if status and not(args.kissat_bin_path):
        print("The execution is secure")
        sys.exit(SECURE)
    elif args.kissat_bin_path:
        print("Skipped checking with CaDiCal, using Kissat instead.")
        result = checker.checkKissat()
        sys.exit(result)
    else:
        sys.stdout.write("The execution is not secure, here are some leaks:\n")
        for i in range(len(leaks)):
            gates = leaks[i]
            sys.stdout.write("leak %d: " % i)
            for g in gates:
                cell = circuit_graph.graph.nodes[g.cell_id]["cell"]
                sys.stdout.write("(cycle: %d, cell: %s, id: %d) " % (g.cycle, cell, g.cell_id))
            sys.stdout.write("\n")
            for g in gates:
                cell = circuit_graph.graph.nodes[g.cell_id]["cell"]
                pretty_error(checker, g.cycle, cell)
        sys.exit(INSECURE)


if __name__ == "__main__":
    main()

