#include"Searcher.hpp"
#include"Reader.hpp"
#include"Algorithms.hpp"
#include"QHD_functions.hpp"

using std::vector;
using std::pair;


/*
    Search for one QHD3 fork, or a Wahl chains.

    Explore every way in which all but one nodes have degree at most 2, and the remaining one has degree at most 3.
    We call the node with degree 3 a central framing if it exists.
    If there is no central framing, split to normal single chain search.
        From single chain search, we can split back to QHD search for a corner case involving short (-2) branches.
    The connected component containing the central framing is called a prefork.
    If the prefork is acyclic, we call it a fork.

    After blowing up, there are several possibilities.

    1. Graph is a single cyclic prefork:
        Blowup every possible intersection in the cycle inside the prefork to make a fork.
        Test for QHD.

    2. Graph is a single fork:
        If an end of the fork was connected to another, discard the case, as it was taken care in 1.
        Test for QHD.

    3. Else, graph is disconnected, ignore.
*/

void Searcher::search_for_QHD3_single_chain() {

#ifdef NO_REPEATED_SEARCH
    seen.reset(G.size);
#endif

    G.begin_search();
    do {
        if (G.frame == -1) {
            if (reader_copy.search_single_chain) {
                search_for_single_chain_inner_loop<true,true>();
            }
            else {
                search_for_single_chain_inner_loop<false,true>();
            }
            continue;
        }
        THREAD_STATIC vector<int> fork[3];
        G.reset_extraction();
        bool cyclic = G.extract_fork(fork);
        if (!cyclic and fork[0].size() + fork[1].size() + fork[2].size() - 2 != G.size) continue;
        if (cyclic and fork[0].size() + fork[2].size() - 1 != G.size) continue;

#ifdef NO_REPEATED_SEARCH
        if (seen.check_and_add(fork)) continue;
#endif

        if (cyclic) {
            // start with fork[0] full and fork[1] empty (except for frame), and sequentially pass curves to it
            fork[1].resize(1);
            G.blowup(fork[0].back(),fork[1].back());

            // new index for the (-2) curve.
            int new_index = G.size;
            fork[1].emplace_back(new_index);
            G.self_int.emplace_back(-2);
            G.self_int[fork[0].back()]--;
            verify_QHD3_single_candidate(fork,new_index,1,fork[1][0],fork[0].back());
            G.self_int[fork[0].back()]++;
            G.self_int.pop_back();
            fork[1].pop_back();

            G.revert();

            while(fork[0].size() > 2) {
                fork[1].emplace_back(fork[0].back());
                fork[0].pop_back();
                G.blowup(fork[0].back(),fork[1].back());
                verify_QHD3_single_candidate(fork);
                G.revert();
            }

            fork[1].emplace_back(fork[0].back());
            fork[0].pop_back();
            // fork[0] now only has the frame

            G.blowup(fork[0].back(),fork[1].back());

            fork[0].emplace_back(new_index);
            G.self_int.emplace_back(-2);
            G.self_int[fork[1].back()]--;
            verify_QHD3_single_candidate(fork,new_index,1,fork[0][0],fork[1].back());
            G.self_int[fork[1].back()]++;
            G.self_int.pop_back();
            fork[0].pop_back();

            G.revert();
        }
        else {
            verify_QHD3_single_candidate(fork);
        }
    } while(G.next_candidate_QHD3());
}

void Searcher::get_fork_from_one_chain_for_single(const vector<int>& chain) {

    // Playing a bit with fire: These changes kinda invalidate G
    // if left unchecked.

    THREAD_STATIC vector<int> fork[3];

    // Add a new curve with new id corresponding to the new (-2) we will be adding
    const int new_id = G.size;
    G.self_int.emplace_back(-2);
    fork[0].resize(2);
    fork[0][1] = new_id;
    for (int frame_index = 1; frame_index < chain.size() - 1; ++frame_index) {
        int frame_cand = chain[frame_index];
        if (!G.disconnections[frame_cand].empty()) {
            fork[0][0] = frame_cand;
            fork[1].resize(frame_index+1);
            fork[2].resize(chain.size() - frame_index);
            for (int i = 0; i <= frame_index; ++i) {
                fork[1][frame_index - i] = chain[i];
            }
            for (int i = frame_index; i < chain.size(); ++i) {
                fork[2][i - frame_index] = chain[i];
            }
            for (int other : G.disconnections[frame_cand]) {
                if (other == chain[0] or other == chain.back()) continue;
                G.self_int[other]--;
                verify_QHD3_single_candidate(fork,new_id,1,frame_cand,other);
                G.self_int[other]++;
            }
        }
    }

    G.self_int.pop_back();
}

