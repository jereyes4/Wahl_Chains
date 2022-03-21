#pragma once
#ifndef WAHL_HPP
#define WAHL_HPP
#include"Reader.hpp" // Reader, vector

#if defined(MULTITHREAD) || defined(CATCH_SIGINT)
#include<atomic> // atomic
#endif

#ifdef CATCH_SIGINT
extern std::atomic<bool> sigint_catched;
#endif

class Searcher;
class Searcher_Wrapper;
struct Example;

class Wahl
{
public:
    Wahl(int argc, char** argv);

    inline void init_tests() {
#ifdef MULTITHREAD
        last_batch = total_tests - ((long long)(BULK_SIZE*MAX_THREADS) + total_tests%(long long)BULK_SIZE);
#endif
    }

    inline long long get_test(long long previous) {
#ifndef MULTITHREAD
        return current_test++;
#else
        // The last bunch if tests should be done one by one, not in bulk.
        if (total_tests < (long long)(BULK_SIZE*MAX_THREADS)) {
            // If there are too few tests, ignore bulks.
            return current_test.fetch_add(1,std::memory_order_relaxed);
        }

        // index where the last batch starts
        // const long long last_batch = (...);

        if (++previous >= last_batch) {
            long long next = current_test.fetch_add(BULK_SIZE,std::memory_order_relaxed);
            next = last_batch + (next - last_batch)/(long long)BULK_SIZE;
            return next;
        }
        if (previous%(long long)BULK_SIZE) {
            // Has not finished bulk
            return previous;
        }
        else {
            long long next = current_test.fetch_add(BULK_SIZE,std::memory_order_relaxed);
            if (next >= last_batch) {
                next = last_batch + (next - last_batch)/(long long)BULK_SIZE;
            }
            return next;
        }
#endif
    }

    // Invalidates searcher
    void Write(Searcher_Wrapper& searcher);

    // Invalidates searchers
    void Write(std::vector<Searcher_Wrapper>& searchers);

    // Receives a processed vector of examples from the queues.
    void Write(std::vector<Example>& example_vector);

    Reader reader;
    std::vector<long long> number_tests;

#ifdef MULTITHREAD
    std::atomic<long long> current_test;
    long long last_batch;
#else
    long long current_test;
#endif

    long long total_tests;
};

#endif
