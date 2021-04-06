#include<vector>
#include<iostream>
#include"../src/QHD_functions.hpp"

using namespace std;

void generate_type_a(int p, int q, int r, vector<int> (&fork)[3], vector<int>& self_int) {
    self_int.resize(1);
    self_int[0] = -4;
    for(auto& x : fork) {
        x.resize(1);
        x[0] = 0;
    }
    int id = 1;
    for (int i = 0; i < q; ++i) {
        fork[0].emplace_back(id++);
        self_int.emplace_back(-2);
    }
    fork[0].emplace_back(id++);
    self_int.emplace_back(-p-3);
    for (int i = 0; i < p; ++i) {
        fork[1].emplace_back(id++);
        self_int.emplace_back(-2);
    }
    fork[1].emplace_back(id++);
    self_int.emplace_back(-r-3);
    for (int i = 0; i < r; ++i) {
        fork[2].emplace_back(id++);
        self_int.emplace_back(-2);
    }
    fork[2].emplace_back(id++);
    self_int.emplace_back(-q-3);
}

void generate_type_b(int p, int q, int r, vector<int> (&fork)[3], vector<int>& self_int) {
    self_int.resize(1);
    self_int[0] = -3;
    for(auto& x : fork) {
        x.resize(1);
        x[0] = 0;
    }
    int id = 1;
    for (int i = 0; i < q; ++i) {
        fork[0].emplace_back(id++);
        self_int.emplace_back(-2);
    }
    fork[0].emplace_back(id++);
    self_int.emplace_back(-3);
    for (int i = 0; i < p; ++i) {
        fork[0].emplace_back(id++);
        self_int.emplace_back(-2);
    }
    fork[0].emplace_back(id++);
    self_int.emplace_back(-r-3);
    for (int i = 0; i < r; ++i) {
        fork[1].emplace_back(id++);
        self_int.emplace_back(-2);
    }
    fork[1].emplace_back(id++);
    self_int.emplace_back(-q-4);
    fork[2].emplace_back(id++);
    self_int.emplace_back(-p-3);
}

void generate_type_c(int q, int r, vector<int> (&fork)[3], vector<int>& self_int) {
    self_int.resize(1);
    self_int[0] = -3;
    for(auto& x : fork) {
        x.resize(1);
        x[0] = 0;
    }
    int id = 1;
    for (int i = 0; i < q; ++i) {
        fork[0].emplace_back(id++);
        self_int.emplace_back(-2);
    }
    fork[0].emplace_back(id++);
    self_int.emplace_back(-r-4);
    fork[1].emplace_back(id++);
    self_int.emplace_back(-2);
    for (int i = 0; i < r; ++i) {
        fork[2].emplace_back(id++);
        self_int.emplace_back(-2);
    }
    fork[2].emplace_back(id++);
    self_int.emplace_back(-q-4);
}


void generate_type_d(int q, int r, vector<int> (&fork)[3], vector<int>& self_int) {
    self_int.resize(1);
    self_int[0] = -2;
    for(auto& x : fork) {
        x.resize(1);
        x[0] = 0;
    }
    int id = 1;
    for (int i = 0; i < q; ++i) {
        fork[0].emplace_back(id++);
        self_int.emplace_back(-2);
    }
    fork[0].emplace_back(id++);
    self_int.emplace_back(-3);
    for (int i = 0; i < r; ++i) {
        fork[0].emplace_back(id++);
        self_int.emplace_back(-2);
    }
    fork[0].emplace_back(id++);
    self_int.emplace_back(-q-5);
    fork[1].emplace_back(id++);
    self_int.emplace_back(-r-4);
    fork[2].emplace_back(id++);
    self_int.emplace_back(-2);
}
void generate_type_e(int p, int q, vector<int> (&fork)[3], vector<int>& self_int) {
    self_int.resize(1);
    self_int[0] = -2;
    for(auto& x : fork) {
        x.resize(1);
        x[0] = 0;
    }
    int id = 1;
    for (int i = 0; i < q; ++i) {
        fork[0].emplace_back(id++);
        self_int.emplace_back(-2);
    }
    fork[0].emplace_back(id++);
    self_int.emplace_back(-4);
    for (int i = 0; i < p; ++i) {
        fork[0].emplace_back(id++);
        self_int.emplace_back(-2);
    }
    fork[0].emplace_back(id++);
    self_int.emplace_back(-q-4);
    fork[1].emplace_back(id++);
    self_int.emplace_back(-3);
    fork[2].emplace_back(id++);
    self_int.emplace_back(-p-3);
}
void generate_type_f(int q, vector<int> (&fork)[3], vector<int>& self_int) {
    self_int.resize(1);
    self_int[0] = -2;
    for(auto& x : fork) {
        x.resize(1);
        x[0] = 0;
    }
    int id = 1;
    fork[0].emplace_back(id++);
    self_int.emplace_back(-2);
    fork[1].emplace_back(id++);
    self_int.emplace_back(-3);
    for (int i = 0; i < q; ++i) {
        fork[2].emplace_back(id++);
        self_int.emplace_back(-2);
    }
    fork[2].emplace_back(id++);
    self_int.emplace_back(-q-6);
}

