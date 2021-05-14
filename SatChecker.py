from defines import *
from helpers import label_type, parity
import time
import sys
import pickle
import networkx as nx
from classes import ActiveInfo, VariableInfo, PropVarSet
from Solver import *

class Formula:
    def __init__(self, num_vars):
        self.num_vars = num_vars     # int
        self.node_vars_stable = []   # cycle -> node -> PropVarSet id
        self.node_vars_trans = []    # cycle -> node -> PropVarSet id
        self.prop_var_sets = {}      # id -> PropVarSet
        self.linear_gate_set = {}    # vars -> (vars...)
        self.linear_set_cache = {}   # (vars...) -> vars
        self.nonlin_gate_set = {}    # vars -> (vars...)
        self.nonlin_set_cache = {}   # (vars...) -> vars
        self.biased_cache = set()    # {vars...}
        self.biased_vars = set()     # {vars...}
        self.active = {}             # vars -> (cycle, node, prop)
        self.check_vars = {}         # labeled node -> prop
        self.assume_act = {}         # labeled node -> prop
        self.covering_vars = {}      # vars -> {vars...}
        self.covered_vars = set()    # {vars...}
        self.vars_to_info = {}       # vars -> (cycle, node)
        self.solver = Solver(store_clauses=False, store_comments=True)

    def assure_biased(self, vars_id):
        if vars_id in self.biased_cache:
            self.biased_cache.remove(vars_id)
            return vars_id
        biased_vars = PropVarSet(biased=self.prop_var_sets[vars_id], solver=self.solver)
        self.solver.add_comment("defined %d as biased %d (%s)" %
                                (biased_vars.id, vars_id, biased_vars))
        self.prop_var_sets[biased_vars.id] = biased_vars
        return biased_vars.id

    def add_cover(self, top, bot):
        if top not in self.covering_vars:
            self.covering_vars[top] = set()
        self.covering_vars[top].add(bot)
        self.covered_vars.add(bot)

    def union_gate_set(self, out, param1, param2, union=None):
        assert(out not in self.nonlin_gate_set)
        if union is None:
            p1 = self.nonlin_gate_set[param1]
            p2 = self.nonlin_gate_set[param2]
            union = tuple(sorted(set(p1).union(p2)))
        self.nonlin_gate_set[out] = union
        self.nonlin_set_cache[union] = out

    def symdiff_gate_set(self, out, param1, param2, symdiff=None):
        assert(out not in self.linear_gate_set)
        if symdiff is None:
            p1 = self.linear_gate_set[param1]
            p2 = self.linear_gate_set[param2]
            symdiff = tuple(sorted(set(p1).symmetric_difference(p2)))
        self.linear_gate_set[out] = symdiff
        self.linear_set_cache[symdiff] = out

    # @profile
    def make_simple(self, gate_type, vars_id1, vars_id2):
        assert(gate_type in GATE_TYPES)
        # simple application of (a ^ a) == 0 and (a & a) == a
        if vars_id1 == vars_id2: return None if (gate_type in LINEAR_TYPES) else vars_id1

        l1 = self.linear_gate_set[vars_id1]
        l2 = self.linear_gate_set[vars_id2]
        n1 = self.nonlin_gate_set[vars_id1]
        n2 = self.nonlin_gate_set[vars_id2]

        if gate_type in LINEAR_TYPES:
            sd = tuple(sorted(set(l1).symmetric_difference(l2)))
            assert(len(sd) != 0)
            cached = self.linear_set_cache.get(sd)
            if cached is not None:
                self.solver.add_comment("found duplicate xor %s for %s" % (sd, cached))
                return cached

            xor_args = tuple(self.prop_var_sets[x] for x in (vars_id1, vars_id2))
            gate_vars = PropVarSet(xor=xor_args, solver=self.solver)
            self.solver.add_comment("defined %d == %d xor %d" % (gate_vars.id, vars_id1, vars_id2))
            if len(gate_vars.ones) == 0 and len(gate_vars.vars) == 0: return None
            self.prop_var_sets[gate_vars.id] = gate_vars
            self.symdiff_gate_set(gate_vars.id, vars_id1, vars_id2, symdiff=sd)
            # self.nonlin_gate_set[gate_vars.id] = (gate_vars.id,)
            # self.nonlin_set_cache[(gate_vars.id,)] = gate_vars.id

            # if both inputs are biased, this is also a non-linear result
            if vars_id1 in self.biased_vars and vars_id2 in self.biased_vars:
                self.biased_vars.add(gate_vars.id)
                self.biased_cache.add(gate_vars.id)
                self.biased_cache.discard(vars_id1)
                self.biased_cache.discard(vars_id2)
                self.union_gate_set(gate_vars.id, vars_id1, vars_id2)
            else:
                self.nonlin_gate_set[gate_vars.id] = (gate_vars.id,)
                self.nonlin_set_cache[(gate_vars.id,)] = gate_vars.id
            self.biased_cache.discard(vars_id1)
            self.biased_cache.discard(vars_id2)
        else:  # gate_type in NONLINEAR_TYPES
            un = tuple(sorted(set(n1).union(n2)))
            if n1 == un:
                self.add_cover(vars_id1, vars_id2)
                self.solver.add_comment("%s is super of %s" % (vars_id1, vars_id2))
                return vars_id1
            if n2 == un:
                self.add_cover(vars_id2, vars_id1)
                self.solver.add_comment("%s is super of %s" % (vars_id2, vars_id1))
                return vars_id2
            cached = self.nonlin_set_cache.get(un)
            if cached is not None:
                self.add_cover(cached, vars_id1)
                self.add_cover(cached, vars_id2)
                self.solver.add_comment("found duplicate and %s for %s" % (un, cached))
                return cached

            biased_id1 = self.assure_biased(vars_id1)
            biased_id2 = self.assure_biased(vars_id2)

            xor_args = tuple(self.prop_var_sets[x] for x in (biased_id1, biased_id2))
            gate_vars = PropVarSet(xor=xor_args, solver=self.solver)
            self.solver.add_comment("defined %d == %d and %d (%d xor %d)"
                                    % (gate_vars.id, vars_id1, vars_id2, biased_id1, biased_id2))
            if len(gate_vars.ones) == 0 and len(gate_vars.vars) == 0: return None
            self.prop_var_sets[gate_vars.id] = gate_vars
            self.union_gate_set(gate_vars.id, vars_id1, vars_id2, union=un)
            self.add_cover(gate_vars.id, vars_id1)
            self.add_cover(gate_vars.id, vars_id2)

            # self.linear_gate_set[gate_vars.id] = (gate_vars.id,)
            # self.linear_set_cache[(gate_vars.id,)] = gate_vars.id

            self.biased_cache.add(gate_vars.id)
            self.biased_vars.add(gate_vars.id)

            if vars_id1 == biased_id1 and vars_id2 == biased_id2:
                self.symdiff_gate_set(gate_vars.id, vars_id1, vars_id2)
            else:
                self.linear_gate_set[gate_vars.id] = (gate_vars.id,)
                self.linear_set_cache[(gate_vars.id,)] = gate_vars.id
        return gate_vars.id

    def collect_active(self, mode):
        node_vars = [self.node_vars_stable, self.node_vars_trans][(mode == TRANSIENT) & 1]
        for cycle, vars in enumerate(node_vars):
            for node in vars.keys():
                self.vars_to_info[vars[node]] = VariableInfo(cycle, node)
                if vars[node] in self.active.keys(): continue
                if vars[node] in self.covered_vars: continue
                self.active[vars[node]] = ActiveInfo(cycle, node, self.solver.get_var())

    def make_assumes(self, shares):
        assert(len(self.assume_act) == 0)
        for ss in sorted(list(shares.keys())):
            # if either all or none are active, there is a leak
            tmp_pos, tmp_neg, act = [self.solver.get_var() for _ in range(3)]
            self.solver.add_clauses(make_and_bool([self.check_vars[share] for share in shares[ss]], tmp_pos))
            self.solver.add_clauses(make_and_bool([-self.check_vars[share] for share in shares[ss]], tmp_neg))
            self.solver.add_clause([-act, tmp_pos, tmp_neg])
            self.assume_act[ss] = act

    def model_for_vars(self, model, vars_id):
        props = self.prop_var_sets[vars_id]
        l = [((x in model) & 1) if type(x) == int else int(x) for x in props.tuple()]
        return tuple(l)

    def __backtrack_fault(self, model, location, mode):
        node_vars = [self.node_vars_stable, self.node_vars_trans][(mode == TRANSIENT) & 1]
        result = []
        for act in location:
            cycle = act.cycle
            cell_id = act.cell_id
            found = True
            while found:
                vars_id = node_vars[cycle][cell_id]
                covered = self.covering_vars.get(vars_id)
                found = False
                if covered is not None:
                    vals = self.model_for_vars(model, vars_id)
                    found = False
                    for vs in covered:
                        info = self.vars_to_info.get(vs)
                        if info is None or info.cycle > cycle: continue
                        other = self.model_for_vars(model, vs)
                        if vals == other:
                            cycle = info.cycle
                            cell_id = info.cell_id
                            found = True
                            # print("reduction to (%d %d)" % (cycle, cell_id))
                            break
            result.append(VariableInfo(cycle, cell_id))
        return result

    def analyse(self, assumes, num_leaks, mode):
        fault = []
        res = True
        cache = set()
        enable = self.solver.get_var()
        last = -1
        while res:
            fault.clear()
            model = set(self.solver.get_model())
            for act in self.active.values():
                if act.prop_var in model:
                    fault.append(act)
            last = max(map(lambda x: x.cycle, fault))
            # forbid all cycles >= last
            for act in self.active.values():
                if act.prop_var in cache or act.cycle < last: continue
                self.solver.add_clause([-enable, -act.prop_var])
                cache.add(act.prop_var)
            res = self.solver.solve(assumes + [enable])
        assert(last >= 0)
        # add a new enable for > last cycle
        restrict = self.solver.get_var()
        for act in self.active.values():
            if act.cycle <= last: continue
            self.solver.add_clause([-restrict, -act.prop_var])
        result = []  # list of (model, list act)
        seen = []
        for i in range(num_leaks):
            res = self.solver.solve(assumes + [-enable, restrict] + seen)
            if not res: break
            model = set(self.solver.get_model())
            fault = list(filter(lambda x: x.prop_var in model, self.active.values()))
            fault_min = self.__backtrack_fault(model, fault, mode)
            result.append((model, fault_min))
            seen += [-act.prop_var for act in fault]
        return result


