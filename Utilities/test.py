from sage.all_cmdline import *
from make_test_file import make_test_file

load("global_log_resolution.sage")

FILENAME = "test5.txt"

t = SR.var('t')


# FF = NumberField(t**2 + 1,'a')
# FF = NumberField(t**2 - 2,'a')
FF = NumberField(t**2 - 5,'a')
a = FF.gen()
FF = FF.extension(t**2 + t + 1, 'b')
b = FF.gen()
FF = FF.absolute_field('m')
_,inF = FF.structure()
a = inF(a)
b = inF(b)

Rxyz = PolynomialRing(FF,['x','y','z'])

X,Y,Z = Rxyz.gens()
x,y,z = Rxyz.gens()

# curve_names = [
#     ["A1" , x*x + y*y + z*z],
#     ["A2" , x*x + y*y - z*z],
#     ["A3" , x*x - y*y + z*z],
#     ["A4" , x*x - y*y - z*z],
#     ["A5" , (2*a - 1)*(x*x + y*y + z*z) + 4*(x*y + y*z + z*x)],
#     ["L1" , x],
#     ["L2" , y],
#     ["L3" , z],
#     ["L4", x-y],
#     ["L5", y-z],
#     ["L6", x-z],
# ]


# curve_names = [
#     ["L_X" , X],
#     ["L_Y" , Y],
#     ["L_Z" , Z],
#     ["A" , X - Z],
#     ["B" , X + Y + Z],
#     ["C" , X - Y + Z],
#     ["Q_1" , (X + Z)**2 - Y*(X - Z)],
#     ["L_1" , X + Y - Z],
#     ["Q_2" , (X + Z)**2 + Y*(X - Z)],
#     ["L_2" , X - Y - Z],
#     ["H" , X + Z],
#     ["N", (2 + a)*X + a*Z + a*Y]
# ]

curve_names = [
    ["X" , x],
    ["Y" , y],
    ["Z" , z],
    ["F_1", 3*(x*y*z) + (y**3 - x**2*z + x*z**2)],
    ["F_2", 3*b*(x*y*z) + (y**3 - x**2*z + x*z**2)],
    ["F_3", 3*b**2*(x*y*z) + (y**3 - x**2*z + x*z**2)],
    ["H", x-z],
    ["K", x+z],
    ["G", x*z-y**2],
    ["S^T", (-1+a)*x**2*z + (4 - 2*a)*x*y*z + (1 + a)*y**2*z +2*y**3 +2*x*z**2],
    ["R^T", y**3 + x**2*z],
    ["Q^T", y**5 - x**3*y*z + x*x*y*y*z + 3*x*y**3*z -2*x**3*z**2 + 5*x*x*y*z*z + x**2*z**3],
    ["V^T", y**4 - x**2*y*z + 4*x*y**2*z - y**3*z + x**2*z**2],
    ["T_1", y - x],
    ["T_2", y - b**2*x],
    ["T_3", y - b*x],
    ["S_1", y + z],
    ["S_2", y + b**2*z],
    # ["S_3", y + b*z],
    ["R_1", 2*y + (z-x)],
    # ["R_2", 2*y + b**2*(z-x)],
    # ["R_3", 2*y + b*(z-x)],
    ["D_1^T", 2*y**4 - 2*x**2*y*z + (-5*a - 5)*x*y**2*z + (-10*a - 20)*x*y*z**2 + (-5*a - 11)*x*z**3],
    ["D_2^T", 2*y**4 - 2*x**2*y*z + (+5*a - 5)*x*y**2*z + (+10*a - 20)*x*y*z**2 + (+5*a - 11)*x*z**3],
]


make_test_file(FILENAME,curve_names)

