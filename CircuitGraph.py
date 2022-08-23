import json
import networkx as nx
import helpers
from defines import *
from classes import Cell
import time

CONST_TO_BIT = {"0": 0, "1": 1, "x": -1, "z": -2}


class CircuitGraph:
    def __init__(self, circuit_json, top_module):
        self.graph = nx.DiGraph()
        self.circuit_json = circuit_json
        self.top_module = top_module
        self.parse_json()
        #self.write_graph()
        self.print_graph_info()

    def parse_json(self):
        wires = {}  # output -> inputs
        module = self.circuit_json["modules"][self.top_module]
        net_bits, bit_info, _ = helpers.bit_to_net(module)
        self.graph.clear()

        # Add constants, to make things easier
        for name in CONST_TO_BIT.keys():
            self.add_cell(CONST_TO_BIT[name], Cell("const_%s" % name, CONST_TYPE, 0))
            wires[CONST_TO_BIT[name]] = []

        # Add port cells
        for port in module["ports"]:
            if module["ports"][port]["direction"] == "output": continue
            for bit in module["ports"][port]["bits"]:
                assert(type(bit) == int), "Port bits must be integers"
                info = bit_info[bit]
                self.add_cell(bit, Cell(info[0], PORT_TYPE, info[1]))
                wires[bit] = []

        # Gather all cells
        for cell in module["cells"]:
            cell_json = module["cells"][cell]
            cell_type_str = cell_json["type"].split("_")[1].lower()
            cell_type = cell_enum[cell_type_str]

            directions = cell_json["port_directions"]
            connections = cell_json["connections"]
            # Special treatment for mux and dff
            select = None
            mux_ins = None
            if cell_type == MUX_TYPE:
                select = connections.pop("S")[0]
                if type(select) == str:
                    select = CONST_TO_BIT[select]
            clock = None
            if cell_type in REGISTER_TYPES:
                clock = connections.pop("C")[0]
            reset = None
            if cell_type in REGISTER_TYPES and "R" in connections.keys():
                reset = connections.pop("R")[0]
            preset = None
            if cell_type in REGISTER_TYPES and "S" in connections.keys():
                preset = connections.pop("S")[0]

            # Get the cell ports
            keys = sorted(list(connections.keys()))
            in_ports = [x for x in keys if directions[x] == "input"]
            assert((cell_type not in REGISTER_TYPES) or len(in_ports) == 1)
            out_ports = [x for x in keys if directions[x] == "output"]
            assert(len(out_ports) == 1)

            # Get the cell wires
            in_wires = [connections[x][0] for x in in_ports]
            in_wires = [(CONST_TO_BIT[x] if type(x) == str else x) for x in in_wires]
            out_wires = connections[out_ports[0]]
            assert(len(out_wires) == 1)
            out_wire = out_wires[0]
            assert(type(out_wire) == int)

            # Add the cell and its wires
            wires[out_wire] = in_wires
            if cell_type == MUX_TYPE: mux_ins = in_wires
            info = bit_info[out_wire]
            cell = Cell(info[0], cell_type, info[1], select, mux_ins, clock, reset, preset)
            self.add_cell(out_wire, cell)

        # Create all connections
        for cell_bit in self.graph.nodes():
            for n in wires[cell_bit]:
                assert(n in self.graph.nodes), "input wire %d of %s is not defined" % (n, self.graph.nodes[cell_bit])
            for in_wire in wires[cell_bit]:
                self.graph.add_edge(in_wire, cell_bit)

    def add_cell(self, bit, cell):
        assert(bit not in self.graph.nodes())
        self.graph.add_node(bit, **{"cell": cell})

    def print_graph_info(self):
        num_regs = len([n for n in self.graph.nodes() if self.graph.nodes[n]["cell"].type in REGISTER_TYPES])
        num_lin = len([n for n in self.graph.nodes() if self.graph.nodes[n]["cell"].type in LINEAR_TYPES])
        num_nonlin = len([n for n in self.graph.nodes() if self.graph.nodes[n]["cell"].type in NONLINEAR_TYPES])
        num_muxs = len([n for n in self.graph.nodes() if self.graph.nodes[n]["cell"].type == MUX_TYPE])
        total = len(list(self.graph.nodes()))
        print("| CircuitGraph | Total: %4d | Linear: %4d | Non-linear: %4d | Registers: %4d | Mux: %4d | " %
              (total, num_lin, num_nonlin, num_regs, num_muxs))

    def write_graph(self):
        dot = "strict digraph  {\n"
        for e in self.graph.edges():
            src_cell = self.graph.nodes[e[0]]["cell"]
            dst_cell = self.graph.nodes[e[1]]["cell"]
            #dot += "\"%s\" -> \"%s\";\n" % (src_cell.name, dst_cell.name)
            types = [inv_cell_enum[x.type] for x in (src_cell, dst_cell)]
            src_str = "\"%s_%s_%d\"" % (types[0], src_cell.name, e[0])
            dst_str = "\"%s_%s_%d\"" % (types[1], dst_cell.name, e[1])
            dot += "%s -> %s;\n" % (src_str, dst_str)
        dot += "}\n"
        with open(TMP_DIR + "/circuit.dot", "w") as f:
            f.write(dot)

    def write_pickle(self):
        t1 = time.time()
        nx.write_gpickle(self.graph, TMP_DIR + "/circuit_graph.gpickle") 
        t2 = time.time()
        print("Writing CircuitGraph: %.2f" % (t2-t1))
