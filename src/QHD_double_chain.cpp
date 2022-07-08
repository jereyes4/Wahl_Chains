#include"Searcher.hpp"
#include"Reader.hpp"
#include"Algorithms.hpp"
#include"QHD_functions.hpp"

using std::vector;
using std::pair;
using std::unordered_set;

constexpr bool ignore_partial_resolution = true;

/*
    Search for one QHD3 fork and a Wahl chain, or two Wahl chains.

    Explore every way in which all but one nodes have degree at most 2, and the remaining one has degree at most 3.
    We call the node with degree 3 a central framing if it exists.
    If there is no central framing, split to normal double chain search.
        Checking for QHD-partial resolutions (and another weird corner case involving (-2) branches in types
        is easier from double chain, so it can split back from there.
    The connected component containing the central framing is called a prefork.
    If the prefork is acyclic, we call it a fork.

    After blowing up, there are several possibilities.

    1. Graph is a single cyclic prefork:
        Blowup every possible intersection in the cycle inside the prefork to make a fork.
            If the intersection blown up included the central framing, blowup again over the (-1) as to append a (-2) to the missing branch.
            Test for QHD-partial resolution.
            Blowup every possible intersection of the second and third branch.
                Do not blow up the central framing if it was done already.
                Test for QHD + Wahl.

    2. Graph is a single fork:
        If an end of the fork was connected to another, discard the case, as it was taken care in 1.
        Test for QHD-partial resolution.
        Blowup every possible intersection in the fork.
        If the intersection blown up included the central framing, blowup again over the (-1) as to append a (-2) to the missing branch.
        Test for QHD + Wahl.

    3. Graph is the disjoint union of a cyclic prefork and a cycle:
        Blowup every possible pair of intersections in the cycle and the prefork to make a fork and a chain.
        If some end of the fork was connected to some end of the chain, discard the case, as it was taken care in 1. or 2.
        If the intersection blown up included the central framing, blowup again over the (-1) as to append a (-2) to the missing branch.
        Test for QHD + Wahl.

    4. Graph is the disjoint union of a cyclic prefork and a chain:
        If the ends of the chain were connected, discard the case, as it was taken care in 3.
        Blowup every possible intersection of the cycle in the prefork to make a fork.
        If an end of the fork was connected to an end of the chain, but no (-2) was appended at the frame, discard the case as it was taken care in 1. or 2.
        If the intersection blown up included the central framing, blowup again over the (-1) as to append a (-2) to the missing branch.
        Test for QHD + Wahl.

    5. Graph is the disjoint union of a fork and a cycle:
        If some end of the fork was connected to another, discard the case as it was taken care in 3.
        Blowup every possible intersection of the cycle to make a chain.
        If an end of the chain was connected to an end of the fork, discard the case, as it was taken care in 1. or 2.
        Otherwise, test for QHD + Wahl.

    6. Graph is the disjoint union of a fork and a chain:
        If some of the five ends was connected to another, discard the case as it was taken care in 1,2,3,4 or 5.
        Otherwise, test for QHD + Wahl.

    7. Otherwise, the graph has at least 3 components, discard.

    There may be many repetitions anyways.

    1. [x]
    2. [x]
    3. [x]
    4. [x]
    5. [x]
    6. [x]
*/

