#include"Graph.hpp"
#include<algorithm> // min, max

using std::vector;
using std::set;
using std::multiset;
using std::pair;
using std::tuple;
using std::min;
using std::max;
using std::make_pair;
using std::make_tuple;

void Graph::reset() {
    size = 0;
    self_int.resize(0);
    adj_list.resize(0);
    connections.resize(0);
    disconnections.resize(0);
    big_degree_nodes.clear();
    frame = -1;
    frame_depth = -1;
    while (!history.empty()) history.pop();
}

void Graph::blowup(int a, int b) {
    if (adj_list[a].find(b) == adj_list[a].end()) return;
    adj_list[a].erase(adj_list[a].find(b));
    adj_list[b].erase(adj_list[b].find(a));
    disconnections[a].insert(b);
    disconnections[b].insert(a);
    self_int[a]--;
    self_int[b]--;
    connections.push_back(make_pair(min(a,b),max(a,b)));
}

void Graph::revert() {
    if (connections.empty()) return;
    auto x = connections.back();
    connections.pop_back();
    adj_list[x.first].insert(x.second);
    adj_list[x.second].insert(x.first);
    disconnections[x.first].erase(disconnections[x.first].find(x.second));
    disconnections[x.second].erase(disconnections[x.second].find(x.first));
    self_int[x.first]++;
    self_int[x.second]++;
}

void Graph::begin_search() {
    while (!big_degree_nodes.empty()) {
        // Selects the next node to lower its degree
        int next_l = *big_degree_nodes.begin();
        history.push({0,set<pair<int,int>> ()});

        //Select the first pair of neighbors to keep, blowup the rest and add this pair to history
        auto iter = adj_list[next_l].begin();
        int a = *(iter++);
        int b = *(iter++);
        history.top().second.insert({a,b});

        //select all vertexes other than a and b. Using temp as to not mess up iterator of the set
        multiset<int> temp;
        while (iter != adj_list[next_l].end()) {
            temp.insert(*(iter++));
        }
        for (int x : temp) {
            //blowup the at the intersection.
            blowup(next_l,x);
            history.top().first++;
            if (adj_list[x].size() == 2) {
                //if x has now degree 2, erase from the set of big degree vertexes
                big_degree_nodes.erase(x);
            }
        }
        //now next_l should have degree 2.
        big_degree_nodes.erase(next_l);
    }
}


bool Graph::next_candidate(){
    // if there's no histoy, we already exhausted all possibilities.
    if (history.empty()) return 0;
    // revert last changes. There are history.top().first of them. If reverting a blowup increases the degree of a vertex up to 3, add it to the big degree set.
    for (int i = 0; i < history.top().first; ++i) {
        auto x = connections.back();
        revert();
        if (adj_list[x.first].size() == 3) {
            big_degree_nodes.insert(x.first);
        }
        if (adj_list[x.second].size() == 3) {
            big_degree_nodes.insert(x.second);
        }
    }
    // afterwards, last_l should be the smallest index in the big degree set, since it was the first one in the set at this point in the stack
    int last_l = *big_degree_nodes.begin();
    int a,b;
    a = -1;
    bool exit_loop = false;
    // we select the next pair we haven't selected before.
    for (auto iter = adj_list[last_l].begin(); iter != adj_list[last_l].end(); ++iter) {
        for (auto iter2 = next(iter); iter2 != adj_list[last_l].end(); ++iter2) {
            if (!contains(history.top().second,make_pair(*iter,*iter2))) {
                a = *iter;
                b = *iter2;
                exit_loop = true;
                break;
            }
        }
        if (exit_loop) break;
    }
    // if we haven't used all pairs
    if (a != -1) {
        // reset the current changes. Add the current pair to history.
        history.top().first = 0;
        history.top().second.insert({a,b});
        // select all vertexes other than a and b. Using temp as to not mess up iterator of the set
        multiset<int> temp(adj_list[last_l]);
        temp.erase(temp.find(a));
        temp.erase(temp.find(b));
        for (int x : temp) {
            // blowup at the intersection.
            blowup(x,last_l);
            history.top().first++;
            if (adj_list[x].size() == 2) {
                // if x has now degree 2, erase from the set of big degree vertexes
                big_degree_nodes.erase(x);
            }
        }
        // last_l now has degree 2.
        big_degree_nodes.erase(last_l);
        // As we are trying a new configuration, all the following possibilities are fresh. Therefore call begin_search.
        begin_search();
        return 1;
    }
    // all pairs are used. Go back in dfs.
    history.pop();
    return next_candidate();
}

