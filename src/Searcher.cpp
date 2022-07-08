#include"Searcher.hpp"
#include"Wahl.hpp" // sigint_catched
#include"Algorithms.hpp" // algs::ith_combination
#include<climits> // INT_MAX

#if !defined(MULTITHREAD) && defined(PRINT_STATUS)
#include<chrono> // steady_clock, milliseconds
#endif

void Searcher_Wrapper::search() {
    Searcher worker;
    worker.parent = parent;
    worker.reader_copy = parent->reader;
    worker.results = &results;
    worker.err = &err;
    worker.wrapper_current_test = &current_test;
    #ifdef EXPORT_PRETEST_DATA
        worker.passed_pretest_list = &passed_pretest_list;
    #endif
    #if defined(PRINT_PASSED_PRETESTS_END) || defined(PRINT_STATUS_EXTRA)
        worker.wrapper_passed_pretests = &passed_pretests;
        passed_pretests = 0;
    #endif
    #ifdef PRINT_STATUS_EXTRA
        worker.wrapper_total_examples = &total_examples;
        total_examples = 0;
    #endif

    worker.search();
}


void Searcher::init() {
    const int s = reader_copy.curve_no;
    for (int i = 0; i < s; ++i) {
        original_adj_map[i] = reader_copy.adj_list[i];
    }
    temp_try_included_exceptional.assign(s,-1);
    temp_marked_exceptional.assign(s,-1);
    current_complete_fibers = 0;
    current_no_obstruction = false;
}


