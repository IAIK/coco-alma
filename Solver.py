from pysat.solvers import Cadical
from defines import LIST_XOR
import helpers


class Solver(Cadical):
    def __init__(self, store_clauses=False, store_comments=False):
        Cadical.__init__(self)
        self.__var = 1
        self.clock_act = None
        self.__dbg_clauses = []
        self.__dbg_comments = {}
        self.num_clauses = 0
        self.store_clauses = store_clauses
        self.store_comments = store_comments

    def __del__(self):
        self.delete()

    def dbg_print(self):
        if self.store_clauses:
            print("p cnf %d %d" % (self.__var - 1, self.num_clauses))
        for idx in range(self.num_clauses):
            if self.store_comments:
                if idx in self.__dbg_comments:
                    for line in self.__dbg_comments[idx]:
                        print("c " + line)
            if self.store_clauses:
                print(" ".join(map(str, self.__dbg_clauses[idx])) + " 0")
                
    def dbg_print_cnf(self, name, assumes, positive, dbg_output_dir_path):
        assert(self.store_clauses)

        for ip,p in enumerate(positive):
            assumes.append(p)

            f = open("%s/formula_%s_%d.cnf"%(dbg_output_dir_path, name, ip), "w")

            header = "p cnf %d %d\n"%(self.__var-1, self.num_clauses+len(assumes))
            f.write(header)

            for clause in self.__dbg_clauses:
                cnf_line = "%s 0\n" % (" ".join([str(c) for c in clause]))
                f.write(cnf_line)
            
            for assume in assumes:
                cnf_line = "%s 0\n" % (assume)
                f.write(cnf_line)
            f.close()
            assumes.pop()

    def get_vars_(self, num):
        r = self.__var
        self.__var += num
        return r

    def get_vars(self, num):
        r = self.get_vars_(num)
        return range(r, r + num)

    def get_var(self):
        return self.get_vars_(1)

    def add_comment(self, text):
        if self.num_clauses not in self.__dbg_comments.keys():
            self.__dbg_comments[self.num_clauses] = []
        self.__dbg_comments[self.num_clauses].append(text)

    # @profile
    def add_clause(self, clause, no_return=True):
        assert(max(map(abs, clause)) < self.__var)
        if self.store_clauses:
            self.__dbg_clauses.append(clause)
        self.num_clauses += 1
        cl = clause.copy()
        if self.clock_act is not None: cl.append(-self.clock_act)
        Cadical.add_clause(self, cl, no_return)

    def add_clauses(self, clauses):
        for clause in clauses:
            self.add_clause(clause)

    def xor_list(self, lst):
        assert(len(lst) >= 1)
        assert (LIST_XOR in ("shallow", "tree", "chain"))
        if LIST_XOR == "shallow":
            # xor shallow
            STEP = 5
            tmp_lst = lst
            clauses = []
            while len(tmp_lst) != 1:
                new_lst = []
                for start in range(0, len(tmp_lst), STEP):
                    tmp_lst_ = tmp_lst[start:start+STEP]
                    if len(tmp_lst_) == 1:
                        new_lst.append(tmp_lst_[0])
                        break
                    tmp_var = self.get_var()
                    clauses += make_xor_bool_exp(tmp_lst_, tmp_var)
                    new_lst.append(tmp_var)
                tmp_lst = new_lst
            self.add_clauses(clauses)
            return tmp_lst[0]
        elif LIST_XOR == "tree":
            # xor tree
            while len(lst) != 1:
                tmp_lst = [self.get_var() for _ in range(len(lst) // 2)]
                if len(lst) % 2 == 1:
                    tmp_lst.append(lst[-1])
                for i in range(0, len(lst)-1, 2):
                    self.add_clauses(make_xor_bool(lst[i], lst[i+1], tmp_lst[i // 2]))
                # random.shuffle(tmp_lst)
                lst = tmp_lst
            return lst[0]
        elif LIST_XOR == "chain":
            # xor chain
            a = lst[0]
            clauses = []
            for b in lst[1:]:
                c = self.get_var()
                clauses += make_xor_bool(a, b, c)
                a = c
            self.add_clauses(clauses)
            return a
        assert False

    # @profile
    def make_bitvec_op(self, in1, in2, op, out=None):
        assert (len(in1) == len(in2))
        if out is None:
            num = len(in1)
            out = self.get_vars(num)
        res = []
        for a, b, c in zip(in1, in2, out):
            res += op(a, b, c)
        # print(op, in1, in2, res)
        self.add_clauses(res)
        return out

    # implementation of https://link.springer.com/content/pdf/10.1007%2F11564751_73.pdf
    def at_most_k_of_n(self, k, xs):
        assert (k > 0)
        n = len(xs)
        if k >= n: return
        clauses = []
        # tmp = self.get_vars(k * n)
        ss = [self.get_vars(k) for i in range(n)]
        # ss = [list(range(tmp + i * k, tmp + (i + 1) * k)) for i in range(n)]
        clauses.append([-xs[0], ss[0][0]])
        for j in range(1, k):
            clauses.append([-ss[0][j]])
        for i in range(1, n - 1):
            clauses.append([-xs[i], ss[i][0]])
            clauses.append([-ss[i - 1][0], ss[i][0]])
            for j in range(1, k):
                clauses.append([-xs[i], -ss[i - 1][j - 1], ss[i][j]])
                clauses.append([-ss[i - 1][j], ss[i][j]])
            clauses.append([-xs[i], -ss[i - 1][k - 1]])
        clauses.append([-xs[n - 1], -ss[n - 2][k - 1]])
        self.add_comment("at most %d of %s" % (k, xs))
        self.add_clauses(clauses)
        # print("Choose k of n literals:", xs, "\n", clauses)


def make_xor_bool_exp(lst, res):
    assert(len(lst) >= 2)
    assert(res > 0)
    clauses = []
    for i in range(0, 2 ** len(lst)):
        c = [res if helpers.parity(i) else -res]
        for j in range(0, len(lst)):
            if (1 << j) & i:
                c.append(-lst[j])
            else:
                c.append(lst[j])
        clauses.append(c)
    return clauses

# static methods that don't need the solver class


def make_xor_bool(a, b, c):
    return [[-a, -b, -c], [+a, +b, -c], [+a, -b, +c], [-a, +b, +c]]


def make_xor_side(a, b, c):
    return [[-a, -b, -c], [+a, +b, -c]]


def make_impl_xor_bool(x, a, b, c):
    return [[-x, -a, -b, -c], [-x, +a, +b, -c], [-x, +a, -b, +c], [-x, -a, +b, +c]]


def make_equal_bool(a, b, c):
    return [[-a, -b, +c], [+a, +b, +c], [+a, -b, -c], [-a, +b, -c]]


def make_equal_side(a, b, c):
    return [[-a, +b, -c], [+a, -b, -c]]


# equivalence <=>
def make_and_bool(inputs, output):
    assert (len(inputs) != 0)
    clauses = [[-x for x in inputs] + [output]]
    clauses += [[x, -output] for x in inputs]
    return clauses


# implication <=
def make_and_bool_top(inputs, output):
    assert (len(inputs) != 0)
    return [[x, -output] for x in inputs]


def make_and_bool_(a, b, c):
    return make_and_bool([a, b], c)


def make_or_bool(inputs, output):
    assert (len(inputs) != 0)
    clauses = [inputs + [-output]]
    clauses += [[-x, output] for x in inputs]
    return clauses


def make_or_bool_(a, b, c):
    return make_or_bool([a, b], c)


def make_equal_bool_top(a, b):
    return [[+a, -b], [-a, +b]]