void Searcher::search_for_QHD3_double_chain() {

#ifdef NO_REPEATED_SEARCH
    seen.reset(G.size);
#endif

    G.begin_search();
    do {
        if (G.frame == -1) {
            if (reader_copy.search_double_chain) {
                search_for_double_chain_inner_loop<true,true>();
            }
            else {
                search_for_double_chain_inner_loop<false,true>();
            }
            continue;
        }
        G.reset_extraction();

        THREAD_STATIC vector<int> fork[3];
        THREAD_STATIC vector<int> chain;

        bool cyclic = G.extract_fork(fork);
        if ((!cyclic and fork[0].size() + fork[1].size() + fork[2].size() - 2 == G.size)
            or (cyclic and fork[0].size() + fork[2].size() - 1 == G.size)) {

#ifdef NO_REPEATED_SEARCH
            if (seen.check_and_add(fork)) continue;
#endif
            // Cases 1 and 2.
            chain.clear();
            if (cyclic) {

                // Case 1

                /*
                Start with fork[0] full and fork[1] empty. and sequentially pass curves from the first to the latter.
                At each point:
                    Test for partial resolution
                    Iterate through fork[0] and fork[2] to make a new chain.
                Be careful with adding (-2) curves to the central framing. There must be at most one.
                */

                fork[1].resize(1);
                const int new_index = G.size;
                while(fork[0].size() > 0) {
                    int which_branch_extra = -1;
                    int extra_n = 0;
                    int extra_id = -1;
                    int extra_orig = -1;
                    int extra_pos = -1;
                    G.blowup(fork[0].back(),fork[1].back());
                    if (fork[1].size() == 1) {
                        which_branch_extra = 1;
                        extra_n = 1;
                        extra_id = new_index;
                        extra_orig = fork[0][0];
                        extra_pos = fork[0].back();
                        fork[1].emplace_back(new_index);
                        G.self_int.emplace_back(-2);
                        G.self_int[fork[0].back()]--;
                    }
                    else if (fork[0].size() == 1) {
                        which_branch_extra = 0;
                        extra_n = 1;
                        extra_id = new_index;
                        extra_orig = fork[0][0];
                        extra_pos = fork[1].back();
                        fork[0].emplace_back(new_index);
                        G.self_int.emplace_back(-2);
                        G.self_int[fork[1].back()]--;
                    }
                    explore_QHD3_partial_resolution(fork,extra_id,extra_n,extra_orig,extra_pos);

                    // Extract chain from fork[2]
                    while (fork[2].size() > 2) {
                        chain.emplace_back(fork[2].back());
                        fork[2].pop_back();
                        G.blowup(chain.back(),fork[2].back());

                        /*
                        TODO: verify this didn't come from some other cycle
                        */

                       explore_QHD3_double_candidate(fork,chain,extra_id,extra_n,extra_orig,extra_pos);
                       G.revert();
                    }

                    if (!extra_n) {
                        // blowup at the end: append a -2 to the central framing.
                        chain.emplace_back(fork[2].back());
                        fork[2].pop_back();
                        G.blowup(chain.back(),fork[2].back());

                        fork[2].emplace_back(new_index);
                        G.self_int.emplace_back(-2);
                        G.self_int[chain.back()]--;
                        explore_QHD3_double_candidate(fork,chain,new_index,1,fork[0][0],chain.back());
                        G.self_int[chain.back()]++;
                        G.self_int.pop_back();
                        fork[2].pop_back();

                        G.revert();
                    }

                    // insert the elements from the chain back into the fork
                    fork[2].insert(fork[2].end(),chain.rbegin(),chain.rend());
                    chain.clear();

                    // Extract chain from fork[0]
                    while (fork[0].size() > 2) {
                        chain.emplace_back(fork[0].back());
                        fork[0].pop_back();
                        G.blowup(chain.back(),fork[0].back());

                        /*
                        TODO: verify this didn't come from some other cycle
                        */

                       explore_QHD3_double_candidate(fork,chain,extra_id,extra_n,extra_orig,extra_pos);
                       G.revert();
                    }

                    if (!extra_n) {
                        // blowup at the end: append a (-2) to the central framing.
                        chain.emplace_back(fork[0].back());
                        fork[0].pop_back();
                        G.blowup(chain.back(),fork[0].back());

                        fork[0].emplace_back(new_index);
                        G.self_int.emplace_back(-2);
                        G.self_int[chain.back()]--;
                        explore_QHD3_double_candidate(fork,chain,new_index,1,fork[0][0],chain.back());
                        G.self_int[chain.back()]++;
                        G.self_int.pop_back();
                        fork[0].pop_back();

                        G.revert();
                    }

                    // insert the elements from the chain back into the fork
                    fork[0].insert(fork[0].end(),chain.rbegin(),chain.rend());
                    chain.clear();

                    // undo extra blowup at central frame if there was one
                    if (extra_n) {
                        G.self_int[extra_pos]++;
                        G.self_int.pop_back();
                        fork[which_branch_extra].pop_back();
                    }
                    G.revert();
                    fork[1].emplace_back(fork[0].back());
                    fork[0].pop_back();
                }
            }
            else {

                // Case 2
                if (contains(G.disconnections[fork[0].back()],fork[1].back())) continue;
                if (contains(G.disconnections[fork[0].back()],fork[2].back())) continue;
                if (contains(G.disconnections[fork[1].back()],fork[2].back())) continue;

                explore_QHD3_partial_resolution(fork);

                const int new_index = G.size;
                for (int branch = 0; branch < 3; ++branch) {
                    while(fork[branch].size() > 2) {
                        chain.emplace_back(fork[branch].back());
                        fork[branch].pop_back();
                        G.blowup(chain.back(),fork[branch].back());

                        /*
                        TODO: verify this didn't come from some other cycle
                        */

                       explore_QHD3_double_candidate(fork,chain);
                       G.revert();
                    }

                    // blowup at the end: append a -2 to the central framing.
                    chain.emplace_back(fork[branch].back());
                    fork[branch].pop_back();
                    G.blowup(chain.back(),fork[branch].back());

                    fork[branch].emplace_back(new_index);
                    G.self_int.emplace_back(-2);
                    G.self_int[chain.back()]--;
                    explore_QHD3_double_candidate(fork,chain,new_index,1,fork[0][0],chain.back());
                    G.self_int[chain.back()]++;
                    G.self_int.pop_back();
                    fork[branch].pop_back();

                    G.revert();

                    // insert the elements from the chain back into the fork
                    fork[branch].insert(fork[branch].end(),chain.rbegin(),chain.rend());
                    chain.clear();
                }
            }
        }
        else {
            // Cases 3 - 7
            G.extract_chain(chain);
            if (chain.empty()) {
                // Cases 3,5,7
                THREAD_STATIC vector<int> cycle;
                G.extract_chain(cycle);
                if ((!cyclic and fork[0].size() + fork[1].size() + fork[2].size() - 2 + cycle.size() == G.size)
                    or (cyclic and fork[0].size() + fork[2].size() - 1 + cycle.size() == G.size)) {

#ifdef NO_REPEATED_SEARCH
                    if (seen.check_and_add(fork,cycle)) continue;
#endif
                    // Cases 3,5
                    if (cyclic) {

                        // Case 3

                        // new index for the possible extra (-2) curve.
                        const int new_index = G.size;
                        for (int i = 0; i < cycle.size(); ++i) {
                            int a = cycle[i];
                            int b = cycle[(i+1)%cycle.size()];

                            if (
                                contains(G.disconnections[a],fork[2].back())
                                or contains(G.disconnections[b],fork[2].back())
                            ) {
                                // Taken care in case 1 or 2.
                                continue;
                            }

                            G.blowup(a,b);
                            // blowup at a.b, chain starts at b.
                            for (int j = 0; j < cycle.size(); ++j) {
                                chain.push_back(cycle[(i+j+1)%cycle.size()]);
                            }

                            {
                                // start with fork[0] full and fork[1] empty (except for frame), and sequentially pass curves to it
                                fork[1].resize(1);
                                G.blowup(fork[0].back(),fork[1].back());

                                fork[1].emplace_back(new_index);
                                G.self_int.emplace_back(-2);
                                G.self_int[fork[0].back()]--;
                                explore_QHD3_double_candidate(fork,chain,new_index,1,fork[0][0],fork[0].back());
                                G.self_int[fork[0].back()]++;
                                G.self_int.pop_back();
                                fork[1].pop_back();

                                G.revert();

                                while(fork[0].size() > 2) {
                                    fork[1].emplace_back(fork[0].back());
                                    fork[0].pop_back();
                                    G.blowup(fork[0].back(),fork[1].back());
                                    if (
                                        !contains(G.disconnections[fork[0].back()],chain[0])
                                        and !contains(G.disconnections[fork[0].back()],chain.back())
                                        and !contains(G.disconnections[fork[1].back()],chain[0])
                                        and !contains(G.disconnections[fork[1].back()],chain.back())
                                    ) {
                                        explore_QHD3_double_candidate(fork,chain);
                                    }
                                    G.revert();
                                }

                                fork[1].emplace_back(fork[0].back());
                                fork[0].pop_back();
                                // fork[0] now only has the frame

                                G.blowup(fork[0].back(),fork[1].back());

                                fork[0].emplace_back(new_index);
                                G.self_int.emplace_back(-2);
                                G.self_int[fork[1].back()]--;
                                explore_QHD3_double_candidate(fork,chain,new_index,1,fork[0][0],fork[1].back());
                                G.self_int[fork[1].back()]++;
                                G.self_int.pop_back();
                                fork[0].pop_back();

                                G.revert();
                            }

                            G.revert();
                            chain.resize(0);
                        }
                    }
                    else {

                        // Case 5
                        if (contains(G.disconnections[fork[0].back()],fork[1].back())) continue;
                        if (contains(G.disconnections[fork[0].back()],fork[2].back())) continue;
                        if (contains(G.disconnections[fork[1].back()],fork[2].back())) continue;

                        for (int i = 0; i < cycle.size(); ++i) {
                            int a = cycle[i];
                            int b = cycle[(i+1)%cycle.size()];

                            if (
                                contains(G.disconnections[a],fork[0].back())
                                or contains(G.disconnections[a],fork[1].back())
                                or contains(G.disconnections[a],fork[2].back())
                                or contains(G.disconnections[b],fork[0].back())
                                or contains(G.disconnections[b],fork[1].back())
                                or contains(G.disconnections[b],fork[2].back())
                            ) {
                                // Taken care in case 1 or 2.
                                continue;
                            }

                            G.blowup(a,b);
                            // blowup at a.b, chain starts at b.
                            for (int j = 0; j < cycle.size(); ++j) {
                                chain.push_back(cycle[(i+j+1)%cycle.size()]);
                            }
                            explore_QHD3_double_candidate(fork,chain);
                            G.revert();
                            chain.resize(0);
                        }
                    }
                }
                else {
                    // Case 7
                    continue;
                }
            }
            else if ((!cyclic and fork[0].size() + fork[1].size() + fork[2].size() - 2 + chain.size() == G.size)
                    or (cyclic and fork[0].size() + fork[2].size() - 1 + chain.size() == G.size)) {

#ifdef NO_REPEATED_SEARCH
                if (seen.check_and_add(fork,chain)) continue;
#endif
                // Cases 4,6
                if (cyclic) {

                    // Case 4
                    if (contains(G.disconnections[chain[0]],chain.back())) continue;
                    if (contains(G.disconnections[chain[0]],fork[2].back())) continue;
                    if (contains(G.disconnections[chain.back()],fork[2].back())) continue;

                    // start with fork[0] full and fork[1] empty (except for frame), and sequentially pass curves to it
                    fork[1].resize(1);
                    G.blowup(fork[0].back(),fork[1].back());

                    // new index for the (-2) curve.
                    int new_index = G.size;
                    fork[1].emplace_back(new_index);
                    G.self_int.emplace_back(-2);
                    G.self_int[fork[0].back()]--;
                    explore_QHD3_double_candidate(fork,chain,new_index,1,fork[1][0],fork[0].back());
                    G.self_int[fork[0].back()]++;
                    G.self_int.pop_back();
                    fork[1].pop_back();

                    G.revert();

                    while(fork[0].size() > 2) {
                        fork[1].emplace_back(fork[0].back());
                        fork[0].pop_back();
                        G.blowup(fork[0].back(),fork[1].back());
                        if (
                            !contains(G.disconnections[fork[0].back()],chain[0])
                            and !contains(G.disconnections[fork[0].back()],chain.back())
                            and !contains(G.disconnections[fork[1].back()],chain[0])
                            and !contains(G.disconnections[fork[1].back()],chain.back())
                        ) {
                            explore_QHD3_double_candidate(fork,chain);
                        }
                        G.revert();
                    }

                    fork[1].emplace_back(fork[0].back());
                    fork[0].pop_back();
                    // fork[0] now only has the frame

                    G.blowup(fork[0].back(),fork[1].back());

                    fork[0].emplace_back(new_index);
                    G.self_int.emplace_back(-2);
                    G.self_int[fork[1].back()]--;
                    explore_QHD3_double_candidate(fork,chain,new_index,1,fork[0][0],fork[1].back());
                    G.self_int[fork[1].back()]++;
                    G.self_int.pop_back();
                    fork[0].pop_back();

                    G.revert();
                }
                else {

                    // Case 6
                    if (contains(G.disconnections[fork[0].back()],fork[1].back())) continue;
                    if (contains(G.disconnections[fork[0].back()],fork[2].back())) continue;
                    if (contains(G.disconnections[fork[1].back()],fork[2].back())) continue;
                    if (contains(G.disconnections[fork[0].back()],chain[0])) continue;
                    if (contains(G.disconnections[fork[1].back()],chain[0])) continue;
                    if (contains(G.disconnections[fork[2].back()],chain[0])) continue;
                    if (contains(G.disconnections[fork[0].back()],chain.back())) continue;
                    if (contains(G.disconnections[fork[1].back()],chain.back())) continue;
                    if (contains(G.disconnections[fork[2].back()],chain.back())) continue;

                    explore_QHD3_double_candidate(fork,chain);
                }
            }
            else {
                // Case 7
                continue;
            }
        }
    } while(G.next_candidate_QHD3());
}


