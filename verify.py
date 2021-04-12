import argparse
import sys
from CircuitGraph import CircuitGraph
import time
import networkx as nx
from SatChecker import SatChecker
from defines import *
from classes import *
import helpers
from VCDStorage import *

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
                        help="Verification order, ie the number of probes (default: %(default)s)")
    parser.add_argument("-m", "--mode", dest="mode",
                        required=False, default=STABLE, choices=[STABLE, TRANSIENT],
                        help="The verification mode (default: %(default)s)")
    parser.add_argument("-t", "--probe-duration", dest="probe_duration",
                        required=False, default=ONCE, choices=[ONCE, ALWAYS],
                        help="Specifies how a probe records values (default: %(default)s)")
    parser.add_argument("-x", "--trace-stable", action="store_true", dest="trace_stable",
                        help="Should trace signals be assumed stable")
    parser.set_defaults(trace_stable=False)
    parser.add_argument("-ml", "--minimize-leaks", action="store_true", dest="minimize_leaks",
                        help="Tells the solver to find the smallest correlating linear combination")
    parser.set_defaults(minimize_leaks=True)
    parser.add_argument("-n", "--num-leaks", dest="num_leaks",
                        required=False, type=int, default=1,
                        help="Number of leakage locations to be reported if the circuit is insecure. (default: %(default)s)")
    parser.add_argument("-r", "--rst-name", dest="rst_name",
                        required=False, default="rst_i",
                        help="Name of the reset signal (default: %(default)s)")
    parser.add_argument("-s", "--rst-cycles", dest="rst_cycles",
                        required=False, default=2, type=helpers.ap_check_positive,
                        help="Number of cycles where reset signal is triggered (default: %(default)s)")
    parser.add_argument("-p", "--rst-phase", dest="rst_phase",
                        required=False, default="1", choices=BIN_STR,
                        help="Phase of the reset signal that triggers the reset (default: %(default)s)")
    parser.add_argument("-d", "--dbg-output-dir", dest="dbg_output_dir_path", 
                        required=False, default=TMP_DIR,
                        help="Directory in which debug traces (dbg-label-trace-?.txt, dbg-circuit-?.dot) are written (default: %(default)s)")

    args, unknown = parser.parse_known_args()
    if args.cycles <= 0:    args.cycles = UINT_MAX
    if args.num_leaks <= 0: args.num_leaks = UINT_MAX
    
    #Unfortunately, ap_check_dir_exists does not work for optional parameters
    helpers.check_dir_exists(args.dbg_output_dir_path) 

    return args


def generate_labeling(label_file_path):
    label_data = ""
    with open(label_file_path, "r") as f:
        label_data = f.read()
    label_data = label_data.strip().split("\n")
    label_dict = {}
    for line in label_data:
        if line[0] == "#": continue
        signal_id, signal_label = line.split(": ")
        signal_id = int(signal_id.split(":")[-1])
        signal_label = signal_label.strip().split()
        label_type = signal_label[0]
        assert(label_type in LABEL_TYPES)
        assert((label_type in (LABEL_MASK, LABEL_RANDOM, LABEL_OTHER) and len(signal_label) == 1) or
               (label_type == LABEL_SHARE and len(signal_label) == 2))
        share_num = int(signal_label[1]) if label_type == LABEL_SHARE else None
        label_dict[signal_id] = Label(signal_id, label_type, share_num)
    return label_dict


def vcd_json_sanity_check(trace, circuit_graph, rst_name):
    assert(rst_name in trace.name_to_id), "Reset signal %s not recognized." % (rst_name)
    for node in circuit_graph.nodes():
        graph_node_name = circuit_graph.nodes()[node]["cell"].name
        assert (("const" in graph_node_name) or (graph_node_name in trace.name_to_id)), "%s not recognized"%(graph_node_name)


def main():
    args = parse_arguments()
    label_dict = generate_labeling(args.label_file_path)
    trace = VCDStorage(args.vcd_file_path)
    checker = SatChecker(label_dict, trace, args)

    status, locations = checker.check()
    leaks = [l[1] for l in locations]
    if status:
        print("The execution is secure")
        sys.exit(SECURE)
    else:
        circuit_graph = nx.read_gpickle(TMP_DIR + "/circuit_graph.gpickle") 
        sys.stdout.write("The execution is not secure, here are some leaks:\n")
        for i in range(len(leaks)):
            gates = leaks[i]
            sys.stdout.write("leak %d: " % i)
            for g in gates:
                cell = circuit_graph.nodes[g.cell_id]["cell"]
                sys.stdout.write("(cycle: %d, cell: %s, id: %d) " % (g.cycle, cell, g.cell_id))
            sys.stdout.write("\n")
        sys.exit(INSECURE)


if __name__ == "__main__":
    main()