void Searcher::verify_QHD3_single_candidate(const vector<int> (&fork)[3], int extra_id, int extra_n, int extra_orig, int extra_pos) {

    THREAD_STATIC std::vector<int> reduced_self_int;
    reduced_self_int = G.self_int;
    THREAD_STATIC std::vector<int> reduced_fork[3];
    THREAD_STATIC std::unordered_set<int> empty_set;
    bool admissible = algs::reduce(fork[0],reduced_self_int,reduced_fork[0],empty_set)
                      && algs::reduce(fork[1],reduced_self_int,reduced_fork[1],empty_set)
                      && algs::reduce(fork[2],reduced_self_int,reduced_fork[2],empty_set);
    if (!admissible) return;
    auto data = algs::get_QHD_type(reduced_fork,reduced_self_int);
    if (data.type == algs::QHD_Type::none) return;

    single_QHD_invariant invariant(current_K2, data.type, data.p,data.q,data.r);
    if (reader_copy.keep_first != Reader::no_ and contains(single_QHD_found,invariant)) return;

    // QHD found and not seen before.
    THREAD_STATIC std::vector<long long> discrepancies;
    discrepancies.assign(reduced_self_int.size(),0);
    long long n = algs::get_QHD_discrepancies(reduced_fork,reduced_self_int,data,discrepancies);

    pair<bool,bool> nef_result = {false,false};
    if (reader_copy.nef_check != Reader::no_) {
        nef_result = single_is_nef(reduced_self_int,discrepancies,n,extra_n,extra_orig,extra_pos,extra_id);
    }
    if (reader_copy.nef_check == Reader::skip_ and !nef_result.first) {
        return;
    }

    bool effective = false;
    if (reader_copy.effective_check != Reader::no_) {
        effective = single_is_effective(reduced_self_int,
            discrepancies,n);
    }
    if (reader_copy.effective_check == Reader::skip_ and !effective) {
        return;
    }

    // Example found
    Example example = Example();
    example.test = current_test;
    example.type = Example::Type(Example::QHD_single_a_ + data.type - 1);
    example.K2 = current_K2;
    example.complete_fibers = current_complete_fibers;
    example.used_curves.insert(curve_translate.begin(),curve_translate.end());
    for (int exceptional : reader_copy.K.used_components) {
        if (temp_self_int[exceptional] == INT_MAX) {
            example.blown_down_exceptionals.insert(exceptional);
        }
    }
    example.blowups = G.connections;
    example.n[0] = n;
    example.p = data.p;
    example.q = data.q;
    example.r = data.r;
    for (int i = 0; i < 3; ++i) {
        example.branch_permutation[i] = data.which_branch[i];
    }
    example.extra_n[0] = extra_n;
    example.extra_orig[0] = extra_orig;
    example.extra_pos[0] = extra_pos;
    example.chain[0].reserve(reduced_fork[0].size() + reduced_fork[1].size() + reduced_fork[2].size());
    for (int i = 0; i < 3; ++i) {
        example.chain[0].insert(example.chain[0].end(),reduced_fork[i].begin(),reduced_fork[i].end());
    }
    example.self_int = std::move(reduced_self_int);
    example.discrepancies = std::move(discrepancies);
    example.no_obstruction = current_no_obstruction;
    example.nef = nef_result.first;
    example.nef_warning = nef_result.second;
    example.effective = effective;

    results->push(std::move(example));

    if (reader_copy.keep_first != Reader::no_) {
        single_QHD_found.insert(invariant);
    }
}