void Searcher::get_fork_from_one_chain_for_double(const vector<int>& chain) {

    if constexpr (ignore_partial_resolution) return;

    // Actually this is (almost) the same as the one for single chain.

    THREAD_STATIC vector<int> fork[3];

    // Add a new curve with new id corresponding to the new (-2) we will add
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
                explore_QHD3_partial_resolution(fork,new_id,1,frame_cand,other);
                G.self_int[other]++;
            }
        }
    }

    G.self_int.pop_back();
}


void Searcher::get_fork_from_two_chains_for_double(vector<int> (&chain)[2]) {

    // Actually this is quite similar to the one for single chain.

    THREAD_STATIC vector<int> fork[3];

    // Add a new curve with new id corresponding to the new (-2) we will add
    const int new_id = G.size;
    G.self_int.emplace_back(-2);
    fork[0].resize(2);
    fork[0][1] = new_id;
    for (int chain_index = 0; chain_index < 2; ++chain_index) {
        for (int frame_index = 1; frame_index < chain[chain_index].size() - 1; ++frame_index) {
            int frame_cand = chain[chain_index][frame_index];
            if (!G.disconnections[frame_cand].empty()) {
                fork[0][0] = frame_cand;
                fork[1].resize(frame_index+1);
                fork[2].resize(chain[chain_index].size() - frame_index);
                for (int i = 0; i <= frame_index; ++i) {
                    fork[1][frame_index - i] = chain[chain_index][i];
                }
                for (int i = frame_index; i < chain[chain_index].size(); ++i) {
                    fork[2][i - frame_index] = chain[chain_index][i];
                }
                for (int other : G.disconnections[frame_cand]) {
                    if (other == chain[0][0] or other == chain[0].back()) continue;
                    if (other == chain[1][0] or other == chain[1].back()) continue;
                    G.self_int[other]--;
                    explore_QHD3_double_candidate(fork,chain[1-chain_index],new_id,1,frame_cand,other);
                    G.self_int[other]++;
                }
            }
        }
    }

    G.self_int.pop_back();
}


