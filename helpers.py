import os
import sys
import argparse


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


def bit_to_net(module):
    lookup = {}
    positions = {}
    for netname in module["netnames"]:
        bits = module["netnames"][netname]["bits"]
        for bit, bit_num in zip(bits, range(len(bits))):
            if type(bit) is str: continue
            if bit in lookup.keys():
                private0, private1 = lookup[bit][0] == "_", netname[0] == "_"
                if private0 == private1 and name_cmp(lookup[bit], netname): continue
                if not private0 and private1: continue
            lookup[bit] = netname
            positions[bit] = bit_num
    return lookup, positions


def label_type(label):
    return label.split("_")[0]


def parity(num):
    assert(num >= 0)
    res = 0
    while num != 0:
        res += num & 1
        num >>= 1
    return res % 2

