#ifndef TOOLS_HPP
#define TOOLS_HPP


#if !defined(__cplusplus) || (__cplusplus < 201703L)
#error Requires C++17 or greater.
#endif


// #define NDEBUG

// Uncomment for multi thread executable, or use flag -D WAHL_MULTITHREAD

// #define WAHL_MULTITHREAD


// Status is printed in Wahl::Wahl() if compiled with multithread support.
// Status is printed in Searcher::search() if compiled without multithread.
// It uses \r.

#define PRINT_STATUS

// time to wait between status updates, in milliseconds

#define STATUS_WAIT 1000


// If multithread, we can print a nicer status if we allow ANSI escape characters

// #define WAHL_MULTITHREAD_STATUS_ANSI


// Add some sort of overflow check in calculating continued fractions.
// TODO: If it happens, use Boost multiprecision instead of ll.

// #define OVERFLOW_CHECK


// Avoids searching multiple times some test cases at the cost of some performance.
// (But then, less search = better performance? IDK which is better.)
// I guess really only useful when Keep_first is set to No.

// #define NO_REPEATED_SEARCH

// At the end, print the amount of pretests that passed P and K.

#define PRINT_PASSED_PRETESTS

//////////////////////////////////////////////////////////
// Some hard limits. Keep MAX_THREADS and BULK_SIZE a power of two.

#define MAX_TESTS 100

#ifndef MAX_THREADS
#define MAX_THREADS 4
#endif

// On multithread mode, avoid cache incoherence by giving threads bulks of tests.
// The last MAX_THREADS*BULK_SIZE tests are checked one by one to avoid idle threads.
#define BULK_SIZE 512

//////////////////////////////////////////////////////////

#define DEFAULT_OUTPUT_NAME "OUT"
#define DEFAULT_SUMMARY_NAME "SUMMARY"
#define DEFAULT_PRETEST_NAME "PRETEST"
#define ERROR_FILE "ERRORS.log"

#include<iostream>
#include<assert.h>


// Pre C++20
template <typename Container, typename T>
inline auto contains(const Container& C, const T& x)
-> decltype(C.find(x) != C.end()) {
    return C.find(x) != C.end();
}

template<typename T, typename S>
inline std::ostream& operator<<(std::ostream& os, const std::pair<T,S>& p) {
    return os << '(' << p.first << ',' << p.second << ')';
}

#ifndef NDEBUG

#define DEBUG_STREAM std::cerr

// debugv for containers.
#define debugv(x) do {DEBUG_STREAM << #x << ":\n   "; for(auto v : x) DEBUG_STREAM << " " << v; DEBUG_STREAM << std::endl;} while (0)

// debugx for values
#define debugx(x) DEBUG_STREAM << #x << ": " << x << std::endl

// debugt for values without printing name (for example text).
#define debugt(x) DEBUG_STREAM << x << std::endl

// debugc for code.
#define debugc(x) do {x} while (0)

#else
#define debugv(x) ((void) 0)
#define debugx(x) ((void) 0)
#define debugt(x) ((void) 0)
#define debugc(x) ((void) 0)
#endif

#ifdef NO_MULTITHREAD
#ifdef WAHL_MULTITHREAD
#undef WAHL_MULTITHREAD
#endif
#endif

#ifndef WAHL_MULTITHREAD
// on single thread, disable thread_local storages.
#define thread_local
#endif

#endif
