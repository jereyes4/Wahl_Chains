#ifndef WAHL_HPP
#define WAHL_HPP
#include"Reader.hpp" // Reader, vector

#ifdef WAHL_MULTITHREAD
#include<atomic> // atomic
#endif

class Searcher;
class Searcher_Wrapper;
struct Example;

class Wahl
{
public:
    Wahl(int argc, char** argv);
    inline long long get_test() {
        return current_test++;
    }

    // Invalidates searcher
    void Write(Searcher_Wrapper& searcher);

    // Invalidates searchers
    void Write(std::vector<Searcher_Wrapper>& searchers);

    // Receives a processed vector of examples from the queues.
    void Write(std::vector<Example>& example_vector);

    Reader reader;
    std::vector<long long> number_tests;

#ifdef WAHL_MULTITHREAD
    std::atomic<long long> current_test;
#else
    long long current_test;
#endif

    long long total_tests;
};

#endif
