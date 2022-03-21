#pragma once
#ifndef SEARCHER_HPP
#define SEARCHER_HPP
#include"config.hpp"
#include"Graph.hpp" // Graph, vector, pair, tuple, set, multiset
#include"Reader.hpp" // Reader, Canonical_Divisor
#include<sstream> // stringstream
#include<queue> // queue
#include<unordered_set> // unordered_set
#include<map> // map

#ifdef NO_REPEATED_SEARCH
#include"Algorithms.hpp" // Trie
#endif

#ifdef MULTITHREAD
#include<atomic> // atomic
#endif

typedef std::tuple<int,long long, long long> single_invariant;
typedef std::tuple<int,long long, long long, long long, long long> double_invariant;
typedef std::tuple<int,long long, long long> P_extremal_invariant;
typedef std::tuple<int,char,int,int,int> single_QHD_invariant;
typedef std::tuple<int,char,int,int,int,long long, long long> double_QHD_invariant;

struct ill_hash {
    inline size_t operator()(const std::tuple<int, long long, long long>& v) const {
        return std::get<1>(v)^(std::get<2>(v)<<6)^(std::get<0>(v)<<12);
    }
};
struct illll_hash {
    inline size_t operator()(const std::tuple<int, long long, long long, long long, long long>& v) const {
        return std::get<1>(v)^(std::get<3>(v)<<6)^(std::get<2>(v)<<3)^(std::get<4>(v)<<9)^(std::get<0>(v)<<12);
    }
};

class Wahl;


// The indexes of self_int and discrepancies are the curves id, not the positions they appear in the chains.
// Discrepancies are actually negative integers corresponding to numerators. The denominator is n[0] or n[1], depending on where the curve belongs. If the curve is not included in any chain or it is blown down, it is set to 0.
struct alignas(64) Example {

    // Order very important! arithmetics are done with subjacent chars.
    enum Type : char{
        single_,

        QHD_single_a_,
        QHD_single_b_,
        QHD_single_c_,
        QHD_single_d_,
        QHD_single_e_,
        QHD_single_f_,
        QHD_single_g_,
        QHD_single_h_,
        QHD_single_i_,
        QHD_single_j_,

        double_,
        p_extremal_,

        QHD_double_a_,
        QHD_double_b_,
        QHD_double_c_,
        QHD_double_d_,
        QHD_double_e_,
        QHD_double_f_,
        QHD_double_g_,
        QHD_double_h_,
        QHD_double_i_,
        QHD_double_j_,

        QHD_partial_a_,
        QHD_partial_b_,
        QHD_partial_c_,
        QHD_partial_d_,
        QHD_partial_e_,
        QHD_partial_f_,
        QHD_partial_g_,
        QHD_partial_h_,
        QHD_partial_i_,
        QHD_partial_j_
    };
    int export_id;
    long long test;
    Type type;
    int K2;
    int complete_fibers;
    std::set<int> used_curves;
    std::set<int> blown_down_exceptionals;
    std::vector<std::pair<int,int>> blowups;
    long long n[2];
    long long a[2];
    int p;
    int q;
    int r;
    int branch_permutation[3];
    int extra_n[2];
    int extra_orig[2];
    int extra_pos[2];
    std::vector<int> chain[2]; // For forks, concatenate branches.
    std::vector<int> self_int;
    std::vector<int> chain_original; // For P-extremal resolution
    std::vector<int> self_int_original; // For P-extremal resolution
    std::vector<long long> discrepancies;
    long long Delta;
    long long Omega;
    bool worm_hole;
    int worm_hole_id;
    bool worm_hole_conjecture_counterexample;
    bool no_obstruction;
    bool nef;
    bool nef_warning;
    bool effective;
};

class alignas(64) Searcher {
public:
    Searcher() : test_index(0), test_start(0), current_test(-1){}

    std::stringstream* err;

    Wahl* parent;
    int test_index;
    long long test_start;
    long long current_test;

