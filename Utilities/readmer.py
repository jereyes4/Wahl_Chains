#!/usr/bin/env python3
f = open("README.raw.md","r")
g = open("../README.md","w")
for l in f:
    sep = l.split('$')
    for i in range(len(sep)//2):
        sep[2*i+1] = sep[2*i+1].replace('+','%2B')
        g.write('{0}<img src="https://render.githubusercontent.com/render/math?math={1}">'.format(sep[2*i],sep[2*i+1]))
    g.write(sep[-1])
f.close()
g.close()
