#!/usr/bin/env python3
import sys, os

for name in sys.argv[1:]:

    try:
        f = open(name,"r")
    except:
        print("Error: " + name)
        continue
    g = open(name + "_temp","w")

    for L in f:
        if L[:17] == "\\begin{longtable}":
            A = f.readline()
            B = f.readline()
            C = B.split("{")[3][:-4]
            g.write("\\subsection{" + C + "}\n")
            g.write(L)
            g.write(A)
            g.write(B)
        else:
            g.write(L)
    f.close()
    g.close()
    os.remove(name)
    os.rename(name + "_temp", name)