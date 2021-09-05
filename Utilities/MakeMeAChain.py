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

def node_string(x,y,name,SI):
    return (
    "\\draw ({{{x}}},{{{y}}}) node [shape=circle, fill=black, inner sep=0pt, minimum width=4pt, scale={scale}] {{}};\n"
    "\\draw ({{{x}}},{{{y_up}}}) node [label={{[centered, scale={scale}]${name}$}}, scale={scale}] {{}};\n"
    "\\draw ({{{x}}},{{{y_dn}}}) node [label={{[centered, scale={scale}]${SI}$}}, scale={scale}] {{}};\n"
    ).format(x=x, y=y, y_up = y + 0.33, y_dn = y - 0.6, name=name, SI=SI, scale=DRAWING_SCALE)

def single_chain(graph_info, config_info, use_gcd, use_det):
    base_used, matrix = get_base_used_and_intersection_matrix(graph_info, config_info)
    global_name_dict = graph_info["name"]
    discrepancies = config_info["disc"]
    chain = config_info["chain"]
    used_curves = config_info["used"]
    self_int = config_info["selfint"]

    name_dict = [global_name_dict[used_curves[i]] for i in range(len(used_curves))]

    n = config_info["N"]
    a = min(-discrepancies[chain[0]],-discrepancies[chain[-1]])

    na = "\\begin{{tabular}}{{c}}$({n},{a})$\\end{{tabular}}".format(n=n, a=a)
    length = "\\begin{{tabular}}{{c}}${0}$\\end{{tabular}}".format(len(chain))
    sep_used = (used_curves[i:i+MAX_USED_LENGTH] for i in range(0,len(used_curves), MAX_USED_LENGTH))
    used = "\\begin{{footnotesize}}\\begin{{tabular}}{{c}}{u}\\end{{tabular}}\\end{{footnotesize}}".format(
    u=(
        "\\\\".join(("$" + ", ".join((global_name_dict[j] for j in X)) + "$" for X in sep_used))
    ))
    det_txt = " &\n\\begin{{tabular}}{{c}}${d}$\\end{{tabular}}".format(d=det(matrix)) if use_det else ""
    opt_gcd = " &\n\\begin{tabular}{c}---\\end{tabular}" if use_gcd else ""

    chain_list = [
        name_dict[curve] if curve < len(used_curves)
        else EXTRA_CURVE_NAME.format(curve - len(used_curves) + 1)
        for curve in chain
    ]
    self_int_list = [
        str(-self_int[curve])
        for curve in chain
    ]
    sep_chain_list = [chain_list[i:i+MAX_CHAIN_LENGTH_SINGLE] for i in range(0,len(chain_list), MAX_CHAIN_LENGTH_SINGLE)]
    sep_self_int_list = [self_int_list[i:i+MAX_CHAIN_LENGTH_SINGLE] for i in range(0,len(self_int_list), MAX_CHAIN_LENGTH_SINGLE)]

    tikz = (
        "\\begin{{tikzpicture}}[scale={scale}, baseline={{(bx.base)}}]\n"
        "{body}"
        "\\node[fit=(current bounding box),inner ysep=1mm,inner xsep=0](bx){{}};\n"
        "\\end{{tikzpicture}}"
    )
    body = ""
    for i in range(len(sep_chain_list)):
        if i != 0:
            body += "\\draw ({left},{y}) -- ({right},{y});\n".format(
                left = -0.8,
                right = -0.2,
                y = -2*i
            )
        for j in range(len(sep_chain_list[i])):
            if j != 0:
                body += "\\draw ({left},{y}) -- ({right},{y});\n".format(
                left = j - 0.8,
                right = j - 0.2,
                y = -2*i
            )
            body += node_string(j,-2*i,sep_chain_list[i][j],sep_self_int_list[i][j])
    text = (
    "{na} &\n{length} &\n{used}{det}{opt_gcd} &\n"
    "{body}"
    )
    return text.format(
        na= na,
        length= length,
        used= used,
        det= det_txt,
        opt_gcd= opt_gcd,
        body= tikz.format(body= body, scale= DRAWING_SCALE)
    )


