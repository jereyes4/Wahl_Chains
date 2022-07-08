#include "Searcher.hpp"
#include "Reader.hpp"
#include "Algorithms.hpp"
#include<algorithm> // reverse

using std::unordered_set;
using std::vector;
using std::pair;


/*
    First explore every way we can blowup the graph so there are left only chains and cycles.
    There are several cases:
    1. Graph is a connected cycle:
        Blow up at every possible intersection. Test the chain for P-extremal resolutions.
        Blowup again at every possible intersection.
            TODO Some additional optimizations:
            Suppose the new end of the first chain was connected to the end of the second chain, and the new start of the second chain was connected to the start of the first chain. Furthermore suppose (without loss of generality) that the smallest id of all four ends is in the first chain, and in the original cycle it was connected to the end with strictly bigger id of the second chain. Then discard the case, as it was taken care in another instance of 1.
        Test both chains.
    2. Graph is a connected chain:
        If both ends were connected before, discard the case, as it was taken care in 1.
        Test the chain for P-extremal resolutions.
        Blow up at every possible intersection.
            TODO Some additional optimizations:
            If the new end of the first chain was connected to the end of the second chain, and the new start of the second chain was connected to the start of the first chain, then discard this case, as it was taken care in 1.
            If the new end of the first chain was connected to the end of the second chain, and the id of the new start of ot the second chain is lower than the id from the end of the second chain, discard the case, as it was taken care in another instance of 2.
            Analogous for the start of the second chain with the start of the first.
        Test both chains.
    3. Graph is two disjoint cycles:
        Blow up every possible pair of intersections from both chains.
        If some end of the first chain was connected to some end of the second chain, discard the case, as it was taken care in 1. or 2.
        Else, test both chains.
    4. Graph is a cycle and a chain:
        If both ends of the chain were connected, discard the case, as it was taken care in 3.
        Blow up every possible intersection in the cycle. If some end of this new chain was connected to an end of the other chain, discard the case, as it was taken care in 1. or 2.
        Else, test both chains.
    5. Graph is two disjoint chains:
        If some end was connected to some other end of any chain, discard the case, as it was taken care in 1, 2, 3, or 4.
        Else, test both chains.
    6. Otherwise, the graph has at least 3 connected components. Discard.

    With the additional optimizations, there should not be repeated searches.
*/

void Searcher::search_for_double_chain() {

#ifdef NO_REPEATED_SEARCH
    seen.reset(G.size);
#endif

    G.begin_search();
    do { search_for_double_chain_inner_loop(); } while (G.next_candidate());
}

template<bool chain_search, bool QHD_search>
void Searcher::search_for_double_chain_inner_loop() {
    G.reset_extraction();
    THREAD_STATIC vector<int> chain[2];
    THREAD_STATIC vector<int> cycle[2];
    G.extract_chain(chain[0]);
    if (chain[0].empty()) {
        // Graph is only cycles. Cases 1, 3, 6.

        G.extract_cycle(cycle[0]);
        if (cycle[0].size() == G.size) {
            // Case 1.

#ifdef NO_REPEATED_SEARCH
            if (seen.check_and_add(cycle[0])) return;
#endif

            for (int i = 0; i < G.size; ++i) {
                const int a = cycle[0][i];
                const int b = cycle[0][(i+1)%G.size];

                chain[0].resize(0);
                for (int j = 0; j < G.size; ++j) {
                    chain[0].emplace_back(cycle[0][(i+j+1)%G.size]);
                }
                G.blowup(a,b);

                if constexpr (chain_search) {
                    explore_p_extremal_resolution(chain[0]);
                }

                if constexpr (QHD_search) {
                    get_fork_from_one_chain_for_double(chain[0]);
                }

                // We can avoid checking the same pair twice if we only
                // move from chain[0] to chain[1] i times.
                chain[1].resize(0);
                for (int j = 0; j < i; ++j) {
                    chain[1].emplace_back(chain[0].back());
                    chain[0].pop_back();
                    const int c = chain[0].back();
                    const int d = chain[1].back();
                    G.blowup(c,d);
                    if constexpr (chain_search) {
                        explore_double_candidate(chain);
                    }
                    if constexpr (QHD_search) {
                        get_fork_from_two_chains_for_double(chain);
                    }
                    G.revert();
                }

                G.revert();
            }
        }
        else {
            // Cases 3, 6.
            G.extract_cycle(cycle[1], 1);
            if (cycle[0].size() + cycle[1].size() != G.size) {
                // Case 6.
                return;
            }
            // Case 3.

#ifdef NO_REPEATED_SEARCH
            if (seen.check_and_add(cycle[0],cycle[1])) return;
#endif

            for (int i = 0; i < cycle[0].size(); ++i){
                const int a = cycle[0][i];
                const int b = cycle[0][(i+1)%cycle[0].size()];

                chain[0].resize(0);
                G.blowup(a,b);
                for (int j = 0; j < cycle[0].size(); ++j) {
                    chain[0].emplace_back(cycle[0][(i+j+1)%cycle[0].size()]);
                }
                for (int i2 = 0; i2 < cycle[1].size(); ++i2) {
                    const int c = cycle[1][i2];
                    const int d = cycle[1][(i2+1)%cycle[1].size()];

                    if (contains(G.disconnections[a],c) or
                        contains(G.disconnections[a],d) or
                        contains(G.disconnections[b],c) or
                        contains(G.disconnections[b],d)) {
                        continue;
                    }
                    chain[1].resize(0);
                    G.blowup(c,d);
                    for (int j = 0; j < cycle[1].size(); ++j) {
                        chain[1].emplace_back(cycle[1][(i2+j+1)%cycle[1].size()]);
                    }
                    if constexpr (chain_search) {
                        explore_double_candidate(chain);
                    }
                    if constexpr (QHD_search) {
                        get_fork_from_two_chains_for_double(chain);
                    }
                    G.revert();
                }
                G.revert();
            }
        }
    }
    else if (chain[0].size() == G.size) {
        // Case 2.

        if (contains(G.disconnections[chain[0][0]], chain[0].back())) {
            // Taken care in 1.
            return;
        }

#ifdef NO_REPEATED_SEARCH
        if (seen.check_and_add(chain[0])) return;
#endif

        if constexpr (chain_search) {
            explore_p_extremal_resolution(chain[0]);
        }

        if constexpr (QHD_search) {
            get_fork_from_one_chain_for_double(chain[0]);
        }

        chain[1].resize(0);
        for (int i = 1; i < G.size; ++i) {
            chain[1].emplace_back(chain[0].back());
            chain[0].pop_back();
            const int a = chain[0].back();
            const int b = chain[1].back();
            G.blowup(a,b);
            if constexpr (chain_search) {
                explore_double_candidate(chain);
            }
            if constexpr (QHD_search) {
                get_fork_from_two_chains_for_double(chain);
            }
            G.revert();
        }
    }
    else {
        // Cases 4, 5, 6.

        if (contains(G.disconnections[chain[0][0]], chain[0].back())) {
            // Taken care in 3. if this was case 4, or in 4. if this cas case 5.
            return;
        }

        G.extract_chain(chain[1],chain[0][0] + 1);
        if (chain[1].empty()) {
            // Cases 4, 6.

            G.extract_cycle(cycle[1]);
            if (chain[0].size() + cycle[1].size() != G.size) {
                // Case 6.
                return;
            }

            // Case 4.

#ifdef NO_REPEATED_SEARCH
            if (seen.check_and_add(chain[0],cycle[1])) return;
#endif

            for(int i = 0; i < cycle[1].size(); ++i){
                const int a = cycle[1][i];
                const int b = cycle[1][(i+1)%cycle[1].size()];

                if (contains(G.disconnections[a],chain[0][0]) or
                    contains(G.disconnections[a],chain[0].back()) or
                    contains(G.disconnections[b],chain[0][0]) or
                    contains(G.disconnections[b],chain[0].back())) {
                    continue;
                }
                chain[1].resize(0);
                G.blowup(a,b);
                for (int j = 0; j < cycle[1].size(); ++j){
                    chain[1].emplace_back(cycle[1][(i+j+1)%cycle[1].size()]);
                }

                if constexpr (chain_search) {
                    explore_double_candidate(chain);
                }
                if constexpr (QHD_search) {
                    get_fork_from_two_chains_for_double(chain);
                }

                G.revert();
            }
            return;
        }
        else if (chain[0].size() + chain[1].size() != G.size) {
            // Case 6.
            return;
        }
        // Case 5.
        if (contains(G.disconnections[chain[0][0]],chain[1][0]) or
            contains(G.disconnections[chain[0][0]],chain[1].back()) or
            contains(G.disconnections[chain[0].back()],chain[1][0]) or
            contains(G.disconnections[chain[0].back()],chain[1].back()) or
            contains(G.disconnections[chain[1][0]],chain[1].back()))
        {
            // Taken care in 1,2,3 or 4.
            return;
        }

#ifdef NO_REPEATED_SEARCH
        if (seen.check_and_add(chain[0],chain[1])) return;
#endif

        if constexpr (chain_search) {
            explore_double_candidate(chain);
        }
        if constexpr (QHD_search) {
            get_fork_from_two_chains_for_double(chain);
        }
    }
}

