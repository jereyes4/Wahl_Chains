#pragma once
#ifndef GRAPH_HPP
#define GRAPH_HPP
#include"config.hpp"
#include<vector> // vector
#include<utility> // pair
#include<tuple> // tuple
#include<set> // set, multiset
#include<stack> // stack


class Graph {
public:
    Graph() {}
    void reset();
    inline void add_curve(int self_int_t) {
        self_int.push_back(self_int_t);
        adj_list.emplace_back();
        disconnections.emplace_back();
        size++;
    }
    inline void add_edge(int a, int b) {
        adj_list[a].insert(b);
        if (adj_list[a].size() == 3) {
            big_degree_nodes.insert(a);
        }
        adj_list[b].insert(a);
        if (adj_list[b].size() == 3) {
            big_degree_nodes.insert(b);
        }
    }
    int size;
    std::vector<int> self_int;
    std::vector<std::pair<int,int>> connections;
    std::vector<std::multiset<int>> disconnections;
    void begin_search();
    bool next_candidate();

    // To search for QHD, call begin_search or begin_search_QHD3, either is fine. Then next_candidate_QHD3.
    // It will give both QHD candidates and normal chain candidates.
    void begin_search_QHD3();
    bool next_candidate_QHD3();

    inline void reset_extraction() {is_extracted.assign(size,false);}
    void extract_chain(std::vector<int>& chain, int start_hint = 0);

    // Only use after extracting all chains.
    void extract_cycle(std::vector<int>& cycle, int start_hint = 0);

    // returns true if the fork is cyclic. In that case, the cycle will correspond to the first two branches.
    bool extract_fork(std::vector<int> (&fork)[3]);

    void blowup(int a,int b);
    void revert();
public:
    std::vector<std::multiset<int>> adj_list;
    std::set<int> big_degree_nodes;

    // Data for dfs for searching candidates. The first member of the pair corresponds to the amount
    // of blowups done over this curve at this point in the dfs. Used when going back in dfs to revert changes.
    // Second member is the set of pairs not blown up at this point that we have seen already. Used to not explore
    // the same branch again.
    std::stack<std::pair<int,std::set<std::pair<int,int>>>> history;
    std::vector<bool> is_extracted;

    // indicates the index of the central framing in the search recursion. -1 if frame not chosen.
    int frame;

    // indicates the depth in the recursion where we chose the frame. -1 if frame not chosen.
    // The depth in recursion corresponds to history.size().
    int frame_depth;

    std::set<std::tuple<int,int,int>> central_framing_history;

};


#endif
