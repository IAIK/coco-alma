
STABLE = "stable"
TRANSIENT = "transient"

TIME_CONSTRAINED = "time-constrained"
CLASSIC = "classic"


PER_SECRET = "per-secret"
PER_LOCATION = "per-location"


class VerificationContext:
    def __init__(self, top_module: str, cycles: int, mode: str, probing_model: str, checking_mode: str, order: int = 0):
        self.top_module = top_module
        self.cycles = cycles
        self.mode = mode
        self.probing_model = probing_model
        self.checking_mode = checking_mode
        self.order = order
        self.runtime = 0
        
    def toCmdArgs(self):
        args = ["python3", "verify.py", \
            "--json","tmp/circuit.json", \
            "--label", "tmp/labels.txt", \
            "--vcd", "tmp/tmp.vcd",  \
            "--rst-name", "rst_i", \
            "--glitch-behavior", "strict", \
            "--cycles", str(self.cycles), \
            "--mode", self.mode, \
            "--probing-model", self.probing_model, \
            "--top-module", self.top_module, \
            "--checking-mode", self.checking_mode]
        if self.order != 0:
            args.append("--order")
            args.append(str(self.order))
        return args
    
    def shortStr(self):
        return "%s: [%d cycles, %s, %s, %s]"%(self.top_module, self.cycles, self.mode, self.probing_model, self.checking_mode)