class SatChecker(object):
    def __init__(self, labels, trace, args):
        assert(args.mode in (TRANSIENT, STABLE))

        with open(TMP_DIR + "/safe_graph.pickle",'rb') as f:
            self.circuit = pickle.load(f)
    
        self.labels = labels
        self.trace = trace
        self.order = args.order
        self.cycles = args.cycles
        self.mode = args.mode
        self.rst_name = args.rst_name
        self.rst_cycles = args.rst_cycles
        self.rst_phase = args.rst_phase
        self.num_leaks = args.num_leaks
        self.dbg_output_dir_path = args.dbg_output_dir_path
        self.masks = []
        self.shares = {}
        self.variables = []
        self.pretty_names = []
        self.__extract_label_info(labels)
        self.num_vars = len(self.variables)

        self.formula = Formula(self.num_vars)
        self.__build_formula()

    def __extract_label_info(self, labels):
        for label_id in labels.keys():
            label = labels[label_id]
            if label.type == LABEL_OTHER: continue
            if label.type == LABEL_MASK:
                self.pretty_names.append("m%d" % len(self.masks))
                self.masks.append(label.bit)
            if label.type == LABEL_SHARE:
                if label.num not in self.shares.keys():
                    self.shares[label.num] = []
                self.pretty_names.append("s%d:%d" % (label.num, len(self.shares[label.num])))
                self.shares[label.num].append(label.bit)
            self.variables.append(label.bit)

    # @profile
    def __simple_inherit(self, type_, preds, curr_vars, info):
        nvars = None
        for p0, p1 in zip(preds, reversed(preds)):
            if p0 not in curr_vars.keys(): continue
            p1c = self.circuit.cells[p1]
            value = self.trace.get_signal_value(p1c.name, p1c.pos)
            stable = (info is None) or info[p1]
            if value == TRIGGERS[type_] and stable: continue
            nvars = curr_vars[p0]
        return nvars

    # @profile
    def __proc_simple(self, gate, type_, curr_vars, info=None):
        assert(type_ in GATE_TYPES)
        preds = tuple(self.circuit.predecessors(gate))
        if all(map(lambda p: p in curr_vars.keys(), preds)):
            if curr_vars[preds[0]] == curr_vars[preds[1]]:
                return None if (type_ in LINEAR_TYPES) else curr_vars[preds[0]]
            self.formula.solver.add_comment("Definition for %d %s:" % (gate, self.circuit.cells[gate]))
            return self.formula.make_simple(type_, curr_vars[preds[0]], curr_vars[preds[1]])
        # take the actual type here
        type_ = self.circuit.cells[gate].type
        return self.__simple_inherit(type_, preds, curr_vars, info)

    # @profile
    def __proc_stable_mux(self, gate, select, mux_ins, curr_vars, sel_stable=True):
        assert(select is not None)
        assert(len(mux_ins) == 2)
        # Add needed gate variables into mult
        mult = []
        others = mux_ins
        if select in curr_vars.keys():
            mult.append(curr_vars[select])
        else:  # select not in curr_vars.keys():
            sel_cell = self.circuit.cells[select]
            value = self.trace.get_signal_value(sel_cell.name, sel_cell.pos)
            if sel_stable and value in BIN_STR:
                others = (mux_ins[int(value)],)
        for x in others: mult.append(curr_vars.get(x))
        if len(mult) == 0: return None
        nvars = mult[0]
        for m in mult[1:]:
            if nvars is None:
                nvars = m
            elif m is not None:
                nvars = self.formula.make_simple(AND_TYPE, nvars, m)
        return nvars

    def __init_labeled_vars(self):
        assert(len(self.formula.node_vars_stable) == 0)
        assert(len(self.formula.node_vars_trans) == 0)
        self.formula.node_vars_stable.append({})
        self.formula.node_vars_trans.append({})
        for var, var_idx in zip(self.variables, range(self.num_vars)):
            gate_vars = PropVarSet(num=self.num_vars)
            gate_vars.ones.add(var_idx)
            self.formula.prop_var_sets[gate_vars.id] = gate_vars
            self.formula.nonlin_gate_set[gate_vars.id] = (gate_vars.id,)
            self.formula.linear_gate_set[gate_vars.id] = (gate_vars.id,)
            self.formula.nonlin_set_cache[(gate_vars.id,)] = gate_vars.id
            self.formula.linear_set_cache[(gate_vars.id,)] = gate_vars.id
            cell = self.circuit.cells[var]
            assert (cell.type in REGPORT_TYPES)
            dst = var if cell.type == PORT_TYPE else self.circuit.predecessors(var).__next__()
            self.formula.node_vars_stable[0][dst] = gate_vars.id
            self.formula.node_vars_trans[0][dst] = gate_vars.id

    # @profile
    def __build_stable(self, reset):
        prev_vars = self.formula.node_vars_stable[-1]
        if reset: self.formula.node_vars_stable = []

        curr_vars = {}
        for node_id in self.circuit.nodes:
            cell = self.circuit.cells[node_id]
            nvars = None
            if cell.type in GATE_TYPES:
                nvars = self.__proc_simple(node_id, cell.type, curr_vars)
            elif cell.type == NOT_TYPE or cell.type in REGISTER_TYPES:
                target_vars = curr_vars if cell.type == NOT_TYPE else prev_vars
                pred0 = self.circuit.predecessors(node_id).__next__()
                nvars = target_vars.get(pred0)
            elif cell.type == PORT_TYPE:
                nvars = prev_vars.get(node_id)
            elif cell.type == MUX_TYPE:
                nvars = self.__proc_stable_mux(node_id, cell.select, cell.mux_ins, curr_vars)

            if nvars is not None:
                curr_vars[node_id] = nvars
        self.formula.node_vars_stable.append(curr_vars)

    def __proc_trans_reg(self, reg, prev, curr):
        valid = [(reg in x) for x in (prev.keys(), curr.keys())]
        if all(valid):
            if prev[reg] == curr[reg]: return prev[reg]
            self.formula.solver.add_comment("Definition for trans %d %s" % (reg, self.circuit.cells[reg]))
            return self.formula.make_simple(AND_TYPE, prev[reg], curr[reg])
        elif valid[0]: return prev[reg]
        elif valid[1]: return curr[reg]
        else: return None

    @staticmethod
    def get_blocking(nodes, stability, stable_nodes):
        return {p: (p not in stable_nodes and stability[p])
                for p in nodes}

    # @profile
    def __make_stability_info(self):
        stability = {}  # node -> bool
        for node_id in self.circuit.nodes:
            cell = self.circuit.cells[node_id]
            prev_val = self.trace.get_signal_value(cell.name, cell.pos, True)
            curr_val = self.trace.get_signal_value(cell.name, cell.pos, False)
            stability[node_id] = prev_val in BIN_STR and (prev_val == curr_val)
            if not stability[node_id]: continue
            if cell.type in REGPORT_TYPES: continue
            preds = self.circuit.predecessors(node_id)
            # for linear gates, all inputs must be stable
            if cell.type in LINEAR_TYPES or cell.type == NOT_TYPE:
                stability[node_id] &= all([stability[p] for p in preds])
            # for non-linear gates, if the value stays constant
            # if AND is 1 or OR is 0, then it is already stable because the inputs cant have changed
            # if AND is 0: it is stable if any input that is 0 is stable
            # if  OR is 1: it is stable if any input that is 1 is stable
            # then its stable if any input is stable
            elif cell.type in NONLINEAR_TYPES and curr_val == TRIGGERS[cell.type]:
                stability[node_id] = False
                for p in preds:
                    c = self.circuit.cells[p]
                    if self.trace.get_signal_value(c.name, c.pos) == TRIGGERS[cell.type]:
                        stability[node_id] |= stability[p]
            elif cell.type == MUX_TYPE:
                if stability[cell.select]:
                    # if SELECT is stable, inherit the selected inputs stability
                    sel_cell = self.circuit.cells[cell.select]
                    sel_val = int(self.trace.get_signal_value(sel_cell.name, sel_cell.pos))
                    stability[node_id] &= stability[cell.mux_ins[sel_val]]
                else:
                    # if SELECT is not stable, then both inputs must be stable and equal
                    stability[node_id] &= all([stability[p] for p in preds])
                    if stability[node_id]:
                        in_cells = [self.circuit.cells[x] for x in cell.mux_ins]
                        vals = [self.trace.get_signal_value(c.name, c.pos) for c in in_cells]
                        stability[node_id] &= (vals[0] == vals[1])
        return stability

    # @profile
    def __build_trans(self, reset):
        prev_stable = {}
        if len(self.formula.node_vars_stable) > 1:
            prev_stable = self.formula.node_vars_stable[-2]
        curr_stable = self.formula.node_vars_stable[-1]
        if reset: self.formula.node_vars_trans = []
        all_stable_nodes = set(curr_stable.keys()).union(prev_stable.keys())
        stability = self.__make_stability_info()
        curr_vars = {}
        for node_id in self.circuit.nodes:
            cell = self.circuit.cells[node_id]
            nvars = None
            preds = self.circuit.predecessors(node_id)
            if cell.type in GATE_TYPES:
                info = {p: (p not in all_stable_nodes and stability[p]) for p in preds}
                nvars = self.__proc_simple(node_id, AND_TYPE, curr_vars, info)
            elif cell.type == NOT_TYPE:
                pred0 = preds.__next__()
                nvars = curr_vars.get(pred0)
            elif cell.type in REGISTER_TYPES:
                nvars = self.__proc_trans_reg(node_id, prev_stable, curr_stable)
            elif cell.type == PORT_TYPE:
                nvars = curr_stable.get(node_id)
            elif cell.type == MUX_TYPE:
                sel_stable = (cell.select not in all_stable_nodes) and stability[cell.select]
                nvars = self.__proc_stable_mux(node_id, cell.select, cell.mux_ins, curr_vars, sel_stable)

            if nvars is not None:
                curr_vars[node_id] = nvars
        self.formula.node_vars_trans.append(curr_vars)

    def __make_checks(self):
        assert(len(self.formula.check_vars) == 0)
        # keys is just used to keep the order consistent
        keys = list(self.formula.active.keys())
        act_vars = [self.formula.active[key].prop_var for key in keys]
        self.formula.solver.at_most_k_of_n(self.order, act_vars)
        for var, var_idx in zip(self.variables, range(self.num_vars)):
            ands = []
            for kid, key in enumerate(keys):
                prp = self.formula.prop_var_sets[key][var_idx]
                if prp == "0": pass
                elif prp == "1": ands.append(act_vars[kid])
                else:
                    c = self.formula.solver.get_var()
                    self.formula.solver.add_clauses(
                        make_and_bool_(prp, act_vars[kid], c))
                    ands.append(c)
            self.formula.check_vars[var] = self.formula.solver.xor_list(ands)

    def __build_formula(self):
        self.__init_labeled_vars()

        for i in range(self.rst_cycles): 
            self.trace.parse_next_cycle()
            #print(self.trace.get_signal_value(self.rst_name, 0))
            
        cycle = 0
        RST_VAL_AFTER = BIN_STR[(int(self.rst_phase) + 1) % 2]
        while (self.cycles == -1 or cycle < self.cycles) and self.trace.parse_next_cycle():
            assert (self.trace.get_signal_value(self.rst_name, 0) == RST_VAL_AFTER)
            #self.__dbg_cycle_instruction(cycle)
            print("Building formula for cycle %d vars %d clauses %d\n" % (cycle, self.formula.solver.nof_vars(), self.formula.solver.nof_clauses()), end="")
            self.__build_stable(cycle == 0)
            if self.mode == TRANSIENT:
                self.__build_trans(cycle == 0)
            cycle += 1
        #self.formula.solver.dbg_print()
        self.formula.collect_active(self.mode)
        self.__make_checks()
        self.formula.make_assumes(self.shares)

    def __get_assumes(self, ss):
        assumes = [self.formula.check_vars[share] for share in self.shares[ss]]
        assumes += [self.formula.assume_act[ss_] for ss_ in self.shares if ss != ss_]
        return assumes

    def __dbg_compute_cone(self, location, preds, last):
        cone = {}  # int cycle -> set nodes
        # compute found leakage sources and their cycles
        sources = {}
        for x in location:
            if x.cycle not in sources.keys():
                sources[x.cycle] = []
            sources[x.cycle].append(x.cell_id)
        current = set()
        terminals = set()

        def is_terminal(x):
            t = self.circuit.cells[x].type
            return t in REGISTER_TYPES or t == CONST_TYPE

        for cycle in range(last, -1, -1):
            stable = self.formula.node_vars_stable[cycle]
            trans = self.formula.node_vars_trans[cycle] if self.mode == TRANSIENT else {}
            cycle_nodes = set()
            # start search at sources or already found register predecessors
            if cycle in sources.keys():
                current = current.union(sources[cycle])
            # propagate backwards, ignore terminal nodes
            while len(current) != 0:
                n_current = set()
                for node in current:
                    cycle_nodes.add(node)
                    if is_terminal(node):
                        terminals.add(node)
                        continue
                    for p in preds[node]:
                        if ((p in stable.keys() or (p in trans.keys())) and
                                p not in terminals and p not in current):
                            n_current.add(p)
                current = n_current
            # initialize sources for previous cycle with terminal nodes
            for t in terminals:
                for p in preds[t]:
                    if p in stable.keys() or p in trans.keys():
                        current.add(p)
            terminals = set()
            cone[cycle] = cycle_nodes
        return cone

    def __dbg_write_label_trace(self, leak_num, cone, model, last):
        dbg = sys.stdout
        try:
            dbg_filepath = self.dbg_output_dir_path + ("/dbg-label-trace-%d.txt" % leak_num)
            dbg = open(dbg_filepath, "w")
            print("Writing a trace with the found error to %s" % dbg_filepath)
        except FileNotFoundError:
            pass

        cells = [self.circuit.cells[x] for x in self.variables]
        initial = ["%s:%s" % (c.name, c.pos) for c in cells]

        for name, init in zip(self.pretty_names, initial):
            dbg.write("%s = %s\n" % (name, init))
        # print debug label info
        for cycle in range(last + 1):
            stable = self.formula.node_vars_stable[cycle]
            trans = self.formula.node_vars_trans[cycle] if self.mode == TRANSIENT else None

            for node_id in self.circuit.nodes:
                if cone is not None and node_id not in cone[cycle]: continue
                cell = self.circuit.cells[node_id]
                for mode, mstr in zip((stable, trans), ("stable", "trans ")):
                    if mode is None or node_id not in mode: continue
                    res = self.formula.model_for_vars(model, mode[node_id])
                    line = " ; ".join(["%s=%d" % (n, v) for n, v in zip(self.pretty_names, res)])
                    vars = "%s" % (self.formula.prop_var_sets[mode[node_id]])
                    line = " %s | %s" % (line, vars)
                    dbg.write("%4d %s %20s: %s\n" % (cycle, mstr, cell, line))
        if dbg != sys.stdout: dbg.close()

    def __dbg_draw_dot(self, leak_num, cone_nodes, preds, location):
        all_nodes = set()
        edge_defs = set()

        for node in cone_nodes:
            all_nodes.add(node)
            for i in range(len(preds[node])):
                p = preds[node][i]
                all_nodes.add(p)
                extra = None if self.circuit.cells[node].type != MUX_TYPE else str(i)
                edge_defs.add((p, node, extra))

        dot = sys.stdout
        try:
            dot_filepath = self.dbg_output_dir_path + ("/dbg-circuit-%d.dot" % leak_num)
            dot = open(dot_filepath, "w")
            print("Writing a reduced circuit to %s" % dot_filepath)
        except FileNotFoundError:
            pass
        dot.write("strict digraph  {\n")
        node_attrs = {}
        for node in all_nodes:
            node_attrs[node] = "shape=" + ("oval" if node in cone_nodes else "rect")
        for x in location:
            node_attrs[x.cell_id] += ", style=filled, fillcolor=red"
        for node in all_nodes:
            dot.write("%d [label=\"%s\"; %s]\n" % (node, self.circuit.cells[node], node_attrs[node]))
        for edge in edge_defs:
            extra = ""
            if edge[2] == "2":
                extra = " [style=dashed]"
            elif edge[2] in BIN_STR:
                extra = " [arrowhead=%sdot]" % ("" if edge[2] == "0" else "o")
            dot.write("%d -> %d%s\n" % (edge[0], edge[1], extra))
        dot.write("}\n")
        if dot != sys.stdout: dot.close()

    def __dbg_state(self, leak_num, model, location):
        last = max(map(lambda x: x.cycle, location))
        # quick predecessors cache so that muxes are handled correctly
        predecessors = {}
        for n in self.circuit.nodes:
            cell = self.circuit.cells[n]
            if cell.type == MUX_TYPE:
                predecessors[n] = cell.mux_ins + [cell.select]
            else:
                predecessors[n] = list(self.circuit.predecessors(n))

        # do a backwards search here to find important nodes
        cone = self.__dbg_compute_cone(location, predecessors, last)
        # write a trace of all important labels for each cycle
        self.__dbg_write_label_trace(leak_num, cone, model, last)
        # generate a dot graph of the circuit that only contains stuff from the cones
        cone_nodes = set()
        for cycle in cone.keys(): cone_nodes = cone_nodes.union(cone[cycle])
        self.__dbg_draw_dot(leak_num, cone_nodes, predecessors, location)

    def check(self):
        self.formula.solver.add_clauses([[-self.formula.check_vars[m]] for m in self.masks])
        check_fmt = "Checking secret %%%dd %%s: " % len(str(len(self.shares)))
        for ss in sorted(list(self.shares.keys())):
            assumes = self.__get_assumes(ss)
            print(check_fmt % (ss, assumes[:self.order + 1]))
            r = self.formula.solver.solve(assumes)
            if not r: continue
            location = self.formula.analyse(assumes, self.num_leaks, self.mode)
            for i, loc in enumerate(location):
                model, acts = loc
                self.__dbg_state(i, model, acts)
            return False, location
        return True, []
