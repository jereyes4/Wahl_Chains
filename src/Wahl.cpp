#include"Wahl.hpp" // string, to_string
#include"Searcher.hpp" // Searcher_Wrapper, queue, priority_queue, single_invariant, double_invariant, p_extremal_invariant and their hashes
#include"Algorithms.hpp"
#include<fstream> // ifstream
#include"Writer.hpp" // export_jsonl, less_by_n, less_by_length, stable_sort

#ifdef MULTITHREAD
#include<chrono> // milliseconds
#include<thread> // thread, sleep_for
#endif

#ifdef CATCH_SIGINT
#include<csignal>
std::atomic<bool> sigint_catched;
#endif // CATCH_SIGINT

Wahl::Wahl(int argc, char** argv) {
    std::ifstream f;
    f.open(argv[1]);
    if (f.fail()) {
        std::cout << "Error while opening file \"" << argv[1] << "\". (Does it exist?)." << std::endl;
        return;
    }
    reader.parse(f);
    f.close();

    if (reader.parse_only) {
        Writer::export_jsonl(reader);
        return;
    }

    if (argc > 2 and std::string(argv[2]) == "D") {
        reader.output_filename = "Debug";
        reader.summary_filename = "Debug";
    }

    // debugx(reader.tests_no);

    total_tests = reader.get_test_numbers(number_tests);
    init_tests();
    current_test = 0;

    if (reader.subtest_end != -1) {
        auto temp = total_tests;
        total_tests = std::min(total_tests,reader.subtest_end);
        std:: cout << "From a total of " + std::to_string(temp) + " attempting to test from " + std::to_string(reader.subtest_start) + " to " + std::to_string(total_tests) + "." << std::endl;
        if (total_tests < reader.subtest_start) {
            reader.error("Subtest range out of bounds.");
        }
        total_tests -= reader.subtest_start;
    }
    else {
        reader.subtest_start = 0;
    }
    std::cout << "Total tests: " << total_tests << std::endl;

    #ifdef CATCH_SIGINT
    sigint_catched = false;
    std::signal(SIGINT,
        [] (int sig) {
            std::signal(SIGINT,SIG_DFL);
            sigint_catched = true;
        }
    );
    #endif // CATCH_SIGINT

#ifdef MULTITHREAD
    int threads = reader.threads;
    std::vector<std::thread> spawns;
    std::vector<Searcher_Wrapper> searchers(threads);
    for (Searcher_Wrapper& searcher : searchers) {
        searcher.parent = this;
    }
    spawns.reserve(threads);
    for (int i = 0; i < threads; ++i) {
        spawns.emplace_back(&Searcher_Wrapper::search,&searchers[i]);
    }
#ifdef PRINT_STATUS
#ifdef MULTITHREAD_STATUS_ANSI
    std::cout << std::fixed;
    std::cout.precision(1);
    std::cout << "\e[s";
    long long mintest = 0;
    while (mintest < total_tests) {

#ifdef CATCH_SIGINT
        if (sigint_catched) {
            std::cout << "\n" "Abrupt close. Waiting for threads to finish..." << std::endl;
            for (int i = 0; i < threads; ++i) {
                spawns[i].join();
            }
            Write(searchers);
            return;
        }
#endif // CATCH_SIGINT

        std::cout << "\e[u\e[?25l";
        mintest = total_tests;
        for (int i = 0; i < threads; ++i) {
            long long searcher_test = searchers[i].current_test;
            mintest = std::min(mintest, searcher_test);
            std::cout << "Thread " << i << ": " << searcher_test;
#ifdef PRINT_STATUS_EXTRA
            std::cout << " PPT: " << searchers[i].passed_pretests << " Ex: " << searchers[i].total_examples;
#endif //PRINT_STATUS_EXTRA
            std::cout << '\n';
        }
        std::cout << double(mintest)*100./double(total_tests) << "% " << mintest << "/" << total_tests;
        std::cout << "\e[?25h";

#ifdef PRINT_STATUS_EXTRA
        long long pretests = 0;
        long long examples = 0;
        for (int i = 0; i < threads; ++i) {
            pretests += searchers[i].passed_pretests;
            examples += searchers[i].total_examples;
        }
        std::cout << " PPT: " << pretests << " Ex: " << examples;
#endif //PRINT_STATUS_EXTRA

        std::cout.flush();
        std::this_thread::sleep_for(std::chrono::milliseconds(STATUS_WAIT));
    }
    for (int i = 0; i < threads; ++i) {
        spawns[i].join();
    }
    std::cout << '\r' << 100. << "% " << total_tests << '/' << total_tests << std::endl;
#else // ndef MULTITHREAD_STATUS_ANSI

    std::cout << std::fixed;
    std::cout.precision(1);
    long long mintest = 0;
    while (mintest < total_tests) {

#ifdef CATCH_SIGINT
        if (sigint_catched) {
            std::cout << "\n" "Abrupt close. Waiting for threads to finish..." << std::endl;
            for (int i = 0; i < threads; ++i) {
                spawns[i].join();
            }
            Write(searchers);
            return;
        }
#endif // CATCH_SIGINT
        mintest = total_tests;
        for (int i = 0; i < threads; ++i) {
            mintest = std::min(mintest,(long long) searchers[i].current_test);
        }
        std::cout << '\r' << double(mintest)*100./double(total_tests) << "% " << mintest << '/' << total_tests;

#ifdef PRINT_STATUS_EXTRA
        long long pretests = 0;
        long long examples = 0;
        for (int i = 0; i < threads; ++i) {
            pretests += searchers[i].passed_pretests;
            examples += searchers[i].total_examples;
        }
        std::cout << " PPT: " << pretests << " Ex: " << examples;
#endif //PRINT_STATUS_EXTRA

        std::cout.flush();
        std::this_thread::sleep_for(std::chrono::milliseconds(STATUS_WAIT));
    }
    for (int i = 0; i < threads; ++i) {
        spawns[i].join();
    }
    std::cout << '\r' << 100. << "% " << total_tests << '/' << total_tests << std::endl;

#endif // MULTITHREAD_STATUS_ANSI

#else // ndef PRINT_STATUS
    for(int i = 0; i < threads; ++i) {
        spawns[i].join();
    }
#ifdef CATCH_SIGINT
    if (sigint_catched) {
        std::cout << "\n" "Abrupt close." << std::endl;
    }
#endif
#endif // PRINT_STATUS

#ifdef OVERFLOW_CHECK

    std::ofstream error_file(ERROR_FILE);
    for (Searcher_Wrapper& searcher : searchers) {
        error_file << searcher.err.rdbuf();
    }
    error_file.close();

#endif // OVERFLOW_CHECK

    Write(searchers);

#else // ndef MULTITHREAD
    Searcher_Wrapper searcher;
    searcher.parent = this;
#ifdef PRINT_STATUS
    std::cout << std::fixed;
    std::cout.precision(1);
    searcher.search();
    std::cout << std::endl;
#else // ndef PRINT_STATUS
    searcher.search();
#endif // PRINT_STATUS

#ifdef OVERFLOW_CHECK

    std::ofstream error_file(ERROR_FILE);
    error_file << searcher.err.rdbuf();
    error_file.close();

#endif // OVERFLOW_CHECK

    Write(searcher);

#endif // MULTITHREAD

}

