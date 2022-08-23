def get_initial_label(defmap, id):
    definition = defmap[id]
    if type(definition) == tuple:
        op1 = definition[0]
        op2 = definition[1]
        op3 = definition[2]
        op1 = get_initial_label(defmap, op1)
        op3 = get_initial_label(defmap, op3)
        return "(%s %s %s)"%(op1, op2, op3)
        #and, xor
    elif type(definition) == str:
        #init label reached
        return definition
    elif type(definition) == int:
        return get_initial_label(defmap, definition)
    else:
        assert(False)


class DbgLabels:
    def __init__(self, dump_path):
        self.data = {}
        self.dump_path = dump_path

    def dbg_print_labels(self, node_vars, defmap, cells, cycle, exc=False):
        #if (self.dbg_exact == 0 or self.dbg_exact==3) and not(exc):
        #    return
        
        newdefmap = {}
        self.data[cycle] = {}
   
        for (id, d) in defmap.items():
            newdefmap[id] = get_initial_label(defmap,id)

        for var in node_vars:
            c = cells[var]
            #if self.dbg_exact == 2:
            #    if c.type in REGISTER_TYPES or c.type == PORT_TYPE:
            #        print("%d;%s;%s"%(var, c.short__str(), newdefmap[node_vars[var]]))
            #elif self.dbg_exact == 1:
            print("%s;%s"%(c.short__str(), newdefmap[node_vars[var]]))
            self.data[cycle][c.short__str()] = newdefmap[node_vars[var]]
    
    

    def __del__(self):
        return
        import pickle

        #if self.dbg_exact == 0:
        #    return
        f = open(self.dump_path, "wb")
        print(self.data.keys())
        pickle.dump(self.data, f)
        f.close()