void Graph::begin_search_QHD3() {
    while (!big_degree_nodes.empty()) {
        // Selects the next node to lower its degree
        int next_l = *big_degree_nodes.begin();
        history.push({0,set<pair<int,int>> ()});

        // Select the first pair of neighbors to keep, blowup the rest and add this pair to history
        // Must select frame if it is in the set.
        int a,b;
        multiset<int> temp;
        if (frame < 0 or !contains(adj_list[next_l],frame)) {
            auto iter = adj_list[next_l].begin();
            a = *(iter++);
            b = *(iter++);
            history.top().second.emplace(a,b);
            while (iter != adj_list[next_l].end()) {
                temp.insert(*(iter++));
            }
        }
        else {
            int count = adj_list[next_l].count(frame);
            if (count == 2) {
                a = b = frame;
                history.top().second.emplace(a,b);
                temp = adj_list[next_l];
                temp.erase(frame);
            }
            else {
                a = frame;
                b = -1;
                for (auto iter = adj_list[next_l].begin(); iter != adj_list[next_l].end(); ++iter) {
                    if (*iter == frame) continue;
                    if (b == -1) b = *iter;
                    else temp.insert(*iter);
                }
            }
        }
        //select all vertexes other than a and b. Using temp as to not mess up iterator of the set
        for (int x : temp) {
            //blowup the at the intersection.
            blowup(next_l,x);
            history.top().first++;
            if (adj_list[x].size() == 2) {
                //if x has now degree 2, erase from the set of big degree vertexes
                big_degree_nodes.erase(x);
            }
        }
        //now next_l should have degree 2.
        big_degree_nodes.erase(next_l);
    }
}

bool Graph::next_candidate_QHD3(){
    //if there's no histoy, we already exhausted all possibilities.
    if (history.empty()) return 0;
    //revert last changes. There are history.top().first of them. If reverting a blowup increases the degree of a vertex up to 3, add it to the big degree set.
    for (int i = 0; i < history.top().first; ++i) {
        auto x = connections.back();
        revert();
        if (adj_list[x.first].size() == 3) {
            big_degree_nodes.insert(x.first);
        }
        if (adj_list[x.second].size() == 3) {
            big_degree_nodes.insert(x.second);
        }
    }

    if (history.size() != frame_depth) {
        // frame either not chosen or this is not it.

        // last_l should be the smallest index in the big degree set, since it was the first one in the set at this point in the stack
        int last_l = *big_degree_nodes.begin();

        int a,b;
        a = -1;
        bool exit_loop = false;

        if (frame < 0 or !contains(adj_list[last_l],frame)) {
            // we select the next pair we haven't selected before.
            for (auto iter = adj_list[last_l].begin(); iter != adj_list[last_l].end(); ++iter) {
                for (auto iter2 = next(iter); iter2 != adj_list[last_l].end(); ++iter2) {
                    if (!contains(history.top().second,make_pair(*iter,*iter2))) {
                        a = *iter;
                        b = *iter2;
                        exit_loop = true;
                        break;
                    }
                }
                if (exit_loop) break;
            }
        }
        else {
            // We don't wanna blowup an intersection with the frame.
            int count = adj_list[last_l].count(frame);
            if (count == 2) {
                // Only option for pair.
                if (!contains(history.top().second,make_pair(frame,frame))) {
                    a = b = frame;
                }
            }
            else {
                for (auto iter = adj_list[last_l].begin(); iter != adj_list[last_l].end(); ++iter) {
                    if (*iter == frame) continue;
                    if (!contains(history.top().second,make_pair(frame,*iter))) {
                        a = frame;
                        b = *iter;
                    }
                }
            }
        }
        // if we haven't used all pairs
        if (a != -1) {
            // reset the current changes. Add the current pair to history.
            history.top().first = 0;
            history.top().second.insert({a,b});
            // select all vertexes other than a and b. Using temp as to not mess up iterator of the set
            multiset<int> temp(adj_list[last_l]);
            temp.erase(temp.find(a));
            temp.erase(temp.find(b));
            for (int x : temp) {
                // blowup at the intersection.
                blowup(x,last_l);
                history.top().first++;
                if (adj_list[x].size() == 2) {
                    // if x has now degree 2, erase from the set of big degree vertexes
                    big_degree_nodes.erase(x);
                }
            }
            // last_l now has degree 2.
            big_degree_nodes.erase(last_l);
            // As we are trying a new configuration, all the following possibilities are fresh. Therefore call begin_search.
            begin_search_QHD3();
            return 1;
        }

        if (frame == -1) {
            // All pairs are used. If we haven't chosen a framing yet, this is the time to do it.
            // history is still used to kep track of blowups
            frame = last_l;
            frame_depth = history.size();
        }
        else {
            // All pairs used and frame already chosen. Go back in dfs.
            history.pop();
            return next_candidate_QHD3();
        }
    }
    // At this point, we have chosen (the virtual?) last_l as a frame, and central_framing_history should contain all the visited triples.
    int a,b,c;
    a = -1;
    bool exit_loop = false;
    for (auto iter = adj_list[frame].begin(); iter != adj_list[frame].end(); ++iter) {
        for (auto iter2 = next(iter); iter2 != adj_list[frame].end(); ++iter2) {
            for (auto iter3 = next(iter2); iter3 != adj_list[frame].end(); ++iter3) {
                if (*iter3 == *iter) continue; // We shouldn't have two curves intersecting three times.
                if (!contains(central_framing_history,make_tuple(*iter,*iter2,*iter3))) {
                    a = *iter;
                    b = *iter2;
                    c = *iter3;
                }
            }
        }
    }
    if (a != -1) {
        // A triple we haven't seen before.
        history.top().first = 0;
        central_framing_history.emplace(a,b,c);

        // select all vertexes other than a, b and c. Using temp as to not mess up iterator of the set
        multiset<int> temp(adj_list[frame]);
        temp.erase(temp.find(a));
        temp.erase(temp.find(b));
        temp.erase(temp.find(c));
        for (int x : temp) {
            // blowup at the intersection.
            blowup(x,frame);
            history.top().first++;
            if (adj_list[x].size() == 2) {
                // if x has now degree 2, erase from the set of big degree vertexes
                big_degree_nodes.erase(x);
            }
        }
        // frame now has degree 3, but we still take it out from the big degree set, so that we don't see it again later.
        big_degree_nodes.erase(frame);
        // As we are trying a new configuration, all the following possibilities are fresh. Therefore call the normal begin_search.
        begin_search_QHD3();
        return 1;
    }
    // All triples seen. Unselect this curve as frame
    big_degree_nodes.insert(frame);
    frame = -1;
    frame_depth = -1;
    central_framing_history.clear();
    history.pop();
    return next_candidate_QHD3();
}