void Wahl::Write(std::vector<Searcher_Wrapper>& searchers) {

    #ifdef EXPORT_PRETEST_DATA
    std::vector<long long> pretests_to_export;
    if (reader.export_pretests != Reader::no_) {
        int total_pretests = 0;
        for (auto& s : searchers) {
            total_pretests += s.passed_pretest_list.size();
        }
        pretests_to_export.reserve(total_pretests);
        for (auto& s : searchers) {
            while (!s.passed_pretest_list.empty()) {
                pretests_to_export.push_back(s.passed_pretest_list.front());
                s.passed_pretest_list.pop();
            }
        }
        std::sort(pretests_to_export.begin(),pretests_to_export.end());
        pretests_to_export.resize(std::min((int) pretests_to_export.size(),MAX_PRETEST_EXPORTED));
    }
    if (reader.export_pretests == Reader::only_) {
        // Only export graph data
        #ifdef PRINT_PASSED_PRETESTS_END
        long long passed_pretests = 0;
        for (auto& s : searchers) {
            passed_pretests += s.passed_pretests;
        }
        std::cout << "Done! " << passed_pretests << " pretests passed." << std::endl;
        #else
        std::cout << "Done!" << std::endl;
        #endif
        Writer::export_jsonl(reader);
        Writer::export_pretest_data(reader, pretests_to_export);
        return;
    }
    #endif

    // If keep_first is not global, we can push all examples from all threads in any order, and after sorting the result is deterministic.
    std::vector<Example> example_vector;
    size_t total_examples = 0;
    for (Searcher_Wrapper& searcher : searchers) {
        total_examples += searcher.results.size();
    }
    example_vector.reserve(total_examples);

    if (reader.keep_first != Reader::keep_global_) {
        for (Searcher_Wrapper& searcher : searchers) {
            while(!searcher.results.empty()) {
                example_vector.push_back(std::move(searcher.results.front()));
                searcher.results.pop();
            }
        }
    }
    else {
        // Keep a priority_queue of pairs, the first item being the test number, the second being the id of the searcher.
        // The top item in the queue will have the smallest test number, so it shall be processed, along with all examples of the same test number
        // After processing all examples of the given test number, add to the queue the next test number of the same searcher.
        std::priority_queue<std::pair<long long,int>, std::vector<std::pair<long long,int>>, std::greater<std::pair<long long,int>>> q;

        // Only the first example of each type will be added.
        std::unordered_set<single_invariant,ill_hash> single_found;
        std::unordered_set<double_invariant,illll_hash> double_found;
        std::unordered_set<P_extremal_invariant,ill_hash> p_extremal_found;
        std::set<single_QHD_invariant> single_QHD_found;
        std::set<double_QHD_invariant> double_QHD_found;

        for (int i = 0; i < searchers.size(); ++i) {
            if (!searchers[i].results.empty()) {
                q.emplace(searchers[i].results.front().test,i);
            }
        }

        while (!q.empty()) {
            const auto p = q.top();
            q.pop();
            const long long& test = p.first;
            auto& searcher_queue = searchers[p.second].results;
            while (!searcher_queue.empty() and searcher_queue.front().test == test) {
                auto& ex = searcher_queue.front();

                // In case of seeing an extremal resolution, add the next example too.
                bool next_paired = false;

                if (ex.type == Example::single_) {
                    auto key = std::make_tuple(ex.K2,ex.n[0],std::min(ex.a[0],ex.n[0]-ex.a[0]));
                    if (contains(single_found,key)) {
                        searcher_queue.pop();
                        continue;
                    }
                    else {
                        single_found.insert(key);
                    }
                }
                else if (ex.type == Example::p_extremal_) {
                    auto Delta = ex.Delta;
                    auto Omega = ex.Omega;
                    auto Omega_unif = algs::gcd_invmod(Delta,Omega).second;
                    Omega = std::min(Omega,Omega_unif);
                    auto key = std::make_tuple(ex.K2,Delta,Omega);

                    next_paired = ex.worm_hole and !ex.worm_hole_conjecture_counterexample;

                    if (contains(p_extremal_found,key)) {
                        searcher_queue.pop();
                        if (next_paired) {
                            // Skip the next one too.
                            searcher_queue.pop();
                        }
                        continue;
                    }
                    else {
                        p_extremal_found.insert(key);
                    }
                }
                else if (ex.type == Example::double_) {
                    auto unif_inv = Writer::uniformize_double_by_n(ex.n,ex.a);
                    auto key = std::make_tuple(ex.K2,std::get<0>(unif_inv),std::get<1>(unif_inv),std::get<2>(unif_inv),std::get<3>(unif_inv));
                    if (contains(double_found,key)) {
                        searcher_queue.pop();
                        continue;
                    }
                    else {
                        double_found.insert(key);
                    }
                }
                else if (ex.type >= Example::QHD_single_a_ and ex.type <= Example::QHD_single_j_) {
                    auto key = std::make_tuple(ex.K2,(char)ex.type,ex.p,ex.q,ex.r);
                    if (contains(single_QHD_found,key)) {
                        searcher_queue.pop();
                        continue;
                    }
                    else {
                        single_QHD_found.insert(key);
                    }
                }
                else if (ex.type >= Example::QHD_double_a_ and ex.type <= Example::QHD_partial_j_) {
                    auto key = std::make_tuple(ex.K2,(char)ex.type,ex.p,ex.q,ex.r,ex.n[1],std::min(ex.a[1],ex.n[1] - ex.a[1]));
                    if (contains(double_QHD_found,key)) {
                        searcher_queue.pop();
                        continue;
                    }
                    else {
                        double_QHD_found.insert(key);
                    }
                }
                example_vector.push_back(std::move(ex));
                searcher_queue.pop();
                if (next_paired) {
                    // Add the next one too.
                    example_vector.push_back(std::move(searcher_queue.front()));
                    searcher_queue.pop();
                }
            }
            if (!searcher_queue.empty()) {
                q.emplace(searcher_queue.front().test,p.second);
            }
        }
    }
    #ifdef PRINT_PASSED_PRETESTS_END
    long long passed_pretests = 0;
    for (auto& s : searchers) {
        passed_pretests += s.passed_pretests;
    }
    std::cout << "Done! " << passed_pretests << " pretests passed and found " << example_vector.size() << " examples." << std::endl;
    #else
    std::cout << "Done! Found " << example_vector.size() << " examples." << std::endl;
    #endif

    #ifdef EXPORT_PRETEST_DATA
    if (reader.export_pretests != Reader::no_) {
        Writer::export_pretest_data(reader,pretests_to_export);
    }
    #endif

    Write(example_vector);
}


