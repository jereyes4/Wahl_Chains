#include"Searcher.hpp"
#include"Reader.hpp"
#include"Algorithms.hpp"
#include"QHD_functions.hpp"

using std::vector;
using std::pair;

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
        If an end of the fork was connected to an end of the chain, discard the case as it was taken care in 1. or 2.
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

        static thread_local vector<int> fork[3];
        bool cyclic = G.extract_fork(fork);
        if ((!cyclic and fork[0].size() + fork[1].size() + fork[2].size() - 2 == G.size)
            or (cyclic and fork[0].size() + fork[2].size() - 1 == G.size)) {

#ifdef NO_REPEATED_SEARCH
            if (seen.check_and_add(fork)) continue;
#endif
            // Cases 1 and 2.
            if (cyclic) {
                // Case 1

            }
            else {
                // Case 2
                if (contains(G.disconnections[fork[0].back()],fork[1].back())) continue;
                if (contains(G.disconnections[fork[0].back()],fork[2].back())) continue;
                if (contains(G.disconnections[fork[1].back()],fork[2].back())) continue;
            }
        }
        else {
            // Cases 3 - 7
            static thread_local vector<int> chain;
            G.extract_chain(chain);
            if (chain.empty()) {
                // Cases 3,5,7
                static thread_local vector<int> cycle;
                G.extract_chain(cycle);
                if ((!cyclic and fork[0].size() + fork[1].size() + fork[2].size() - 2 == G.size)
                    or (cyclic and fork[0].size() + fork[2].size() - 1 == G.size)) {

#ifdef NO_REPEATED_SEARCH
                    if (seen.check_and_add(fork,cycle)) continue;
#endif
                    // Cases 3,5
                    if (cyclic) {
                        // Case 3
                    }
                    else {
                        // Case 5
                        if (contains(G.disconnections[fork[0].back()],fork[1].back())) continue;
                        if (contains(G.disconnections[fork[0].back()],fork[2].back())) continue;
                        if (contains(G.disconnections[fork[1].back()],fork[2].back())) continue;

                    }
                }
                else {
                    // Case 7
                    continue;
                }
            }
            else if ((!cyclic and fork[0].size() + fork[1].size() + fork[2].size() - 2 == G.size)
                    or (cyclic and fork[0].size() + fork[2].size() - 1 == G.size)) {

#ifdef NO_REPEATED_SEARCH
                if (seen.check_and_add(fork,chain)) continue;
#endif
                // Cases 4,6
                if (cyclic) {
                    // Case 4
                    if (contains(G.disconnections[chain[0]],chain.back())) continue;
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

                }
            }
            else {
                // Case 7
                continue;
            }
        }
    } while(G.next_candidate_QHD3());
}

// void Searcher::explore_QHD3_double_candidate(vector<int> (&fork)[3], int extra_branch_orig, vector<int>& chain) {
    
// }