def double_chain(graph_info, config_info, use_gcd, used_det):
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

    na = "\\begin{{tabular}}{{c}}$({n0},{a0})$\\\\$({n1},{a1})$\\end{{tabular}}".format(n0=n0, a0=a0, n1=n1, a1=a1)
    length = "\\begin{{tabular}}{{c}}${l0}$\\\\${l1}$\end{{tabular}}".format(l0=len(chain0), l1=len(chain1))
    sep_used = (used_curves[i:i+MAX_USED_LENGTH] for i in range(0,len(used_curves), MAX_USED_LENGTH))
    used = "\\begin{{footnotesize}}\\begin{{tabular}}{{c}}{u}\\end{{tabular}}\\end{{footnotesize}}".format(
    u=(
        "\\\\".join(("$" + ", ".join((global_name_dict[j] for j in X)) + "$" for X in sep_used))
    ))
    det_txt = " &\n\\begin{{tabular}}{{c}}${d}$\\end{{tabular}}".format(d=det(matrix)) if use_det else ""
    opt_gcd = " &\n\\begin{{tabular}}{{c}}${g}$\\end{{tabular}}".format(g=gcd(n0,n1)) if use_gcd else ""

    chain_list0 = [
        name_dict[curve] if curve < len(used_curves)
        else EXTRA_CURVE_NAME.format(curve - len(used_curves) + 1)
        for curve in chain0
    ]
    self_int_list0 = [
        str(-self_int[curve])
        for curve in chain0
    ]
    chain_list1 = [
        name_dict[curve] if curve < len(used_curves)
        else EXTRA_CURVE_NAME.format(curve - len(used_curves) + 1)
        for curve in chain1
    ]
    self_int_list1 = [
        str(-self_int[curve])
        for curve in chain1
    ]
    sep_chain_list0 = [chain_list0[i:i+MAX_CHAIN_LENGTH_SINGLE] for i in range(0,len(chain_list0), MAX_CHAIN_LENGTH_SINGLE)]
    sep_self_int_list0 = [self_int_list0[i:i+MAX_CHAIN_LENGTH_SINGLE] for i in range(0,len(self_int_list0), MAX_CHAIN_LENGTH_SINGLE)]
    sep_chain_list1 = [chain_list1[i:i+MAX_CHAIN_LENGTH_SINGLE] for i in range(0,len(chain_list1), MAX_CHAIN_LENGTH_SINGLE)]
    sep_self_int_list1 = [self_int_list1[i:i+MAX_CHAIN_LENGTH_SINGLE] for i in range(0,len(self_int_list1), MAX_CHAIN_LENGTH_SINGLE)]

    tikz = (
        "\\begin{{tikzpicture}}[scale={scale}, baseline={{(bx.base)}}]\n"
        "{body}"
        "\\node[fit=(current bounding box),inner ysep=1mm,inner xsep=0](bx){{}};\n"
        "\\end{{tikzpicture}}"
    )
    body = ""
    for i in range(len(sep_chain_list0)):
        if i != 0:
            body += "\\draw ({left},{y}) -- ({right},{y});\n".format(
                left = -0.8,
                right = -0.2,
                y = -2*i
            )
        for j in range(len(sep_chain_list0[i])):
            if j != 0:
                body += "\\draw ({left},{y}) -- ({right},{y});\n".format(
                left = j - 0.8,
                right = j - 0.2,
                y = -2*i
            )
            body += node_string(j,-2*i,sep_chain_list0[i][j],sep_self_int_list0[i][j])
        if i != len(sep_chain_list0) - 1:
            body += "\\draw ({left},{y}) -- ({right},{y});\n".format(
                left = MAX_CHAIN_LENGTH_DOUBLE - 0.8,
                right = MAX_CHAIN_LENGTH_DOUBLE - 0.2,
                y = -2*i
            )
    for i in range(len(sep_chain_list1)):
        if i != 0:
            body += "\\draw ({left},{y}) -- ({right},{y});\n".format(
                left = -0.8,
                right = -0.2,
                y = -2*(i + len(sep_chain_list0))
            )
        for j in range(len(sep_chain_list1[i])):
            if j != 0:
                body += "\\draw ({left},{y}) -- ({right},{y});\n".format(
                left = j - 0.8,
                right = j - 0.2,
                y = -2*(i + len(sep_chain_list0))
            )
            body += node_string(j,-2*(i + len(sep_chain_list0)),sep_chain_list1[i][j],sep_self_int_list1[i][j])
        if i != len(sep_chain_list1) - 1:
            body += "\\draw ({left},{y}) -- ({right},{y});\n".format(
                left = MAX_CHAIN_LENGTH_DOUBLE - 0.8,
                right = MAX_CHAIN_LENGTH_DOUBLE - 0.2,
                y = -2*(i + len(sep_chain_list0))
            )
    text = (
    "{na} &\n{length} &\n{used}{det}{opt_gcd} &\n"
    "{body}"
    )
    return text.format(
        na= na,
        length= length,
        used= used,
        det= det_txt,
        opt_gcd= opt_gcd,
        body= tikz.format(body= body, scale= DRAWING_SCALE)
    )

if __name__ == "__main__":
    filename = None
    if len(sys.argv) >= 2:
        filename = sys.argv[1]

    S = (
    "%\\usepackage{{tikz}}\n"
    "%\\usetikzlibrary{{fit}}\n"
    "\\begin{{table}}[]\n"
    "\\begin{{tabular}}{{|c|c|c|c|{opt_cols}}}\n"
    "\hline\n"
    "$(n,a)$ & Length & Used curves{opt_det_text}{opt_gcd_text} & Chain"
    "{body}"
    "\\end{{tabular}}\n"
    "\caption{{{caption}}}\n"
    "\\end{{table}}\n"
    )
    body = ""
    use_gcd = input("Put gcd? (y/n): ")
    use_det = input("Put determinant? (y/n): ")
    use_gcd = (use_gcd=="y")
    use_det = (use_det=="y")

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

        body += "\\\\\n\hline\n"

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
                body += single_chain(graph_info, config_info, use_gcd, use_det)

        else:
            p_extremal = config_info["WH"]
            if p_extremal == 0:
                if "type" in config_info:
                    print("QHD unimplemented")
                else:
                    body += double_chain(graph_info, config_info, use_gcd, use_det)
            else:
                body += double_chain(graph_info, config_info, use_gcd, use_det)
    body += "\\\\\n\hline\n"
    caption = input("Give me a caption: ")
    display_string(S.format(body=body, caption=caption, opt_cols = (use_gcd + use_det)*"c|", opt_gcd_text = " & gcd" if use_gcd else "", opt_det_text = " & Det" if use_det else ""))