/*
    This is similar to the double chain algorithm, but quite a bit simpler.
    Since all borders of the fork are (-3) or lower, we can only append (-2) curves the chain.

    First calculate Delta = - 3*L - 1 - S, where L is the length of the chain and S is the sum of
    its self intersections.

    If Delta < 0, there is nothing to do, ignore the test case.
    If Delta = 0:
        Reduce the fork and test for QHD, if not QHD skip the case.
        Afterwards, treat the chain as a single chain for testing.
    If Delta > 0:
        We need to append exactly Delta (-2) curves at the end of the chain by using a
        connection to the fork. Therefore, reduce the chain, add Delta (-2) at each end.
        If Wahl, for each connection from the given end to the fork, substract Delta from
        the self intersection of that curve in the fork, reduce the fork and test for QHD.

*/

void Searcher::explore_QHD3_double_candidate(vector<int> (&fork)[3], vector<int>& chain, int extra_id, int extra_n, int extra_orig, int extra_pos) {

    THREAD_STATIC vector<int> location;

    location.assign(G.self_int.size(),0);
    int Delta = -3*chain.size() - 1;
    for (int curve : chain) {
        Delta -= G.self_int[curve];
        location[curve] = 1;
    }

    if (Delta < 0) return;

    THREAD_STATIC vector<int> reduced_fork[3];
    THREAD_STATIC vector<int> reduced_chain;
    THREAD_STATIC vector<int> reduced_self_int;
    THREAD_STATIC unordered_set<int> ignore;

    if (Delta == 0) {
        reduced_self_int = G.self_int;
        bool admissible = algs::reduce(fork[0],reduced_self_int,reduced_fork[0],ignore)
                        && algs::reduce(fork[1],reduced_self_int,reduced_fork[1],ignore)
                        && algs::reduce(fork[2],reduced_self_int,reduced_fork[2],ignore);
        if (!admissible) return;
        if (algs::get_QHD_type(reduced_fork,reduced_self_int).type == algs::QHD_Type::none) return;

        // First try no blowups.
        {
            const int start = chain[0];
            const int end = chain.back();
            // Ignore the cases when this can be obtained by partial resolution
            if (
                ignore_partial_resolution
                or (extra_pos != start and extra_pos != end
                    and !contains(G.disconnections[start],fork[0].back())
                    and !contains(G.disconnections[start],fork[1].back())
                    and !contains(G.disconnections[start],fork[2].back())
                    and !contains(G.disconnections[end],fork[0].back())
                    and !contains(G.disconnections[end],fork[1].back())
                    and !contains(G.disconnections[end],fork[2].back())
                )
            ) {
                THREAD_STATIC vector<int> reduced_self_int_2;
                reduced_self_int_2 = reduced_self_int;
                admissible = algs::reduce(chain,reduced_self_int_2,reduced_chain,ignore);
                if (admissible) {
                    int _extra_n[2] = {extra_n,0};
                    int _extra_orig[2] = {extra_orig,-1};
                    int _extra_pos[2] = {extra_pos,-1};
                    verify_QHD3_double_candidate(reduced_fork,reduced_chain,reduced_self_int_2,_extra_n,_extra_orig,_extra_pos,extra_id);
                }
            }
        }

        // This is a modified version of the single chain main explore section
        for (int rev = 2; rev--; std::reverse(chain.begin(),chain.end())) {
            if (reduced_self_int[chain[0]] >= -2) continue;
            const int start = chain[0];
            const int end = chain.back();

            // Cases obtained by partial resolution
            if (
                !ignore_partial_resolution
                and (extra_pos == start
                    or contains(G.disconnections[start],fork[0].back())
                    or contains(G.disconnections[start],fork[1].back())
                    or contains(G.disconnections[start],fork[2].back())
                )
            ) continue;

            // If there are multiple connections to a curve, we only check that curve once.
            int last_seen = -1;
            for (int A : G.disconnections[end]) if (location[A] == 1 and A != last_seen and A != start) {

                last_seen = A;
                THREAD_STATIC vector<int> reduced_self_int_2;
                reduced_self_int_2 = reduced_self_int;
                ignore.insert(A);
                admissible = algs::reduce(chain,reduced_self_int_2,reduced_chain,ignore);
                ignore.clear();
                if (!admissible) continue;
                if (reduced_self_int[start] == -2) continue;

                // size after reducing chain.
                int local_size = reduced_chain.size();
                int extra_curves = 0;

                // size variable used to give new id's to curves.
                int size = reduced_self_int.size();

                // First case: A^2 > -2. Blowup to get A^2 = -2
                if (reduced_self_int_2[A] > -2) {
                    // Blowup successively
                    while (reduced_self_int_2[A] > -2) {
                        reduced_self_int_2.push_back(-2);

                        // size + extra_curves is the id of the new curve.
                        reduced_chain.push_back(size + extra_curves);

                        reduced_self_int_2[A]--;
                        extra_curves++;
                    }
                    int _extra_n[2] = {extra_n,extra_curves};
                    int _extra_orig[2] = {extra_orig,end};
                    int _extra_pos[2] = {extra_pos,A};
                    verify_QHD3_double_candidate(reduced_fork,reduced_chain,reduced_self_int_2,_extra_n,_extra_orig,_extra_pos,extra_id);
                }

                // Here A^2 <= -2.
                // We now count the amount of (-2)-curves at the end of the chain.
                // If we this amount is strictly lower than -(start)^2 - 2, then blowup enough times to make them equal and check if Wahl.
                // The counting stops if we find A, since it will become less than a (-2) after blowing up.

                int index = reduced_chain.size() - 1;
                int amount = 0;
                while (reduced_self_int_2[reduced_chain[index]] == -2 and reduced_chain[index] != A) {
                    index--;
                    amount++;
                }
                if (amount < -reduced_self_int_2[start] - 2) {
                    while (amount < -reduced_self_int_2[start] - 2) {
                        reduced_self_int_2.push_back(-2);

                        // size + extra_curves is the id of the new curve.
                        reduced_chain.push_back(size + extra_curves);

                        reduced_self_int_2[A]--;
                        extra_curves++;
                        amount++;
                    }
                    int _extra_n[2] = {extra_n,extra_curves};
                    int _extra_orig[2] = {extra_orig,end};
                    int _extra_pos[2] = {extra_pos,A};
                    verify_QHD3_double_candidate(reduced_fork,reduced_chain,reduced_self_int_2,_extra_n,_extra_orig,_extra_pos,extra_id);
                }

                // Last case: If possible, blow down at A enough times to make it a (-1), reduce and check if Wahl.

                if (reduced_self_int_2[A] + extra_curves > -1) {
                    // temporary new chain.
                    THREAD_STATIC vector<int> reduced_chain_2;
                    extra_curves = reduced_self_int_2[A] + extra_curves + 1;

                    reduced_self_int_2.resize(size + extra_curves);
                    reduced_chain.resize(local_size + extra_curves);
                    reduced_self_int_2[A] = -1;

                    admissible = algs::reduce(reduced_chain, reduced_self_int_2, reduced_chain_2, ignore);
                    if (admissible) {
                        int _extra_n[2] = {extra_n,extra_curves};
                        int _extra_orig[2] = {extra_orig,end};
                        int _extra_pos[2] = {extra_pos,A};
                        verify_QHD3_double_candidate(reduced_fork,reduced_chain,reduced_self_int_2,_extra_n,_extra_orig,_extra_pos,extra_id);
                    }
                }
            }
        }
        return;
    }

    if (Delta > 0) {
        reduced_self_int = G.self_int;
        bool admissible = algs::reduce(chain,reduced_self_int,reduced_chain,ignore);
        if (!admissible) return;
        int local_size = reduced_chain.size();
        for (int rev = 2; rev--; std::reverse(reduced_chain.begin(),reduced_chain.end())) {
            if (reduced_self_int[reduced_chain[0]] >= -2) continue;
            const int start = reduced_chain[0];
            const int end = reduced_chain.back();

            // Now we skip those coming from partial resolutions.
            if (!ignore_partial_resolution and start == extra_pos) {
                // Always partial resolution
                continue;
            }
            if (!(ignore_partial_resolution
                    or contains(G.disconnections[start],fork[0].back())
                    and contains(G.disconnections[start],fork[1].back())
                    and contains(G.disconnections[start],fork[2].back()))
                )
            {
                // Always partial resolution
                continue;
            }

            int new_id = G.self_int.size();

            for (int extra = 0; extra < Delta; ++extra) {
                reduced_chain.emplace_back(new_id++);
                reduced_self_int.emplace_back(-2);
                location.emplace_back(1);
            }

            auto invariants = algs::get_wahl_numbers(reduced_chain,reduced_self_int);

#ifdef OVERFLOW_CHECK
            if (invariants.second == -1) {
                *err <<  "Overflow:\n"
                        "   Test: " << current_test << ".\n"
                        "   Chain:";
                for (int x : reduced_chain[0]) *err << ' ' << x;
                *err << '\n';
                location.resize(size);
                continue;
            }
#endif

            if (invariants.first == 0) continue; // Skip since not Wahl

            /*
            When there was an extra blowup at the center of the chain, and the other curve
            at that point was end, and here we are considering a connection with the center,
            we must only check it the second time we see it, since the first of them is invalidated
            thanks to the blowup, and actually corresponds to a partial resolution.
            */
            int last_last = -1;
            int last = -1;
            for (int A : G.disconnections[end]) if (location[A] == 0) {
                if (extra_pos == end and A == fork[0][0]) {
                    if (last_last == A) continue;
                    if (last != A) {
                        last_last = last;
                        last = A;
                        continue;
                    }
                }
                else if (last == A) continue;
                last_last = last;
                last = A;
                if (!ignore_partial_resolution and (A == fork[0].back() or A == fork[1].back() or A == fork[2].back())) {
                    // Skip, since this is a partial resolution.
                    continue;
                }
                THREAD_STATIC vector<int> reduced_self_int_2;
                reduced_self_int_2 = reduced_self_int;
                reduced_self_int_2[A] -= Delta;
                admissible = algs::reduce(fork[0],reduced_self_int_2,reduced_fork[0],ignore)
                            && algs::reduce(fork[1],reduced_self_int_2,reduced_fork[1],ignore)
                            && algs::reduce(fork[2],reduced_self_int_2,reduced_fork[2],ignore);
                if (!admissible) continue;
                int _extra_n[2] = {extra_n,Delta};
                int _extra_orig[2] = {extra_orig,end};
                int _extra_pos[2] = {extra_pos,A};
                verify_QHD3_double_candidate(reduced_fork,reduced_chain,reduced_self_int_2,_extra_n,_extra_orig,_extra_pos,extra_id);
            }
        }
        return;
    }
}