// returns true if we should ignore the test. This happens when a try curve is included and contracted, as this is
// the same as not including it and contracting. The choice of ignoring when including is to make exact curves consistent.
bool Searcher::contract_exceptional() {
    /*
    Try to contract (-1) curves in the exceptional divisor.
    There might be some marked (-1) that we don't want to contract.
    First start with all curves unmarked and proceed from the lastest blowup to the first.
    There are several cases:
    If the (-1) doesn't have parents:
         If there are 0, 1 or 2 different curves intersecting the (-1), contract and change intersections appropriately.
         In other case, don't contract.
    If the (-1) has one parent:
         If there are 0 or 1 other curves intersecting, contract.
         If there are 2 other different curves and the parent is not included, contract and mark the parent.
         In other case, don't contract.
    If the (-1) has two parents:
         If there are no other curves, contract.
         If there is one other curve and no parent is included, contract (this allows for one of the parents to contract later, but not both).
         If there is one other curve and only one parent is included, contract, but mark the non included parent.
         If there are two different other curves and no parent is included, contract but mark both parents.
         Otherwise don't contract.
    */
    for (auto canon_iter = reader_copy.K.components.rbegin(); canon_iter != reader_copy.K.components.rend(); ++canon_iter) {
        const auto& comp = *canon_iter;
        if (temp_self_int[comp.id] != -1 or temp_marked_exceptional[comp.id] == current_test) continue;

        auto iter_excep = temp_included_curves.find(comp.id);
        if (comp.left_parent == -1) {
            switch (iter_excep->second.size()) {
            case 0: {
                if (temp_try_included_exceptional[comp.id] == current_test) {
                    return true;
                }
                temp_included_curves.erase(iter_excep);
                temp_self_int[comp.id] = INT_MAX;
                K2++;
                continue;
            }
            case 1: {
                if (temp_try_included_exceptional[comp.id] == current_test) {
                    return true;
                }
                const int curve = *iter_excep->second.begin();
                temp_included_curves[curve].erase(comp.id);
                temp_self_int[curve]++;
                temp_included_curves.erase(iter_excep);
                temp_self_int[comp.id] = INT_MAX;
                K2++;
                continue;
            }
            case 2: {
                const int curve_a = *iter_excep->second.begin();
                const int curve_b = *iter_excep->second.rbegin();
                if (curve_a == curve_b) continue;
                if (temp_try_included_exceptional[comp.id] == current_test) {
                    return true;
                }
                temp_included_curves[curve_a].erase(comp.id);
                temp_included_curves[curve_b].erase(comp.id);
                temp_self_int[curve_a]++;
                temp_self_int[curve_b]++;
                temp_included_curves[curve_a].insert(curve_b);
                temp_included_curves[curve_b].insert(curve_a);
                temp_included_curves.erase(iter_excep);
                temp_self_int[comp.id] = INT_MAX;
                K2++;
                continue;
            }
            default: continue;
            }
        }
        else if(comp.right_parent == -1) {
            switch (iter_excep->second.size()) {
            case 1: {
                if (temp_try_included_exceptional[comp.id] == current_test) {
                    return true;
                }
                temp_included_curves[comp.left_parent].erase(comp.id);
                temp_self_int[comp.left_parent]++;
                temp_included_curves.erase(iter_excep);
                temp_self_int[comp.id] = INT_MAX;
                K2++;
                continue;
            }
            case 2: {
                if (temp_try_included_exceptional[comp.id] == current_test) {
                    return true;
                }
                int curve = *iter_excep->second.begin();
                if (curve == comp.left_parent) {
                    curve = *iter_excep->second.rbegin();
                }
                temp_included_curves[comp.left_parent].erase(comp.id);
                temp_included_curves[curve].erase(comp.id);
                temp_self_int[comp.left_parent]++;
                temp_self_int[curve]++;
                temp_included_curves[curve].insert(comp.left_parent);
                temp_included_curves[comp.left_parent].insert(curve);
                temp_included_curves.erase(iter_excep);
                temp_self_int[comp.id] = INT_MAX;
                K2++;
                continue;
            }
            case 3: {
                if(!contains(temp_ignored_exceptional,comp.left_parent)) {
                    continue;
                }
                auto iter = iter_excep->second.begin();
                int curve_a = *iter++;
                if(curve_a == comp.left_parent) curve_a = *iter++;
                int curve_b = *iter++;
                if(curve_b == comp.left_parent) curve_b = *iter;
                if(curve_a == curve_b) continue;

                if(temp_try_included_exceptional[comp.id] == current_test) {
                    return true;
                }

                temp_included_curves[curve_a].erase(comp.id);
                temp_included_curves[curve_b].erase(comp.id);
                temp_included_curves[comp.left_parent].erase(comp.id);
                temp_self_int[curve_a]++;
                temp_self_int[curve_b]++;
                temp_self_int[comp.left_parent]++;
                temp_included_curves[curve_a].insert({curve_b,comp.left_parent});
                temp_included_curves[curve_b].insert({curve_a,comp.left_parent});
                temp_included_curves[comp.left_parent].insert({curve_a,curve_b});
                temp_included_curves.erase(iter_excep);
                temp_self_int[comp.id] = INT_MAX;
                temp_marked_exceptional[comp.left_parent] = current_test;
                K2++;
                continue;
            }
            default: continue;
            }
        }
        else{
            switch (iter_excep->second.size()) {
            case 2: {
                if (temp_try_included_exceptional[comp.id] == current_test) {
                    return true;
                }
                temp_included_curves[comp.left_parent].erase(comp.id);
                temp_included_curves[comp.right_parent].erase(comp.id);
                temp_self_int[comp.left_parent]++;
                temp_self_int[comp.right_parent]++;
                temp_included_curves[comp.left_parent].insert(comp.right_parent);
                temp_included_curves[comp.right_parent].insert(comp.left_parent);
                temp_included_curves.erase(iter_excep);
                temp_self_int[comp.id] = INT_MAX;
                K2++;
                continue;
            }
            case 3: {
                if (!contains(temp_ignored_exceptional,comp.left_parent) and !contains(temp_ignored_exceptional,comp.right_parent)) {
                    continue;
                }
                if(temp_try_included_exceptional[comp.id] == current_test) {
                    return true;
                }
                auto iter = iter_excep->second.begin();
                int curve = *iter++;
                while (curve == comp.left_parent or curve == comp.right_parent) curve = *iter++;

                temp_included_curves[curve].erase(comp.id);
                temp_self_int[curve]++;
                if (!contains(temp_ignored_exceptional,comp.left_parent)) {
                    temp_marked_exceptional[comp.right_parent] = current_test;
                }
                else if (!contains(temp_ignored_exceptional,comp.right_parent)) {
                    temp_marked_exceptional[comp.left_parent] = current_test;
                }
                temp_included_curves[comp.left_parent].erase(comp.id);
                temp_included_curves[comp.right_parent].erase(comp.id);
                temp_self_int[comp.left_parent]++;
                temp_self_int[comp.right_parent]++;
                temp_included_curves[comp.left_parent].insert({comp.right_parent,curve});
                temp_included_curves[comp.right_parent].insert({comp.left_parent,curve});
                temp_included_curves[curve].insert({comp.left_parent,comp.right_parent});
                temp_included_curves.erase(iter_excep);
                temp_self_int[comp.id] = INT_MAX;
                K2++;
                continue;
            }
            case 4: {
                if (!contains(temp_ignored_exceptional,comp.left_parent) or !contains(temp_ignored_exceptional,comp.right_parent)) {
                    continue;
                }
                auto iter = iter_excep->second.begin();
                int curve_a = *iter++;
                while (curve_a == comp.left_parent or curve_a == comp.right_parent) curve_a = *iter++;
                int curve_b = *iter++;
                while (curve_b == comp.left_parent or curve_b == comp.right_parent) curve_b = *iter++;
                if(curve_a == curve_b) continue;
                if(temp_try_included_exceptional[comp.id] == current_test) {
                    return true;
                }
                temp_included_curves[curve_a].erase(comp.id);
                temp_included_curves[curve_b].erase(comp.id);
                temp_self_int[curve_a]++;
                temp_self_int[curve_b]++;
                temp_self_int[comp.left_parent]++;
                temp_self_int[comp.right_parent]++;
                temp_included_curves[curve_a].insert({curve_b,comp.left_parent,comp.right_parent});
                temp_included_curves[curve_b].insert({curve_a,comp.left_parent,comp.right_parent});
                temp_included_curves[comp.left_parent].insert({curve_a,curve_b,comp.right_parent});
                temp_included_curves[comp.right_parent].insert({curve_a,curve_b,comp.left_parent});
                temp_included_curves.erase(iter_excep);
                temp_self_int[comp.id] = INT_MAX;
                temp_marked_exceptional[comp.left_parent] = current_test;
                temp_marked_exceptional[comp.right_parent] = current_test;
                K2++;
                continue;
            }
            default: continue;
            }
        }
    }
    return false;
}

