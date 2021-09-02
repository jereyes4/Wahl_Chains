#!/usr/bin/env python3
from os import name
import sys, json, getopt, linecache
from math import gcd

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

base_header =(
    "\\begin{{longtable}}{{{table_sep}}}\n"
    "\\hline\n"
    "\\multicolumn{{{column_number}}}{{|c|}}{{{title}}}\\\\\n"
    "\\hline\n"
    "{column_values}\\\\\n"
    "\\hline\n"
    "\\endfirsthead\n"
    "\n"
    "\\hline\n"
    "{column_values}\\\\\n"
    "\\hline\n"
    "\\endhead\n"
    "\\hline\n"
    "\\endfoot\n"
    "\n"
)

def get_baseused_id(graph_info, config_info):
    total_curves = len(graph_info["graph"])
    exceptionals = graph_info["blps"]
    baseused = [x for x in config_info["used"] if x not in exceptionals]
    mask = 0
    for i in range(total_curves):
        if i in exceptionals:
            continue
        mask *= 2
        if i in baseused:
            mask += 1
    return mask

def encode_id(mask):
    if mask == 0:
        return "0"
    S = ""
    while mask:
        d = mask % 62
        mask //= 62
        if d < 10:
            S += str(d)
        elif d < 36:
            S += chr(ord('A') + d - 10)
        else:
            S += chr(ord('a') + d - 36)
    return S[::-1]

def get_header_and_title(K2, chain_amount, option_mask):

    by_length           , option_mask = option_mask & 1, option_mask//2
    chern_fraction      , option_mask = option_mask & 1, option_mask//2
    include_determinant , option_mask = option_mask & 1, option_mask//2
    include_pk          , option_mask = option_mask & 1, option_mask//2
    include_chern       , option_mask = option_mask & 1, option_mask//2
    include_gcd         , option_mask = option_mask & 1, option_mask//2
    include_eff         , option_mask = option_mask & 1, option_mask//2
    include_obs         , option_mask = option_mask & 1, option_mask//2
    include_nef         , option_mask = option_mask & 1, option_mask//2
    group_by_base       , option_mask = option_mask & 1, option_mask//2

    columns = 2*chain_amount + 1
    title = "1 chain" if chain_amount == 1 else "2 chains"
    title += ", \(K^2 = {0}\)".format(K2)
    column_values = "$(n,a)$ & Len & "*chain_amount
    if chain_amount == 2 and include_gcd:
        column_values += "GCD & "
        columns += 1
    if include_nef:
        column_values += "Nef & "
        columns += 1
    if include_eff:
        column_values += "$\\mathbb Q$-ef & "
        columns += 1
    if include_obs:
        column_values += "Obs 0 & "
        columns += 1
    if include_chern:
        if chern_fraction:
            column_values += "$\\overline c_1^2 / \\overline c_2$ & "
        else:
            column_values += "$(\\overline c_1^2,\\overline c_2)$ & "
        columns += 1
    if include_pk:
        column_values += "$(P,K)$ & "
        columns += 1
    if include_determinant:
        column_values += "Det & "
        columns += 1
    if chain_amount == 2:
        column_values += "WH & "
        columns += 1
    column_values += "Index"
    if group_by_base:
        column_values += " & Base Config"
        columns += 1

    return base_header.format(
        table_sep = "c".join((columns+1)*["|"]),
        column_number = columns,
        title = title,
        column_values = column_values
    ), title


