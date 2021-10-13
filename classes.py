from dataclasses import dataclass
from defines import inv_cell_enum
from Solver import Solver, make_xor_bool, make_and_bool_, make_or_bool_


@dataclass
class Label:
    bit: int
    type: str
    num: int


@dataclass
class ActiveInfo:
    cycle: int
    cell_id: int
    prop_var: int


@dataclass
class VariableInfo:
    cycle: int
    cell_id: int


@dataclass
class PropVarSet:
    __counter = 0   # static counter
    __num_vars = 0  # total number of variables for a node
    id:   int       # unique identifier
    vars: dict      # index -> prop
    ones: set       # set of fixed ones

    def __init__(self, num=None, biased=None, xor=None, choice=None, solver=None):
        self.id = PropVarSet.__counter
        PropVarSet.__counter += 1

        n_good = (num is not None) & 1
        b_good = (biased is not None) & 1
        x_good = (xor is not None) & 1
        c_good = (choice is not None) & 1

        # one of the constructors must apply
        assert(n_good + b_good + x_good + c_good != 0)
        # for the complex constructors, solver must be given
        if b_good + x_good + c_good != 0:
            assert(solver is not None)
            assert(isinstance(solver, Solver))
        # cannot do both biased and xor constructor
        assert(b_good + x_good + c_good <= 1)

        # xor constructor requires two compatible arguments
        def check_bin(args, num_):
            assert (len(args) == 2)
            assert (args[0].__num_vars == args[1].__num_vars)
            if num_ is None:
                num_ = args[0].__num_vars
            else:
                assert (num_ == args[0].__num_vars)
            return num_

        if x_good: num = check_bin(xor, num)
        if c_good: num = check_bin(choice, num)

        # biased constructor requires compatible length
        if b_good:
            if num is None: num = biased.__num_vars
            else: assert(num == biased.__num_vars)

        self.__num_vars = num
        self.vars = {}
        self.ones = set()

        if biased is not None:
            # add the condition that (new = old) or (new = 0)
            # when encoded normally: use p, q as tmp variables
            # (p -> (a = b)) | (q -> -a) | (p | q)
            # when p = q, then -b is encoded twice, so we replace q with -p
            # (p -> (a = b)) | (-p -> -a)

            assert(len(biased.vars) != 0 or len(biased.ones) != 0)
            p = solver.get_var()
            solver.add_comment("bias var %d used for %d" % (p, self.id))
            # for a fixed one, the new value is equal to p:
            # if p is 1, then we have to take the old value, which is 1
            # if p is 0, we have to take the value 0, which is also p
            for i in biased.ones:
                self.vars[i] = p
            # this is the simplified version that uses resolution
            for i in biased.vars:
                b = biased.vars[i]
                a = solver.get_var()
                self.vars[i] = a
                solver.add_clauses([[-a, b], [p, -a], [-p, a, -b]])

        if c_good:
            arg0, arg1 = choice
            p = solver.get_var()
            for i in range(self.__num_vars):
                x, y = arg0[i], arg1[i]
                # check and simplification
                if x == "1":
                    x = -p
                if y == "1":
                    y = p
                # check or simplification
                if type(x) == int and type(y) == int and x == -y:
                    self.ones.add(i)
                    continue
                # build formula
                res = None
                if x == -p or y == p:
                    if y == "0": res = x
                    elif x == "0": res = y
                    else:
                        res = solver.get_var()
                        solver.add_clauses(make_or_bool_(x, y, res))
                else:
                    x_, y_ = None, None
                    if x != "0":
                        x_ = solver.get_var()
                        solver.add_clauses(make_and_bool_(-p, x, x_))
                    if y != "0":
                        y_ = solver.get_var()
                        solver.add_clauses(make_and_bool_(+p, y, y_))
                    if x_ is None and y_ is None: continue

                    if x_ is None: res = y_
                    elif y_ is None: res = x_
                    else:
                        res = solver.get_var()
                        solver.add_clauses(make_or_bool_(x_, y_, res))
                assert(res is not None)
                self.vars[i] = res

        if x_good:
            arg0, arg1 = xor
            # print(arg0, arg1)

            for i in range(self.__num_vars):
                x, y = arg0[i], arg1[i]
                tx, ty = type(x), type(y)
                # xor is zero
                if x == y: pass
                # xor is one
                elif ((x, y) in (("0", "1"), ("1", "0")) or
                      (ty == int and tx == int and x == -y)):
                    self.ones.add(i)
                # xor is the left
                elif tx == int and ty == str:
                    self.vars[i] = x if (y == "0") else -x
                # xor is the right
                elif tx == str and ty == int:
                    self.vars[i] = y if (x == "0") else -y
                else:
                    assert(tx == int and ty == int), "type mismatch: %s %s ; %s %s" % (x, y, tx, ty)
                    z = solver.get_var()
                    solver.add_clauses(make_xor_bool(x, y, z))
                    self.vars[i] = z
        pass

    def __getitem__(self, idx):
        assert(idx < self.__num_vars), "%d and %d" % (idx, self.__num_vars)
        q = self.vars.get(idx)
        if q is not None: return self.vars[idx]
        else: return str((idx in self.ones) & 1)

    def tuple(self):
        return tuple(self.__getitem__(i) for i in range(self.__num_vars))


@dataclass
class Cell:
    name: str
    type: int
    pos: int
    select: int = None
    mux_ins: list = None
    clock: int = None
    reset: int = None
    preset: int = None

    def __lt__(self, other):
        return str(self.name) < str(other.name)

    def __str__(self):
        return "%s %s[%d]" % (inv_cell_enum[self.type], self.name, self.pos)