void Searcher::search() {
    init();
    while (true) {

#ifdef CATCH_SIGINT
        if (sigint_catched) {

#ifndef MULTITHREAD
            std::cout << "\n" "Abrupt close.";
#endif
            return;
        }
#endif

#if !defined(MULTITHREAD) && defined(PRINT_STATUS)
        //print here status.
        static auto last_time = std::chrono::steady_clock::now();
        auto this_time = std::chrono::steady_clock::now();
        if (this_time - last_time >= std::chrono::milliseconds(STATUS_WAIT)) {
            std::cout << '\r' << double(std::min((long long)current_test,parent->total_tests))*100./double(parent->total_tests) << "% " << std::min((long long)current_test,parent->total_tests) << "/" << parent->total_tests;

#ifdef PRINT_STATUS_EXTRA
            std::cout << " PPT: " << *wrapper_passed_pretests << " Ex: " << *wrapper_total_examples;
#endif //PRINT_STATUS_EXTRA

            std::cout.flush();
            last_time = this_time;
        }
#endif
        *wrapper_current_test = current_test = parent->get_test(current_test);

        long long real_test = current_test + reader_copy.subtest_start;
        if (current_test >= parent->total_tests) {
            return;
        }
        while (parent->number_tests[test_index] + test_start <= real_test) {
            test_start += parent->number_tests[test_index];
            test_index++;
        }
        //from this mask read in order first try_curves and then choose_curves
        long long mask = real_test - test_start;

        K2 = reader_copy.K.self_int;

        if (reader_copy.curves_used_exactly == -1) {
            get_curves_from_mask(mask);
        }
        else {
            bool ignore_test = get_curves_from_mask_exact_curves(mask);
            if (ignore_test) continue;
        }

        bool ignore_test = contract_exceptional();
        if(ignore_test) continue;

        for (int ignored_ex : temp_ignored_exceptional) {
            auto iter = temp_included_curves.find(ignored_ex);
            if (iter == temp_included_curves.end()) continue;
            for (int i : iter->second) {
                temp_included_curves[i].erase(ignored_ex);
            }
        }

        int sum_self_int = 0;
        int double_singularities = 0;
        int curve_number = 0;
        for (auto& curve_data : temp_included_curves) {
            if (contains(temp_ignored_exceptional, curve_data.first)) continue;
            sum_self_int += temp_self_int[curve_data.first];
            double_singularities += curve_data.second.size();
            curve_number++;
        }
        // P is the amount of Wahl chains that can be extracted from this example
        int P = sum_self_int + 5*curve_number - double_singularities;

        if (!(P == 1 and (reader_copy.search_single_chain or reader_copy.search_single_QHD)) and
            !(P == 2 and (reader_copy.search_double_chain or reader_copy.search_double_QHD))) continue;

        // K is the K^2 of the resulting X
        int K = K2 - 3*curve_number - sum_self_int + double_singularities/2;
        current_K2 = K;

        if (!contains(reader_copy.search_for,K)) continue;

        // Pretest passed.
        #ifdef PRINT_PASSED_PRETESTS_END
        (*wrapper_passed_pretests)++;
        #endif

        #ifdef EXPORT_PRETEST_DATA
        if (passed_pretest_list->size() < MAX_PRETEST_EXPORTED) {
            passed_pretest_list->push(real_test);
        }
        if (reader_copy.export_pretests == Reader::only_) continue;
        #endif

        G.reset();
        curve_dict.clear();
        curve_translate.resize(0);
        ex_number = 0;

        for (auto& curve_data : temp_included_curves) {
            if (contains(temp_ignored_exceptional, curve_data.first)) continue;
            const int curve = curve_data.first;
            curve_dict[curve] = ex_number;
            curve_translate.emplace_back(curve);
            G.add_curve(temp_self_int[curve]);
            for (int other : curve_data.second) {
                if (other > curve) break;
                G.add_edge(curve_dict[other],ex_number);
            }
            ex_number++;
        }

        if (reader_copy.obstruction_check != Reader::no_) {
            auto obstruction_test = check_obstruction();
            current_no_obstruction = obstruction_test.first;
            if (!current_no_obstruction and reader_copy.obstruction_check == Reader::skip_) {
                continue;
            }
            current_complete_fibers = obstruction_test.second;
        }

        if (P == 1) {
            if (reader_copy.keep_first == Reader::keep_local_) {
                single_found.clear();
                single_QHD_found.clear();
            }
            if (reader_copy.search_single_QHD) search_for_QHD3_single_chain();
            else search_for_single_chain();
#ifdef PRINT_STATUS_EXTRA
            if (results->size() != *wrapper_total_examples) {
                *wrapper_total_examples = results->size();
            }
#endif //PRINT_STATUS_EXTRA
        }
        else if (P == 2) {
            if (reader_copy.keep_first == Reader::keep_local_) {
                double_found.clear();
                P_extremal_found.clear();
                double_QHD_found.clear();
            }
            if (reader_copy.search_double_QHD) search_for_QHD3_double_chain();
            else search_for_double_chain();
#ifdef PRINT_STATUS_EXTRA
            if (results->size() != *wrapper_total_examples) {
                *wrapper_total_examples = results->size();
            }
#endif //PRINT_STATUS_EXTRA
        }
    }
}

