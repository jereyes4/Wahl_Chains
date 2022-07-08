#include "Searcher.hpp"
#include "Wahl.hpp"
#include "Algorithms.hpp"
#include<algorithm> // reverse

using std::unordered_set;
using std::vector;
using std::pair;


/*
    First explore every way we can blowup the graph so there are left only chains and cycles.
    If the graph is a connected cycle:
        Try blowing up at every possible intersection and treat the result as a chain to test.
    If the graph is a connected chain:
        If both ends weren't connected to begin with (since the complete cycle would appear somewhere in this analysis)
        Else test.
    In any other case, ignore.

    If searching for QHD, we want to check the case when a branch is generated by exactly one (-2) curve product of a repeated blowup.

    If the graph is a connected cycle:
        Try blowing up at every possible intersection and treat the result as a chain.
        Try every curve in the interior of the chain as a candidate for central framing.
            For each candidate and for every (-1) that connects this candidate and another curve which is not an end of the chain,
            try blowing up one more at that intersection, effectively appending a (-2) curve to the frame candidate.
            Test for QHD.
    If the graph is a connected chain:
        If both ends weren't connected to begin with (since the complete cycle would appear somewhere in this analysis).
        Else, try every curve in the interior of the chain as a candidate for central framing.
            For each candidate and for every (-1) that connects this candidate and another curve which is not an end of the chain,
            try blowing up one more at that intersection, effectively appending a (-2) curve to the frame candidate.
            Test for QHD.
*/
void Searcher::search_for_single_chain() {

#ifdef NO_REPEATED_SEARCH
    seen.reset(G.size);
#endif

    G.begin_search();
    do { search_for_single_chain_inner_loop(); } while (G.next_candidate());
}

template<bool chain_search, bool QHD_search>
void Searcher::search_for_single_chain_inner_loop() {
    G.reset_extraction();
    THREAD_STATIC vector<int> chain;
    THREAD_STATIC vector<int> cycle;
    G.extract_chain(chain);
    if (chain.size() < G.size) {
        if (chain.size() == 0) {
            // There are no chains in G, only cycles.
            G.extract_cycle(cycle);
            if (cycle.size() < G.size) {
                // G is disconnected. Exclude this case.
                return;
            }

#ifdef NO_REPEATED_SEARCH
            if (seen.check_and_add(cycle)) return;
#endif

            for (int i = 0; i < G.size; ++i) {
                int a = cycle[i];
                int b = cycle[(i+1)%G.size];
                G.blowup(a,b);
                // blowup at a.b, chain starts at b.
                for (int j = 0; j < G.size; ++j) {
                    chain.push_back(cycle[(i+j+1)%G.size]);
                }
                if constexpr(chain_search) {
                    explore_single_candidate(chain);
                }
                if constexpr(QHD_search) {
                    get_fork_from_one_chain_for_single(chain);
                }
                G.revert();
                chain.resize(0);
            }
        }
        else {
            // G is disconnected. Exclude this case
            return;
        }
    }
    else {
        if (contains(G.disconnections[chain[0]],chain.back())) {
            // This chain comes from a cycle, so it is already taken care of.
            return;
        }
        else {

#ifdef NO_REPEATED_SEARCH
            if (seen.check_and_add(chain)) return;
#endif

            if constexpr (chain_search) {
                explore_single_candidate(chain);
            }
            if constexpr (QHD_search) {
                get_fork_from_one_chain_for_single(chain);
            }
        }
    }
}

template void Searcher::search_for_single_chain_inner_loop<true,true>();
template void Searcher::search_for_single_chain_inner_loop<false,true>();
// template void Searcher::search_for_single_chain_inner_loop<false,false>(); // UNUSED