template void Searcher::search_for_double_chain_inner_loop<true,true>();
template void Searcher::search_for_double_chain_inner_loop<false,true>();
// template void Searcher::search_for_double_chain_inner_loop<false,false>(); // UNUSED

/*

DOUBLE CHAIN ALGORITHM

There are several cases with two chains. First we assume we already have two distinct chains that should be modified to get Wahl chains.
Let C_0 and C_1 be the chains, L_0 and L_1 their lengths, S_0 and S_1 the sum of their self intersections.
Let Delta_i = -3 L_i - 1 - S_i.
They measure how many (-2) or blowups inside the chain are needed for the chain to satisfy the first Wahl invariant. The difference between (-2) curves appended to C_i and blowups done at a curve in C_i is Delta_i.

Note that Delta_0 + Delta_1 = 0. Therefore, we only consider Delta = Delta_0, as it gives us all the information we want.

By a connection C_i -> C_j we mean a connecting (-1) that we intend to blowup at the intersection with C_j, thus appending a (-2) to C_i. We always assume that a connection is blown up at least once.

By reducing we mean contracting all (-1)-curves in the chain and all subsequent (-1)-curves that appear. If anywhere in the process we go out of bounds (i.e. a (-1) at the border of the chain) or get a non-negative curve, discard the case.

We iterate over all pairs of connections (or sometimes just one connection or none) that begin from the end of both chains.

First case: C_0 -> C_0 and C_1 -> C_1 (or no connection at some or both chains (*))
    In this case, the chains act independently just as single chains.
    Required:
        Delta = 0.
        If we consider a connection C_0 -> C_0, then the begining of C_0 must be a (-3) or lower. Same with C_1.
        (*) If we don't consider a connection C_0 -> C_0, there shouldn't be a connecting (-1) from the an end of C_0 to the begining of C_1 and viceversa.

Second case: C_0 -> C_1 and C_1 -> C_1 (or no connection C_1 -> C_1 (*)) and C_0 -> C_1 doesn't end at the begining of C_1.
    The amount of blowups C_0 -> C_1 is fixed as Delta, which must be positive (the case Delta = 0 is taken care above). Fixing this amount, C_1 acts as a single chain. Also reduce C_0 and check if it is Wahl.
    Required:
        Delta > 0
        The begining of C_0 must be a (-3) or lower.
        C_0 -> C_1 doesn't end at the begining of C_1.
        (*) If we don't consider a connection C_1 -> C_1, there shouldn't be a connecting (-1) from an end of C_1 to the begining of C_0

Third case: C_0 -> C_0 and C_1 -> C_0 (or no connection C_0 -> C_0 (*)) and C_1 -> C_0 doesn't end at the begining of C_0.
    Analogous.
    Required:
        Delta < 0
        The begining of C_1 must be a (-3) or lower.
        C_1 -> C_0 doesn't end at the begining of C_0.
        (*) If we don't consider a connection C_0 -> C_0, there shouldn't be a connecting (-1) from an end of C_0 to the begining of C_1.

Fourth case: C_0 -> C_1 and C_1 -> C_0 and they don't end at the begining of the other.
    Let A be the curve in C_1 that intersect C_0 -> C_1 and B be the curve in C_0 that intersect C_1 -> C_0 The difference between the blowups of C_0 -> C_1 and C_1 -> C_0 should always be Delta, so if anywhere in the following analysis one of them should be non positive, discard that case (as either it is impossible or already taken care of).

    Reduce both chains but don't blowdown A and B yet. Suppose wlog that A^2 - Delta >= B^2.
    There are several non exclusive cases:
        If A^2 > -1, then blowup A enough times as to make it a (-1). Reduce both chains and check if they are Wahl. Subsequently, undo this reduction.
        If A^2 > -2, then blowup A enough times as to make it a (-2). Both chains should be reduced and admissible. Check if they are Wahl.
        In any case, we should blowup A at least enough times to get it as a (-3) or lower. This sets a uniform restriction of (-2)-curves at the end of C_1 given by the begining of C_1. Blowup B enough times as to satisfy that restriction. Both chains should be reduced. Check if they are Wahl.
    Required:
        The begining of both chains must be (-3) or lower.
        C_0 -> C_1 doesn't end at the begining of C_1, and viceversa.

Fifth case: The connection C_0 -> C_1 ends at the begining of C_1
    It shouldn't happen that C_1 -> C_0 ends at the begining of C_0 (if no connection C_1 -> C_0 is considered, then there shouldn't be a connecting (-1) from an end of C_1 to he begining of C_0). This would cause problems with discrepancies, and chains may explode.
    Let A be the curve in C_0 that intersects C_1 -> C_0. Consider the chain [C_0] - [-1] - [C_1] and reduce it, without blowing down A yet.
    There are several non exclusive cases:
        If A^2 > -1, then blowup A enough times as to make it a (-1). Reduce the chain. All possible pairs of Wahl chains are in correspondence with P-extremal resolutions of the singularity given by contracting the chain. It is calculated with the dual continued fraction. Subsequently, undo this reduction.
        If A^2 > -2, then blowup A enough times as to make it a (-2). The chain should be reduced and admissible. Check all P-extremal resolutions.
        In any case, we should blowup A at least enough times to make it a (-3) or lower. The chain is therefore reduced and admissible. This sets a uniform restriction on the amount of (-2)-curves at the end of the chain given by a + b - 3, where a = -D^2, D being the first curve of the chain, and b = max { -B^2 : B != A }. Try all those possibilities and check P-extremal resolutions for all of them.

    In the implementation, the fifth case actually receives the single (possibly partially reduced) chain, and iterates through the connections starting at an end of the chain. We also separate the case when we want to blowup or not.
*/

