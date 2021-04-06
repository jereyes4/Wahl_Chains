#include<bits/stdc++.h>
#include"../src/Graph.hpp"

using namespace std;



int main() {
    Graph G;
    G.reset();
    int a,b,n;
    cin >> n;
    while(n--) G.add_curve(0);
    while(cin >> a >> b) {
        G.add_edge(a,b);
    }
    G.begin_search();
    do {
        debugx(G.frame);
        debugv(G.connections);
        cout << '\n';
    } while(G.next_candidate_QHD3());
}