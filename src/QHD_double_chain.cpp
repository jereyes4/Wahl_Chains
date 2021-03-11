#include"Searcher.hpp"
#include"Reader.hpp"
#include"Algorithms.hpp"
#include"QHD_functions.hpp"

using std::vector;
using std::pair;
using std::unordered_set;

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
                if ((!cyclic and fork[0].size() + fork[1].size() + fork[2].size() - 2 + cycle.size() == G.size)
                    or (cyclic and fork[0].size() + fork[2].size() - 1 + cycle.size() == G.size)) {

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
void Searcher::explore_QHD3_double_candidate(vector<int> (&fork)[3], vector<int>& chain, int extra_id = -1, int extra_n = 0, int extra_orig = -1, int extra_pos = -1) {

    static thread_local vector<int> location;

    location.assign(G.self_int.size(),0);
    int Delta = -3*chain.size() - 1;
    for (int curve : chain) {
        Delta -= G.self_int[curve];
        location[curve] = 1;
    }

    if (Delta < 0) return;

    static thread_local vector<int> reduced_fork[3];
    static thread_local vector<int> reduced_chain;
    static thread_local vector<int> reduced_self_int;
    static thread_local unordered_set<int> ignore;

    if (Delta == 0) {
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
                extra_pos != start and extra_pos != end
                and !contains(G.disconnections[start],fork[0].back())
                and !contains(G.disconnections[start],fork[1].back())
                and !contains(G.disconnections[start],fork[2].back())
                and !contains(G.disconnections[end],fork[0].back())
                and !contains(G.disconnections[end],fork[1].back())
                and !contains(G.disconnections[end],fork[2].back())
            ) {
                static thread_local vector<int> reduced_self_int_2;
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
                extra_pos == start
                or contains(G.disconnections[start],fork[0].back())
                or contains(G.disconnections[start],fork[1].back())
                or contains(G.disconnections[start],fork[2].back())
            ) continue;

            // If there are multiple connections to a curve, we only check that curve once.
            int last_seen = -1;
            for (int A : G.disconnections[end]) if (location[A] == 1 and A != last_seen and A != start) {
                last_seen = A;
                static thread_local vector<int> reduced_self_int_2;
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
                    static thread_local vector<int> reduced_chain_2;
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
            if (start == extra_pos) {
                // Always partial resolution
                continue;
            }
            if (contains(G.disconnections[start],fork[0].back())
                or contains(G.disconnections[start],fork[1].back())
                or contains(G.disconnections[start],fork[2].back())) {
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
                if (A == fork[0].back() or A == fork[1].back() or A == fork[2].back()) {
                    // Skip, since this is a partial resolution.
                    continue;
                }
                static thread_local vector<int> reduced_self_int_2;
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

void Searcher::verify_QHD3_double_candidate(const std::vector<int> (&fork)[3], const std::vector<int>& chain, const std::vector<int>& local_self_int, int extra_n[2], int extra_orig[2], int extra_pos[2], int extra_id = -1) {
    
}