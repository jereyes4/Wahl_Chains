#pragma GCC optimize("Ofast")

#include<vector>
#include<iostream>
#include<fstream>


const int min_n2 = 9;
const int max_n2 = 12;

const int min_n1 = 0;
const int max_n1 = 5;

const int min_p0 = 0;
const int max_p0 = 4;

const int min_p1 = 0;
const int max_p1 = 2;

const int min_p2 = 0;
const int max_p2 = 1;

const int max_triple = 5;
const int max_quadruple = 1;
const int max_quintuple = 0;

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
"\\begin{longtable}{|c|c|c|c|c||||c|c|c|c||||c|c|c|}\n"
"\\hline\n"
"\\multicolumn{12}{|c|}{ " xstr(PP) " chains, $K^2 = " xstr(KK) "$, Singular fibers: " xstr(FF) " }\\\\\n"
"\\hline\n"
"$e_{-2}$ & $e_{-1}$ & $e_{0}$ & $e_{1}$ & $e_{2}$ & $c_2$ & $c_3$ & $c_4$ & $c_5$ & $c_3^\\bullet$ & $c_4^\\bullet$ & $c_5^\\bullet$\\\\\n"
"\\hline\n"
"\\endfirsthead\n"
"\n"
"\\hline\n"
"$e_{-2}$ & $e_{-1}$ & $e_{0}$ & $e_{1}$ & $e_{2}$ & $c_2$ & $c_3$ & $c_4$ & $c_5$ & $c_3^\\bullet$ & $c_4^\\bullet$ & $c_5^\\bullet$\\\\\n"
"\\hline\n"
"\\endhead\n"
"\\hline\n"
"\\endfoot\n"
"\n";

const char header_no_bullet[] = 
"%\\usepackage{longtable}\n"
"\\begin{longtable}{|c|c|c|c|c||||c|c|c|c|}\n"
"\\hline\n"
"\\multicolumn{9}{|c|}{ " xstr(PP) " chains, $K^2 = " xstr(KK) "$, Singular fibers: " xstr(FF) " }\\\\\n"
"\\hline\n"
"$e_{-2}$ & $e_{-1}$ & $e_{0}$ & $e_{1}$ & $e_{2}$ & $c_2$ & $c_3$ & $c_4$ & $c_5$\\\\\n"
"\\hline\n"
"\\endfirsthead\n"
"\n"
"\\hline\n"
"$e_{-2}$ & $e_{-1}$ & $e_{0}$ & $e_{1}$ & $e_{2}$ & $c_2$ & $c_3$ & $c_4$ & $c_5$\\\\\n"
"\\hline\n"
"\\endhead\n"
"\\hline\n"
"\\endfoot\n"
"\n";

int main() {
    std::ofstream f("THINGS.tex");
    if (bullets) f << header;
    else f << header_no_bullet;
    bool first = true;
    for (int n2 = min_n2; n2 <= max_n2; ++n2)
    for (int n1 = min_n1; n1 <= max_n1; ++n1)
    for (int p0 = min_p0; p0 <= max_p0; ++p0)
    for (int p1 = min_p1; p1 <= max_p1; ++p1)
    for (int p2 = min_p2; p2 <= max_p2; ++p2)
    for (int c3 = 0; c3 <= max_triple; ++c3)
    for (int c3p = 0; c3p <= c3; ++c3p)
    for (int c4 = 0; c4 <= max_quadruple; ++c4)
    for (int c4p = 0; c4p <= c4; ++c4p)
    for (int c5 = 0; c5 <= max_quintuple; ++c5)
    for (int c5p = 0; c5p <= c5; ++c5p){
        int c2 = KK - (2*c3 + c3p + 3*c4 + 2*c4p + 4*c5 + 3*c5p) + (n2 + 2*n1 + 3*p0 + 4*p1 + 5*p2);
        int c2_2 = - PP - (3*c3 + 2*c3p + 4*c4 + 4*c4p + 5*c5 + 6*c5p) + (3*n2 + 4*n1 + 5*p0 + 6*p1 + 7*p2);
        if (2*c2 != c2_2) continue;
        if (c2 < 2*n1 + n2 - 2) continue;
        if (n2 == max_n2 and c2 + 3*c3 + 6*c4 + 10*c5 < max_n2 + FF*(n1 + 2*p0 + 3*p1 + 4*p2)) continue;
        if (not (n1 or p0 or p1 or p2)) continue;
        if (!first) f << "\\\\\n";
        first = false;
        f << n2 << " & ";
        f << n1 << " & ";
        f << p0 << " & ";
        f << p1 << " & ";
        f << p2 << " & ";
        f << c2 << " & ";
        f << c3 << " & ";
        f << c4 << " & ";
        f << c5;
        if (bullets) {
            f << " & ";
            f << c3p << " & ";
            f << c4p << " & ";
            f << c5p;
        }
    }
    f << "\n\\end{longtable}\n";
    f.close();
}