def get_example_line(graph_info, config_info, this_id, Intersection_Matrix, option_mask):

    by_length           , option_mask = option_mask & 1, option_mask//2
    chern_fraction      , option_mask = option_mask & 1, option_mask//2
    include_determinant , option_mask = option_mask & 1, option_mask//2
    include_pk          , option_mask = option_mask & 1, option_mask//2
    include_chern       , option_mask = option_mask & 1, option_mask//2
    include_gcd         , option_mask = option_mask & 1, option_mask//2
    include_eff         , option_mask = option_mask & 1, option_mask//2
    include_obs         , option_mask = option_mask & 1, option_mask//2
    include_nef         , option_mask = option_mask & 1, option_mask//2
    group_by_base       , option_mask = option_mask & 1, option_mask//2

    real_graph = graph_info["graph"]
    Fibers = graph_info["Fibers"]
    real_exceptionals = graph_info["blps"]
    real_selfint = graph_info["selfint"]

    S = ""

    chain_amount = config_info["#"]
    K2 = config_info["K2"]
    used = config_info["used"]

    original_K2 = graph_info["K2"]

    if chain_amount == 1:
        if "type" in config_info:
            S += ("$({type};{p},{q},{r};{n})$ & {size} & ".format(
                type = config_info["type"],
                p = config_info["p"],
                q = config_info["q"],
                r = config_info["r"],
                n = config_info["N"],
                size = len(config_info["fork"])-2
            ))
        else:
            disc = config_info["disc"]
            chain = config_info["chain"]
            a = min(-disc[chain[0]],-disc[chain[-1]])
            S += ("$({n},{a})$ & {size} & ".format(
                n = config_info["N"],
                a = a,
                size = len(config_info["chain"])
            ))
    else:
        if "type" in config_info:
            S += ("$({type};{p},{q},{r};{n})$ & {size} & ".format(
                type = config_info["type"],
                p = config_info["p"],
                q = config_info["q"],
                r = config_info["r"],
                n = config_info["N0"],
                size = len(config_info["fork"])-2
            ))
            disc = config_info["disc"]
            chain = config_info["chain"]
            a = min(-disc[chain[0]],-disc[chain[-1]])
            S += ("$({n},{a})$ & {size} & ".format(
                n = config_info["N1"],
                a = a,
                size = len(chain)
            ))
        else:
            disc = config_info["disc"]
            chain0 = config_info["chain0"]
            chain1 = config_info["chain1"]
            n0 = config_info["N0"]
            n1 = config_info["N1"]
            a0 = min(-disc[chain0[0]],-disc[chain0[-1]])
            a1 = min(-disc[chain1[0]],-disc[chain1[-1]])
            if by_length:
                ex0 = (len(chain0),n0,a0)
                ex1 = (len(chain1),n1,a1)
                if ex1 > ex0:
                    ex0,ex1 = ex1, ex0
                S += ("$({n},{a})$ & {size} & ".format(
                    n = ex0[1],
                    a = ex0[2],
                    size = ex0[0]
                ))
                S += ("$({n},{a})$ & {size} & ".format(
                    n = ex1[1],
                    a = ex1[2],
                    size = ex1[0]
                ))
            else:
                ex0 = (n0,a0,len(chain0))
                ex1 = (n1,a1,len(chain1))
                if ex1 > ex0:
                    ex0,ex1 = ex1, ex0
                S += ("$({n},{a})$ & {size} & ".format(
                    n = ex0[0],
                    a = ex0[1],
                    size = ex0[2]
                ))
                S += ("$({n},{a})$ & {size} & ".format(
                    n = ex1[0],
                    a = ex1[1],
                    size = ex1[2]
                ))
    if chain_amount == 2 and include_gcd:
        n0 = config_info["N0"]
        n1 = config_info["N1"]
        S += ("{0} & ".format(gcd(n0,n1)))
    if include_nef:
        S += ("YES & " if config_info["nef"] else "NO & ")
    if include_eff:
        S += ("YES & " if config_info["Qef"] else "NO & ")
    if include_obs:
        if config_info["obs"]:
            S += ("YES & ")
        else:
            complete_fibers = 0
            for fiber in Fibers:
                comp = True
                for curve in fiber:
                    if curve not in used:
                        comp = False
                        break
                if comp:
                    complete_fibers += 1
            S += ("NO({0}) & ".format(complete_fibers))
    if include_chern or include_pk:
        selfint = list(real_selfint) #deep copy
        exceptionals = list(real_exceptionals) #deep copy
        graph = [list(x) for x in real_graph] #deep copy
        blowdowns = config_info["blds"][::-1]
        for curve in range(len(graph)):
            if curve not in exceptionals and curve not in used:
                for other in graph[curve]:
                    graph[other] = [c for c in graph[other] if c != curve]
                graph[curve] = []
        normal_crossing = True
        for exc in blowdowns:
            other_exc_count = 0
            for i in range(len(graph[exc])):
                curve = graph[exc][i]
                if curve in exceptionals:
                    other_exc_count += 1
                graph[curve] = [c for c in graph[curve] if c != exc]
                selfint[curve] += 1
                for j in range(i+1,len(graph[exc])):
                    other = graph[exc][j]
                    graph[curve].append(other)
                    graph[other].append(curve)
            if other_exc_count > 1 and len(graph[exc]) != other_exc_count:
                normal_crossing = False
                break
            graph[exc] = []
            exceptionals.remove(exc)
            original_K2 += 1
        if normal_crossing:
            for exc in exceptionals:
                for curve in graph[exc]:
                    if curve in exceptionals:
                        normal_crossing = False
                        break
                if not normal_crossing:
                    break
        if not normal_crossing:
            if include_chern:
                S += ("-- & ")
            if include_pk:
                S += ("-- & ")
        else:
            used = set(used + exceptionals)
            double_points_2 = 0
            for curve in used:
                double_points_2 += len(graph[curve])
            mpoints = dict()
            for exc in exceptionals:
                m = len(graph[exc])
                if m not in mpoints:
                    mpoints[m] = 1
                else:
                    mpoints[m] += 1


            Ptilde = -double_points_2
            Ktilde = original_K2 - double_points_2//2
            c_1_tilde = -len(exceptionals) - 4*len(used) + double_points_2
            c_2_tilde = 12 + len(exceptionals) - 2*len(used) + double_points_2//2
            for curve in used:
                Ptilde += selfint[curve] + 5
                Ktilde += 2
                c_1_tilde -= selfint[curve]
            Ktilde -= Ptilde
            for m,tm in mpoints.items():
                Ptilde += (2*m - 4)*tm
                Ktilde += (2 - m)*tm



            if include_chern:
                if chern_fraction:
                    if c_2_tilde == 0:
                        S += ("$\\infty$ & ")
                    else:
                        S += ("${0:.2f}$ & ".format(c_1_tilde/c_2_tilde))
                else:
                    S += ("$({0},{1})$ & ".format(c_1_tilde,c_2_tilde))
            if include_pk:
                S += ("$({0},{1})$ & ".format(Ptilde,Ktilde))
    if include_determinant:
        baseused = []
        for i in used:
            if i not in real_exceptionals:
                baseused.append(i)
        baseused = sorted(baseused)
        a = 0
        used_matrix = [len(baseused)*[0] for i in range(len(baseused))]
        for x in range(len(Intersection_Matrix)):
            if x not in baseused:
                continue
            b = 0
            for y in range(len(Intersection_Matrix[x])):
                if y not in baseused:
                    continue
                used_matrix[a][b] = Intersection_Matrix[x][y]
                b += 1
            a += 1
        S += ("{0} & ".format(det(used_matrix)))
    if chain_amount == 2:
        if config_info["WH"] == 0:
            S += ("-- & ")
        else:
            if config_info["WH"] == 1:
                S += ("NO & ")
            else:
                if config_info["WH_CE"]:
                    S += ("CE ${}^\\dagger$ & ")
                else:
                    if "WHid" in config_info:
                        S += ("{0} & ".format(config_info["WHid"] + 1))
                    else:
                        S += ("YES & ")
    S += str(this_id)

    if config_info["nef_warn"]:
        S += (" ${}^\\dagger$")

    if group_by_base:
        S += (" & {0}".format(encode_id(get_baseused_id(graph_info, config_info))))

    return S

