from bisect import bisect_left
import os

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
        self.parse_header()

    def __del__(self):
        self.vcd_file.close()

    def parse_signal(self, vcd_line):
        signal_value, signal_id = None, None
        if vcd_line[0] == "b":
            assert(" " in vcd_line[1:])
            signal_value, signal_id = vcd_line[1:].split(" ")
        else:
            assert (" " not in vcd_line)
            signal_value, signal_id = vcd_line[0], vcd_line[1:]
        fill = "x" if "x" in signal_value else "0"
        signal_value = signal_value.rjust(self.id_to_width[signal_id], fill)
        return signal_value, signal_id

    def parse_header(self):
        while True:
            line = self.vcd_file.readline()
            if line == "": return
            if line == "\n": continue
            if line.strip() == "#0": break

            line = line.strip().split()
            while line[-1] != "$end":
                line += self.vcd_file.readline().strip().split()

            if line[0] in DUMMIES:
                continue
            elif line[0] == "$var":
                signal_width, signal_id, signal_name = line[2:5]
                signal_name = signal_name.replace("\\", "")  # to ensure CircuitGraph compatibility
                if signal_name in self.name_to_id:
                    print("WARNING Entry for name %s already exists in namemap (%s -> %s)" % (
                    signal_name, signal_name, self.name_to_id[signal_name]))
                self.name_to_id[signal_name] = signal_id
                self.id_to_width[signal_id] = int(signal_width)
            elif line[0] == "$timescale":
                num, unit = int(line[1][:-2]), line[1][-2:]
                assert (num == 1), "Unsupported timescale factor"
                assert (unit in ["ps", "ns"]), "Unsupported timescale unit"
            else:
                raise Exception("Detected unknown VCD header line: %s." % (line[0]))
        while peek_line(self.vcd_file)[0] != "#":
            line = self.vcd_file.readline()
            if line.strip() in ("$dumpvars", "$end"): continue
            if line == "": return
            signal_value, signal_id = self.parse_signal(line.strip())
            self.current_values[signal_id] = str(signal_value)

    def parse_next_cycle(self):
        self.cycle += 1
        self.previous_values = self.current_values.copy()
        for line in self.vcd_file:
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
                assert(signal_id in self.current_values), "Signal with id %s not found." % signal_id
                self.current_values[signal_id] = str(signal_value)
        return False

    # @profile
    def get_signal_value(self, signal_name, bit_num, prev=False):
        if signal_name in CONST_NAMES: return signal_name[-1]
        values = self.previous_values if prev else self.current_values
        # signal_value = values[self.name_to_id[signal_name]]
        # assert(bit_num >= 0 and bit_num < len(signal_value)), (signal_name, bit_num)
        # return signal_value[-1 - bit_num]
        # Verilator < 4.106 truncates long signal names
        assert (signal_name in self.name_to_id), "Signal not found in VCD file"
        full_val = values[self.name_to_id[signal_name]]
        if bit_num is None: return full_val
        return full_val[-1 - bit_num]