void Searcher::get_curves_from_mask(long long mask) {
    // If a try curve is not included in the graph and contracted, we can ignore this case as it's the same as if the curve was included to begin with.
    temp_ignored_exceptional.clear();
    temp_included_curves.clear();

    // These two lines might be extremely expensive. TODO: think an alternative.
    temp_self_int = reader_copy.self_int;
    temp_included_curves = original_adj_map;

    for (int curve : reader_copy.ignored_curves[test_index]) {
        if (contains(reader_copy.K.used_components,curve)) {
            temp_ignored_exceptional.insert(curve);
        }
        else {
            remove_curve(curve);
        }
    }

    for (int curve : reader_copy.try_curves[test_index]) {
        if (!(mask & 1)) {
            if (contains(reader_copy.K.used_components,curve)) {
                temp_ignored_exceptional.insert(curve);
            }
            else {
                remove_curve(curve);
            }
        }
        else {
            temp_try_included_exceptional[curve] = current_test;
        }
        mask >>= 1;
    }

    for (auto& choose_set : reader_copy.choose_curves[test_index]) {
        long long choose_total = (1ll<<choose_set.size()) - 1ll;
        long long choose_mask = mask%choose_total;
        for (int curve : choose_set) {
            if (!(choose_mask & 1)) {
                if (contains(reader_copy.K.used_components,curve)) {
                    temp_ignored_exceptional.insert(curve);
                }
                else {
                    remove_curve(curve);
                }
            }
            choose_mask >>= 1;
        }
        mask /= choose_total;
    }
}

