from bisect import bisect_left
import os

import helpers
from CircuitGraph import CONST_TO_BIT
CONST_NAMES = {("const_%s" % x) for x in CONST_TO_BIT.keys()}
DUMMIES = ["$scope", "$upscope", "$enddefinitions", "$date", "$version"]


def peek_line(f):
    pos = f.tell()
    line = f.readline()
    f.seek(pos)
    return line


class VCDStorage:
    def __init__(self, vcd_file_path):
        self.name_to_id = {}       # key: name, value: vcd_id
        self.id_to_width = {}      # key: vcd_id, value: int width
        self.current_values = {}   # key: vcd_id, value: current value
        self.previous_values = {}  # key: vcd_id, value: previous value
        self.vcd_file = open(vcd_file_path, "r")
        self.cycle = 0
        self.timestamps = []
        self.vcd_file_path = vcd_file_path
        self.line_nr = 0  # last read line number
        self.parse_header()

    def __del__(self):
        self.vcd_file.close()

    def _readline(self):
        self.line_nr = self.line_nr + 1
        return self.vcd_file.readline()

    def parse_signal(self, vcd_line):
        signal_value, signal_id = None, None
        if vcd_line[0] == "b":
            assert(" " in vcd_line[1:])
            signal_value, signal_id = vcd_line[1:].split(" ")#
        elif vcd_line[0] == "r":
            print("{}:{}: [WARNING] ignoring unsupported assignment of type real".format(self.vcd_file_path, self.line_nr))
            signal_id = vcd_line[1:].split(" ")[1]
            signal_value = 64*'0'
        else:
            assert (" " not in vcd_line)
            signal_value, signal_id = vcd_line[0], vcd_line[1:]
        fill = "x" if "x" in signal_value else "0"
        signal_value = signal_value.rjust(self.id_to_width[signal_id], fill)
        # print("{}:{}: {} = {}".format(self.vcd_file_path, self.line_nr, signal_id, signal_value))
        return signal_value, signal_id

    def parse_header(self):
        while True:
            line = self._readline()
            if line == "": return
            if line == "\n": continue
            if line.strip() == "#0" or line.strip() == "$dumpvars": break

            line = line.strip().split()
            while line[-1] != "$end":
                line += self._readline().strip().split()

            if line[0] in DUMMIES:
                continue
            elif line[0] == "$var":
                signal_width_str, signal_id = line[2:4]
                signal_width = int(signal_width_str)
                if line[5][0] == "[" and line[5][-1] == "]" and ':' in line[5]:
                    # Assumes that signals with a range span the full width and store only the name.
                    # e.g., ['$var', 'reg', '64', ')', 'pt1', '[63:0]', '$end'] -> 'pt1'
                    signal_name = line[4]
                    up, down = helpers.get_slice(line[5][1:-1], self.line_nr, None, True)
                    top, bot = max(up, down), min(up, down)
                    assert (top - bot + 1 == signal_width), "%s:%d Signal width does not match" % (self.vcd_file_path, self.line_nr)
                else:
                    # Use the remaining components for the signal name.
                    # e.g., ['$var', 'wire', '1', '7', '_00004_', '$end']        -> '_00004_'
                    # e.g., ['$var', 'wire', '1', '8', '_00005_', '[1]', '$end'] -> '_00005_ [1]'
                    signal_name = " ".join(line[4:-1])

                signal_name = signal_name.replace("\\", "")  # to ensure CircuitGraph compatibility
                if signal_name in self.name_to_id:
                    print("%s:%d: [WARNING] Entry for name %s already exists in namemap (%s -> %s)" % (
                        self.vcd_file_path, self.line_nr, signal_name, signal_name, self.name_to_id[signal_name]))
                self.name_to_id[signal_name] = signal_id
                # print("{}:{}: {} = {}".format(self.vcd_file_path, self.line_nr, signal_name, signal_id))
                self.id_to_width[signal_id] = signal_width
            elif line[0] == "$timescale":
                if line[1].isnumeric() and len(line) == 4:
                    # support cadence: ['$timescale', '1', 'ps', '$end']
                    num, unit = int(line[1]), line[2]
                else:
                    # support verilator: ['$timescale', '1ps', '$end']
                    num, unit = int(line[1][:-2]), line[1][-2:]
                assert num == 1, "%s:%d: Unsupported timescale factor" % (self.vcd_file_path, self.line_nr)
                assert unit in ["ps", "ns"], "%s:%d: Unsupported timescale unit" % (self.vcd_file_path, self.line_nr)
            else:
                print("%s:%d: [WARNING] Ignoring unknown VCD header line: %s." % (
                    self.vcd_file_path, self.line_nr, line[0]))
                break
        while True:
            line = peek_line(self.vcd_file)
            if line[0] == "#": break
            line = self._readline()
            if line.strip() in ("$dumpvars", "$end"): continue
            if line == "": return
            signal_value, signal_id = self.parse_signal(line.strip())
            self.current_values[signal_id] = str(signal_value)

    def parse_next_cycle(self):
        self.cycle += 1
        self.previous_values = self.current_values.copy()
        while True:
            line = self._readline()
            if not line:
                break
            line = line.strip()
            if line.startswith("#"):
                ts = int(line.replace("#", ""))
                assert(len(self.timestamps) < 2 or
                       self.timestamps[-1] - self.timestamps[-2] == ts - self.timestamps[-1])
                self.timestamps.append(ts)
                if len(self.timestamps) % 2 == 1:
                    return True
            else:
                signal_value, signal_id = self.parse_signal(line)
                assert(signal_id in self.current_values), "%s:%d: Signal with id %s not found." % (
                    self.vcd_file_path, self.line_nr, signal_id)
                self.current_values[signal_id] = str(signal_value)
        return False

    # @profile
    def get_signal_value(self, signal_name, bit_num, prev=False):
        if signal_name in CONST_NAMES: return signal_name[-1]
        values = self.previous_values if prev else self.current_values
        # Verilator < 4.106 truncates long signal names
        # assert (signal_name in self.name_to_id), "Signal not found in VCD file"

        if bit_num is not None:
            # Look for the signal incl. the specific index.
            id_name = self.name_to_id.get("{} [{}]".format(signal_name, bit_num), None)
            if id_name:
                return values[id_name]

            # Look for the signal in full width and extract the specific bit.
            id_name = self.name_to_id.get(signal_name, None)
            assert id_name, "Signal %s not found in VCD file" % signal_name

            full_val = values[id_name]
            assert (bit_num >= 0 and bit_num < len(full_val)), "Invalid bit index %d for %s" % (bit_num, signal_name)
            return full_val[-1 - bit_num]

        # All bits of a signal are requested, look for the signal.
        id_name = self.name_to_id.get(signal_name, None)
        if id_name:
            return values[id_name]

        # Reconstruct the full width signal by concatenating individual bits.
        idx = 0
        full_val = ""
        while True:
            id_name = self.name_to_id.get("{} [{}]".format(signal_name, idx), None)
            if id_name is None: break
            full_val = values[id_name] + full_val
            idx = idx + 1

        assert len(full_val), "Signal %s not found in VCD file" % signal_name
        return full_val
