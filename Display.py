#!/usr/bin/env python3
import sys, json, linecache, tkinter

def display_string(Str):
    root = tkinter.Tk()
    root.geometry("1280x720")
    frame = tkinter.Frame(root)
    frame.pack(side=tkinter.BOTTOM,fill=tkinter.X)
    SV = tkinter.Scrollbar(root)
    SV.pack(side = tkinter.RIGHT, fill = tkinter.Y)
    SH = tkinter.Scrollbar(frame, orient=tkinter.HORIZONTAL)
    T = tkinter.Text(root, height=1000, width=1000, relief='flat',yscrollcommand=SV.set,xscrollcommand=SH.set,wrap=tkinter.NONE)
    T.insert(tkinter.END, Str)
    T.config(state=tkinter.DISABLED)
    T.pack(side=tkinter.TOP,fill=tkinter.BOTH)
    SV.config(command=T.yview)
    SH.pack(side=tkinter.TOP, fill = tkinter.X)
    SH.config(command=T.xview)

    def copy():
        root.clipboard_clear()
        root.clipboard_append(Str)
        B["text"] = "Copied!"
        root.update()

    frame2 = tkinter.Frame(frame)
    frame2.pack(side=tkinter.BOTTOM)
    B = tkinter.Button(frame2,text="Copy to clipboard", command=copy)
    B.pack(side = tkinter.LEFT)
    warning = tkinter.Label(frame2,text="Please paste before closing window. Esc to exit.")
    warning.pack(side = tkinter.RIGHT)
    root.bind("<Escape>",lambda e: root.destroy())
    tkinter.mainloop()

def print_single_chain(graph_info, config_info):
    graph = graph_info["graph"]
    global_name_dict = graph_info["name"]
    used_curves = config_info["used"]

    name_dict = [global_name_dict[used_curves[i]] for i in range(len(used_curves))]

    exceptionals = graph_info["blps"]
    contracted_ex = config_info["blds"]
    chain = config_info["chain"]
    self_int = config_info["selfint"]
    discrepancies = config_info["disc"]
    n = config_info["N"]

    used_string = "      "
    for name in name_dict:
        used_string += "  {0}".format(name)
        
    blowups_string = ""
    for ex in exceptionals:
        if ex not in contracted_ex:
            blowups_string += "        "
            intersect = []
            
            maybe_missing = False
            
            for curve in graph[ex]:
                if curve in used_curves:
                    intersect.append(curve)
                if curve in contracted_ex:
                    maybe_missing = True
            
            blowups_string += " - ".join([global_name_dict[curve] for curve in intersect])
            blowups_string += " =: {0}".format(global_name_dict[ex])
            if maybe_missing:
                blowups_string += "(Maybe missing some intersections)"
            blowups_string += "\n"

    for connection in config_info["blps"]:
        blowups_string += "        {0} - {1}\n".format(
            name_dict[connection[0]],name_dict[connection[1]])

    extra_blowups_string = ""
    if config_info["en"] != 0:
        extra_blowups_string = "    Needed {0} extra blowups at {1} - {2}\n".format(
            config_info["en"],
            name_dict[config_info["ea"]],
            name_dict[config_info["eb"]])
            
    chain_list = [
        name_dict[curve] if curve < len(used_curves)
        else "*A_{0}".format(curve - len(used_curves) + 1)
        for curve in chain
    ]

    self_int_list = [
        str(-self_int[curve])
        for curve in chain
    ]

    discrepancy_list = [
        "{0} / {1}".format(discrepancies[curve],n)
        for curve in chain
    ]

    max_length = [
        max(
            len(chain_list[i]),
            len(self_int_list[i]),
            len(discrepancy_list[i])
        )
        for i in range(len(chain))
    ]

    chain_string = "        C=  " + " - ".join(["{{:^{0}}}".format(max_length[i]) for i in range(len(chain))]).format(*chain_list) + "\n"

    self_int_string = "        S=  " + " , ".join(["{{:^{0}}}".format(max_length[i]) for i in range(len(chain))]).format(*self_int_list) + "\n"

    discrepancy_string = "        d=[ " + " , ".join(["{{:^{0}}}".format(max_length[i]) for i in range(len(chain))]).format(*discrepancy_list) + " ]\n"

    a = min(-discrepancies[chain[0]],-discrepancies[chain[-1]])


    #String to print the info.
    S = (
    "Example with K^2 = {K2}, (n,a) = ({n},{a}), length = {length}\n"
    "    Used curves:\n"
    "{used_curves}\n"
    "\n"
    "    Blowups:\n"
    "{blowups}"
    "{extra_blowups}"
    "\n"
    "    Resulting chain with self intersections and discrepancies:\n"
    "{chain_names}"
    "{self_int}"
    "{discrepancies}"
    )

    display_string(S.format(
        K2 = config_info["K2"],
        n = n,
        a = a,
        length = len(chain),
        used_curves = used_string,
        blowups = blowups_string,
        extra_blowups = extra_blowups_string,
        chain_names = chain_string,
        self_int = self_int_string,
        discrepancies = discrepancy_string
    ))