void generate_type_g(int p, int q, int r, vector<int> (&fork)[3], vector<int>& self_int) {
    self_int.resize(1);
    self_int[0] = -2;
    for(auto& x : fork) {
        x.resize(1);
        x[0] = 0;
    }
    int id = 1;
    for (int i = 0; i < q; ++i) {
        fork[0].emplace_back(id++);
        self_int.emplace_back(-2);
    }
    fork[0].emplace_back(id++);
    self_int.emplace_back(-3);
    for (int i = 0; i < r; ++i) {
        fork[0].emplace_back(id++);
        self_int.emplace_back(-2);
    }
    fork[0].emplace_back(id++);
    self_int.emplace_back(-3);
    for (int i = 0; i < p; ++i) {
        fork[0].emplace_back(id++);
        self_int.emplace_back(-2);
    }
    fork[0].emplace_back(id++);
    self_int.emplace_back(-q-4);
    fork[1].emplace_back(id++);
    self_int.emplace_back(-p-3);
    fork[2].emplace_back(id++);
    self_int.emplace_back(-r-4);
}

void generate_type_h(int q, vector<int> (&fork)[3], vector<int>& self_int) {
    self_int.resize(1);
    self_int[0] = -2;
    for(auto& x : fork) {
        x.resize(1);
        x[0] = 0;
    }
    int id = 1;
    for (int i = 0; i < q; ++i) {
        fork[0].emplace_back(id++);
        self_int.emplace_back(-2);
    }
    fork[0].emplace_back(id++);
    self_int.emplace_back(-3);
    fork[0].emplace_back(id++);
    self_int.emplace_back(-q-3);
    fork[1].emplace_back(id++);
    self_int.emplace_back(-4);
    fork[2].emplace_back(id++);
    self_int.emplace_back(-4);
}
void generate_type_i(int q, vector<int> (&fork)[3], vector<int>& self_int) {
    self_int.resize(1);
    self_int[0] = -2;
    for(auto& x : fork) {
        x.resize(1);
        x[0] = 0;
    }
    int id = 1;
    for (int i = 0; i < q+1; ++i) {
        fork[0].emplace_back(id++);
        self_int.emplace_back(-2);
    }
    fork[0].emplace_back(id++);
    self_int.emplace_back(-q-3);
    fork[1].emplace_back(id++);
    self_int.emplace_back(-3);
    fork[2].emplace_back(id++);
    self_int.emplace_back(-6);
}
void generate_type_j(int q, vector<int> (&fork)[3], vector<int>& self_int) {
    self_int.resize(1);
    self_int[0] = -2;
    for(auto& x : fork) {
        x.resize(1);
        x[0] = 0;
    }
    int id = 1;
    fork[0].emplace_back(id++);
    self_int.emplace_back(-2);
    for (int i = 0; i < q+1; ++i) {
        fork[1].emplace_back(id++);
        self_int.emplace_back(-2);
    }
    fork[1].emplace_back(id++);
    self_int.emplace_back(-q-4);
    fork[2].emplace_back(id++);
    self_int.emplace_back(-6);
}

const int MINTEST = 0;
const int MAXTEST = 10;

void fail(string s) {
    cout << s << '\n';
}

bool verify_discrepancy(const vector<int> (&fork)[3], const vector<int>& self_int, algs::QHD_data data) {
    vector<long long> discrepancies(self_int.size());
    long long d = algs::get_QHD_discrepancies(fork,self_int,data,discrepancies);
    auto denominator = d;
    // debugx(denominator);
    // debugv(discrepancies);
    // debugt("\n");
    if ((-2 - self_int[fork[0][0]])*d != discrepancies[fork[0][1]] + discrepancies[fork[1][1]] + discrepancies[fork[2][1]] + self_int[fork[0][0]]*discrepancies[fork[0][0]]) return false;
    for (int i = 0; i < 3; ++i) {
        for (int j = 1; j < fork[i].size() - 1; ++j) {
            if ((-2 - self_int[fork[i][j]])*d != discrepancies[fork[i][j-1]] + discrepancies[fork[i][j+1]] + self_int[fork[i][j]]*discrepancies[fork[i][j]]) return false;
        }
        if ((-2 - self_int[fork[i].back()])*d != discrepancies[*(fork[i].rbegin()+1)] + self_int[fork[i].back()]*discrepancies[fork[i].back()]) return false;
    }
    return true;
}