void Graph::extract_chain(vector<int>& chain, int start_hint) {
    chain.resize(0);
    for (int i = start_hint; i < size; ++i) {
        if (!is_extracted[i] and adj_list[i].size() < 2) {
            // node i is extremal
            is_extracted[i] = true;
            chain.emplace_back(i);
            if (adj_list[i].size() == 0) {
                return;
            }
            // iterate through the chain
            int last = i;
            int current = *adj_list[i].begin();
            while(adj_list[current].size() == 2) {
                is_extracted[current] = true;
                chain.emplace_back(current);
                int next = *adj_list[current].begin();
                if (next == last) next = *adj_list[current].rbegin();
                last = current;
                current = next;
            }
            is_extracted[current] = true;
            chain.emplace_back(current);
            return;
        }
    }
}

void Graph::extract_cycle(vector<int>& cycle, int start_hint) {
    cycle.resize(0);
    for (int i = start_hint; i < size; ++i) {
        if (!is_extracted[i]) {
            // i is the starting point
            is_extracted[i] = true;
            cycle.emplace_back(i);
            // iterate through the cycle
            int last = i;
            int current = *adj_list[i].begin();
            while(current != i) {
                is_extracted[current] = true;
                cycle.emplace_back(current);
                int next = *adj_list[current].begin();
                if (next == last) next = *adj_list[current].rbegin();
                last = current;
                current = next;
            }
            return;
        }
    }
}

bool Graph::extract_fork(vector<int> (&fork)[3]) {
    for (auto& branch : fork) branch.assign(1,frame);
    is_extracted[frame] = true;
    int branch = 0;
    bool cyclic = false;
    int straight_branch = -1;
    for (int index : adj_list[frame]) {
        int last = frame;
        while (index != frame) {
            fork[branch].emplace_back(index);
            is_extracted[index] = true;
            if (adj_list[index].size() == 1) break;
            int next = *adj_list[index].begin();
            if (next == last) next = *adj_list[index].rbegin();
            last = index;
            index = next;
        }
        if (index == frame) cyclic = true;
        else straight_branch = branch;

        branch++;
    }
    if (cyclic) {
        if (straight_branch != 2) fork[2].swap(fork[straight_branch]);
    }
    return cyclic;
}