def print_double_chain(graph_info, config_info):
    graph = graph_info["graph"]
    global_name_dict = graph_info["name"]
    used_curves = config_info["used"]

    name_dict = [global_name_dict[used_curves[i]] for i in range(len(used_curves))]

    exceptionals = graph_info["blps"]
    contracted_ex = config_info["blds"]
    chain0 = config_info["chain0"]
    chain1 = config_info["chain1"]
    self_int = config_info["selfint"]
    discrepancies = config_info["disc"]
    n0 = config_info["N0"]
    n1 = config_info["N1"]

    used_string = "      "
    for name in name_dict:
        used_string += "  {0}".format(name)
        
    blowups_string = ""
    for ex in exceptionals:
        if ex not in contracted_ex:
            blowups_string += "        "
            intersect = []
            
            maybe_missing = False
            
            for curve in graph[ex]:
                if curve in used_curves:
                    intersect.append(curve)
                if curve in contracted_ex:
                    maybe_missing = True
            
            blowups_string += " - ".join([global_name_dict[curve] for curve in intersect])
            blowups_string += " =: {0}".format(global_name_dict[ex])
            if maybe_missing:
                blowups_string += "(Maybe missing some intersections)"
            blowups_string += "\n"

    for connection in config_info["blps"]:
        blowups_string += "        {0} - {1}\n".format(
            name_dict[connection[0]],name_dict[connection[1]])

    extra_blowups_string = ""
    if config_info["en0"] != 0:
        extra_blowups_string = "    Needed {0} extra blowups at {1} - {2}\n".format(
            config_info["en0"],
            name_dict[config_info["ea0"]],
            name_dict[config_info["eb0"]])
    if config_info["en1"] != 0:
        extra_blowups_string += "    Needed {0} extra blowups at {1} - {2}\n".format(
            config_info["en1"],
            name_dict[config_info["ea1"]],
            name_dict[config_info["eb1"]])
            
    chain_list0 = [
        name_dict[curve] if curve < len(used_curves)
        else "*A_{0}".format(curve - len(used_curves) + 1)
        for curve in chain0
    ]       
    chain_list1 = [
        name_dict[curve] if curve < len(used_curves)
        else "*A_{0}".format(curve - len(used_curves) + 1)
        for curve in chain1
    ]

    self_int_list0 = [
        str(-self_int[curve])
        for curve in chain0
    ]
    self_int_list1 = [
        str(-self_int[curve])
        for curve in chain1
    ]

    discrepancy_list0 = [
        "{0} / {1}".format(discrepancies[curve],n0)
        for curve in chain0
    ]
    discrepancy_list1 = [
        "{0} / {1}".format(discrepancies[curve],n1)
        for curve in chain1
    ]

    max_length0 = [
        max(
            len(chain_list0[i]),
            len(self_int_list0[i]),
            len(discrepancy_list0[i])
        )
        for i in range(len(chain0))
    ]
    max_length1 = [
        max(
            len(chain_list1[i]),
            len(self_int_list1[i]),
            len(discrepancy_list1[i])
        )
        for i in range(len(chain1))
    ]

    chain_string0 = "        C=  " + " - ".join(["{{:^{0}}}".format(max_length0[i]) for i in range(len(chain0))]).format(*chain_list0) + "\n"
    chain_string1 = "        C=  " + " - ".join(["{{:^{0}}}".format(max_length1[i]) for i in range(len(chain1))]).format(*chain_list1) + "\n"

    self_int_string0 = "        S=  " + " , ".join(["{{:^{0}}}".format(max_length0[i]) for i in range(len(chain0))]).format(*self_int_list0) + "\n"
    self_int_string1 = "        S=  " + " , ".join(["{{:^{0}}}".format(max_length1[i]) for i in range(len(chain1))]).format(*self_int_list1) + "\n"

    discrepancy_string0 = "        d=[ " + " , ".join(["{{:^{0}}}".format(max_length0[i]) for i in range(len(chain0))]).format(*discrepancy_list0) + " ]\n"
    discrepancy_string1 = "        d=[ " + " , ".join(["{{:^{0}}}".format(max_length1[i]) for i in range(len(chain1))]).format(*discrepancy_list1) + " ]\n"

    a0 = min(-discrepancies[chain0[0]],-discrepancies[chain0[-1]])
    a1 = min(-discrepancies[chain1[0]],-discrepancies[chain1[-1]])


    #String to print the info.
    S = (
    "Example with two chains K^2 = {K2}:\n"
    "    (n1,a1) = ({n0},{a0}), length = {length0}\n"
    "    (n2,a2) = ({n1},{a1}), length = {length1}\n"
    "\n"
    "    Used curves:\n"
    "{used_curves}\n"
    "\n"
    "    Blowups:\n"
    "{blowups}"
    "{extra_blowups}"
    "\n"
    "    First chain with self intersections and discrepancies:\n"
    "{chain_names0}"
    "{self_int0}"
    "{discrepancies0}"
    "\n"
    "    Second chain with self intersections and discrepancies:\n"
    "{chain_names1}"
    "{self_int1}"
    "{discrepancies1}"
    )

    display_string(S.format(
        K2 = config_info["K2"],
        n0 = n0,
        a0 = a0,
        length0 = len(chain0),
        n1 = n1,
        a1 = a1,
        length1 = len(chain1),
        used_curves = used_string,
        blowups = blowups_string,
        extra_blowups = extra_blowups_string,
        chain_names0 = chain_string0,
        self_int0 = self_int_string0,
        discrepancies0 = discrepancy_string0,
        chain_names1 = chain_string1,
        self_int1 = self_int_string1,
        discrepancies1 = discrepancy_string1
    ))