    #ifdef MULTITHREAD
        std::atomic<long long> *wrapper_current_test;
        #if defined(PRINT_PASSED_PRETESTS_END) || defined(PRINT_STATUS_EXTRA)
            std::atomic<long long> *wrapper_passed_pretests;
        #endif
        #ifdef PRINT_STATUS_EXTRA
            std::atomic<long long> *wrapper_total_examples;
        #endif
    #else
        long long *wrapper_current_test;
        #if defined(PRINT_PASSED_PRETESTS_END) || defined(PRINT_STATUS_EXTRA)
            long long *wrapper_passed_pretests;
        #endif
        #ifdef PRINT_STATUS_EXTRA
            long long *wrapper_total_examples;
        #endif
    #endif

    #ifdef EXPORT_PRETEST_DATA
        std::queue<long long> *passed_pretest_list;
    #endif

    bool current_no_obstruction;
    int current_complete_fibers;

    // expected K^2
    int current_K2;
    std::pair<bool,int> check_obstruction();

    //////////////////////////////////////////////////////
    // Result data

    std::queue<Example> *results;
    std::unordered_set<single_invariant,ill_hash> single_found;
    std::unordered_set<double_invariant,illll_hash> double_found;
    std::unordered_set<P_extremal_invariant,ill_hash> P_extremal_found;
    std::set<single_QHD_invariant> single_QHD_found;
    std::set<double_QHD_invariant> double_QHD_found;
    Graph G;

    // A local copy to not access shared memory. Is this useful? IDK.
    Reader reader_copy;

    // Takes an included curve and gives it an index in the local example
    std::map<int,int> curve_dict;

    // Takes an index in the local example and gives its original index
    std::vector<int> curve_translate;

    // Number of curves in the example
    int ex_number;
    void search();
    // K^2 before processing the example
    int K2;

    //////////////////////////////////////////////////////
    // temporary to build graph from the test info

    inline void remove_curve(int curve) {
        auto iter = temp_included_curves.find(curve);
        for (int other : iter->second) {
            temp_included_curves[other].erase(curve);
        }
        temp_included_curves.erase(iter);
    }
    bool contract_exceptional();

    // Has the indexes of curves included but also of curves in the exceptional divisor. These indexes are mapped to a set of all the included curves which it intersects.
    std::map<int,std::multiset<int>> temp_included_curves;
    std::vector<int> temp_self_int;
    std::unordered_set<int> temp_ignored_exceptional;

    // using vector as set for O(0) clear. True is current_test, False is anything else.
    std::vector<long long> temp_try_included_exceptional;
    std::vector<long long> temp_marked_exceptional;

    std::map<int,std::multiset<int>> original_adj_map;

    void get_mask_and_real_test(long long& mask, long long& real_test);

    void get_curves_from_mask(long long mask);

    bool get_curves_from_mask_exact_curves(long long mask);

    //////////////////////////////////////////////////////

    void init();

    //////////////////////////////////////////////////////
    // single_chain.cpp

    void search_for_single_chain();

    // Since the search for QHD includes the search for single chains, for that case we split the inner loop.
    // This may split back to QHD, thus the template.
    template<bool chain_search = true, bool QHD_search = false>
    void search_for_single_chain_inner_loop();

    void explore_single_candidate(std::vector<int>& chain);
    // Checks if chain is Wahl, if it makes K + D effective and if it's not repeated, add it to results.
    // extra_n is the number of extra blowups at the end of the chain, so it's appended that amount of (-2)-curves.
    // extra_orig is the position of the original end of the chain.
    // extra_pos is the position at the other end of the blowups, the one that must have extra_n less self intersection.
    void verify_single_candidate(
        const std::vector<int>& chain, const std::vector<int>& local_self_int,
        int extra_n = 0, int extra_orig = -1, int extra_pos = -1
    );

    std::pair<bool,bool> single_is_nef(
        const std::vector<int>& local_self_int,
        const std::vector<long long>& discrepancies, long long n,
        int extra_n, int extra_orig, int extra_pos, int extra_new_border
    );

    bool single_is_effective(
        const std::vector<int>& local_self_int,
        const std::vector<long long>& discrepancies, long long n
    );
    //////////////////////////////////////////////////////
    // double_chain.cpp

    void search_for_double_chain();

    // Since the search for QHD includes the search for double chains, for that case we split the inner loop.
    // This may split back to QHD, thus the template.
    template<bool chain_search = true, bool QHD_search = false>
    void search_for_double_chain_inner_loop();

    // Takes an array of two chains.
    void explore_double_candidate(std::vector<int> (&chain)[2]);

    // Checks if chain[0] is already Wahl and explores chain[1] independently. Mostly a copy-paste of explore_single_chain.
    // chain is returned the same as it was.
    // inline
    void verify_first_explore_independent_second(
        std::vector<int> (&chain)[2], const std::vector<int>& local_self_int,
        const std::vector<int>& locaton,
        int extra_n_0, int extra_orig_0, int extra_pos_0, bool no_verify = false
    );

    void verify_double_candidate(
        const std::vector<int> (&chain)[2], const std::vector<int>& local_self_int,
        int extra_n[2], int extra_orig[2], int extra_pos[2]
    );

    void explore_p_extremal_resolution(std::vector<int> &chain);

    // Also receives the unreduced chain. uses the G.self_int
    void verify_p_extremal_resolution(
        const std::vector<int>& reduced_chain, const std::vector<int>& reduced_self_int,
        const std::vector<int>& chain, int extra_n, int extra_orig, int extra_pos
    );

    std::pair<bool,bool> double_is_nef(
        const std::vector<int>& local_self_int, const std::vector<long long>& discrepancies,
        const std::vector<int>& location, long long n[2],
        int extra_n[2], int extra_orig[2], int extra_pos[2], int extra_new_border[2]
    );

    bool double_is_effective(
        const std::vector<int>& local_self_int, const std::vector<long long>& discrepancies,
        const std::vector<int>& location, long long n[2]
    );

    //////////////////////////////////////////////////////
    // QHD_single_chain.cpp

    // Function can split into the regular double chain version.
    void search_for_QHD3_single_chain();

    // Weird case.
    void get_fork_from_one_chain_for_single(const std::vector<int>& chain);

    // Reduces the fork here, so it just uses G.self_int
    void verify_QHD3_single_candidate(
        const std::vector<int> (&fork)[3], int extra_id = -1,
        int extra_n = 0, int extra_orig = -1, int extra_pos = -1
    );

    //////////////////////////////////////////////////////
    // QHD_double_chain.cpp

    // Function can split into the regular double chain version.
    void search_for_QHD3_double_chain();

    // Weird case. First one only used for QHD-partial resolutions.
    void get_fork_from_one_chain_for_double(const std::vector<int>& chain);
    void get_fork_from_two_chains_for_double(std::vector<int> (&chain)[2]);

    void explore_QHD3_double_candidate(
        std::vector<int> (&fork)[3], std::vector<int>& chain, int extra_id = -1,
        int extra_n = 0, int extra_orig = -1, int extra_po = -1
    );

    void explore_QHD3_partial_resolution(
        std::vector<int> (&fork)[3], int extra_id = -1, int extra_n = 0,
        int extra_orig = -1, int extra_pos = -1
    );

    void verify_QHD3_double_candidate(
        const std::vector<int> (&fork)[3], const std::vector<int>& chain,
        const std::vector<int>& local_self_int,
        int extra_n[2], int extra_orig[2], int extra_pos[2], int extra_id = -1
    );

    //////////////////////////////////////////////////////

#ifdef NO_REPEATED_SEARCH
    algs::Trie seen;
#endif
};

// A Wrapper to try to use the least amount of shared memory possible. Build everything on 'local stack'.
// Is this useful? IDK.
class alignas(64) Searcher_Wrapper {
public:
    std::queue<Example> results;
    Wahl *parent;
    std::stringstream err;

    void search();

    #ifdef EXPORT_PRETEST_DATA
        std::queue<long long> passed_pretest_list;
    #endif // EXPORT_PRETEST_DATA

    #ifdef MULTITHREAD
        std::atomic<long long> current_test;
        #if defined(PRINT_PASSED_PRETESTS_END) || defined(PRINT_STATUS_EXTRA)
            std::atomic<long long> passed_pretests;
        #endif
        #ifdef PRINT_STATUS_EXTRA
            std::atomic<long long> total_examples;
        #endif
    #else
            long long current_test;
        #if defined(PRINT_PASSED_PRETESTS_END) || defined(PRINT_STATUS_EXTRA)
            long long passed_pretests;
        #endif
        #ifdef PRINT_STATUS_EXTRA
            long long total_examples;
        #endif
    #endif
};

#endif