/*

SINGLE CHAIN ALGORITHM

Assume we have a chain C which we would like to modify to get a Wahl chain.

First, assume we make no blowups. Reduce the chain and check if it is Wahl.

Then we iterate over all connecting (-1)-curves from the end of the chain to some curve inside the chain. This curve should not be the begining of the chain, as it makes the chains explode. Assume we will blowup this connection at least once.

Since we're gonna add (-2)-curves at the end of the chain, here we are assuming that the start of the chain is a (-3) or less.

Let A be the curve at the other end of the connection. Reduce the chain, but avoid contracting A. There are several non exclusive cases:
    If A^2 > -1, then blowup A enough times as to make it a (-1). Reduce the chain and check if it is Wahl. Subsequently, undo this reduction.

    If A^2 > -2, then blowup A enough times as to make it a (-2). Chain should be reduced and admissible. Check if it is Wahl.

    In any case, we should blowup A at least enough times to get it as a (-3) or lower. This sets a uniform restriction of (-2)-curves at the end of C given by the begining of C. Blowup A enough times as to satisfy that restriction. Chain should be reduced. Check if they are Wahl.

*/
void Searcher::explore_single_candidate(vector<int>& chain) {
    /////////////////////////////////////////////////
    // Temporary stuff for reducing chains

    THREAD_STATIC vector<int> reduced_chain;
    THREAD_STATIC vector<int> reduced_self_int;
    THREAD_STATIC unordered_set<int> ignore;

    // At this point, ignore is assumed to be empty.

    /////////////////////////////////////////////////

    const int size = G.size;

    reduced_self_int = G.self_int;
    bool admissible = algs::reduce(chain,reduced_self_int,reduced_chain,ignore);
    if (admissible) {
        verify_single_candidate(reduced_chain,reduced_self_int);
    }

    // We use the same block twice, but the secondtime with the chain reversed.
    for (int rev = 2; rev--; std::reverse(chain.begin(),chain.end())) {
        if (G.self_int[chain[0]] >= -2) continue;
        const int start = chain[0];
        const int end = chain.back();
        // If there are multiple connections to a curve, we only check that curve once.
        int last_seen = -1;
        for (int A : G.disconnections[end]) if (A != last_seen and A != start) {
            last_seen = A;
            ignore.insert(A);
            reduced_self_int = G.self_int;
            admissible = algs::reduce(chain,reduced_self_int,reduced_chain,ignore);
            ignore.clear();
            if (!admissible) continue;
            if (reduced_self_int[start] == -2) continue;

            // size after reducing chain.
            int local_size = reduced_chain.size();
            int extra_curves = 0;

            // First case: A^2 > -2. Blowup to get A^2 = -2
            if (reduced_self_int[A] > -2) {
                // Blowup successively
                while (reduced_self_int[A] > -2) {
                    reduced_self_int.push_back(-2);

                    // size + extra_curves is the id of the new curve.
                    reduced_chain.push_back(size + extra_curves);

                    reduced_self_int[A]--;
                    extra_curves++;
                }
                verify_single_candidate(reduced_chain, reduced_self_int, extra_curves, end, A);
            }

            // Here A^2 <= -2.
            // We now count the amount of (-2)-curves at the end of the chain.
            // If we this amount is strictly lower than -(start)^2 - 2, then blowup enough times to make them equal and check if Wahl.
            // The counting stops if we find A, since it will become less than a (-2) after blowing up.

            int index = reduced_chain.size() - 1;
            int amount = 0;
            while (reduced_self_int[reduced_chain[index]] == -2 and reduced_chain[index] != A) {
                index--;
                amount++;
            }
            if (amount < -reduced_self_int[start] - 2) {
                while (amount < -reduced_self_int[start] - 2) {
                    reduced_self_int.push_back(-2);

                    // size + extra_curves is the id of the new curve.
                    reduced_chain.push_back(size + extra_curves);

                    reduced_self_int[A]--;
                    extra_curves++;
                    amount++;
                }
                verify_single_candidate(reduced_chain, reduced_self_int, extra_curves, end, A);
            }

            // Last case: If possible, blow down at A enough times to make it a (-1), reduce and check if Wahl.

            if (reduced_self_int[A] + extra_curves > -1) {
                // temporary new chain.
                THREAD_STATIC vector<int> reduced_chain_2;
                extra_curves = reduced_self_int[A] + extra_curves + 1;

                reduced_self_int.resize(size + extra_curves);
                reduced_chain.resize(local_size + extra_curves);
                reduced_self_int[A] = -1;

                admissible = algs::reduce(reduced_chain, reduced_self_int, reduced_chain_2, ignore);
                if (admissible) {
                    verify_single_candidate(reduced_chain_2, reduced_self_int, extra_curves,end,A);
                }
            }
        }
    }
}

void Searcher::verify_single_candidate(const vector<int>& chain, const vector<int>& local_self_int, int extra_n, int extra_orig, int extra_pos) {
    auto invariants = algs::get_wahl_numbers(chain,local_self_int);
    const long long n = invariants.first;
    const long long a = invariants.second;

#ifdef OVERFLOW_CHECK
    if (a == -1){
        *err <<  "Overflow:\n"
                "   Test: " << current_test << ".\n"
                "   Chain:";
        for (int x : chain) *err << ' ' << x;
        *err << '\n';
        return;
    }
#endif

    if (invariants.first == 0) return;
    single_invariant unif_invariants(current_K2,n,std::min(a,n-a));
    if (reader_copy.keep_first != Reader::no_ and contains(single_found,unif_invariants)) return;

    // Chain is Wahl, and we haven't seen these invariants before.
    THREAD_STATIC vector<long long> discrepancies;
    discrepancies.assign(local_self_int.size(),0);
    algs::get_discrepancies(invariants.first, invariants.second, chain, discrepancies);

    pair<bool,bool> nef_result = {false,false};
    if (reader_copy.nef_check != Reader::no_) {
        nef_result = single_is_nef(local_self_int,
            discrepancies,n,extra_n,extra_orig,extra_pos,chain.back());
    }
    if (reader_copy.nef_check == Reader::skip_ and !nef_result.first) {
        return;
    }

    bool effective = false;
    if (reader_copy.effective_check != Reader::no_) {
        effective = single_is_effective(local_self_int,
            discrepancies,n);
    }
    if (reader_copy.effective_check == Reader::skip_ and !effective) {
        return;
    }

    // Example found
    Example example = Example();
    example.test = current_test;
    example.type = Example::single_;
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
    example.a[0] = a;
    example.extra_n[0] = extra_n;
    example.extra_orig[0] = extra_orig;
    example.extra_pos[0] = extra_pos;
    example.chain[0] = chain;
    example.self_int = local_self_int;
    example.discrepancies = std::move(discrepancies);
    example.no_obstruction = current_no_obstruction;
    example.nef = nef_result.first;
    example.nef_warning = nef_result.second;
    example.effective = effective;

    results->push(std::move(example));

    if (reader_copy.keep_first != Reader::no_) {
        single_found.insert(unif_invariants);
    }
}


pair<bool,bool> Searcher::single_is_nef(
    const std::vector<int>& local_self_int,
    const std::vector<long long>& discrepancies, long long n,
    int extra_n, int extra_orig, int extra_pos, int extra_new_border
) {

    // If there were extra blowups, the (-1) curve between the end of the chain and extra_pos will be 'moved' and intersect the new end of the chain.
    // We must 'move' only one of these (-1) curves.
    bool seen_extra = !extra_n;

    for (auto& c : G.connections) {
        //This would mean that the curve was blown down. In that case this is a non negative curve, and therefore non problematic if K + D is effective.
        if (local_self_int[c.first] == INT_MAX or local_self_int[c.second] == INT_MAX) continue;

        //one of these connections is originally the blowup done if there was some. In case there were many connections between these curves, only consider the first.
        if (!seen_extra and ((c.first == extra_orig and c.second == extra_pos) or (c.first == extra_pos and c.second == extra_orig))) {
            seen_extra = true;
            if (-discrepancies[extra_new_border] - discrepancies[extra_pos] < n) {
                // Not nef.
                return {false,false};
            }
        }
        else if (-discrepancies[c.first] - discrepancies[c.second] < n) {
            // Not nef
            return {false,false};
        }
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
            else {
                disc_sum -= discrepancies[local_curve_id];
            }
        }
        if (skip_this) continue;

        // Here there was no contraction.

        if (temp_self_int[exceptional.id] != -1) {
            // marked, no contraction and E^2 != -1
            continue;
        }
        if (disc_sum >= n) continue;

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

bool Searcher::single_is_effective(const vector<int>& local_self_int, const vector<long long>& discrepancies, long long n) {
    // Verifies if K + D is effective.
    // For every fiber F that is included completely in the example, calculate d(F) = max{max{d(E) : E in F},1/2}. If the sum of d(F) is <= -1, then K + D is nef.
    // This test doesn't work with funny fibers like IV.
    // Here, all calculations are done in numerators with common denominator 2n.
    // I really doubt there is an example with at least two complete fibers which fails this test.
    long long sum = 0;
    for (auto& fiber : reader_copy.fibers) {
        long long minval = n;
        for (int curve : fiber) {
            auto iter = curve_dict.find(curve);
            if (iter == curve_dict.end()) {
                minval = 0;
                break;
            }
            if (local_self_int[iter->second] == INT_MAX) {
                // contracted curves are not taken in account for this calculation.
                continue;
            }
            minval = std::min(minval, -2ll * discrepancies[iter->second]);
        }
        sum += minval;
        if (sum >= 2ll*n) return true;
    }
    return false;
}