def print_p_extremal(graph_info, config_info):
    graph = graph_info["graph"]
    global_name_dict = graph_info["name"]
    used_curves = config_info["used"]

    name_dict = [global_name_dict[used_curves[i]] for i in range(len(used_curves))]

    exceptionals = graph_info["blps"]
    contracted_ex = config_info["blds"]
    chain0 = config_info["chain0"]
    chain1 = config_info["chain1"]
    self_int = config_info["selfint"]
    discrepancies = config_info["disc"]
    n0 = config_info["N0"]
    n1 = config_info["N1"]
    Delta = config_info["Delta"]
    Omega = config_info["Omega"]

    original_chain = config_info["chain_orig"]
    original_self_int = config_info["selfint_orig"]

    used_string = "      "
    for name in name_dict:
        used_string += "  {0}".format(name)
        
    blowups_string = ""
    for ex in exceptionals:
        if ex not in contracted_ex:
            blowups_string += "        "
            intersect = []
            
            maybe_missing = False
            
            for curve in graph[ex]:
                if curve in used_curves:
                    intersect.append(curve)
                if curve in contracted_ex:
                    maybe_missing = True
            
            blowups_string += " - ".join([global_name_dict[curve] for curve in intersect])
            blowups_string += " =: {0}".format(global_name_dict[ex])
            if maybe_missing:
                blowups_string += "(Maybe missing some intersections)"
            blowups_string += "\n"

    for connection in config_info["blps"]:
        blowups_string += "        {0} - {1}\n".format(
            name_dict[connection[0]],name_dict[connection[1]])

    extra_blowups_string = ""
    if config_info["en1"] != 0:
        extra_blowups_string = "    Needed {0} extra blowups at {1} - {2}\n".format(
            config_info["en1"],
            name_dict[config_info["ea1"]],
            name_dict[config_info["eb1"]])
    
    original_chain_list = [
        name_dict[curve] if curve < len(used_curves)
        else "*A_{0}".format(curve - len(used_curves) + 1)
        for curve in original_chain
    ]
    chain_list0 = [
        name_dict[curve] if curve < len(used_curves)
        else "*A_{0}".format(curve - len(used_curves) + 1)
        for curve in chain0
    ]
    chain_list1 = [
        name_dict[curve] if curve < len(used_curves)
        else "*A_{0}".format(curve - len(used_curves) + 1)
        for curve in chain1
    ]

    original_self_int_list = [
        str(-original_self_int[curve])
        for curve in original_chain
    ]
    self_int_list0 = [
        str(-self_int[curve])
        for curve in chain0
    ]
    self_int_list1 = [
        str(-self_int[curve])
        for curve in chain1
    ]

    discrepancy_list0 = [
        "{0} / {1}".format(discrepancies[curve],n0)
        for curve in chain0
    ]
    discrepancy_list1 = [
        "{0} / {1}".format(discrepancies[curve],n1)
        for curve in chain1
    ]

    original_max_length = [
        max(
            len(original_chain_list[i]),
            len(original_self_int_list[i])
        )
        for i in range(len(original_chain))
    ]
    max_length0 = [
        max(
            len(chain_list0[i]),
            len(self_int_list0[i]),
            len(discrepancy_list0[i])
        )
        for i in range(len(chain0))
    ]
    max_length1 = [
        max(
            len(chain_list1[i]),
            len(self_int_list1[i]),
            len(discrepancy_list1[i])
        )
        for i in range(len(chain1))
    ]

    original_chain_string = "        C=  " + " - ".join(["{{:^{0}}}".format(original_max_length[i]) for i in range(len(original_chain))]).format(*original_chain_list) + "\n"
    chain_string0 = "        C=  " + " - ".join(["{{:^{0}}}".format(max_length0[i]) for i in range(len(chain0))]).format(*chain_list0) + "\n"
    chain_string1 = "        C=  " + " - ".join(["{{:^{0}}}".format(max_length1[i]) for i in range(len(chain1))]).format(*chain_list1) + "\n"

    original_self_int_string = "        S=  " + " , ".join(["{{:^{0}}}".format(original_max_length[i]) for i in range(len(original_chain))]).format(*original_self_int_list) + "\n"
    self_int_string0 = "        S=  " + " , ".join(["{{:^{0}}}".format(max_length0[i]) for i in range(len(chain0))]).format(*self_int_list0) + "\n"
    self_int_string1 = "        S=  " + " , ".join(["{{:^{0}}}".format(max_length1[i]) for i in range(len(chain1))]).format(*self_int_list1) + "\n"

    discrepancy_string0 = "        d=[ " + " , ".join(["{{:^{0}}}".format(max_length0[i]) for i in range(len(chain0))]).format(*discrepancy_list0) + " ]\n"
    discrepancy_string1 = "        d=[ " + " , ".join(["{{:^{0}}}".format(max_length1[i]) for i in range(len(chain1))]).format(*discrepancy_list1) + " ]\n"

    a0 = min(-discrepancies[chain0[0]],-discrepancies[chain0[-1]])
    a1 = min(-discrepancies[chain1[0]],-discrepancies[chain1[-1]])


    #String to print the info.
    S = (
    "Example by P-extremal resolution K^2 = {K2}, Delta = {Delta}, Omega = {Omega}\n"
    "    (n1,a1) = ({n0},{a0}), length = {length0}\n"
    "    (n2,a2) = ({n1},{a1}), length = {length1}\n"
    "\n"
    "    Used curves:\n"
    "{used_curves}\n"
    "\n"
    "    Blowups:\n"
    "{blowups}"
    "{extra_blowups}"
    "\n"
    "    Original chain with self intersections:\n"
    "{original_names}"
    "{original_self_int}"
    "\n"
    "    First chain with self intersections and discrepancies:\n"
    "{chain_names0}"
    "{self_int0}"
    "{discrepancies0}"
    "\n"
    "    Second chain with self intersections and discrepancies:\n"
    "{chain_names1}"
    "{self_int1}"
    "{discrepancies1}"
    )

    display_string(S.format(
        K2 = config_info["K2"],
        Delta = Delta,
        Omega = Omega,
        n0 = n0,
        a0 = a0,
        length0 = len(chain0),
        n1 = n1,
        a1 = a1,
        length1 = len(chain1),
        used_curves = used_string,
        blowups = blowups_string,
        extra_blowups = extra_blowups_string,
        original_names = original_chain_string,
        original_self_int = original_self_int_string,
        chain_names0 = chain_string0,
        self_int0 = self_int_string0,
        discrepancies0 = discrepancy_string0,
        chain_names1 = chain_string1,
        self_int1 = self_int_string1,
        discrepancies1 = discrepancy_string1
    ))

if __name__ == "__main__":
    if len(sys.argv) <= 2:
        print("Usage: {0} <jsonl filname> index".format(sys.argv[0]),file=sys.stderr)
    
    else:
        filename = sys.argv[1]

        try:
            index = int(sys.argv[2])
        except:
            print("Error: Invalid index.")
            exit(0)

        if index == 0:
            print("Unimplemented.")
            exit(0)
        
        s1 = linecache.getline(filename,1)
        if s1 == '':
            print("Error while opening file.")
            exit(0)
        
        s2 = linecache.getline(filename,index+1)
        if s2 == '':
            print("Error: Invalid index (Out of bounds).")
            exit(0)

        # try:
        if True:
            graph_info = json.loads(s1)
            config_info = json.loads(s2)

            amount = config_info["#"]
            if amount == 1:
                print_single_chain(graph_info, config_info)
            
            else:
                p_extremal = config_info["WH"]
                if p_extremal == 0:
                    print_double_chain(graph_info, config_info)
                else:
                    print_p_extremal(graph_info, config_info)

        # except:
        #     print("Error: corrupted/incompatible jsonl file.")
        #     exit(0)