void Searcher::explore_double_candidate(vector<int> (&chain)[2]) {
    // to which chain belongs a curve.
    THREAD_STATIC vector<int> location;
    // For swapping chains in O(1)
    THREAD_STATIC vector<int> Swap_location;

    const int size = G.size;

    int Delta = -1 - 3*chain[0].size();
    location.assign(size,1);
    Swap_location.assign(size,0);
    for (int c : chain[0]) {
        Delta -= G.self_int[c];
        location[c] = 0;
        Swap_location[c] = 1;
    }

    // Reversed chains for swapping in O(1)
    THREAD_STATIC vector<int> Rev_chain[2];
    Rev_chain[0].assign(chain[0].rbegin(),chain[0].rend());
    Rev_chain[1].assign(chain[1].rbegin(),chain[1].rend());

    THREAD_STATIC vector<int> reduced_chain[2];
    THREAD_STATIC vector<int> reduced_self_int;
    THREAD_STATIC unordered_set<int> ignore;


    if (Delta == 0) { // Case 1.

        // First don't try blowing up chain[0].
        reduced_self_int = G.self_int;

        reduced_chain[1] = chain[1];

        bool admissible = algs::reduce(chain[0],reduced_self_int,reduced_chain[0],ignore);
        if (admissible) {
            verify_first_explore_independent_second(reduced_chain, reduced_self_int, location, 0,-1,-1);
        }


        // Try both un-reversed and reversed
        for (int rev_0 = 2; rev_0--; chain[0].swap(Rev_chain[0])) {
            if (G.self_int[chain[0][0]] >= -2) continue;
            const int start = chain[0][0];
            const int end = chain[0].back();
            // If there are multiple connections to a curve, we only check that curve once.
            int last_seen = -1;
            for (int A : G.disconnections[end]) if (A != last_seen and A != start and location[A] == 0) {
                last_seen = A;
                ignore.insert(A);
                reduced_self_int = G.self_int;
                admissible = algs::reduce(chain[0],reduced_self_int,reduced_chain[0],ignore);
                ignore.clear();
                if (!admissible) continue;
                if (reduced_self_int[start] == -2) continue;

                // size after reducing chain.
                int local_size = reduced_chain[0].size();
                int extra_curves = 0;

                // First case: A^2 > -2. Blowup to get A^2 = -2
                if (reduced_self_int[A] > -2) {
                    // Blowup successively
                    while (reduced_self_int[A] > -2) {
                        reduced_self_int.push_back(-2);

                        // size + extra_curves is the id of the new curve.
                        reduced_chain[0].push_back(size + extra_curves);

                        reduced_self_int[A]--;
                        extra_curves++;
                    }
                    verify_first_explore_independent_second(reduced_chain, reduced_self_int, location, extra_curves, end, A);
                }

                // Here A^2 <= -2.
                // We now count the amount of (-2)-curves at the end of the chain.
                // If we this amount is strictly lower than -(start)^2 - 2, then blowup enough times to make them equal and check if Wahl.
                // The counting stops if we find A, since it will become less than a (-2) after blowing up.

                int index = reduced_chain[0].size() - 1;
                int amount = 0;
                while (reduced_self_int[reduced_chain[0][index]] == -2 and reduced_chain[0][index] != A) {
                    index--;
                    amount++;
                }
                if (amount < -reduced_self_int[start] - 2) {
                    while (amount < -reduced_self_int[start] - 2) {
                        reduced_self_int.push_back(-2);

                        // size + extra_curves is the id of the new curve.
                        reduced_chain[0].push_back(size + extra_curves);

                        reduced_self_int[A]--;
                        extra_curves++;
                        amount++;
                    }
                    verify_first_explore_independent_second(reduced_chain, reduced_self_int, location, extra_curves, end, A);
                }

                // Last case: If possible, blow down at A enough times to make it a (-1), reduce and check if Wahl.

                if (reduced_self_int[A] + extra_curves > -1) {
                    // temporary new chain.
                    THREAD_STATIC vector<int> reduced_chain_2;
                    reduced_chain_2 = std::move(reduced_chain[0]);
                    extra_curves = reduced_self_int[A] + extra_curves + 1;
                    reduced_self_int.resize(local_size + extra_curves);
                    reduced_chain_2.resize(local_size + extra_curves);
                    reduced_self_int[A] = -1;
                    admissible = algs::reduce(reduced_chain_2, reduced_self_int, reduced_chain[0], ignore);
                    if (admissible) {
                        verify_first_explore_independent_second(reduced_chain, reduced_self_int, location, extra_curves, end, A);
                    }
                    reduced_chain[0] = std::move(reduced_chain_2);
                }
            }
        }
    }
    else { // Delta != 0, Case 2,3.

        // We can always assume Delta > 0 by swapping the chains. This reduces the third case to the second case. keep track of swaps since we must return chain as it was before.
        bool swapped = false;
        if (Delta < 0) {
            Delta = -Delta;
            chain[0].swap(chain[1]);
            Rev_chain[0].swap(Rev_chain[1]);
            location.swap(Swap_location);
            swapped = true;
        }

        // Case 2:

        reduced_chain[1] = chain[1];

        for (int i = 2; i--; chain[0].swap(Rev_chain[0])) {
            if (G.self_int[chain[0][0]] >= -2) continue;
            else {
                const int end = chain[0].back();
                reduced_self_int = G.self_int;
                bool admissible = algs::reduce(chain[0],reduced_self_int,reduced_chain[0],ignore);
                if (!admissible) continue;
                if (reduced_self_int[reduced_chain[0][0]] == -2) continue;

                // Every possibility with blow ups will result in the same first chain, so check beforehand if it is Wahl.
                for (int extra_n = 0; extra_n < Delta; ++extra_n) {
                    reduced_chain[0].emplace_back(size + extra_n);
                    reduced_self_int.emplace_back(-2);
                    location.emplace_back(0);
                }

                auto invariants = algs::get_wahl_numbers(reduced_chain[0],reduced_self_int);

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

                if (invariants.first != 0) {
                    int last = -1;
                    for (int A : G.disconnections[end]) if (location[A] == 1) {
                        if (last == A) continue;
                        last = A;
                        reduced_self_int[A] -= Delta;
                        verify_first_explore_independent_second(reduced_chain, reduced_self_int, location, Delta,end, A, true);
                        reduced_self_int[A] += Delta;
                    }
                }
                location.resize(size);
            }
        }
        if (swapped) {
            Delta = -Delta;
            chain[0].swap(chain[1]);
            Rev_chain[0].swap(Rev_chain[1]);
            location.swap(Swap_location);
        }
    }

    // Case 4.

    // Iterate through all connections C_0 -> C_1 and C_1 -> C_0 that don't end at the begining of the other.
    // We must take care that if (end0,A) == (B,end1), then we at least have two connections between them. Since the sets are ordered, we can save the last connection we saw.
    // If there are n connections between A and B, we will be seeing n^2 pairs. To pick only one, we choose the second time we see A, and the first time we see B with that A.

    for (int rev_0 = 2; rev_0--; chain[0].swap(Rev_chain[0])) {
        if (G.self_int[chain[0][0]] >= -2) continue;
        for (int rev_1 = 2; rev_1--; chain[1].swap(Rev_chain[1])) {
            if (G.self_int[chain[1][0]] >= -2) continue;
            int end0 = chain[0].back();
            int end1 = chain[1].back();
            int prev_0 = -1;
            int prev_prev_0 = -1;
            for (int A : G.disconnections[end0]) if (location[A] == 1 and A != chain[1][0] and prev_prev_0 != A) {
                int prev_1 = -1;
                for (int B : G.disconnections[end1]) if (location[B] == 0 and B != chain[0][0]) {
                    int prev_1_temp = prev_1;
                    prev_1 = B;
                    if (B == end0 and A == end1) {
                        if (prev_0 != A) {
                            // We must have seen A only once before, as to verify that there are at least two (-1)-curves between A and B, but not analyse again if there are more.
                            continue;
                        }
                        if (prev_1_temp == B) {
                            // We are seeing this a second time, so skip.
                            continue;
                        }
                        // Here, this is the second time we see A, and the first we see B with that A.
                    }
                    else if (prev_0 == A or prev_1_temp == B) {
                        // (end0,A) != (B,end1) so if we see something repeated, ignore.
                        continue;
                    }
                    reduced_self_int = G.self_int;

                    ignore.insert({A,B});
                    bool admissible = algs::reduce(chain[0],reduced_self_int,reduced_chain[0],ignore)
                                    && algs::reduce(chain[1],reduced_self_int,reduced_chain[1],ignore);
                    ignore.clear();
                    if (!admissible) continue;
                    if (reduced_self_int[reduced_chain[0][0]] == -2 or reduced_self_int[reduced_chain[1][0]] == -2) continue;

                    // self intersections
                    int A2 = reduced_self_int[A];
                    int B2 = reduced_self_int[B];

                    bool swapped = false;
                    if (A2 - Delta < B2) {
                        swapped = true;
                        std::swap(A,B);
                        std::swap(A2,B2);
                        std::swap(end0,end1);
                        Delta = -Delta;
                        reduced_chain[0].swap(reduced_chain[1]);
                        location.swap(Swap_location);
                    }
                    int local_size_0 = reduced_chain[0].size();
                    int local_size_1 = reduced_chain[1].size();

                    // Now A^2 - Delta >= B^2.

                    int extra_n[2];
                    int extra_orig[2] = {end0,end1};
                    int extra_pos[2] = {A,B};
                    // First sub case: A^2 > -2 and we want to make it exactly a (-2). We must do at least max(Delta,0) + 1 blowups over A, so it must be that A^2 - max(Delta,0) > -2.
                    if (A2 > -2 and A2 - Delta > -2) {
                        extra_n[0] = A2 + 2;
                        extra_n[1] = extra_n[0] - Delta;

                        // All the new (-2)'s at both ends
                        reduced_self_int.resize(size + extra_n[0] + extra_n[1],-2);

                        // New id's
                        int index = size;
                        for (int i = 0; i < extra_n[0]; ++i) {
                            reduced_chain[0].emplace_back(index++);
                            location.emplace_back(0);
                        }
                        for (int i = 0; i < extra_n[1]; ++i) {
                            reduced_chain[1].emplace_back(index++);
                            location.emplace_back(1);
                        }
                        reduced_self_int[A] -= extra_n[0];
                        reduced_self_int[B] -= extra_n[1];

                        verify_double_candidate(reduced_chain,reduced_self_int,extra_n,extra_orig,extra_pos);

                        // Undo changes
                        reduced_self_int[A] += extra_n[0];
                        reduced_self_int[B] += extra_n[1];
                        reduced_self_int.resize(size);
                        location.resize(size);
                        reduced_chain[0].resize(local_size_0);
                        reduced_chain[1].resize(local_size_1);
                    }

                    // Now assume that both A and B will end up (-3) or less.
                    // Count the amount of (-2)-curves at the end of both chains.
                    int count0 = 0;
                    int index = local_size_0 - 1;
                    while (reduced_self_int[reduced_chain[0][index]] == -2 and reduced_chain[0][index] != B) {
                        index--;
                        count0++;
                    }
                    int count1 = 0;
                    index = local_size_1 - 1;
                    while (reduced_self_int[reduced_chain[1][index]] == -2 and reduced_chain[1][index] != A) {
                        index--;
                        count1++;
                    }

                    extra_n[0] = -reduced_self_int[reduced_chain[0][0]] - 2 - count0;
                    extra_n[1] = -reduced_self_int[reduced_chain[1][0]] - 2 - count1;

                    // We want several things to happen for this test case:
                    // First, we need to do at least one blowup in both ends, so extra_n > 0 fo both chains
                    // Second, we want A^2 <= -3, so extra_n[0] >= A^2 + 3.
                    // Third, extra_n[0] - extra_n[1] = Delta by definition.

                    if (extra_n[0] > 0 and extra_n[1] > 0 and extra_n[0] >= A2 + 3 and extra_n[0] - extra_n[1] == Delta) {
                        // All the new (-2)'s at both ends
                        reduced_self_int.resize(size + extra_n[0] + extra_n[1],-2);

                        // New id's
                        int index = size;
                        for (int i = 0; i < extra_n[0]; ++i) {
                            reduced_chain[0].emplace_back(index++);
                            location.emplace_back(0);
                        }
                        for (int i = 0; i < extra_n[1]; ++i) {
                            reduced_chain[1].emplace_back(index++);
                            location.emplace_back(1);
                        }
                        reduced_self_int[A] -= extra_n[0];
                        reduced_self_int[B] -= extra_n[1];

                        verify_double_candidate(reduced_chain,reduced_self_int,extra_n,extra_orig,extra_pos);

                        // Undo changes
                        reduced_self_int[A] += extra_n[0];
                        reduced_self_int[B] += extra_n[1];
                        reduced_self_int.resize(size);
                        location.resize(size);
                        reduced_chain[0].resize(local_size_0);
                        reduced_chain[1].resize(local_size_1);
                    }

                    // Now we try and make A^2 = -1.
                    if (A2 > -1 and A2 - Delta > -1) {
                        extra_n[0] = A2 + 1;
                        extra_n[1] = extra_n[0] - Delta;

                        // All the new (-2)'s at both ends
                        reduced_self_int.resize(size + extra_n[0] + extra_n[1],-2);

                        // New id's
                        int index = size;
                        for (int i = 0; i < extra_n[0]; ++i) {
                            reduced_chain[0].emplace_back(index++);
                            location.emplace_back(0);
                        }
                        for (int i = 0; i < extra_n[1]; ++i) {
                            reduced_chain[1].emplace_back(index++);
                            location.emplace_back(1);
                        }
                        reduced_self_int[A] -= extra_n[0];
                        reduced_self_int[B] -= extra_n[1];

                        THREAD_STATIC vector<int> reduced_chain_2[2];
                        reduced_chain_2[0] = std::move(reduced_chain[0]);
                        reduced_chain_2[1] = std::move(reduced_chain[1]);

                        bool admissible = algs::reduce(reduced_chain_2[0],reduced_self_int,reduced_chain[0],ignore);
                        admissible = admissible && algs::reduce(reduced_chain_2[1],reduced_self_int,reduced_chain[1],ignore);

                        if (admissible) {
                            verify_double_candidate(reduced_chain,reduced_self_int,extra_n,extra_orig,extra_pos);
                        }

                        // The only non local stuff that changed is location, so it's the only change we undo.
                        location.resize(size);
                    }

                    // Finally, if swapped, un-swap (only nonlocal stuff).
                    if (swapped) {
                        Delta = -Delta;
                        location.swap(Swap_location);
                        std::swap(A,B);
                        std::swap(end0,end1);
                    }
                }
                prev_prev_0 = prev_0;
                prev_0 = A;
            }
        }
    }
}


