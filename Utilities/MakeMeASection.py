#!/usr/bin/env python3
import sys, json, linecache, tkinter, os
from math import gcd

MAX_CHAIN_LENGTH_SINGLE = 7
MAX_CHAIN_LENGTH_DOUBLE = 7

MAX_USED_LENGTH = 4

EXTRA_CURVE_NAME = "E^*_{{{0}}}"

DRAWING_SCALE = 0.45

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

def det(M):
    M = [row[:] for row in M] # make a copy to keep original M unmodified
    N, sign, prev = len(M), 1, 1
    for i in range(N-1):
        if M[i][i] == 0: # swap with another row having nonzero i's elem
            swapto = next( (j for j in range(i+1,N) if M[j][i] != 0), None )
            if swapto is None:
                return 0 # all M[*][i] are zero => zero determinant
            M[i], M[swapto], sign = M[swapto], M[i], -sign
        for j in range(i+1,N):
            for k in range(i+1,N):
                # assert ( M[j][k] * M[i][i] - M[j][i] * M[i][k] ) % prev == 0
                M[j][k] = ( M[j][k] * M[i][i] - M[j][i] * M[i][k] ) // prev
        prev = M[i][i]
    return sign * M[-1][-1]

def get_base_used_and_intersection_matrix(graph_info, config_info):

    real_graph = graph_info["graph"]
    real_exceptionals = graph_info["blps"]
    real_selfint = graph_info["selfint"]

    selfint = list(real_selfint) #deep copy
    exceptionals = list(real_exceptionals) #deep copy
    Intersection_Matrix = [len(real_graph)*[0] for i in range(len(real_graph))]
    count_graph = [len(real_graph)*[0] for i in range(len(real_graph))]
    for i in range(len(real_graph)):
        for x in real_graph[i]:
            count_graph[i][x] += 1
    for i in exceptionals[::-1]:
        for a in range(len(real_graph)):
            selfint[a] += count_graph[i][a]*count_graph[i][a]
            for b in range(a+1,len(real_graph)):
                count_graph[a][b] += count_graph[i][a]*count_graph[i][b]
                count_graph[b][a] += count_graph[i][a]*count_graph[i][b]
            count_graph[a][i] = 0
            count_graph[i][a] = 0
        selfint[i] = 0
    for a in range(len(real_graph)):
        for b in range(len(real_graph)):
            if a == b:
                Intersection_Matrix[a][b] = selfint[a]
            else:
                Intersection_Matrix[a][b] = count_graph[a][b]

    used = config_info["used"]
    base_used = []
    for i in used:
        if i not in exceptionals:
            base_used.append(i)
    base_used = sorted(base_used)
    a = 0
    used_matrix = [len(base_used)*[0] for i in range(len(base_used))]
    for x in range(len(Intersection_Matrix)):
        if x not in base_used:
            continue
        b = 0
        for y in range(len(Intersection_Matrix[x])):
            if y not in base_used:
                continue
            used_matrix[a][b] = Intersection_Matrix[x][y]
            b += 1
        a += 1
    return base_used, used_matrix

def single_chain(graph_info, config_info):
    base_used, matrix = get_base_used_and_intersection_matrix(graph_info, config_info)
    global_name_dict = graph_info["name"]
    discrepancies = config_info["disc"]
    chain = config_info["chain"]
    used_curves = config_info["used"]
    self_int = config_info["selfint"]

    name_dict = [global_name_dict[used_curves[i]] for i in range(len(used_curves))]

    n = config_info["N"]
    a = min(-discrepancies[chain[0]],-discrepancies[chain[-1]])

    K2 = config_info["K2"]

    S = f"\\subsection{{Example with \\(K^2={K2}\\)}}\n"

    S += "This example uses the following curves:\n"
    S += "\\[" + ",~".join(name_dict) +".\\]\n"

    S += f"The determinant of their intersection matrix is ${det(matrix)}$. Requires blow ups at the following points:\n"

    S += "\\[" + ",~".join((f"{name_dict[c[0]]} \\cap {name_dict[c[1]]}" for c in config_info["blps"])) + ".\\]\n"

    if config_info["en"] != 0:
        S += "It also needs extra blow ups at:\n"
        S += "\\[{0} \\times ({1} \\cap {2})".format(
                config_info["en"],
                name_dict[config_info["ea"]],
                name_dict[config_info["eb"]]
            )

    S += f"The resulting chain is a $(n,a) = ({n},{a})$.\n"

    return S


