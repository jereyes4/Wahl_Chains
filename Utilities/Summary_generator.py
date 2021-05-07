#!/usr/bin/env python3
import sys, json, getopt
from math import gcd

header =(
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

if __name__ == "__main__":
    options = ["All","Subsection", "Nef", "Obstruction", "Effective", "Gcd", "Chern", "PK", "Length_Sort","Output ="]
    options_short = "asnbegcpklo:"
    try:
        args, extra = getopt.gnu_getopt(sys.argv[1:],options_short,options)
    except:
        print("Unknown command.")
        exit(0)
    try:
        infile = open(extra[0],'r')
    except:
        print("Unable to open file.")
        exit(0)
    # try:
    if True:
        L = infile.readline()
        graph_info = json.loads(L)

        outname = "OUT.tex"
        include_subsection = False
        include_nef = False
        include_obs = False
        include_eff = False
        include_gcd = False
        include_chern = False
        include_pk = False
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
                include_pk = True
                by_length = True
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
            if arg in ["-o","--Output"]:
                outname = val
        try:
            outfile = open(outname,"w")
        except:
            print("Unable to open out file.")
            exit(0)
        
        outfile.write("%\\usepackage{longtable}\n")

        prev_chain_amount = 0
        prevK = 0
        is_first = False
        ex_id = 1
        real_graph = graph_info["graph"]
        Fibers = graph_info["Fibers"]
        real_exceptionals = graph_info["blps"]
        real_selfint = graph_info["selfint"]

        for L in infile:
            config_info = json.loads(L)
            chain_amount = config_info["#"]
            K2 = config_info["K2"]
            used = config_info["used"]

            original_K2 = graph_info["K2"]
            selfint = list(real_selfint) #deep copy
            exceptionals = list(real_exceptionals) #deep copy
            graph = [list(x) for x in real_graph] #deep copy

            if prev_chain_amount != chain_amount or prevK != K2:
                if prev_chain_amount != 0:
                    outfile.write("\n\\end{longtable}\n")
            
                is_first = True
                prev_chain_amount = chain_amount
                prevK = K2
                columns = 2*chain_amount + 1
                title = "1 chain" if chain_amount == 1 else "2 chains"
                title += ", $K^2 = {0}$".format(K2)
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
                    column_values += "$(\\overline c_1^2,\\overline c_2)$ & "
                    columns += 1
                if include_pk:
                    column_values += "$(P,K)$ & "
                    columns += 1
                if chain_amount == 2:
                    column_values += "WH & "
                    columns += 1
                column_values += "Index"

                if include_subsection:
                    outfile.write("\\subsection{{{0}}}\n".format(title))
                outfile.write(header.format(
                    table_sep = "c".join((columns+1)*["|"]),
                    column_number = columns,
                    title = title,
                    column_values = column_values
                ))
            if not is_first:
                outfile.write("\\\\\n")
            else:
                is_first = False
            if chain_amount == 1:
                if "type" in config_info:
                    outfile.write("$({type};{p},{q},{r};{n})$ & {size} & ".format(
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
                    outfile.write("$({n},{a})$ & {size} & ".format(
                        n = config_info["N"],
                        a = a,
                        size = len(config_info["chain"])
                    ))
            else:
                if "type" in config_info:
                    outfile.write("$({type};{p},{q},{r};{n})$ & {size} & ".format(
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
                    outfile.write("$({n},{a})$ & {size} & ".format(
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
                        if ex1 < ex0:
                            ex0,ex1 = ex1, ex0
                        outfile.write("$({n},{a})$ & {size} & ".format(
                        n = ex0[1],
                        a = ex0[2],
                        size = ex0[0]
                        ))
                        outfile.write("$({n},{a})$ & {size} & ".format(
                        n = ex1[1],
                        a = ex1[2],
                        size = ex1[0]
                        ))
                    else:
                        ex0 = (n0,a0,len(chain0))
                        ex1 = (n1,a1,len(chain1))
                        if ex1 < ex0:
                            ex0,ex1 = ex1, ex0
                        outfile.write("$({n},{a})$ & {size} & ".format(
                        n = ex0[1],
                        a = ex0[2],
                        size = ex0[0]
                        ))
                        outfile.write("$({n},{a})$ & {size} & ".format(
                        n = ex1[1],
                        a = ex1[2],
                        size = ex1[0]
                        ))
            if chain_amount == 2 and include_gcd:
                n0 = config_info["N0"]
                n1 = config_info["N1"]
                outfile.write("{0} & ".format(gcd(n0,n1)))
            if include_nef:
                outfile.write("YES & " if config_info["nef"] else "NO & ")
            if include_eff:
                outfile.write("YES & " if config_info["Qef"] else "NO & ")
            if include_obs:
                if config_info["obs"]:
                    outfile.write("YES & ")
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
                    outfile.write("NO({0}) & ".format(complete_fibers))
            if include_chern or include_pk:
                blowdowns = config_info["blds"][::-1]
                for curve in range(len(graph)):
                    if curve not in blowdowns and curve not in used:
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
                        outfile.write("-- & ")
                    if include_pk:
                        outfile.write("-- & ")
                    continue


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
                    outfile.write("${0}$ & ".format(c_1_tilde/(c_2_tilde if c_2_tilde != 0 else 1)))
                if include_pk:
                    outfile.write("$({0},{1})$ & ".format(Ptilde,Ktilde))
            if chain_amount == 2:
                if config_info["WH"] == 0:
                    outfile.write("-- & ")
                else:
                    if config_info["WH"] == 1:
                        outfile.write("NO & ")
                    else:
                        if config_info["WH_CE"]:
                            outfile.write("CE ${}^\\dagger$ & ")
                        else:
                            if "WHid" in config_info:
                                outfile.write("{0} & ".format(config_info["WHid"]))
                            else:
                                outfile.write("YES & ")
            outfile.write(str(ex_id))
            if config_info["nef_warn"]:
                outfile.write(" ${}^\\dagger$")
            ex_id += 1
        if prev_chain_amount != 0:
            outfile.write("\n\\end{longtable}\n")
    infile.close()
    outfile.close()

    # except: