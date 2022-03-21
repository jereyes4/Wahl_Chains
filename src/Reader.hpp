#pragma once
#ifndef READER_HPP
#define READER_HPP
#include"config.hpp"
#include<vector> // vector
#include<set> // set, multiset
#include<map> // map
#include<string> // string
#include<unordered_set> // unordered_set
#include<iostream> // istream, ostream

/*

summary_style: either plaintext or latex table.

summary_sort: to sort by (n,a) or by length of chain.

nef_check: wether to check nefness of the canonical divisor. If check is done, also determines wether to skip the examples that fail, or to include them while printing the result of this check.
Assumes that K+D is effective, so only tests for (-1)-curves in K.

effective_check: Same but with Q-effectiveness.

obstruction_check: same as before but with obstruction

keep_first: what to do if we find two examples with the same (n,a)? If keep_first is local, then we keep the first example with that (n,a) for any given test, that is, include other examples with the same (n,a) for other tests.
If global, then we keep the first example with given (n,a) for all tests.
In any case, examples by P-extremal resolutions are treated differently, and we keep the first example with the given (Delta,Omega)

section_input_mode: determines if we consider sections by self intersection or by the intersection with canonical divisor.

search_single_chain, search_double_chain: wether we search for these configurations.

summary_include_gcd: if the example is with double chain, this determines if in the summary is included de gcd of n_1 and n_2

latex_include_subsection: if the summary style is a latex table, this includes \subsection before each table.

search_for: the K^2 we want to search for.

threads: number of threads to run.

test_no: the number of different tests.

curve_no: number of curves.

current_section_argument: either C^2 or C.K depending on section_input_mode

K: Canonical divisor that keeps track of exceptional curves

output_filename, summary_filename: self explanatory. Doesn't include extension.

curve_id: dictionary which gives an id to a curve as in the input.

curve_name: name of the curve with given id.

fibers: self explanatory.

fiber_type: the type of fiber, as given in singular_fibers.

sections: self explanatory.

adj_list: adjacency list of the dual graph.

self_int: self intersection of the curves.

fixed_curves: for every test t, fixed_curves[t] contains the curves fixed in that test.

try_curves, ignored_curves: same but with try curves and ignored curves.

choose_curves: for every test t, choose_curves[t] contains the lists of groups of curves we want to try from, but not including all of them at once.
*/

// Keeps track of the exceptional divisor for calculating self intersections.
// Becomes invalid if there were forgotten exceptionals...
class Canonical_Divisor {
public:
    int self_int;
    void blowup(int id, const std::map<int,int>& intersections);

    // intersections is modified.
    int blowup_curve_self_int_delta(std::map<int,int>& intersections);
    int exceptional_intersection(std::map<int,int>& intersections);
    struct Component {
        int id;
        int multiplicity;
        int left_parent;
        int right_parent;
        Component(int id, int multiplicity, int left_parent, int right_parent) :
            id(id), multiplicity(multiplicity), left_parent(left_parent), right_parent(right_parent) {}
    };
    std::unordered_set<int> used_components;
    std::unordered_set<int> used_components_including_forgotten;
    std::vector<Component> components;
    std::vector<Component> components_including_forgotten;
};

class Reader
{
public:

    enum State : char {
        start_,
        comment_,
        fibers_,
        sections_,
        merge_,
        make_fibers_,
        forget_exceptionals_
    };
    enum Setting : char {
        no_,
        yes_,
        skip_,
        only_,
        print_,
        keep_local_,
        keep_global_,
        by_self_intersection_,
        by_canonical_intersection_,
        sort_by_n_,
        sort_by_length_,
        plain_text_,
        latex_table_
    };

    Setting summary_style;
    Setting summary_sort;
    Setting nef_check;
    Setting effective_check;
    Setting obstruction_check;
    Setting keep_first;
    Setting section_input_mode;
    Setting export_pretests;
    bool parse_only;
    bool search_single_chain;
    bool search_double_chain;
    bool search_single_QHD;
    bool search_double_QHD;
    bool summary_include_gcd;
    bool latex_include_subsection;
    std::set<int> search_for;

#ifdef MULTITHREAD
    int threads;
#endif

    int tests_no;
    int tests_start_index;
    int max_test_number;

    int curves_used_exactly;

    long long subtest_start;
    long long subtest_end;

    int line_no;
    int curve_no;
    int current_section_argument;
    Canonical_Divisor K;
    std::string output_filename;
    std::string summary_filename;
    std::string pretest_filename;
    std::map<std::string,int> curve_id;
    std::vector<std::string> curve_name;
    std::vector<std::vector<int>> fibers;
    std::vector<std::string> fiber_type;
    std::unordered_set<int> curves_in_fibers;
    std::unordered_set<int> forgotten_exceptionals;
    std::set<int> sections;
    std::vector<std::multiset<int>> adj_list;
    std::vector<int> self_int;
    std::vector<std::vector<int>> fixed_curves;
    std::vector<std::vector<int>> try_curves;
    std::vector<std::vector<std::vector<int>>> choose_curves;
    std::vector<std::vector<int>> ignored_curves;

    mutable std::ostream* error_stream;

    inline void error(const std::string msg, int where = -1) const {
        (*error_stream) << "Error(" << (where == -1 ? line_no : where) << "): " << msg << std::endl;
        exit(1);
    }
    inline void warning(const std::string msg, int where = -1) const {
        (*error_stream) << "Warning(" << (where == -1 ? line_no : where) << "): " << msg << std::endl;
    }
    Reader();
    void parse(std::istream& input);
    void parse_option(const std::vector<std::string>& tokens);
    void parse_fiber(const std::vector<std::string>& def_tokens, const std::vector<std::string>& content_tokens);
    void parse_section(const std::vector<std::string>& def_tokens, const std::vector<std::string>& content_tokens);
    void parse_merge(const std::vector<std::string>& def_tokens, const std::vector<std::string>& content_tokens);
    void parse_make_fiber(const std::vector<std::string>& def_tokens, const std::vector<std::string>& content_tokens);
    void parse_forget_exceptional(const std::vector<std::string>& tokens);
    long long get_test_numbers(std::vector<long long>& test_numbers) const;
    long long get_test_numbers_exact_curves(std::vector<long long>& test_numbers) const;
};

#endif