def double_chain(graph_info, config_info):
    base_used, matrix = get_base_used_and_intersection_matrix(graph_info, config_info)
    global_name_dict = graph_info["name"]
    discrepancies = config_info["disc"]
    chain0 = config_info["chain0"]
    chain1 = config_info["chain1"]
    used_curves = config_info["used"]
    self_int = config_info["selfint"]

    name_dict = [global_name_dict[used_curves[i]] for i in range(len(used_curves))]

    n0 = config_info["N0"]
    n1 = config_info["N1"]
    a0 = min(-discrepancies[chain0[0]],-discrepancies[chain0[-1]])
    a1 = min(-discrepancies[chain1[0]],-discrepancies[chain1[-1]])

    K2 = config_info["K2"]

    S = f"\\subsection{{Example with \\(K^2={K2}\\)}}\n"

    S += "This example uses the following curves:\n"
    S += "\\[" + ",~".join(name_dict) +".\\]\n"

    S += f"The determinant of their intersection matrix is ${det(matrix)}$. Requires blow ups at the following points:\n"

    S += "\\[" + ",~".join((f"{name_dict[c[0]]} \\cap {name_dict[c[1]]}" for c in config_info["blps"])) + ".\\]\n"

    if config_info["en0"] != 0 or config_info["en1"] != 0:
        S += "It also needs extra blow ups at:\n"
        S += "\\[" + ",\\quad ".join(("{0} \\times ({1} \\cap {2})".format(
                config_info[f"en{c}"],
                name_dict[config_info[f"ea{c}"]],
                name_dict[config_info[f"eb{c}"]]
            ) for c in range(2) if config_info[f"en{c}"] != 0)
        ) + ".\\]\n"

    S += f"The resulting chains are  $(n_1,a_1) = ({n0},{a0})$ and $(n_2,a_2) = ({n1},{a1})$. This example satisfies $\\gcd(n_1,n_2) = {gcd(n0,n1)}$.\n"

    return S

if __name__ == "__main__":
    filename = None
    if len(sys.argv) >= 2:
        filename = sys.argv[1]

    body = ""

    while True:
        if filename == None:
            filename = input("Give me a jsonl file: ")
        s1 = linecache.getline(os.path.expanduser(filename),1)
        if s1 == '':
            print("Error while opening file.")
            filename = None
            continue

        try:
            graph_info = json.loads(s1)
        except:
            print("Error: corrupted/incompatible jsonl file.")
            filename = None
            continue
        break

    while True:
        value = input("Give me an index of {fn} (0 to quit, c to change file): ".format(fn=filename))
        if value == 'c':
            while True:
                filename = input("Give me a jsonl file: ")
                s1 = linecache.getline(os.path.expanduser(filename),1)
                if s1 == '':
                    print("Error while opening file.")
                    continue

                try:
                    graph_info = json.loads(s1)
                except:
                    print("Error: corrupted/incompatible jsonl file.")
                    continue
                break
            continue

        if value == '0':
            break

        try:
            value = int(value)
        except:
            print("Error: Invalid index.")
            continue

        s2 = linecache.getline(os.path.expanduser(filename),value+1)
        if s2 == '':
            print("Error: Invalid index (Out of bounds).")
            continue
        try:
            config_info = json.loads(s2)
        except:
            print("Error: corrupted/incompatible jsonl file.")
            continue

        amount = config_info["#"]
        if amount == 1:
            if "type" in config_info:
                print("QHD unimplemented")
                continue
            else:
                body += single_chain(graph_info, config_info)
                body += f"\nExample taken from \\verb|{filename} : {value}|\n\n"

        else:
            p_extremal = config_info["WH"]
            if p_extremal == 0:
                if "type" in config_info:
                    print("QHD unimplemented")
                else:
                    body += double_chain(graph_info, config_info)
                    body += f"\nExample taken from \\verb|{filename} : {value}|\n\n"
            else:
                body += double_chain(graph_info, config_info)
                body += f"\nExample taken from \\verb|{filename} : {value}|\n\n"

    display_string(body)