void Searcher::explore_QHD3_partial_resolution(std::vector<int> (&fork)[3], int extra_id, int extra_n, int extra_orig, int extra_pos) {
    for (int i = 0; i < 2; ++i) {
        // TODO
    }
}


void Searcher::verify_QHD3_double_candidate(const std::vector<int> (&fork)[3], const std::vector<int>& chain, const std::vector<int>& local_self_int, int extra_n[2], int extra_orig[2], int extra_pos[2], int extra_id) {
    auto QHD_invariants = algs::get_QHD_type(fork,local_self_int);
    if (QHD_invariants.type == algs::QHD_Type::none) return;

    auto chain_invariants = algs::get_wahl_numbers(chain,local_self_int);

#ifdef OVERFLOW_CHECK
    if (chain_invariants.second == -1){
        *err <<  "Overflow:\n"
                "   Test: " << current_test << ".\n"
                "   Chain:";
        for (int x : chain) *err << ' ' << x;
        *err << '\n';
        return;
    }
#endif

    if (chain_invariants.first == 0) return;
    double_QHD_invariant unif_invariants(
        current_K2,
        QHD_invariants.type,
        QHD_invariants.p,
        QHD_invariants.q,
        QHD_invariants.r,
        chain_invariants.first,
        std::min(chain_invariants.second, chain_invariants.first - chain_invariants.second)
    );
    if (reader_copy.keep_first != Reader::no_ and contains(double_QHD_found,unif_invariants)) return;
    THREAD_STATIC vector<long long> discrepancies;
    discrepancies.resize(local_self_int.size(),0);
    long long QHD_denominator = algs::get_QHD_discrepancies(fork,local_self_int,QHD_invariants,discrepancies);
    algs::get_discrepancies(chain_invariants.first,chain_invariants.second,chain,discrepancies);


    THREAD_STATIC vector<int> location;
    location.assign(local_self_int.size(),-1);
    for (int i = 0; i < 3; ++i) for (int curve : fork[i]) location[curve] = 0;
    for (int curve : chain) location[curve] = 1;
    long long n[2] = {QHD_denominator,chain_invariants.first};

    pair<bool,bool> nef_result;
    if (reader_copy.nef_check != Reader::no_) {
        int borders[2] = {extra_id,chain.back()};
        nef_result = double_is_nef(local_self_int,discrepancies,location,n,extra_n,extra_orig,extra_pos,borders);
    }
    if (reader_copy.nef_check == Reader::skip_ and !nef_result.first) {
        return;
    }

    bool effective = false;
    if (reader_copy.effective_check != Reader::no_) {
        effective = double_is_effective(local_self_int,discrepancies,location,n);
    }
    if (reader_copy.effective_check == Reader::skip_ and !effective) {
        return;
    }

    // Example found
    Example example = Example();

    example.test = current_test;
    example.type = Example::Type(Example::QHD_double_a_ + QHD_invariants.type - 1);
    example.K2 = current_K2;
    example.complete_fibers = current_complete_fibers;
    example.used_curves.insert(curve_translate.begin(),curve_translate.end());
    for (int exceptional : reader_copy.K.used_components) {
        if (temp_self_int[exceptional] == INT_MAX) {
            example.blown_down_exceptionals.insert(exceptional);
        }
    }
    example.blowups = G.connections;
    example.n[0] = n[0];
    example.n[1] = n[1];
    example.a[1] = chain_invariants.second;
    example.p = QHD_invariants.p;
    example.q = QHD_invariants.q;
    example.r = QHD_invariants.r;
    for (int i = 0; i < 3; ++i) {
        example.branch_permutation[i] = QHD_invariants.which_branch[i];
    }
    example.extra_n[0] = extra_n[0];
    example.extra_n[1] = extra_n[1];
    example.extra_orig[0] = extra_orig[0];
    example.extra_orig[1] = extra_orig[1];
    example.extra_pos[0] = extra_pos[0];
    example.extra_pos[1] = extra_pos[1];

    example.chain[0].reserve(fork[0].size() + fork[1].size() + fork[2].size());
    for (int i = 0; i < 3; ++i) {
        example.chain[0].insert(example.chain[0].end(),fork[i].begin(),fork[i].end());
    }
    example.chain[1] = chain;
    example.self_int = local_self_int;
    example.discrepancies = std::move(discrepancies);
    example.no_obstruction = current_no_obstruction;
    example.nef = nef_result.first;
    example.nef_warning = nef_result.second;
    example.effective = effective;

    results->push(std::move(example));

    if (reader_copy.keep_first != Reader::no_) {
        double_QHD_found.insert(unif_invariants);
    }
}
