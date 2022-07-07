import os, json

os.chdir(os.path.dirname(os.path.realpath(__file__)))
PATH = "data"

Total_Singularities = [set() for i in range(4)]
Total_Configurations = [set() for i in range(4)]

Total = 0

for filename in os.listdir(PATH):
    true_filename = os.path.join(PATH,filename)
    if not os.path.isfile(true_filename):
        continue
    file = open(true_filename)
    graph_info = json.loads(file.readline())

    for line in file:
        config_info = json.loads(line)
        if not config_info["nef"]:
            continue
        if config_info["nef_warn"]:
            continue
        if not config_info["obs"]:
            continue
        if not config_info["Qef"]:
            continue
        if "type" in config_info:
            # QHD. Skip
            continue
        if config_info["K2"] not in [1,2,3,4]:
            continue
        Total += 1
        if config_info["#"] == 1:
            chain = config_info["chain"]
            discrepancies = config_info["disc"]
            n = config_info["N"]
            a = min(-discrepancies[chain[0]],-discrepancies[chain[-1]])
            Total_Singularities[config_info["K2"]-1].add((n,a))
            Total_Configurations[config_info["K2"]-1].add((n,a))
        elif config_info["#"] == 2:
            chain0 = config_info["chain0"]
            chain1 = config_info["chain1"]
            discrepancies = config_info["disc"]
            n0 = config_info["N0"]
            n1 = config_info["N1"]
            a0 = min(-discrepancies[chain0[0]],-discrepancies[chain0[-1]])
            a1 = min(-discrepancies[chain1[0]],-discrepancies[chain1[-1]])
            if (n0,a0) < (n1,a1):
                Total_Configurations[config_info["K2"]-1].add((n0,a0,n1,a1))
            elif (n0,a0) == (n1,a1) and config_info["WH"] != 0:
                # Not Wahl
                continue
            else:
                Total_Configurations[config_info["K2"]-1].add((n1,a1,n0,a0))
            Total_Singularities[config_info["K2"]-1].add((n0,a0))
            Total_Singularities[config_info["K2"]-1].add((n1,a1))
            Total_Configurations[config_info["K2"]-1].add((n0,a0))
            Total_Configurations[config_info["K2"]-1].add((n1,a1))
    file.close()


print("Total surfaces:",Total)
for K in range(4):
    print("K^2 =",K+1)
    print("    Total Singularities:", len(Total_Singularities[K]))
    print("    Total Configurations:", len(Total_Configurations[K]))