int main(){
    vector<int> fork[3];
    vector<int> self_int;
    algs::QHD_data data;
    for (int q = MINTEST; q < MAXTEST; ++q) {
        generate_type_f(q,fork,self_int);
        data = algs::get_QHD_type(fork,self_int);
        if (data.type != algs::type_f) fail("type f miss");
        if (data.q != q) fail("type f q miss " + to_string(q));
        if(!verify_discrepancy(fork,self_int,data)) fail("type f disc miss " + to_string(q));

        generate_type_h(q,fork,self_int);
        data = algs::get_QHD_type(fork,self_int);
        if (data.type != algs::type_h) fail("type h miss");
        if (data.q != q) fail("type h q miss " + to_string(q));
        if(!verify_discrepancy(fork,self_int,data)) fail("type h disc miss " + to_string(q));

        generate_type_i(q,fork,self_int);
        data = algs::get_QHD_type(fork,self_int);
        if (data.type != algs::type_i) fail("type i miss");
        if (data.q != q) fail("type i q miss " + to_string(q));
        if(!verify_discrepancy(fork,self_int,data)) fail("type i disc miss " + to_string(q));

        generate_type_j(q,fork,self_int);
        data = algs::get_QHD_type(fork,self_int);
        if (data.type != algs::type_j) fail("type j miss");
        if (data.q != q) fail("type j q miss " + to_string(q));
        if(!verify_discrepancy(fork,self_int,data)) fail("type j disc miss " + to_string(q));

        for (int r = MINTEST; r < MAXTEST; ++r) {
            generate_type_c(q,r,fork,self_int);
            data = algs::get_QHD_type(fork,self_int);
            if (data.type != algs::type_c) fail("type c miss");
            if (data.q != q) fail("type c q miss " + to_string(q) + " " + to_string(r));
            if (data.r != r) fail("type c r miss " + to_string(q) + " " + to_string(r));
            if(!verify_discrepancy(fork,self_int,data)) fail("type f disc miss " + to_string(q) + " " + to_string(r));

            generate_type_d(q,r,fork,self_int);
            data = algs::get_QHD_type(fork,self_int);
            if (data.type != algs::type_d) fail("type d miss");
            if (data.q != q) fail("type d q miss " + to_string(q) + " " + to_string(r));
            if (data.r != r) fail("type d r miss " + to_string(q) + " " + to_string(r));
            if(!verify_discrepancy(fork,self_int,data)) fail("type d disc miss " + to_string(q) + " " + to_string(r));

            generate_type_e(r,q,fork,self_int);
            data = algs::get_QHD_type(fork,self_int);
            if (data.type != algs::type_e) fail("type e miss");
            if (data.p != r) fail("type c p miss " + to_string(r) + " " + to_string(q));
            if (data.q != q) fail("type c q miss " + to_string(r) + " " + to_string(q));
            if(!verify_discrepancy(fork,self_int,data)) fail("type e disc miss " + to_string(r) + " " + to_string(q));

            for (int p = MINTEST; p < MAXTEST; ++p) {

                generate_type_a(p,q,r,fork,self_int);
                data = algs::get_QHD_type(fork,self_int);
                if (data.type != algs::type_a) fail("type a miss");
                if (data.p != p) fail("type a p miss " + to_string(p) + " " + to_string(q) + " " + to_string(r));
                if (data.q != q) fail("type a q miss " + to_string(p) + " " + to_string(q) + " " + to_string(r));
                if (data.r != r) fail("type a r miss " + to_string(p) + " " + to_string(q) + " " + to_string(r));
                if(!verify_discrepancy(fork,self_int,data)) fail("type a disc miss " + to_string(p) + " " + to_string(q) + " " + to_string(r));

                generate_type_b(p,q,r,fork,self_int);
                data = algs::get_QHD_type(fork,self_int);
                if (data.type != algs::type_b) fail("type b miss");
                if (data.p != p) fail("type b p miss " + to_string(p) + " " + to_string(q) + " " + to_string(r));
                if (data.q != q) fail("type b q miss " + to_string(p) + " " + to_string(q) + " " + to_string(r));
                if (data.r != r) fail("type b r miss " + to_string(p) + " " + to_string(q) + " " + to_string(r));
                if(!verify_discrepancy(fork,self_int,data)) fail("type b disc miss " + to_string(p) + " " + to_string(q) + " " + to_string(r));

                generate_type_g(p,q,r,fork,self_int);
                data = algs::get_QHD_type(fork,self_int);
                if (data.type != algs::type_g) fail("type g miss");
                if (data.p != p) fail("type g p miss " + to_string(p) + " " + to_string(q) + " " + to_string(r));
                if (data.q != q) fail("type g q miss " + to_string(p) + " " + to_string(q) + " " + to_string(r));
                if (data.r != r) fail("type g r miss " + to_string(p) + " " + to_string(q) + " " + to_string(r));
                if(!verify_discrepancy(fork,self_int,data)) fail("type g disc miss " + to_string(p) + " " + to_string(q) + " " + to_string(r));
            }
        }
    }
}