bool Searcher::get_curves_from_mask_exact_curves(long long mask) {
    // If a try curve is not included in the graph and contracted, we can ignore this case as it's the same as if the curve was included to begin with.
    temp_ignored_exceptional.clear();
    temp_included_curves.clear();

    // These two lines might be extremely expensive. TODO: think an alternative.
    temp_self_int = reader_copy.self_int;
    temp_included_curves = original_adj_map;

    for (int curve : reader_copy.ignored_curves[test_index]) {
        if (contains(reader_copy.K.used_components,curve)) {
            temp_ignored_exceptional.insert(curve);
        }
        else {
            remove_curve(curve);
        }
    }

    THREAD_STATIC std::vector<int> chosen_curves;
    chosen_curves.resize(0);
    int to_choose_from = reader_copy.try_curves[test_index].size();
    for (auto& choose_set : reader_copy.choose_curves[test_index]) {
        to_choose_from += choose_set.size();
    }

    int to_choose = reader_copy.curves_used_exactly - reader_copy.fixed_curves[test_index].size();
    algs::ith_combination(to_choose_from, to_choose, mask, chosen_curves);

    int curve_index = 0;
    int choice_index = 0;
    for (int curve : reader_copy.try_curves[test_index]) {
        if (choice_index >= to_choose or chosen_curves[choice_index] != curve_index) {
            if (contains(reader_copy.K.used_components,curve)) {
                temp_ignored_exceptional.insert(curve);
            }
            else {
                remove_curve(curve);
            }
        }
        else {
            temp_try_included_exceptional[curve] = current_test;
            choice_index++;
        }
        curve_index++;
    }

    for (auto& choose_set : reader_copy.choose_curves[test_index]) {
        int included = 0;
        for (int curve : choose_set) {
            if (choice_index >= to_choose or chosen_curves[choice_index] != curve_index) {
                if (contains(reader_copy.K.used_components,curve)) {
                    temp_ignored_exceptional.insert(curve);
                }
                else {
                    remove_curve(curve);
                }
            }
            else {
                choice_index++;
                included++;
            }
            curve_index++;
        }
        if (included == choose_set.size()) return true;
    }
    return false;
}

std::pair<bool,int> Searcher::check_obstruction() {
    // Two facts are sufficient to prove 0 obstruction:
    // That there are only two complete fibers (this assumes that they are of type I_n)
    // Every non fiber & non exceptional curve is at least a (-1) after all blowups
    // The second condition is specially too strict if there are nested blowups, but this suffices for now.

    int complete_fibers = 0;
    for (auto& f : reader_copy.fibers) {
        bool is_complete = true;
        for (int curve : f) {
            if (!contains(curve_dict,curve)) {
                is_complete = false;
                break;
            }
        }
        if (is_complete) {
            ++complete_fibers;
        }
    }
    for (int curve : reader_copy.sections) {
        if (contains(curve_dict,curve) and temp_self_int[curve] < -1) {
            return {false,complete_fibers};
        }
    }
    return {complete_fibers <= 2,complete_fibers};
}
