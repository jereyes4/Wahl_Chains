#!/usr/bin/env python3
# Run from ../
import glob, json

target_K = 5


Examples = [[],[]]

Header = (
    "%\\usepackage{{longtable}}\n"
    "\\begin{{longtable}}{{|c|c|c|c|c||||c|c|c|c||||c||||c|}}\n"
    "\\hline\n"
    "\\multicolumn{{11}}{{|c|}}{{ Realizable configurations, {P} chains, $K^2 = {K}$ }}\\\\\n"
    "\\hline\n"
    "$e_{{-2}}$ & $e_{{-1}}$ & $e_{{0}}$ & $e_{{1}}$ & $e_{{2}}$ & $t_2$ & $t_3$ & $t_4$ & $t_5$ & $\\overline c_1^2 / \\overline c_2$ & ID\\\\\n"
    "\\hline\n"
    "\\endfirsthead\n"
    "\n"
    "\\hline\n"
    "$e_{{-2}}$ & $e_{{-1}}$ & $e_{{0}}$ & $e_{{1}}$ & $e_{{2}}$ & $t_2$ & $t_3$ & $t_4$ & $t_5$ & $\\overline c_1^2 / \\overline c_2$ & ID\\\\\n"
    "\\hline\n"
    "\\endhead\n"
    "\\hline\n"
    "\\endfoot\n"
    "\n"
)

for filename in glob.glob("jsonl/*.jsonl"):
    file = open(filename,'r')

    graph_info = json.loads(file.readline())
    real_graph = graph_info["graph"]
    Fibers = graph_info["Fibers"]
    real_exceptionals = graph_info["blps"]
    real_selfint = graph_info["selfint"]

    downstairs_canint = len(real_selfint)*[0]
    for curve in range(len(real_selfint)):
        if curve in real_exceptionals:
            continue
        graph = [list(x) for x in real_graph]
        exceptionals = list(real_exceptionals)[::-1]

        canint = -real_selfint[curve] - 2
        for exc in exceptionals:
            mult = 0
            for other in graph[exc]:
                if other == curve:
                    mult += 1
            canint -= mult
            parents = [e for e in graph[exc] if e in exceptionals]
            if len(parents) == 1:
                graph[parents[0]].remove(exc)
                graph[parents[0]] += mult*[curve]
            elif len(parents) == 2:
                graph[parents[0]].remove(exc)
                graph[parents[1]].remove(exc)
                graph[parents[0]] += mult*[curve]
                graph[parents[1]] += mult*[curve]
                graph[parents[0]].append(parents[1])
                graph[parents[1]].append(parents[0])
            graph[exc] = []
        downstairs_canint[curve] = canint
    
    id = 0
    for L in file:
        id += 1
        config_info = json.loads(L)
        K2 = config_info["K2"]
        
        if K2 != 5:
            continue

        chain_amount = config_info["#"]
        used = config_info["used"]

        original_K2 = graph_info["K2"]
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

        c_1_tilde = -len(exceptionals) - 4*len(used) + double_points_2
        c_2_tilde = 12 + len(exceptionals) - 2*len(used) + double_points_2//2
        for curve in used:
            c_1_tilde -= selfint[curve]

        latexified = filename.replace("_","\\_")
        Ex = [[0,0,0,0,0], [0,0,0,0], 0, "\\texttt{{{}}} -- {}".format(latexified,id)]
        Ex[1][0] = double_points_2//2

        skip = False

        for point in mpoints.items():
            if point[0] == 2:
                Ex[1][0] -= point[1]
                continue
            if point[0] > 5:
                skip = True
                break
            Ex[1][point[0]-2] = point[1]
            Ex[1][0] -= point[1]*point[0]

        if skip:
            continue

        for curve in used:
            if curve in exceptionals:
                continue
            curve_type = -2 - downstairs_canint[curve]
            if curve_type < -2:
                print("something is wrong")
                exit(0)
            if curve_type > 2:
                skip = True
                break
            Ex[0][curve_type + 2] += 1

        if skip:
            continue
            
        if c_2_tilde == 0:
            Ex[2] = "$\\infty$"
        else:
            Ex[2] = "${0:.2f}$".format(c_1_tilde/c_2_tilde)
        
        Examples[chain_amount-1].append(Ex)

    file.close()


for P in range(2):
    This_Examples = Examples[P]
    if not This_Examples:
        continue
    This_Examples.sort()
    This_Examples = [This_Examples[i] for i in range(len(This_Examples)) if i == 0 or This_Examples[i][:3] != This_Examples[i-1][:3]]
    file = open("Found_K{K}P{P}.tex".format(K=target_K,P=P+1),'w')
    file.write(Header.format(
        P = P + 1,
        K = target_K
    ))
    first = True
    for ex in This_Examples:
        if not first:
            file.write("\\\\\n")
        first = False

        file.write("{} & {} & {} & {} & {} & {} & {} & {} & {} & {} & {}".format(
            ex[0][0],
            ex[0][1],
            ex[0][2],
            ex[0][3],
            ex[0][4],
            ex[1][0],
            ex[1][1],
            ex[1][2],
            ex[1][3],
            ex[2],
            ex[3]
        ))
    
    file.write("\n\\end{longtable}\n")
    file.close()