void Wahl::Write(Searcher_Wrapper& searcher) {

    #ifdef EXPORT_PRETEST_DATA
    std::vector<long long> pretests_to_export;
    if (reader.export_pretests != Reader::no_) {
        pretests_to_export.reserve(searcher.passed_pretest_list.size());
        while (!searcher.passed_pretest_list.empty()) {
            pretests_to_export.push_back(searcher.passed_pretest_list.front());
            searcher.passed_pretest_list.pop();
        }
    }
    if (reader.export_pretests == Reader::only_) {
        // Only export graph data
        #ifdef PRINT_PASSED_PRETESTS_END
        std::cout << "Done! " << searcher.passed_pretests << " pretests passed." << std::endl;
        #else
        std::cout << "Done!" << std::endl;
        #endif
        Writer::export_jsonl(reader);
        Writer::export_pretest_data(reader, pretests_to_export);
        return;
    }
    #endif

    // Move all the results into a vector and also create another vector of references to it. We will sort the second one to export to the jsonl and summary files. Also, use the un sorted vector to recover the pairs of examples given by worm holes.
    std::vector<Example> example_vector;
    example_vector.reserve(searcher.results.size());
    while (!searcher.results.empty()) {
        example_vector.push_back(std::move(searcher.results.front()));
        searcher.results.pop();
    }
    #ifdef PRINT_PASSED_PRETESTS_END
    std::cout << "Done! " << searcher.passed_pretests << " pretests passed and found " << example_vector.size() << " examples." << std::endl;
    #else
    std::cout << "Done! Found " << example_vector.size() << " examples." << std::endl;
    #endif

    #ifdef EXPORT_PRETEST_DATA
    if (reader.export_pretests != Reader::no_) {
        Writer::export_pretest_data(reader,pretests_to_export);
    }
    #endif

    Write(example_vector);
}