void Searcher::verify_first_explore_independent_second(vector<int> (&chain)[2], const vector<int>& local_self_int, const vector<int>& location, int extra_n_0, int extra_orig_0, int extra_pos_0, bool skip_verify) {
    if (!skip_verify) {
        auto invariants = algs::get_wahl_numbers(chain[0],local_self_int);

#ifdef OVERFLOW_CHECK
        if (invariants.second == -1){
            *err <<  "Overflow:\n"
                    "   Test: " << current_test << ".\n"
                    "   Chain:";
            for (int x : chain[0]) *err << ' ' << x;
            *err << '\n';
            return;
        }
#endif

        if (invariants.first == 0) return;
    }

    THREAD_STATIC unordered_set<int> ignore;

    // original chain will keep the contents of chain[1], so we can modify chain[1] without worrying, which will be used as the local reduced_chain of explore_double_candidate.
    THREAD_STATIC vector<int> original_chain;
    THREAD_STATIC vector<int> reduced_self_int;

    original_chain = std::move(chain[1]);

    int extra_n[2] = {extra_n_0,0};
    int extra_orig[2] = {extra_orig_0,-1};
    int extra_pos[2] = {extra_pos_0,-1};

    // the size of the partially modified graph, used to give new id's later.
    const int size = local_self_int.size();

    // first try no blowups. For this we require that no end of chains were connected by (-1)'s.
    if (
        !contains(G.disconnections[original_chain[0]],chain[0][0]) and
        !contains(G.disconnections[original_chain.back()],chain[0].back()) and
        !contains(G.disconnections[original_chain[0]],chain[0][0]) and
        !contains(G.disconnections[original_chain.back()],chain[0].back())
    ) {
        reduced_self_int = local_self_int;
        bool admissible = algs::reduce(original_chain,reduced_self_int,chain[1],ignore);
        if (admissible) {
            verify_double_candidate(chain, reduced_self_int, extra_n, extra_orig, extra_pos);
        }
    }

    for (int rev = 2; rev--; std::reverse(original_chain.begin(),original_chain.end())) {

        const int start = original_chain[0];

        if (local_self_int[start] >= -2) continue;
        // The curve at the begining shouldn't be the one blown up before, or have been connected to some end of chain[0] if there were no blow ups. (Analysis done in P-extremal resolution).
        if (start == extra_pos_0) continue;
        if (!extra_n_0 and (
            contains(G.disconnections[start],chain[0][0]) or
            contains(G.disconnections[start],chain[0].back())
        )) continue;

        const int end = original_chain.back();
        extra_orig[1] = end;

        int last_seen = -1;
        for (int A : G.disconnections[end]) if (A != last_seen and location[A] == 1 and A != start) {
            last_seen = A;
            reduced_self_int = local_self_int;

            ignore.insert(A);
            bool admissible = algs::reduce(original_chain,reduced_self_int,chain[1],ignore);
            ignore.clear();
            if (!admissible) continue;

            extra_pos[1] = A;
            // size after reducing chain.
            int local_size = chain[1].size();
            int extra_curves = 0;

            // First case: A^2 > -2. Blowup to get A^2 = -2
            if (reduced_self_int[A] > -2) {
                // Blowup successively
                while (reduced_self_int[A] > -2) {
                    reduced_self_int.push_back(-2);

                    // size + extra_curves is the id of the new curve.
                    chain[1].push_back(size + extra_curves);

                    reduced_self_int[A]--;
                    extra_curves++;
                }
                extra_n[1] = extra_curves;
                verify_double_candidate(chain, reduced_self_int, extra_n, extra_orig, extra_pos);
            }

            // Here A^2 <= -2.
            // We now count the amount of (-2)-curves at the end of the chain.
            // If we this amount is strictly lower than -(start)^2 - 2, then blowup enough times to make them equal and check if Wahl.
            // The counting stops if we find A, since it will become less than a (-2) after blowing up.

            int index = chain[1].size() - 1;
            int amount = 0;
            while (reduced_self_int[chain[1][index]] == -2 and chain[1][index] != A) {
                index--;
                amount++;
            }
            if (amount < -reduced_self_int[start] - 2) {
                while (amount < -reduced_self_int[start] - 2) {
                    reduced_self_int.push_back(-2);

                    // size + extra_curves is the id of the new curve.
                    chain[1].push_back(size + extra_curves);

                    reduced_self_int[A]--;
                    extra_curves++;
                    amount++;
                }
                extra_n[1] = extra_curves;
                verify_double_candidate(chain, reduced_self_int, extra_n, extra_orig, extra_pos);
            }

            // Last case: If possible, blow down at A enough times to make it a (-1), reduce and check if Wahl.

            if (reduced_self_int[A] + extra_curves > -1) {
                // temporary new chain.
                THREAD_STATIC vector<int> reduced_chain_2;
                reduced_chain_2 = std::move(chain[1]);
                extra_curves = reduced_self_int[A] + extra_curves + 1;

                reduced_self_int.resize(size + extra_curves);
                reduced_chain_2.resize(local_size + extra_curves);
                reduced_self_int[A] = -1;

                admissible = algs::reduce(reduced_chain_2, reduced_self_int, chain[1], ignore);
                if (admissible) {
                    extra_n[1] = extra_curves;
                    verify_double_candidate(chain, reduced_self_int, extra_n, extra_orig, extra_pos);
                }
            }
        }
    }
    chain[1] = std::move(original_chain);
}