def main():
    options = ["All","Subsection", "Nef", "Obstruction", "Effective", "Gcd", "Chern", "PK", "Length_Sort","Fraction","Determinant","Base","Output ="]
    options_short = "asnbegcpklfdvo:"
    infilename = ""
    try:
        args, extra = getopt.gnu_getopt(sys.argv[1:],options_short,options)
    except:
        print("Unknown command.")
        exit(0)
    try:
        infilename = extra[0]
        infile = open(infilename,'r')
    except:
        print("Unable to open file.")
        exit(0)
    # try:
    if True:
        L = infile.readline()
        graph_info = json.loads(L)

        outname = "OUT.tex"
        include_subsection = False
        group_by_base = False
        include_nef = False
        include_obs = False
        include_eff = False
        include_gcd = False
        include_chern = False
        include_pk = False
        include_determinant = False
        chern_fraction = False
        by_length = False
        for arg, val in args:
            if arg in ["-a","--All"]:
                include_subsection = True
                if graph_info["nef_check"]:
                    include_nef = True
                else:
                    print("No nef check in json")
                if graph_info["obstruction_check"]:
                    include_obs = True
                else:
                    print("No obstruction check in json")
                if graph_info["effective_check"]:
                    include_eff = True
                else:
                    print("No effective check in json")
                include_gcd = True
                include_chern = True
                include_determinant = True
                include_pk = True
            if arg in ["-s","--Subsection"]:
                include_subsection = True
            if arg in ["-n","--Nef"]:
                if graph_info["nef_check"]:
                    include_nef = True
                else:
                    print("No nef check in json")
            if arg in ["-b","--Obstruction"]:
                if graph_info["obstruction_check"]:
                    include_obs = True
                else:
                    print("No obstruction check in json")
            if arg in ["-e","--Effective"]:
                if graph_info["effective_check"]:
                    include_eff = True
                else:
                    print("No effective check in json")
            if arg in ["-g","--Gcd"]:
                include_gcd = True
            if arg in ["-c","--Chern"]:
                include_chern = True
            if arg in ["-p","-k","--PK"]:
                include_pk = True
            if arg in ["-l","--Length_Sort"]:
                by_length = True
            if arg in ["-f","--Fraction"]:
                chern_fraction = True
            if arg in ["-d","--Determinant"]:
                include_determinant = True
            if arg in ["-o","--Output"]:
                outname = val
            if arg in ["-v","--Base"]:
                group_by_base = True
        try:
            outfile = open(outname,"w")
        except:
            print("Unable to open out file.")
            exit(0)

        option_mask = 0
        for v in (group_by_base, include_nef, include_obs, include_eff, include_gcd, include_chern, include_pk, include_determinant, chern_fraction, by_length):
            option_mask = 2*option_mask + v

        real_graph = graph_info["graph"]
        real_exceptionals = graph_info["blps"]
        real_selfint = graph_info["selfint"]

        Intersection_Matrix = None

        if include_determinant:
            selfint = list(real_selfint) #deep copy

            Intersection_Matrix = [len(real_graph)*[0] for i in range(len(real_graph))]
            count_graph = [len(real_graph)*[0] for i in range(len(real_graph))]
            for i in range(len(real_graph)):
                for x in real_graph[i]:
                    count_graph[i][x] += 1
            for i in real_exceptionals[::-1]:
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

        outfile.write("%\\usepackage{longtable}\n")

        if not group_by_base:
            prev_chain_amount = 0
            prevK = 0
            is_first = False
            ex_id = 1
            for L in infile:
                config_info = json.loads(L)
                chain_amount = config_info["#"]
                K2 = config_info["K2"]

                if prev_chain_amount != chain_amount or prevK != K2:
                    if prev_chain_amount != 0:
                        outfile.write("\n\\end{longtable}\n")

                    is_first = True
                    prev_chain_amount = chain_amount
                    prevK = K2
                    header, title = get_header_and_title(K2,chain_amount,option_mask)
                    if include_subsection:
                        outfile.write("\\subsection{{{0}}}\n".format(title))
                    outfile.write(header)
                if not is_first:
                    outfile.write("\\\\\n")
                else:
                    is_first = False
                S = get_example_line(graph_info, config_info, ex_id, Intersection_Matrix, option_mask)
                outfile.write(S)
                ex_id += 1
            if prev_chain_amount != 0:
                outfile.write("\n\\end{longtable}\n")
            infile.close()

        else:
            infile.close()
            Data = []
            prev_chain_amount = 0
            prevK = 0
            ex_id = 1
            while True:
                L = linecache.getline(infilename,ex_id + 1)
                if L == "":
                    break
                config_info = json.loads(L)
                chain_amount = config_info["#"]
                K2 = config_info["K2"]

                if prev_chain_amount != chain_amount or prevK != K2:
                    if prev_chain_amount != 0:
                        Data.sort(key=lambda x : x[0])
                        is_first = True
                        for _,id in Data:
                            temp_info = json.loads(linecache.getline(infilename,id+1))
                            if not is_first:
                                outfile.write("\\\\\n")
                            else:
                                is_first = False
                            S = get_example_line(graph_info,temp_info,id,Intersection_Matrix,option_mask)
                            outfile.write(S)
                        outfile.write("\n\\end{longtable}\n")
                        Data.clear()
                    prev_chain_amount = chain_amount
                    prevK = K2
                    header, title = get_header_and_title(K2, chain_amount, option_mask)
                    if include_subsection:
                        outfile.write("\\subsection{{{0}}}\n".format(title))
                    outfile.write(header)
                Data.append((get_baseused_id(graph_info, config_info),ex_id))
                ex_id += 1
            if prev_chain_amount != 0:
                Data.sort(key=lambda x : x[0])
                is_first = True
                for _,id in Data:
                    temp_info = json.loads(linecache.getline(infilename,id+1))
                    if not is_first:
                        outfile.write("\\\\\n")
                    else:
                        is_first = False
                    S = get_example_line(graph_info,temp_info,id,Intersection_Matrix,option_mask)
                    outfile.write(S)
                outfile.write("\n\\end{longtable}\n")
        outfile.close()

    # except:
        # print("Incompatible or corrupted jsonl file.")

if __name__ == "__main__":
    main()
