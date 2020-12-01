import os

REGISTER_NAMES = ("dff", "dffsr", "dffe", "ff")
REGISTER_TYPES = range(0, 4)

PORT_NAME = "port"
PORT_TYPE = 4

REGPORT_TYPES = range(0, 5)

LINEAR_NAMES = ("xor", "xnor")
XOR_TYPE, XNOR_TYPE = 5, 6
LINEAR_TYPES = range(5, 7)

NONLINEAR_NAMES = ("and", "or")
AND_TYPE, OR_TYPE = 7, 8
NONLINEAR_TYPES = range(7, 9)

GATE_NAMES = LINEAR_NAMES + NONLINEAR_NAMES
GATE_TYPES = range(5, 9)

OTHER_NAMES = ("mux", "const", "not")
MUX_TYPE, CONST_TYPE, NOT_TYPE = 9, 10, 11
OTHER_TYPES = range(9, 12)

# define the cell type enum and strings
cell_enum = {}
name_val_pairs = ((REGISTER_NAMES, REGISTER_TYPES),
                  (GATE_NAMES, GATE_TYPES),
                  (OTHER_NAMES, OTHER_TYPES))

for names, vals in name_val_pairs:
    for name, val in zip(names, vals):
        cell_enum[name] = val
cell_enum[PORT_NAME] = PORT_TYPE

inv_cell_enum = {cell_enum[x]: x for x in cell_enum}

LIST_XOR = "shallow"

TRIGGERS = {AND_TYPE: "0", OR_TYPE: "1", XOR_TYPE: None, XNOR_TYPE: None}
BIN_STR = ("0", "1")
UINT_MAX = 0xffffffffff

# define commonly used keys
TRANSIENT = "transient"
STABLE = "stable"

# define label types
LABEL_SHARE = "share"
LABEL_MASK = "mask"
LABEL_OTHER = "unimportant"
LABEL_TYPES = (LABEL_SHARE, LABEL_MASK, LABEL_OTHER)

# directory defines
ALMA_DIR = os.path.dirname(os.path.realpath(__file__))
TEMPLATE_DIR = ALMA_DIR + "/templates/"
TMP_DIR = ALMA_DIR + "/tmp"
ROOT_DIR = "/".join(ALMA_DIR.split("/")[:-1])