from sage.all_cmdline import *
load("global_log_resolution.sage")

def make_test_file(filename : str, curve_dict : list, outname : str = "out"):
    base = len(curve_dict)

    curves = [u for _,u in curve_dict]

    loginfo = global_log_resolution(curves)

    cid = base

    graph = [[] for i in range(base)]

    def dfs(parents : dict, current : list,parent):
        nonlocal graph
        nonlocal cid
        number = current[0]
        mult = current[1]
        if len(current) == 2:
            # number < base:
            graph[number] += mult*[parent]
            return
        direct_parents = current[2]
        for i in range(mult):
            cparents = parents.copy()
            thisid = cid
            graph.append([*[parents[x] for x in direct_parents]])
            cparents[number] = cid
            cid += 1
            for x in current[3:]:
                dfs(cparents,x,thisid)

    for x in loginfo:
        if len(x) == 5:
            if x[3][0] != x[4][0] and x[3][0] < base and x[4][0] < base and x[3][1] == x[4][1] == 1:
                graph[max(x[3][0],x[4][0])] += x[1]*[min(x[3][0],x[4][0])]
                continue
        dfs({},x,-1)

    configtext = (
        "Output: jsonl/{name}\n"
        "Summary_Output: summary/{name}\n"
        "Summary_Style: LaTeX_Table\n\n"
        "Single_Chain: Y\n"
        "Double_Chain: Y\n"
        "Single_QHD: Y\n"
        "Double_QHD: Y\n"
        "Keep_First: global\n"
        "Search_For: 5 6 7 8 9\n\n"
        "Nef_Check: N\n"
        "Effective_Check: N\n"
        "Obstruction_Check: N\n\n"
        "Summary_Include_GCD: Y\n"
        "LaTeX_Include_Subsection: Y\n\n"
        "Tests: 1\n"
        "K2: 9\n\n"
    ).format(name=outname)
    exctext = "Name:\n"
    basetext = ""

    def name(i):
        nonlocal curve_dict
        if i < base:
            return curve_dict[i][0]
        else:
            return "E{}".format(i-base+1)

    last_section_degree = 0
    for i in range(len(graph)):
        if i < base:
            thing = " ".join([name(x) for x in graph[i]])
            if last_section_degree != curves[i].degree():
                basetext += "Sections({}):\n".format(curves[i].degree()**2)
            basetext += (
                "  {} Try\n"
                "    {}\n"
            ).format(
                name(i),
                thing
            )
            last_section_degree = curves[i].degree()
        else:
            thing = " ".join([name(x) for x in graph[i]])
            exctext += (
                "  {} Try\n"
                "    {}\n"
            ).format(
                name(i),
                thing
            )

    f = open(filename,'w')
    f.write(configtext)
    f.write(exctext)
    f.write(basetext)
    f.close()