void Searcher::explore_p_extremal_resolution(vector<int>& chain) {
    const int size = G.size;

    THREAD_STATIC vector<int> reduced_self_int;
    THREAD_STATIC vector<int> reduced_chain;
    THREAD_STATIC unordered_set<int> ignore;

    // First no blow ups
    if (!contains(G.disconnections[chain[0]],chain.back())) {
        reduced_self_int = G.self_int;
        bool admissible = algs::reduce(chain,reduced_self_int,reduced_chain,ignore);
        if (admissible) {
            verify_p_extremal_resolution(reduced_chain, reduced_self_int, chain, 0, -1 ,-1);
        }
    }

    for (int rev = 2; rev--; std::reverse(chain.begin(),chain.end())) {
        const int start = chain[0];
        const int end = chain.back();
        int last_seen = -1;
        for (int A : G.disconnections[end]) if (A != start) {
            if (last_seen == A) continue;
            last_seen = A;

            reduced_self_int = G.self_int;
            ignore.insert(A);
            bool admissible = algs::reduce(chain,reduced_self_int,reduced_chain,ignore);
            ignore.clear();
            if (!admissible) continue;

            int local_size = reduced_chain.size();
            int extra_curves = 0;

            // First, make A^2 = -2

            if (reduced_self_int[A] > -2) {
                extra_curves = reduced_self_int[A] + 2;

                // Add all (-2)'s
                reduced_self_int.resize(size + extra_curves, -2);

                // The new id's
                int index = size;
                for (int i = 0; i < extra_curves; ++i) {
                    reduced_chain.emplace_back(index++);
                }

                reduced_self_int[A] = -2;
                verify_p_extremal_resolution(reduced_chain, reduced_self_int, chain, extra_curves, end, A);
            }

            int max_2_curves = 0;
            for (int i = 1; i < reduced_chain.size(); ++i) {
                if (reduced_chain[i] != A) {
                    max_2_curves = std::max(max_2_curves, -reduced_self_int[reduced_chain[i]]);
                }
            }
            max_2_curves += -reduced_self_int[reduced_chain[0]] - 3;

            int index = reduced_chain.size() - 1;
            int amount = 0;
            while (reduced_self_int[reduced_chain[index]] == -2 and reduced_chain[index] != A) {
                index--;
                amount++;
            }

            // There are currently 'amount' (-2) curves at the end, and we want to try having all possibilities in [amount + 1, max_2_curves].
            for (int i = amount; i < max_2_curves; ++i) {
                reduced_self_int.emplace_back(-2);
                reduced_chain.emplace_back(size + extra_curves);
                extra_curves++;
                reduced_self_int[A]--;
                verify_p_extremal_resolution(reduced_chain, reduced_self_int, chain, extra_curves, end, A);
            }

            // Try making A a (-1)
            if (reduced_self_int[A] + extra_curves > -1) {
                THREAD_STATIC vector<int> reduced_chain_2;
                extra_curves = reduced_self_int[A] + extra_curves + 1;
                reduced_self_int.resize(size + extra_curves);
                reduced_chain.resize(local_size + extra_curves);
                reduced_self_int[A] = -1;
                bool admissible = algs::reduce(reduced_chain, reduced_self_int, reduced_chain_2, ignore);
                if (admissible) {
                    verify_p_extremal_resolution(reduced_chain_2, reduced_self_int, chain, extra_curves, end, A);
                }
            }
        }
    }
}

