#pragma GCC optimize("Ofast")

#include<vector>
#include<iostream>
#include<fstream>


const int min_n2 = 10;
const int max_n2 = 12;

const int min_n1 = 0;
const int max_n1 = 6;

const int min_p0 = 0;
const int max_p0 = 4;

const int min_p1 = 0;
const int max_p1 = 2;

const int min_p2 = 0;
const int max_p2 = 1;

const int max_triple = 5;
const int max_quadruple = 1;
const int max_quintuple = 1;

// K objetivo
#define KK 6
// numero de cadenas objetivo
#define PP 2
// numero de fibras singulares
#define FF 4

// poner bullets en la lista
bool bullets = false;


#define xstr(s) str(s)
#define str(s) #s

const char header[] =
"%\\usepackage{longtable}\n"
"\\begin{longtable}{|c|c|c|c|c||||c|c|c|c||||c|c|c||||c||||c|}\n"
"\\hline\n"
"\\multicolumn{14}{|c|}{ " xstr(PP) " chains, $K^2 = " xstr(KK) "$, Singular fibers: " xstr(FF) " }\\\\\n"
"\\hline\n"
"$e_{-2}$ & $e_{-1}$ & $e_{0}$ & $e_{1}$ & $e_{2}$ & $t_2$ & $t_3$ & $t_4$ & $t_5$ & $t_3^\\bullet$ & $t_4^\\bullet$ & $t_5^\\bullet$ & $\\overline c_1^2 / \\overline c_2$ & ID\\\\\n"
"\\hline\n"
"\\endfirsthead\n"
"\n"
"\\hline\n"
"$e_{-2}$ & $e_{-1}$ & $e_{0}$ & $e_{1}$ & $e_{2}$ & $t_2$ & $t_3$ & $t_4$ & $t_5$ & $t_3^\\bullet$ & $t_4^\\bullet$ & $t_5^\\bullet$ & $\\overline c_1^2 / \\overline c_2$ & ID\\\\\n"
"\\hline\n"
"\\endhead\n"
"\\hline\n"
"\\endfoot\n"
"\n";

const char header_no_bullet[] = 
"%\\usepackage{longtable}\n"
"\\begin{longtable}{|c|c|c|c|c||||c|c|c|c||||c||||c|}\n"
"\\hline\n"
"\\multicolumn{11}{|c|}{ " xstr(PP) " chains, $K^2 = " xstr(KK) "$, Singular fibers: " xstr(FF) " }\\\\\n"
"\\hline\n"
"$e_{-2}$ & $e_{-1}$ & $e_{0}$ & $e_{1}$ & $e_{2}$ & $t_2$ & $t_3$ & $t_4$ & $t_5$ & $\\overline c_1^2 / \\overline c_2$ & ID\\\\\n"
"\\hline\n"
"\\endfirsthead\n"
"\n"
"\\hline\n"
"$e_{-2}$ & $e_{-1}$ & $e_{0}$ & $e_{1}$ & $e_{2}$ & $t_2$ & $t_3$ & $t_4$ & $t_5$ & $\\overline c_1^2 / \\overline c_2$ & ID\\\\\n"
"\\hline\n"
"\\endhead\n"
"\\hline\n"
"\\endfoot\n"
"\n";

int main() {
    std::ofstream f("N" xstr(PP) "K" xstr(KK) ".tex");
    f.precision(2);
    f << std::fixed;
    if (bullets) f << header;
    else f << header_no_bullet;
    bool first = true;
    int id = 1;
    for (int n2 = min_n2; n2 <= max_n2; ++n2)
    for (int n1 = min_n1; n1 <= max_n1; ++n1)
    for (int p0 = min_p0; p0 <= max_p0; ++p0)
    for (int p1 = min_p1; p1 <= max_p1; ++p1)
    for (int p2 = min_p2; p2 <= max_p2; ++p2)
    for (int t3 = 0; t3 <= max_triple; ++t3)
    for (int t3p = 0; t3p <= t3; ++t3p)
    for (int t4 = 0; t4 <= max_quadruple; ++t4)
    for (int t4p = 0; t4p <= t4; ++t4p)
    for (int t5 = 0; t5 <= max_quintuple; ++t5)
    for (int t5p = 0; t5p <= t5; ++t5p){
        int t2 = KK - (2*t3 + t3p + 3*t4 + 2*t4p + 4*t5 + 3*t5p) + (n2 + 2*n1 + 3*p0 + 4*p1 + 5*p2);
        int t2_2 = - PP - (3*t3 + 2*t3p + 4*t4 + 4*t4p + 5*t5 + 6*t5p) + (3*n2 + 4*n1 + 5*p0 + 6*p1 + 7*p2);
        if (2*t2 != t2_2) continue;
        if (t2 < 2*n1 + n2 - 2) continue;
        if (n2 == max_n2 and t2 + 3*t3 + 6*t4 + 10*t5 < max_n2 + FF*(n1 + 2*p0 + 3*p1 + 4*p2)) continue;
        if (n2 == max_n2-1 and t2 + 3*t3 + 6*t4 + 10*t5 < max_n2-2 + (FF-1)*(n1 + 2*p0 + 3*p1 + 4*p2)) continue;
        if (not (n1 or p0 or p1 or p2)) continue;
        if (!first) f << "\\\\\n";
        first = false;
        f << n2 << " & ";
        f << n1 << " & ";
        f << p0 << " & ";
        f << p1 << " & ";
        f << p2 << " & ";
        f << t2 << " & ";
        f << t3 << " & ";
        f << t4 << " & ";
        f << t5 << " & ";
        if (bullets) {
            f << t3p << " & ";
            f << t4p << " & ";
            f << t5p << " & ";
        }
        int c1 = -(n1 + 2*p0 + 3*p1 + 4*p2) - 2*(n2 + n1 + p0 + p1 + p2) + (2*t2 + 5*t3 + 8*t4 + 11*t5);
        int c2 = 12 + (t2 + 2*t3 + 3*t4 + 4*t5) - 2*(n2 + n1 + p0 + p1 + p2);
        if (c2 == 0) {
            f << "\\infty & ";
        }
        else {
            f << double(c1)/double(c2) << " & ";
        }
        f << id++;
    }
    f << "\n\\end{longtable}\n";
    f.close();
} 