void Wahl::Write(std::vector<Example>& example_vector) {

    std::vector<Example*> ptr_example_vector;
    ptr_example_vector.reserve(example_vector.size());
    for (Example& ex : example_vector) ptr_example_vector.push_back(&ex);

    if (reader.summary_sort == Reader::sort_by_n_) {
        std::stable_sort(ptr_example_vector.begin(),ptr_example_vector.end(),
            [] (const Example* a, const Example* b) -> bool {
                return Writer::less_by_n(*a,*b);
            }
        );
    }
    else if (reader.summary_sort == Reader::sort_by_length_) {
        std::stable_sort(ptr_example_vector.begin(),ptr_example_vector.end(),
            [] (const Example* a, const Example* b) -> bool {
                return Writer::less_by_length(*a,*b);
            }
        );
    }

    for (int i = 0; i < ptr_example_vector.size(); ++i) {
        ptr_example_vector[i]->export_id = i;
    }

    for (int i = 0; i < example_vector.size(); ++i) {
        auto& ex = example_vector[i];
        if (ex.type == Example::p_extremal_ and ex.worm_hole and !ex.worm_hole_conjecture_counterexample) {
            // wormhole found. This example is paired with the next one in example_vector
            ex.worm_hole_id = example_vector[i+1].export_id;
            example_vector[i+1].worm_hole_id = ex.export_id;

            // Skip next example as it's already paired with this one.
            i++;
        }
    }
    Writer::export_jsonl(reader,ptr_example_vector);

    if (reader.summary_style == Reader::plain_text_) {
        Writer::export_summary_text(reader,ptr_example_vector);
    }
    else if (reader.summary_style == Reader::latex_table_) {
        Writer::export_summary_latex(reader,ptr_example_vector);
    }
}