void Searcher::verify_double_candidate(const vector<int> (&chain)[2], const vector<int>& local_self_int, int extra_n[2], int extra_orig[2], int extra_pos[2]) {

    std::pair<long long, long long> invariants[2];
    invariants[0] = algs::get_wahl_numbers(chain[0],local_self_int);
    invariants[1] = algs::get_wahl_numbers(chain[1],local_self_int);
    long long n[2] = {invariants[0].first, invariants[1].first};
    long long a[2] = {invariants[0].second, invariants[1].second};


#ifdef OVERFLOW_CHECK
    if (a[0] == -1 or a[1] == -1){
        *err <<  "Overflow:\n"
                "   Test: " << current_test << ".\n"
                "   Chain[0]:";
        for (int x : chain[0]) *err << ' ' << x;
        *err <<  "\n"
                "   Chain[1]:";
        for (int x : chain[1]) *err << ' ' << x;
        *err << '\n';
        return;
    }
#endif

    if (invariants[0].first == 0 or invariants[1].first == 0) return;
    double_invariant unif_invariants(current_K2,n[0],std::min(a[0],n[0]-a[0]),n[1],std::min(a[1],n[1]-a[1]));

    if (n[0] < n[1]) {
        std::swap(std::get<1>(unif_invariants),std::get<3>(unif_invariants));
        std::swap(std::get<2>(unif_invariants),std::get<4>(unif_invariants));
    }

    if (n[0] == n[1] and std::get<2>(unif_invariants) < std::get<4>(unif_invariants)) {
        std::swap(std::get<2>(unif_invariants),std::get<4>(unif_invariants));
    }
    if (reader_copy.keep_first != Reader::no_ and contains(double_found,unif_invariants)) return;

    // Chain is Wahl, and we haven't seen these invariants before.

    THREAD_STATIC vector<long long> discrepancies;
    discrepancies.assign(local_self_int.size(),0);
    algs::get_discrepancies(n[0], a[0], chain[0], discrepancies);
    algs::get_discrepancies(n[1], a[1], chain[1], discrepancies);

    THREAD_STATIC vector<int> location;
    location.assign(local_self_int.size(),-1);
    for (int curve : chain[0]) location[curve] = 0;
    for (int curve : chain[1]) location[curve] = 1;

    pair<bool,bool> nef_result;
    if (reader_copy.nef_check != Reader::no_) {
        int borders[2] = {chain[0].back(),chain[1].back()};
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

    // Found example
    // Local stuff such as discrepancies be std::moved
    Example example = Example();
    example.test = current_test;
    example.type = Example::double_;
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
    example.a[0] = a[0];
    example.a[1] = a[1];
    example.extra_n[0] = extra_n[0];
    example.extra_n[1] = extra_n[1];
    example.extra_orig[0] = extra_orig[0];
    example.extra_orig[1] = extra_orig[1];
    example.extra_pos[0] = extra_pos[0];
    example.extra_pos[1] = extra_pos[1];
    example.chain[0] = chain[0];
    example.chain[1] = chain[1];
    example.self_int = local_self_int;
    example.discrepancies = std::move(discrepancies);
    example.no_obstruction = current_no_obstruction;
    example.nef = nef_result.first;
    example.nef_warning = nef_result.second;
    example.effective = effective;

    results->push(std::move(example));

    if (reader_copy.keep_first != Reader::no_) {
        double_found.insert(unif_invariants);
    }
}

/*
Calculates the dual of chain (with negative numbers).
For every pair of possitions in the dual, add 1 in those possitions and check if the sequence represents zero.
If it does, the invariants of both singularities can be obtained from this dual chain. Construct the sequence representing those Wahl singularities and compare the sequence to the original (non reduced) chain to keep track of non contracted curves.
*/
void Searcher::verify_p_extremal_resolution(const vector<int>& reduced_chain, const vector<int>& reduced_self_int, const vector<int>& chain, int extra_n, int extra_orig, int extra_pos) {

    THREAD_STATIC vector<int> dual_chain;
    THREAD_STATIC vector<pair<int,int>> pairs;

    const auto fraction = algs::to_rational(reduced_chain,reduced_self_int);

#ifdef OVERFLOW_CHECK
    if (fraction.second == -1){
        *err <<  "Overflow:\n"
                "   Test: " << current_test << ".\n"
                "   Chain:";
        for (int x : chain) *err << ' ' << x;
        *err << '\n';
        return;
    }
#endif

    const long long Delta = fraction.first;
    const long long Omega = fraction.second;

    if (Delta == -1) return;

    long long Omega_unif = algs::gcd_invmod(Delta,Omega).second;

    P_extremal_invariant unif_invariants = std::make_tuple(current_K2,Delta,std::min(Omega,Omega_unif));

    if (reader_copy.keep_first != Reader::no_) {
        if (contains(P_extremal_found,unif_invariants)) return;
    }

    algs::to_chain(Delta,Delta - Omega,dual_chain);

    for (int alpha = 1; alpha < dual_chain.size() - 1; ++alpha) {
        dual_chain[alpha]++;
        for (int beta = alpha; beta < dual_chain.size() - 1; ++beta) {
            dual_chain[beta]++;

            auto value = algs::to_rational(dual_chain);

#ifdef OVERFLOW_CHECK
            if (value.second == -1){
                *err <<  "Overflow:\n"
                        "   Test: " << current_test << ".\n"
                        "   Chain:";
                for (int x : dual_chain) *err << ' ' << x;
                *err << '\n';
                continue;
            }
#endif

            if (value.first == 0) {
                pairs.emplace_back(alpha,beta);
            }
            dual_chain[beta]--;
        }
        dual_chain[alpha]--;
    }
    if (pairs.empty()) return;

    // Wormhole conjecture: if the first is nef but the second not, say something!
    bool first_included = false;
    assert(pairs.size() < 3);
    for (int pair_index = 0; pair_index < pairs.size(); ++pair_index) {
        auto& p = pairs[pair_index];
        const int& alpha = p.first;
        const int& beta = p.second;
        // We have alpha, beta (alpha <= beta) such that the dual chain with 1 added to those possitions represets zero.
        // The rational number representing the subchains [start,alpha-1], and [beta+1,end] are precisely (n0,a0) and (n1,a1^-1) respectively, where the representation as chain starts from the (-1)-curve connecting both chains.

        std::pair<long long, long long> invariants[2];
        invariants[0] = algs::to_rational(dual_chain,0,alpha-1);
        invariants[1] = algs::to_rational(dual_chain,beta+1,-1);

        long long n[2] = {invariants[0].first, invariants[1].first};
        long long a[2] = {invariants[0].second, invariants[1].second};

        a[1] = algs::gcd_invmod(n[1],a[1]).second;

        const auto& n0 = n[0];
        const auto& n1 = n[1];
        const auto& a0 = a[0];
        const auto& a1 = a[1];

        int _temp_extra_n[2] = {0,extra_n};
        int _temp_extra_orig[2] = {-1,extra_orig};
        int _temp_extra_pos[2] = {-1,extra_pos};

        // We want the Wahl chains starting from the ends of the big chain, tht is, we construct them with (n0,n0-a0) and (n1,n1-a1)
        THREAD_STATIC vector<int> expected_chain[2];

        algs::to_chain(n0*n0,n0*(n0-a0)-1ll,expected_chain[0]);
        algs::to_chain(n1*n1,n1*(n1-a1)-1ll,expected_chain[1]);

        THREAD_STATIC algs::BlowDownLinkedList bd_linked_list;

        // Use a structue that remembers blowdown and blowup order using the original un-reduced chain, which should be
        bd_linked_list.reset(G.size,chain,G.self_int);

        // Add the curves at the end of the chain
        int start = chain[0];
        int end = chain.back();
        for (int i = 0; i < extra_n; ++i) {
            end = bd_linked_list.add_curve_end(end,-2);
        }
        bd_linked_list.self_int[extra_pos] -= extra_n;

        bd_linked_list.reduce(start);
        bool x = bd_linked_list.compare_forward(start,expected_chain[0],0);
        bool y = bd_linked_list.compare_backward(end,expected_chain[1],1);
        assert(x);
        assert(y);

        THREAD_STATIC vector<int> Wahl_chains[2];

        Wahl_chains[0].reserve(expected_chain[0].size());
        Wahl_chains[1].reserve(expected_chain[1].size());

        bd_linked_list.get_chain_forward(start, Wahl_chains[0],0);
        bd_linked_list.get_chain_backward(end, Wahl_chains[1],1);

        assert(bd_linked_list.next_in_chain[Wahl_chains[0].back()] == bd_linked_list.prev_in_chain[Wahl_chains[1].back()]);

        THREAD_STATIC vector<long long> discrepancies;
        discrepancies.assign(bd_linked_list.size,0);

        // Swap a0 with n0 - a0 to correctly represent the chain starting at the begining.
        a[0] = n[0] - a[0];
        algs::get_discrepancies(n0,a0,Wahl_chains[0],discrepancies);

        // Reverse the second chain for consistency with a[1] and for not messing up the nef check, since we want the tail of (-2)-curves at the end of the chain, not at the begining.
        std::reverse(Wahl_chains[1].begin(),Wahl_chains[1].end());
        algs::get_discrepancies(n1,a1,Wahl_chains[1],discrepancies);

        pair<bool,bool> nef_result(false,false);
        if (reader_copy.nef_check != Reader::no_) {
            int borders[2] = {Wahl_chains[0].back(), Wahl_chains[1].back()};
            nef_result = double_is_nef(bd_linked_list.self_int, discrepancies,
                bd_linked_list.location,n,_temp_extra_n,_temp_extra_orig,_temp_extra_pos,borders);

        }
        if (reader_copy.nef_check == Reader::skip_ and !nef_result.first) {
            if (pair_index == 1 and first_included) {
                // counter example to wormhole conjecture.
                results->back().worm_hole_conjecture_counterexample = true;
            }
            continue;
        }

        bool effective = false;
        if (reader_copy.effective_check != Reader::no_) {
            effective = double_is_effective(bd_linked_list.self_int, discrepancies, bd_linked_list.location, n);
        }
        if (reader_copy.effective_check == Reader::skip_ and !effective) {
            if (pair_index == 1 and first_included) {
                // counter example to wormhole conjecture.
                results->back().worm_hole_conjecture_counterexample = true;
            }
            continue;
        }

        // Found example
        // Local stuff such as Wahl_chain, bd_linked_list can be std::moved
        Example example = Example();
        example.test = current_test;
        example.type = Example::p_extremal_;
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
        example.a[0] = a[0];
        example.a[1] = a[1];
        example.extra_n[0] = _temp_extra_n[0];
        example.extra_n[1] = _temp_extra_n[1];
        example.extra_orig[0] = _temp_extra_orig[0];
        example.extra_orig[1] = _temp_extra_orig[1];
        example.extra_pos[0] = _temp_extra_pos[0];
        example.extra_pos[1] = _temp_extra_pos[1];
        example.chain[0] = std::move(Wahl_chains[0]);
        example.chain[1] = std::move(Wahl_chains[1]);
        example.self_int = std::move(bd_linked_list.self_int);
        example.chain_original = reduced_chain;
        example.self_int_original = reduced_self_int;
        example.discrepancies = std::move(discrepancies);
        example.Delta = Delta;
        example.Omega = Omega;
        example.worm_hole = pairs.size() > 1;
        example.worm_hole_conjecture_counterexample = (pair_index == 1 and !first_included);
        example.no_obstruction = current_no_obstruction;
        example.nef = nef_result.first;
        example.nef_warning = nef_result.second;
        example.effective = effective;

        results->push(std::move(example));

        first_included = true;

        if (reader_copy.keep_first != Reader::no_) {
            P_extremal_found.insert(unif_invariants);
        }
    }
    pairs.clear();
}

pair<bool,bool> Searcher::double_is_nef(const std::vector<int>& local_self_int, const std::vector<long long>& discrepancies, const std::vector<int>& location, long long n[2], int extra_n[2], int extra_orig[2], int extra_pos[2], int extra_new_border[2]) {


    // If there were extra blowups, the (-1) curve between the end of the chain and extra_pos will be 'moved' and intersect the new end of the chain.
    // We must 'move' only one of these (-1) curves.
    bool seen_extra0 = !extra_n[0];
    bool seen_extra1 = !extra_n[1];

    for (auto c : G.connections) {

        // This would mean that the curve was blown down. In that case this is a non negative curve, and therefore non problematic if K + D is effective.
        if (local_self_int[c.first] == INT_MAX or local_self_int[c.second] == INT_MAX) continue;

        // If a curve doesn't belong to any chain (for example a (-1)-curve connecting two chains in a P-extremal-resolution), then not nef
        if (location[c.first] == -1 or location[c.second] == -1) return {false, false};

        //one of these connections is originally the blowup done if there was some. In case there were many connections between these curves, only consider the first.
        if (!seen_extra0 and ((c.first == extra_orig[0] and c.second == extra_pos[0]) or (c.first == extra_pos[0] and c.second == extra_orig[0]))) {
            seen_extra0 = true;
            c.first = extra_new_border[0];
            c.second = extra_pos[0];
        }

        if (!seen_extra1 and ((c.first == extra_orig[1] and c.second == extra_pos[1]) or (c.first == extra_pos[1] and c.second == extra_orig[1]))) {
            seen_extra1 = true;
            c.first = extra_new_border[1];
            c.second = extra_pos[1];
        }

        long long d1 = -discrepancies[c.first];
        long long d2 = -discrepancies[c.second];
        long long n1 = n[location[c.first]];
        long long n2 = n[location[c.second]];
        if (d1*n2 + d2*n1 < n1*n2) return {false,false};
    }
    /*
    We now check exceptionals in the original graph.
    If the curve is contracted, ignore it.
    Else if it is marked (i.e. intersects more than one curve at a point):
        If a curve that it intersects gets contracted:
            Issue a nef warning. This is because the contraction increments its self intersection, but if it goes through a node and the node is blown up, it decreases it. If the result were to be a (-1) curve, funny stuff could happen.
        If it doesn't intersect curves that get contracted:
            If E isn't a (-1) ignore.
            If E.D' >= 1 for D' < D the curves we knew it intersected before, then we safely ignore. Either the intersection blows up, so E^2 <= -2, or it doesn't in which case E.D >= 1.
            Else issue a nef warning. This is because if the intersection is not blown up then E.K = -1 and E.D < 1.
    Else if the curve is not marked and a curve it intersects gets contracted, issue a nef warning.
        Same as before.
    Else if the curve is a (-1):
        If E.D >= 1 ignore.
        Else discard this example. K_X not nef.
    Else ignore.

    Calculations are done in common denominator n0*n1
    */
    bool nef_warning = false;
    for (auto& exceptional : reader_copy.K.components) {
        if (temp_self_int[exceptional.id] == INT_MAX) {
            continue;
        }
        if (temp_self_int[exceptional.id] != -1 and temp_marked_exceptional[exceptional.id] != current_test) {
            continue;
        }
        long long disc_sum = 0;
        bool skip_this = false;
        for (int curve : temp_included_curves[exceptional.id]) {
            auto local_curve_iter = curve_dict.find(curve);
            if (local_curve_iter == curve_dict.end()) continue;
            int local_curve_id = local_curve_iter->second;
            if (local_self_int[local_curve_id] == INT_MAX) {
                nef_warning = true;
                skip_this = true;
                break;
            }
            else if (location[local_curve_id] != -1) {
                disc_sum -= discrepancies[local_curve_id]*n[1-location[local_curve_id]];
            }
        }
        if (skip_this) continue;

        // Here there was no contraction.

        if (temp_self_int[exceptional.id] != -1) {
            // marked, no contraction and E^2 != -1
            continue;
        }
        if (disc_sum >= n[0]*n[1]) continue;

        // Here E.D' < 1

        if (temp_marked_exceptional[exceptional.id] == current_test) {
            // marked, no contraction, E^2 = -1 and E.D < -1
            nef_warning = true;
            continue;
        }

        // Here K_X is waranteed not nef
        return {false,false};
    }

    return {true,nef_warning};
}

bool Searcher::double_is_effective(const std::vector<int>& local_self_int, const std::vector<long long>& discrepancies, const std::vector<int>& location, long long n[2]) {

    // Verifies if K + D is effective.
    // For every fiber F that is included completely in the example, calculate d(F) = max{max{d(E) : E in F},1/2}. If the sum of d(F) is <= -1, then K + D is nef.
    // This test doesn't work with funny fibers like IV.
    // Here, all calculations are done in numerators with common denominator 2*n[0]*n[1].
    // I really doubt there is an example with at least two complete fibers which fails this test.
    long long sum = 0;
    for (auto& fiber : reader_copy.fibers) {
        long long minval = n[0]*n[1];
        for (int curve : fiber) {
            auto iter = curve_dict.find(curve);
            if (iter == curve_dict.end()) {
                minval = 0;
                break;
            }
            if (location[iter->second] != 0 and location[iter->second] != 1) {
                // curve is a (-1) connecting both chains in a P-extremal resolution
                minval = 0;
                break;
            }
            if (local_self_int[iter->second] == INT_MAX) {
                // contracted curves are not taken in account for this calculation.
                continue;
            }
            if (location[iter->second] == 0) {
                minval = std::min(minval, -2ll * discrepancies[iter->second]*n[1]);
            }
            else {
                minval = std::min(minval, -2ll * discrepancies[iter->second]*n[0]);
            }
        }
        sum += minval;
        if (sum >= 2ll*n[0]*n[1]) return true;
    }
    return false;
}
