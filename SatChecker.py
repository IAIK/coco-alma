import itertools

from defines import *
from helpers import label_type, parity
import time
import sys
import pickle
import networkx as nx
from classes import ActiveInfo, VariableInfo, PropVarSet
from Solver import *
import dbg

class Formula:
    def __init__(self, num_vars):
        self.num_vars = num_vars     # int
        self.node_vars_stable = []   # cycle -> node -> PropVarSet id
        self.node_vars_trans = []    # cycle -> node -> PropVarSet id
        self.node_vars_diff = []     # cycle -> node -> PropVarSet id
        self.prop_var_sets = {}      # id -> PropVarSet
        self.linear_gate_set = {}    # vars -> (vars...)
        self.linear_set_cache = {}   # (vars...) -> vars
        self.nonlin_gate_set = {}    # vars -> (vars...)
        self.nonlin_set_cache = {}   # (vars...) -> vars
        self.biased_cache = set()    # {vars...}
        self.biased_vars = set()     # {vars...}
        self.check_vars = {}         # labeled node -> prop
        self.assume_act = {}         # labeled node -> prop
        self.covering_top_vars = {}  # vars -> {vars...}
        self.covered_bot_vars = {}   # {vars...}
        self.vars_to_info = {}       # vars -> (cycle, node)
        self.solver = Solver(store_clauses=True, store_comments=True)

        self.dbg_defmap = {}

    def make_act_assumes(self, shares):
        assume_act = {}
        for ss in sorted(list(shares.keys())):
            # if either all or none are active, there is a leak
            tmp_pos, tmp_neg, act = [self.solver.get_var() for _ in range(3)]
            self.solver.add_clauses(make_and_bool([self.check_vars[share] for share in shares[ss]], tmp_pos))
            self.solver.add_clauses(make_and_bool([-self.check_vars[share] for share in shares[ss]], tmp_neg))
            self.solver.add_clause([-act, tmp_pos, tmp_neg])
            assume_act[ss] = act
        return assume_act


    def analyse(self, assumes, num_leaks, mode, active):
        fault = []
        res = True
        cache = set()
        enable = self.solver.get_var()
        last = -1
        while res:
            fault.clear()
            model = set(self.solver.get_model())
            for act in active.values():
                if act.prop_var in model:
                    fault.append(act)
            last = max(map(lambda x: x.cycle, fault))
            # forbid all cycles >= last
            for act in active.values():
                if act.prop_var in cache or act.cycle < last: continue
                self.solver.add_clause([-enable, -act.prop_var])
                cache.add(act.prop_var)
            res = self.solver.solve(assumes + [enable])
        assert(last >= 0)
        # add a new enable for > last cycle
        restrict = self.solver.get_var()
        for act in active.values():
            if act.cycle <= last: continue
            self.solver.add_clause([-restrict, -act.prop_var])
        result = []  # list of (model, list act)
        seen = []
        for i in range(num_leaks):
            res = self.solver.solve(assumes + [-enable, restrict] + seen)
            if not res: break
            model = set(self.solver.get_model())
            fault = list(filter(lambda x: x.prop_var in model, active.values()))
            fault_min = self.__backtrack_fault(model, fault, mode)
            result.append((model, fault_min))
            seen += [-act.prop_var for act in fault]
        return result

    def assure_biased(self, vars_id):
        if vars_id in self.biased_cache:
            self.biased_cache.remove(vars_id)
            return vars_id
        biased_vars = PropVarSet(biased=self.prop_var_sets[vars_id], solver=self.solver)
        self.solver.add_comment("defined %d as biased %d (%s)" %
                                (biased_vars.id, vars_id, biased_vars))
        self.prop_var_sets[biased_vars.id] = biased_vars
        self.nonlin_gate_set[biased_vars.id] = self.nonlin_gate_set[vars_id]
        self.linear_gate_set[biased_vars.id] = (biased_vars.id,)
        self.nonlin_set_cache[(biased_vars.id,)] = biased_vars.id
        self.linear_set_cache[(biased_vars.id,)] = biased_vars.id
        self.dbg_defmap[biased_vars.id] = vars_id

        self.add_cover(biased_vars.id, vars_id)
        return biased_vars.id

    def add_cover(self, top, bot):
        if top not in self.covering_top_vars:
            self.covering_top_vars[top] = set()
        self.covering_top_vars[top].add(bot)
        if bot not in self.covered_bot_vars:
            self.covered_bot_vars[bot] = set()
        self.covered_bot_vars[bot].add(top)

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

            self.dbg_defmap[gate_vars.id] = (vars_id1, "xor", vars_id2)

            if len(gate_vars.ones) == 0 and len(gate_vars.vars) == 0: return None
            self.prop_var_sets[gate_vars.id] = gate_vars

            self.symdiff_gate_set(gate_vars.id, vars_id1, vars_id2, symdiff=sd)
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
            
            self.dbg_defmap[gate_vars.id] = (vars_id1, "and", vars_id2)

            if len(gate_vars.ones) == 0 and len(gate_vars.vars) == 0: return None
            self.prop_var_sets[gate_vars.id] = gate_vars

            self.union_gate_set(gate_vars.id, vars_id1, vars_id2, union=un)
            self.add_cover(gate_vars.id, vars_id1)
            self.add_cover(gate_vars.id, vars_id2)
            self.linear_gate_set[gate_vars.id] = (gate_vars.id,)
            self.linear_set_cache[(gate_vars.id,)] = gate_vars.id

            self.biased_cache.add(gate_vars.id)
            self.biased_vars.add(gate_vars.id)
        return gate_vars.id

    def make_choice(self, arg1, arg2):
        if arg1 == arg2: return arg1
        arg_pvs = tuple(self.prop_var_sets[x] for x in (arg1, arg2))
        res = PropVarSet(choice=arg_pvs, solver=self.solver)
        self.prop_var_sets[res.id] = res
        self.nonlin_gate_set[res.id] = (res.id,)
        self.linear_gate_set[res.id] = (res.id,)
        self.nonlin_set_cache[(res.id,)] = res.id
        self.linear_set_cache[(res.id,)] = res.id
        self.add_cover(res.id, arg1)
        self.add_cover(res.id, arg2)
        return res.id

    def collect_active_time_constrained(self, mode, hamming, glitch_behavior, cycle, ignored):
        node_vars = self.node_vars_stable if (mode == STABLE) else self.node_vars_trans
        active = set()
        vars = node_vars[cycle].copy()
        if hamming or (mode == TRANSIENT and glitch_behavior == LOOSE):
            for node in vars.keys():
                if node in self.node_vars_diff[cycle]:
                    vars[node] = self.make_choice(vars[node], self.node_vars_diff[cycle][node])

        for node in vars.keys():
            if node in ignored: continue
            self.vars_to_info[vars[node]] = VariableInfo(cycle, node)
            if vars[node] in self.covered_bot_vars: continue
            active.add(vars[node])

        return [(x,) for x in sorted(active)]

    def collect_active_classic(self, mode):
        node_vars = [self.node_vars_stable, self.node_vars_trans][(mode == TRANSIENT) & 1]
        active = set()
        for cycle, vars in enumerate(node_vars):
            for node in vars.keys():
                self.vars_to_info[vars[node]] = VariableInfo(cycle, node)
                active.add(vars[node])

        # collect vars from different clock cycles belonging to a node
        node_to_vars = {}
        node_to_covers = {}
        all_active = []
        for vars_id in active:
            info = self.vars_to_info[vars_id]
            if info.cell_id not in node_to_vars:
                node_to_vars[info.cell_id] = set()
            node_to_vars[info.cell_id].add(vars_id)
            if vars_id not in self.covered_bot_vars:
                node_to_covers[info.cell_id] = set()
                continue
            covers = set()
            for x in self.covered_bot_vars[vars_id]:
                if x not in self.vars_to_info: continue
                covers.add(self.vars_to_info[x].cell_id)
            if info.cell_id not in node_to_covers:
                node_to_covers[info.cell_id] = covers
            else:
                node_to_covers[info.cell_id].intersection_update(covers)

        for nid in node_to_vars:
            if len(node_to_covers[nid]) != 0:
                continue
            all_active.append(tuple(node_to_vars[nid]))
        return all_active

    def model_for_vars(self, model, vars_id):
        props = self.prop_var_sets[vars_id]
        l = tuple(((x in model) & 1) if type(x) == int else int(x) for x in props.tuple())
        return l

    def __backtrack_fault(self, model, location, mode):
        node_vars = [self.node_vars_stable, self.node_vars_trans][(mode == TRANSIENT) & 1]
        result = []
        for act in location:
            cycle = act.cycle
            cell_id = act.cell_id
            found = True
            while found:
                vars_id = node_vars[cycle][cell_id]
                covered = self.covering_top_vars.get(vars_id)
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


