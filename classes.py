from dataclasses import dataclass
from defines import inv_cell_enum
from Solver import Solver, make_xor_bool


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
    activator: int  # activation variable for constraints

    def __init__(self, num=None, biased=None, xor=None, solver=None):
        self.id = PropVarSet.__counter
        PropVarSet.__counter += 1

        # one of the constructors must apply
        assert(biased is not None or xor is not None or num is not None)
        # for the complex constructors, solver must be given
        if biased is not None or xor is not None:
            assert(solver is not None)
            assert(isinstance(solver, Solver))
        # cannot do both biased and xor constructor
        assert(biased is None or xor is None)

        # xor constructor requires two compatible arguments
        if xor is not None:
            assert(len(xor) == 2)
            assert(xor[0].__num_vars == xor[1].__num_vars)
            if num is None: num = xor[0].__num_vars
            else: assert(num == xor[0].__num_vars)
        # biased constructor requires compatible length
        if biased is not None:
            if num is None: num = biased.__num_vars
            else: assert(num == biased.__num_vars)

        self.__num_vars = num
        self.vars = {}
        self.ones = set()
        self.activator = None

        if biased is not None:
            # add the condition that (new = old) or (new = 0)
            # when encoded normally: use p, q as tmp variables
            # (p -> (a = b)) | (q -> -a) | (p | q)
            # when p = q, then -b is encoded twice, so we replace q with -p
            # (p -> (a = b)) | (-p -> -a)

            if len(biased.vars) != 0:
                self.activator = solver.get_var()
                if biased.activator is not None:
                    solver.add_clause([-self.activator, biased.activator])

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
                cls = [[-a, b], [p, -a], [-p, a, -b]]
                for cl in cls:
                    cl.append(-self.activator)
                    solver.add_clause(cl)
        if xor is not None:
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
                    cls = make_xor_bool(x, y, z)
                    if self.activator is None:
                        self.activator = solver.get_var()
                    for cl in cls:
                        cl.append(-self.activator)
                        solver.add_clause(cl)
                    self.vars[i] = z
            pref = [-self.activator] if self.activator is not None else []
            if arg0.activator is not None:
                solver.add_clause(pref + [arg0.activator])
            if arg1.activator is not None:
                solver.add_clause(pref + [arg1.activator])
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

    def __lt__(self, other):
        return str(self.name) < str(other.name)

    def __str__(self):
        return "%s %s[%d]" % (inv_cell_enum[self.type], self.name, self.pos)
