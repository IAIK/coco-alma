import os
import sys
import argparse
import defines
import functools

def ap_check_file_exists(file_path):
    if not os.path.isfile(file_path):
        raise argparse.ArgumentTypeError("File '%s' does not exist" % file_path)
    return file_path


def ap_check_dir_exists(file_path):
    dir_path = os.path.dirname(os.path.abspath(file_path))
    if not os.path.isdir(dir_path):
        raise argparse.ArgumentTypeError("Directory '%s' does not exist" % dir_path)
    return file_path


def check_dir_exists(dir_path):
    if not os.path.exists(dir_path):
        raise argparse.ArgumentTypeError("Directory '%s' does not exist" % dir_path)


def ap_check_positive(num_str):
    try:
        val = int(num_str)
        if val <= 0: raise ValueError
    except ValueError:
        raise argparse.ArgumentTypeError("'%s' is not a positive integer value" % num_str)
    return val


def name_cmp(a, b):
    na = len(a.split(".")) - 1
    nb = len(b.split(".")) - 1
    return na < nb or na == nb and a < b


def get_slice(expr, li, max_):
    num_top, num_bot = None, None
    if ":" in expr:
        signal_top, num_bot = [int(x) for x in expr.split(":")]
        assert (signal_top >= num_bot), "label range inverted in line %d" % li
        assert (num_bot >= 0), "label range negative in line %d" % li
        assert (type(max_) is not int or signal_top < max_), "label range longer than signal in line %d" % li
    else:
        num_top, num_bot = int(expr), int(expr)
    return signal_top, num_bot


def bit_to_net(module):
    net_bits = {}         # name -> [bits...]
    bit_pos_in_net = {}   # bit -> map [name -> pos]
    bit_info = {}         # bit -> (name, pos)

    for netname in module["netnames"]:
        bits = module["netnames"][netname]["bits"]
        net_bits[netname] = bits
        for pos, bit in enumerate(bits):
            if type(bit) is not int: continue
            if bit not in bit_pos_in_net:
                bit_pos_in_net[bit] = {}
            bit_pos_in_net[bit][netname] = pos

    reg_bits = set()
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
        reg_bits.add(bits[0])

    reg_nets = set()
    for netname in net_bits:
        if all(map(lambda x: x in reg_bits, net_bits[netname])):
            reg_nets.add(netname)

    def cmp_names(a, b):
        na = len(a.split(".")) - 1
        nb = len(b.split(".")) - 1
        if na != nb:
            return -1 if na < nb else 1
        sa = len(net_bits[a])
        sb = len(net_bits[b])
        if sa != sb:
            return -1 if sa > sb else 1
        return 0

    order = sorted(reg_nets, key=functools.cmp_to_key(cmp_names))
    used_regs = []
    for rname in order:
        bits = net_bits[rname]
        if not all(map(lambda x: x not in bit_info, bits)):
            continue
        used_regs.append(rname)
        for bit_pos, bit in enumerate(bits):
            bit_info[bit] = (rname, bit_pos)

    for bit in bit_pos_in_net:
        if bit in bit_info: continue
        cand = bit_pos_in_net[bit]
        order = sorted(cand.keys(), key=functools.cmp_to_key(cmp_names))
        bit_info[bit] = (order[0], cand[order[0]])

    return net_bits, bit_info, used_regs

def label_type(label):
    return label.split("_")[0]


def parity(num):
    assert(num >= 0)
    res = 0
    while num != 0:
        res += num & 1
        num >>= 1
    return res % 2