class SatChecker(object):
    def __init__(self, labels, ignored, trace, safe_graph, args):
        assert(args.mode in (TRANSIENT, STABLE))

        self.circuit = safe_graph

        assert(args.probing_model != CLASSIC or args.cycles != UINT_MAX)
        self.labels = labels
        self.trace = trace
        self.order = args.order
        self.cycles = args.cycles
        self.from_cycle = args.from_cycle
        self.hamming = args.hamming
        self.mode = args.mode
        self.glitch_behavior = args.glitch_behavior
        self.probing_model = args.probing_model
        self.trace_stable = args.trace_stable
        self.rst_name = args.rst_name
        self.rst_cycles = args.rst_cycles
        self.rst_phase = args.rst_phase
        self.num_leaks = args.num_leaks
        self.minimize_leaks = args.minimize_leaks
        self.dbg_output_dir_path = args.dbg_output_dir_path
        self.export_cnf = args.export_cnf
        self.kissat_bin_path = args.kissat_bin_path
        if self.kissat_bin_path:
            self.kissat_dbg_map = {}
        self.static_randoms = []
        self.volatile_randoms = []
        self.shares = {}
        self.variables = []
        self.var_indexes = {}
        self.pretty_names = []
        self.debugs = set(args.debugs)
        self.ignored = ignored
        self.dbg_exact_formula = args.dbg_exact_formula
        self.checking_mode = args.checking_mode
        self.__extract_label_info(labels)
        self.num_vars = len(self.variables) + (self.cycles * len(self.volatile_randoms))
        assert (self.num_vars == len(self.pretty_names))

        self.formula = Formula(self.num_vars)

    def __extract_label_info(self, labels):
        for label_id in labels.keys():
            label = labels[label_id]
            if label.type == LABEL_OTHER: continue
            if label.type == LABEL_VOLATILE_RANDOM: continue
            if label.type == LABEL_STATIC_RANDOM:
                self.pretty_names.append("m%d" % len(self.static_randoms))
                self.static_randoms.append(label.bit)
            if label.type == LABEL_SHARE:
                if label.num not in self.shares.keys():
                    self.shares[label.num] = []
                self.pretty_names.append("s%d:%d" % (label.num, len(self.shares[label.num])))
                self.shares[label.num].append(label.bit)
            self.var_indexes[label.bit] = len(self.variables)
            self.variables.append(label.bit)
        pnames = []
        for label_id in labels.keys():
            label = labels[label_id]
            if label.type == LABEL_VOLATILE_RANDOM:
                pnames.append("r%d" % len(self.volatile_randoms))
                self.volatile_randoms.append(label.bit)
        if len(self.volatile_randoms) == 0: return
        for i in range(self.cycles):
            self.pretty_names += ["%s:%d" % (p, i) for p in pnames]
            for idx, r in enumerate(self.volatile_randoms):
                self.var_indexes[(r, i)] = len(self.variables) + i * len(self.volatile_randoms) + idx

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

    def __make_mux_sel_not_stable(self, mode, mux_ins, curr_vars):
        vars_ = [curr_vars.get(m) for m in mux_ins]
        vars_ = [v for v in vars_ if v is not None]
        res_vars = None
        if len(vars_) == 1:
            res_vars = vars_[0]
        elif len(vars_) == 2:
            if mode == STABLE or self.glitch_behavior == LOOSE:
                res_vars = self.formula.make_choice(vars_[0], vars_[1])
            else:
                res_vars = self.formula.make_simple(AND_TYPE, vars_[0], vars_[1])
        return res_vars

    # @profile
    def __proc_mux(self, mode, select, mux_ins, curr_vars, sel_stable=True):
        assert(select is not None)
        assert(len(mux_ins) == 2)
        if select not in curr_vars.keys():
            if sel_stable:
                sel_cell = self.circuit.cells[select]
                value = self.trace.get_signal_value(sel_cell.name, sel_cell.pos)
                assert(value in BIN_STR), "invalid select value is '%s'" % value
                return curr_vars.get(mux_ins[int(value)])
            else:
                return self.__make_mux_sel_not_stable(mode, mux_ins, curr_vars)
            assert(False)
        res_vars = self.__make_mux_sel_not_stable(mode, mux_ins, curr_vars)
        if res_vars is None: return curr_vars[select]
        return self.formula.make_simple(AND_TYPE, res_vars, curr_vars[select])

    def __build_node_stable(self, node_id, curr_vars, prev_vars):
        cell = self.circuit.cells[node_id]
        nvars = None
        if cell.type in GATE_TYPES:
            nvars = self.__proc_simple(node_id, cell.type, curr_vars)
        elif cell.type == NOT_TYPE or cell.type in REGISTER_TYPES:
            if cell.type in REGISTER_TYPES and node_id in curr_vars: return None
            target_vars = curr_vars if cell.type == NOT_TYPE else prev_vars
            pred0 = self.circuit.predecessors(node_id).__next__()
            nvars = target_vars.get(pred0)
        elif cell.type == PORT_TYPE:
            return None
        elif cell.type == MUX_TYPE:
            nvars = self.__proc_mux(STABLE, cell.select, cell.mux_ins, curr_vars)
        return nvars

    # @profile
    def __build_stable(self):
        curr_vars = self.formula.node_vars_stable[-1]
        prev_vars = {}
        if len(self.formula.node_vars_stable) > 1:
            prev_vars = self.formula.node_vars_stable[-2]

        for node_id in self.circuit.nodes:
            nvars = self.__build_node_stable(node_id, curr_vars, prev_vars)
            if nvars is not None: curr_vars[node_id] = nvars
        pass

    def __proc_trans_reg(self, reg, prev, curr):
        if self.probing_model == CLASSIC:
            return curr.get(reg)
        # in the case it only records ONCE, handle transition leakage
        valid = [(reg in x) for x in (prev.keys(), curr.keys())]
        if all(valid):
            if prev[reg] == curr[reg]: return prev[reg]
            self.formula.solver.add_comment("Definition for trans %d %s" % (reg, self.circuit.cells[reg]))
            if self.glitch_behavior == LOOSE:
                return self.formula.make_choice(prev[reg], curr[reg])
            else:
                return self.formula.make_simple(AND_TYPE, prev[reg], curr[reg])
        elif not any(valid): return None

        signal = prev[reg] if (reg in prev) else curr[reg]
        biased = self.formula.assure_biased(signal)
        return biased

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
            # XOR, XNOR, NOT gates are stable if all inputs are stable
            # same goes for AND with value 1 and OR with value 0
            if cell.type in LINEAR_TYPES or cell.type == NOT_TYPE or (
               cell.type in NONLINEAR_TYPES and curr_val != TRIGGERS[cell.type]):
                stability[node_id] &= all([stability[p] for p in preds])
            # if AND is 0, it is stable if any input that is 0 is stable
            # if OR is 1, it is stable if any input that is 1 is stable
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
    def __build_trans(self):
        prev_stable = {}
        if len(self.formula.node_vars_stable) > 1:
            prev_stable = self.formula.node_vars_stable[-2]
        curr_stable = self.formula.node_vars_stable[-1]
        curr_vars = self.formula.node_vars_trans[-1]
        all_stable_nodes = set(curr_stable.keys()).union(prev_stable.keys())
        stability = self.__make_stability_info()
        for node_id in self.circuit.nodes:
            if node_id in self.ignored:
                if node_id in curr_stable: curr_vars[node_id] = curr_stable[node_id]
                continue

            cell = self.circuit.cells[node_id]
            nvars = None
            preds = self.circuit.predecessors(node_id)

            if cell.type in GATE_TYPES:
                info = {p: ((p not in all_stable_nodes) and (self.trace_stable or stability[p])) for p in preds}
                type_ = cell.type if (self.glitch_behavior == LOOSE) else AND_TYPE
                nvars = self.__proc_simple(node_id, type_, curr_vars, info)
            elif cell.type == NOT_TYPE:
                pred0 = preds.__next__()
                nvars = curr_vars.get(pred0)
            elif cell.type in REGISTER_TYPES:
                nvars = self.__proc_trans_reg(node_id, prev_stable, curr_stable)
            elif cell.type == PORT_TYPE:
                nvars = curr_stable.get(node_id)
            elif cell.type == MUX_TYPE:
                sel_stable = (cell.select not in all_stable_nodes) and stability[cell.select]
                nvars = self.__proc_mux(TRANSIENT, cell.select, cell.mux_ins, curr_vars, sel_stable)

            if nvars is not None: curr_vars[node_id] = nvars

    def __init_propvarset(self, var_idx, var):
        gate_vars = PropVarSet(num=self.num_vars)
        gate_vars.ones.add(var_idx)
        self.formula.prop_var_sets[gate_vars.id] = gate_vars
        self.formula.nonlin_gate_set[gate_vars.id] = (gate_vars.id,)
        self.formula.linear_gate_set[gate_vars.id] = (gate_vars.id,)
        self.formula.nonlin_set_cache[(gate_vars.id,)] = gate_vars.id
        self.formula.linear_set_cache[(gate_vars.id,)] = gate_vars.id
        assert (self.circuit.cells[var].type in REGPORT_TYPES)
        self.formula.node_vars_stable[-1][var] = gate_vars.id
        self.formula.node_vars_trans[-1][var] = gate_vars.id

    def __init_cycle(self, cycle):
        self.formula.node_vars_stable.append({})
        self.formula.node_vars_trans.append({})
        self.formula.node_vars_diff.append({})
        assert(len(self.formula.node_vars_stable) == (cycle+1))

        #variable = static_randoms + shares
        for var_idx, var, in enumerate(self.variables):
            if (self.circuit.cells[var].type in REGISTER_TYPES) and cycle != 0: continue
            if cycle == 0:
                self.__init_propvarset(var_idx, var)
                # DEBUG
                gate_vars_id = self.formula.node_vars_stable[-1][var]
                self.formula.dbg_defmap[gate_vars_id] = (self.pretty_names[var_idx])

            else:
                # reuse
                assert(var in self.formula.node_vars_stable[cycle-1])
                gate_vars_id = self.formula.node_vars_stable[cycle-1][var]
                self.formula.node_vars_stable[-1][var] = gate_vars_id
                self.formula.node_vars_trans[-1][var] = gate_vars_id

        #randoms = randoms
        start = len(self.variables) + cycle * len(self.volatile_randoms)
        assert (start + len(self.volatile_randoms) <= self.num_vars)
        for var, var_idx in zip(self.volatile_randoms, range(start, start + len(self.volatile_randoms))):
            self.__init_propvarset(var_idx, var)

    def __find_reset(self, reset):
        for i in range(self.rst_cycles):
            self.trace.parse_next_cycle()
            assert(self.trace.get_signal_value(self.rst_name, 0) == reset), "reset duration mismatch"
        assert (len(self.formula.node_vars_stable) == 0)
        assert (len(self.formula.node_vars_trans) == 0)

    def __build_hamming(self):
        prev_stable = self.formula.node_vars_stable[-2]
        curr_stable = self.formula.node_vars_stable[-1]
        curr_diff = self.formula.node_vars_diff[-1]
        for node_id in prev_stable:
            if node_id not in curr_stable: continue
            if prev_stable[node_id] == curr_stable[node_id]: continue
            nvars = self.formula.make_simple(XOR_TYPE, prev_stable[node_id], curr_stable[node_id])
            if nvars is None: continue
            curr_diff[node_id] = nvars

    def __build_cycle(self, reset, cycle):
        print("RST value: ", self.trace.get_signal_value(self.rst_name, 0))
        print(reset)
        assert (self.trace.get_signal_value(self.rst_name, 0) == reset)
        self.__init_cycle(cycle)
        print("Building formula for cycle %d: " % cycle)

        for dbg_name in self.debugs:
            try:
                res = int(self.trace.get_signal_value(dbg_name, None), 2)
                print("debug: %s = %x" % (dbg_name, res))
            except Exception as e:
                # print("%s\ncannot find %s" % (e, dbg_name))
                pass

        self.__build_stable()
        if self.mode == TRANSIENT:
            self.__build_trans()
        if self.mode == STABLE and self.hamming and cycle > 0:
            self.__build_hamming()
        target_vars = self.formula.node_vars_stable[-1]
        if self.mode == TRANSIENT:
            target_vars = self.formula.node_vars_trans[-1]
        for node_id in target_vars:
            cell = self.circuit.cells[node_id]
            for debug in self.debugs:
                if debug in cell.name:
                    print(cell, self.formula.prop_var_sets[target_vars[node_id]])

        # if self.glitch_behavior == LOOSE and cycle > 0:
        #     self.__build_hamming()

        #DEBUG
        if self.dbg_exact_formula:
            self.dbgLabelsStable.dbg_print_labels(self.formula.node_vars_stable[cycle], self.formula.dbg_defmap, self.circuit.cells, cycle)

        print("vars %d clauses %d" % (self.formula.solver.nof_vars(), self.formula.solver.nof_clauses()))

    def __build_formula(self):
        self.__find_reset(self.rst_phase)
        cycle = 0
        inactive_val = str((self.rst_phase == "0") & 1)

        while (cycle < self.cycles) and self.trace.parse_next_cycle():
            self.__build_cycle(inactive_val, cycle)
            cycle += 1
        self.cycles = cycle

    def __get_assumes_per_secret(self, ss, assume_act):
        assumes = [self.formula.check_vars[share] for share in self.shares[ss]]
        assumes += [assume_act[ss_] for ss_ in self.shares if ss != ss_]
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
        initial = ["%s[%s]" % (c.name, c.pos) for c in cells]

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
                    line = " ; ".join(["%s" % n for n, v in zip(self.pretty_names, res) if v == 1])
                    # vars = "%s" % (self.formula.prop_var_sets[mode[node_id]])
                    # line = " %s | %s" % (line, vars)
                    dbg.write("%4d %s %20s: %s\n" % (cycle, mstr, cell, line))
        if dbg != sys.stdout: dbg.close()

    def __dbg_draw_dot(self, leak_num, cone_nodes, preds, location):
        all_nodes = set()
        edge_defs = set()

        for node in cone_nodes:
            all_nodes.add(node)
            if self.circuit.cells[node].type in REGISTER_TYPES: continue
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

    def __get_assumes_per_location(self, pvs: PropVarSet):
        act_assumes = []
        positive = []
        trivial = False
        for ss in sorted(list(self.shares.keys())):
            # print(ss, self.shares[ss], [self.var_indexes[s] for s in self.shares[ss]])
            vs = {pvs[self.var_indexes[s]] for s in self.shares[ss]}
            # print(pvs, vs)
            found_0 = "0" in vs
            found_1 = "1" in vs
            vs = {v for v in vs if type(v) != str}
            if found_0 and found_1:
                trivial = True
                break
            if len(vs) == 0: continue

            pos, neg = None, None
            if len(vs) == 1:
                pos = vs.pop()
                neg = -pos
            else:
                pos = self.formula.solver.get_var()
                neg = self.formula.solver.get_var()

                cls = make_and_bool(vs, pos) + make_and_bool([-x for x in vs], neg)
                self.formula.solver.add_clauses(cls)
            act = None
            if found_0:
                act = neg
            elif found_1:
                act = pos
            else:
                act = self.formula.solver.get_var()
                self.formula.solver.add_clause([-act, pos, neg])
            act_assumes.append(act)
            if not found_0:
                positive.append(pos)

        if trivial or len(positive) == 0: return None, None
        """
        pos = None
        if len(positive) == 1:
            pos = positive[0]
        else:
            pos = self.formula.solver.get_var()
            self.formula.solver.add_clauses(make_or_bool(positive, pos))
        act_assumes.append(pos)
        """
        return act_assumes, positive

    def get_leak_model(self, assumes, positive):
        model = set(self.formula.solver.get_model())
        if self.minimize_leaks:
            opt_assumes = []
            can_assumes = []
            for p in positive:
                if p not in model:
                    opt_assumes.append(-p)
                else:
                    can_assumes.append(p)

            while len(can_assumes):
                c = can_assumes.pop()
                r = self.formula.solver.solve(assumes + opt_assumes + [-c])
                if not r: continue
                opt_assumes.append(-c)

            r = self.formula.solver.solve(assumes + opt_assumes)
            assert (r)
            model = set(self.formula.solver.get_model())
        return model


    def __check_tuple(self, all_ids, masks):
        var_infos = [self.formula.vars_to_info[vid] for vid in all_ids]
        if all(map(lambda x: x.cycle < self.from_cycle, var_infos)): return None
        probe_time = time.time()

        pvs_id = all_ids[0]
        for pvs_id_ in all_ids[1:]:
            pvs_id = self.formula.make_simple(AND_TYPE, pvs_id, pvs_id_)
            assert (pvs_id is not None)

        pvs = self.formula.prop_var_sets[pvs_id]
        mask_assumes = {pvs[self.var_indexes[m]] for m in masks}
        # trivial case, a mask is always active
        if "1" in mask_assumes: return None
        mask_assumes = [-x for x in mask_assumes if type(x) != str]

        act_assumes, positive = self.__get_assumes_per_location(pvs)
        # trivial case, no complete secrets
        if act_assumes is None: return None

        assumes = mask_assumes + act_assumes
        if self.probing_model == TIME_CONSTRAINED:
            cells = [self.circuit.cells[ai.cell_id] for ai in var_infos]
            for vi in var_infos:
                target = self.formula.node_vars_trans if self.mode == TRANSIENT else self.formula.node_vars_stable
                # print("check set %s:" % vi, self.formula.prop_var_sets[target[vi.cycle][vi.cell_id]])
            fmt_list = ["(cycle %d, %s)" % (vi.cycle, c) for vi, c in zip(var_infos, cells)]
        else:
            cell_ids = set(ai.cell_id for ai in var_infos)
            cells = [self.circuit.cells[cid] for cid in cell_ids]
            fmt_list = ["%s" % c for c in cells]
        
        if self.export_cnf:
            self.formula.solver.dbg_print_cnf("_".join(set(str(ai.cell_id) for ai in var_infos)), list(assumes), list(positive), self.dbg_output_dir_path)
            if self.kissat_bin_path:
                self.kissat_dbg_map["_".join(set(str(ai.cell_id) for ai in var_infos))] = var_infos
                return None

        sys.stdout.flush()
        out_fmt = "Checking probe %s: " % "; ".join(fmt_list)
        for ip, p in enumerate(positive):
            assumes.append(p)
            r = self.formula.solver.solve(assumes)
            print("%s[%d/%d]%s\r" % (out_fmt, ip + 1, len(positive), " " * 10), end="")
            sys.stdout.flush()
            if r: break
            assumes.pop()
        end_time = time.time()
        print("%s%.2f%s" % (out_fmt, end_time - probe_time, 10 * " "))
        if not r: return None
        model = self.get_leak_model(assumes, positive)
        return (model, var_infos)

    def __debug_leaks(self, leaks):
        for i, loc in enumerate(leaks):

            model, acts = loc
            self.__dbg_state(i, model, acts)

    def __collect_masks(self, num_cycles):
        cycle_volatile_randoms = []
        for r in self.volatile_randoms:
            for cyc in range(num_cycles):
                cycle_volatile_randoms.append((r, cyc))
        return self.static_randoms + cycle_volatile_randoms

    def __check_secure_classic(self):
        leaks = []
        
        #DEBUG
        if self.dbg_exact_formula:
            self.dbgLabelsStable = dbg.DbgLabels(self.dbg_output_dir_path + "/dbgLabelsStable")

        self.__build_formula()
        active = self.formula.collect_active_classic(self.mode)
        all_masks = self.__collect_masks(self.cycles)
        for probe_i, vars_ids in enumerate(itertools.combinations(active, self.order)):
            all_ids = tuple(set(sum(vars_ids, tuple())))
            leak = self.__check_tuple(all_ids, all_masks)
            if leak is None: continue
            leaks.append(leak)
            if len(leaks) >= self.num_leaks: break
        return leaks


    def __make_checks(self, active):
        # keys is just used to keep the order consistent
        keys = list(active.keys())
        act_vars = [active[key].prop_var for key in keys]
        self.formula.solver.at_most_k_of_n(self.order, act_vars)
        for var_name, var_idx in self.var_indexes.items():
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
            if len(ands) == 0:
                print("Empty xor for variable index ", var_name)
                x = self.formula.solver.get_var()
                self.formula.solver.add_clause([-x])
                self.formula.check_vars[var_name] = x
            else:
                self.formula.check_vars[var_name] = self.formula.solver.xor_list(ands)
        
    



    def __check_secure_time_constrained(self):
        leaks = []
        self.__find_reset(self.rst_phase)
        cycle = 0
        inactive_val = str((self.rst_phase == "0") & 1)

        prev_active = []
        curr_active = []
        
        if self.dbg_exact_formula:
            self.dbgLabelsStable = dbg.DbgLabels(self.dbg_output_dir_path + "/dbgLabelsStable")

        while (cycle < self.cycles) and self.trace.parse_next_cycle():
            self.__build_cycle(inactive_val, cycle)
            # for nid in self.circuit.nodes:
            #     if nid not in self.formula.node_vars_stable[cycle]: continue
            #     print(self.circuit.cells[nid])

            all_masks = self.__collect_masks(cycle + 1)
            prev_active += curr_active
            curr_active = self.formula.collect_active_time_constrained(self.mode, self.hamming, self.glitch_behavior, cycle, self.ignored)

            print("Checking cycle %d:" % cycle)
            if self.checking_mode == PER_SECRET:
                # Collect_active

                node_vars = [self.formula.node_vars_stable, self.formula.node_vars_trans][(self.mode == TRANSIENT) & 1]
                active = {}
                for cycle, vars in enumerate(node_vars):
                    for node in vars.keys():
                        self.formula.vars_to_info[vars[node]] = VariableInfo(cycle, node)
                        if vars[node] in active.keys(): continue
                        if vars[node] in self.formula.covered_bot_vars: continue
                        active[vars[node]] = ActiveInfo(cycle, node, self.formula.solver.get_var())

                self.__make_checks(active)
                assume_act = self.formula.make_act_assumes(self.shares)
                self.formula.solver.add_clauses([[-self.formula.check_vars[m]] for m in all_masks])
                check_fmt = "Checking secret %%%dd %%s: " % len(str(len(self.shares)))
                for ss in sorted(list(self.shares.keys())):
                    assumes = self.__get_assumes_per_secret(ss, assume_act)
                    print(check_fmt % (ss, assumes[:self.order + 1]))
                    r = self.formula.solver.solve(assumes)
                    if not r: continue
                    leak = self.formula.analyse(assumes, self.num_leaks, self.mode,active)
                    
                    leaks.append(leak[0])  
                    if len(leaks) >= self.num_leaks: return leaks
            elif self.checking_mode == PER_LOCATION:
                # done checking comb(prev_active, ord) is checked
                # ...
                # need to check comb(prev_active, 1) + comb(curr_active, ord)
                # need to check comb(prev_active, 0) + comb(curr_active, ord)
                for prev_ord in range(0, self.order):
                    curr_ord = self.order - prev_ord
                    for probe_prev_i, prev_vars_ids in enumerate(itertools.combinations(prev_active, prev_ord)):
                        for probe_curr_i, curr_vars_ids in enumerate(itertools.combinations(curr_active, curr_ord)):
                            all_ids = tuple(set(sum(prev_vars_ids + curr_vars_ids, tuple())))
                            leak = self.__check_tuple(all_ids, all_masks)
                            if leak is None: continue
                            leaks.append(leak)
                            if len(leaks) >= self.num_leaks: return leaks
            cycle += 1
        return leaks

    def check(self):
        start_time = time.time()
        leaks = []
        if self.probing_model == CLASSIC:
            leaks = self.__check_secure_classic()
        else:
            leaks = self.__check_secure_time_constrained()
        print("Finished in %.2f" % (time.time() - start_time))
        self.__debug_leaks(leaks)
        return len(leaks) == 0, leaks

    def checkKissat(self):
        import subprocess as sp

        for cnf_file in os.listdir(self.dbg_output_dir_path):
            if cnf_file.endswith(".cnf"):
                kissat_log_file = open(self.dbg_output_dir_path + "/kissat.log", "w")
                cnf_file_path = os.path.join(self.dbg_output_dir_path, cnf_file)
                print("Solving %s..."%(cnf_file), end="")
                args = [self.kissat_bin_path, "--unsat", cnf_file_path]
                p = sp.Popen(args, stdout=kissat_log_file)
                p.communicate()
                kissat_log_file.close()
                if p.returncode == 10:
                    # SAT
                    print("Found leak. The execution is not secure.")
                    filename = cnf_file.replace("formula_", "").replace(".cnf", "")
                    filename = filename.split("_")[0:-1]
                    filename = "_".join(filename)

                    leaks = [self.kissat_dbg_map[filename]]
                    for i in range(len(leaks)):
                        gates = leaks[i]
                        sys.stdout.write("leak %d: " % i)
                        for g in gates:
                            cell = self.circuit.cells[g.cell_id]
                            sys.stdout.write("(cycle: %d, cell: %s, id: %d) " % (g.cycle, cell, g.cell_id))
                        sys.stdout.write("\n")
                    
                    # model = set()
                    # kissat_log_file = open(self.dbg_output_dir_path + "/kissat.log", "r")
                    # for l in kissat_log_file:
                    #     if l.startswith("v"):
                    #         # Remove 'v' and '\n'
                    #         l = l[:-1]
                    #         l = l.split(" ")
                    #         l = l[1:]

                    #         terms = [int(_) for _ in l]
                    #         model.update(terms)



                    #kissat_log_file.close()



                    return -1
                elif p.returncode == 20:
                    print("OK")
                    # UNSAT
                    pass
        return 0
                

