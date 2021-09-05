from sage.all_cmdline import *
from make_test_file import make_test_file

load("global_log_resolution.sage")

FILENAME = "8211_v2.txt"

t = SR.var('t')

FF = NumberField(t*t+1,'i')
i = FF.gen()

Rxyz = PolynomialRing(FF,['x','y','z'])

X,Y,Z = Rxyz.gens()
x,y,z = Rxyz.gens()

curves = [
    ["x", x],
    ["y", y],
    ["z", z],
    ["L", y-x],
    ["C", z*z-x*y],
    ["F_1", 4*x*y*z + (y-x)*(z*z - x*y)],
    ["F_2", 4*x*y*z - (y-x)*(z*z - x*y)],
    ["R", x+y],
    ["H^+", y-z],
    ["H^-", y+z],
    ["V^+", x-z],
    ["V^-", x+z],
    ["iH^+", y-i*z],
    ["iH^-", y+i*z],
    ["iV^+", x-i*z],
    ["iV^-", x+i*z],
    ["Q_1", x*x + x*y + y*y - z*z],
    ["Q_2", x*x - x*y + y*y - z*z],
    ["G", z*z + x*y],
]

make_test_file(FILENAME,curves)