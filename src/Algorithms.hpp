#pragma once
#ifndef ALGORITHMS_HPP
#define ALGORITHMS_HPP

/*
Some general purpose algorithms and structures.
*/

#include"config.hpp"
#include<vector> // vector
#include<utility> // pair
#include<unordered_set> // unordered_set
#include<climits> // INT_MAX
#include<stack> // stack

namespace algs {

    // if g = gcd(a,b) returns (g, (b/g)^-1 mod a/g). Assumes a >= 0,b > 0
    template <typename T>
    static std::pair<T, T> gcd_invmod(T a, T b) {
        T x = 1, y = 0;
        while (a%b) {
            T temp =  x*(a/b+1) - y;
            y = x; x = temp;
            temp = b - a%b;
            a = b; b = temp;
        }
        return {b,x};
    }

    // Assumes a >= 0, b > 0.
    template <typename T>
    static T gcd(T a, T b) {
        while (a%b) {
            T temp = b - a%b;
            a = b; b = temp;
        }
        return b;
    }

    // Assumes a,b > 0
    template<typename T>
    static T lcm(T a, T b) {
        return a/gcd(a,b)*b;
    }

    // Assigns to result the resolution of the singularity represented by m/q
    static void to_chain(long long m, long long q, std::vector<int>& result) {
        result.resize(0);
        while (m%q) {
            result.push_back(-1-m/q);
            auto temp = q;
            q = q - m%q;
            m = temp;
        }
        result.push_back(-m/q);
    }

    // Returns the fraction representing the singularity using curves from [start, end].
    // Returns -1/-1 on overflow and -1/0 on fail.
    static std::pair<long long, long long>
    to_rational(const std::vector<int>& self_int, int start = 0, int end = -1) {
        long long a = 1, b = 0;
        if (end == -1) end = self_int.size() - 1;
        for (int i = end; i >= start ; i--) {
            auto temp = a;
            a = -a*(long long)self_int[i] - b;
            b = temp;

#ifdef OVERFLOW_CHECK
            if (a > (1ll << 59)) return {-1,-1};
#endif
            if (b <= 0) return {-1,0};
        }
        return {a,b};
    }


    // Returns the fraction representing the singularity using curves from [start, end].
    // This version takes a vector of references to self intersections.
    // Returns -1/-1 on overflow and -1/0 on fail.
    static std::pair<long long, long long>
    to_rational(const std::vector<int>& chain, const std::vector<int>& self_int, int start = 0, int end = -1) {
        long long a = 1, b = 0;
        if (end == -1) end = chain.size() - 1;
        for (int i = end; i >= start ; i--) {
            auto temp = a;
            a = -a*(long long)self_int[chain[i]] - b;
            b = temp;

#ifdef OVERFLOW_CHECK
            if (a > (1ll << 59)) return {-1,-1};
#endif
            if (b <= 0) return {-1,0};
        }
        return {a,b};
    }

    // Returns the pair (n,a) of a Wahl singularity. If not Wahl, returns (0,0).
    // Assumes admissible
    static std::pair<long long, long long>
    get_wahl_numbers(const std::vector<int>& self_int) {
        auto rational = to_rational(self_int);

#ifdef OVERFLOW_CHECK
        if (rational.second == -1) return {-1,-1};
#endif

        long long nn = rational.first;
        long long na = rational.second + 1;
        long long n = gcd(nn,na);
        if (n*n != nn) {
            return {0,0};
        }
        long long a = na/n;
        if (a > n) {
            return {0,0};
        }
        return {n,a};
    }

    // Returns the pair (n,a) of a Wahl singularity. If not Wahl, returns (0,0).
    // This version takes a vector of references to self intersections.
    static std::pair<long long, long long>
    get_wahl_numbers(const std::vector<int>& chain, const std::vector<int>& self_int) {
        auto rational = to_rational(chain,self_int);

#ifdef OVERFLOW_CHECK
        if (rational.second == -1) return {-1,-1};
#endif

        long long nn = rational.first;
        long long na = rational.second + 1;
        long long n = gcd(nn,na);
        if (n*n != nn) {
            return {0,0};
        }
        long long a = na/n;
        if (a > n) {
            return {0,0};
        }
        return {n,a};
    }


    // Assigns the discrepancies of the Wahl chain associated to (n,a) multipled by n (thus negative integers). Assumes 0 < a < n and gcd(a,n) = 1
    static void get_discrepancies(long long n, long long a, std::vector<long long>& discrepancies) {
        discrepancies.resize(0);
        long long q = n*n;
        long long m = n*a-1;
        discrepancies.push_back(a-n);
        long long prev_prev_disc = 0;
        long long prev_disc = a-n;
        while (m > 1ll) {
            discrepancies.push_back(n*(q/m-1ll) + (1ll+q/m)*prev_disc - prev_prev_disc);
            prev_prev_disc = prev_disc;
            prev_disc = discrepancies.back();
            auto temp = m;
            m = m - q%m;
            q = temp;
        }
    }

    // Assigns the discrepancies of the Wahl chain associated to (n,a) multipled by n (thus negative integers). Assumes 0 < a < n and gcd(a,n) = 1
    // This version takes a vector of references representing a chain. The length of the chain is assumed to be l(n,a). Modifies only discrepancies of the positions pointed at by the chain. The size of discrepancies is assumed to be enough.
    static void get_discrepancies(long long n, long long a, const std::vector<int>& chain, std::vector<long long>& discrepancies) {
        long long q = n*n;
        long long m = n*a-1;
        int index = 0;
        discrepancies[chain[index++]] = a-n;
        long long prev_prev_disc = 0;
        long long prev_disc = a-n;
        while (m > 1ll) {
            long long val = n*(q/m-1ll) + (1ll+q/m)*prev_disc - prev_prev_disc;
            discrepancies[chain[index++]] = val;
            prev_prev_disc = prev_disc;
            prev_disc = val;
            auto temp = m;
            m = m - q%m;
            q = temp;
        }
    }

    //Reduces the chain, simulating successive blowdowns. Curves in ignore are not blowndown. Whenever curve not ignored is a (0) or greater, or the process goes out of bounds, stop and return false.
    // reduced_chain is reset.
    // IMPORTANT: self_int is modified!
    // Contracted curves are marked with self intersection INT_MAX
    static bool reduce(const std::vector<int>& chain,
                std::vector<int>& self_int,
                std::vector<int>& reduced_chain,
                const std::unordered_set<int>& ignore) {
        reduced_chain.resize(0);

        /*
        Iterates through chain, adding its members to reduced_chain.
        Any modification done to self_int is done after adding the curve to reduced_chain.
        At each step, we remember in the variable 'erasing' the amount of blowdowns done to the left of the current curve.
        So at that point, this curve should have self intersection C^2 + erasing, where C^2 is the original self intersection.
        If this amount is greater than lower than -1 or is in the 'ignore' set, this curve should not be contracted yet, so it's added to reduced_chain, and self_int is modified. erasing is set to 0, since this curve was not blown down.
        If it is not ignored and its self intersection is 0 or greater, the reduction fails and return false.
        If it is not ignored and its self intersection is -1, then contract it. This means that erasing is set to 1. We must contract all the (-2)-curves at the end of reduced_chain, and for each, erasing is increased by one. The last curve which should not be a (-2)-curve has its self intersection increased by one.
        */

        int erasing = 0;

        for (int curve : chain) {
            int a = -self_int[curve];
            if (a - erasing >= 2 or contains(ignore,curve)) {
                reduced_chain.emplace_back(curve);
                self_int[curve] = -(a-erasing);
                erasing = 0;
            }
            else if (a - erasing <= 0) {
                return false;
            }
            else {
                erasing = 1;
                self_int[curve] = INT_MAX; //this marks that the curve was contracted.
                while (!reduced_chain.empty()) {
                    if (self_int[reduced_chain.back()] == -2 and !contains(ignore,reduced_chain.back())) {
                        self_int[reduced_chain.back()] = INT_MAX;
                        reduced_chain.pop_back();
                        erasing++;
                        continue;
                    }
                    self_int[reduced_chain.back()]++;
                    break;
                }
                if (reduced_chain.empty()) {
                    return false;
                }
            }
        }
        return !erasing;
    }

    // A standard trie that can take one or two arrays of numbers from 0 to n-1.
    // When taking two arrays, it assumes that values in both arrays are not repeated, and assumes the first array is not empty.
    // At the same time, it checks if a given array was inserted and insert it if not.
    struct Trie {
        std::vector<std::vector<int>> nodes;
        int n;

        // Adds array to trie. If it was already added, return true. Otherwise return false.
        bool check_and_add(const std::vector<int>& data) {
            bool found = true;
            int index = 0;
            for (int a : data) {
                if (nodes[index][a] == -1) {
                    found = false;
                    nodes[index][a] = nodes.size();
                    nodes.emplace_back(n,-1);
                }
                index = nodes[index][a];
            }
            return found;
        }

        // Adds fork to trie. If it was already added, return true. Otherwise return false.
        bool check_and_add(const std::vector<int> (&data)[3]) {
            bool found = true;
            int index = 0;
            for (auto& branch : data) for (int a : branch) {
                if (nodes[index][a] == -1) {
                    found = false;
                    nodes[index][a] = nodes.size();
                    nodes.emplace_back(n,-1);
                }
                index = nodes[index][a];
            }
            return found;
        }

        // Adds two arrays to trie. If it was already added, return true. Otherwise return false.
        // Assume data1 is not empty.
        bool check_and_add(const std::vector<int>& data1, const std::vector<int>& data2) {
            bool found = true;
            int index = 0;
            for (int a : data1) {
                if (nodes[index][a] == -1) {
                    found = false;
                    nodes[index][a] = nodes.size();
                    nodes.emplace_back(n,-1);
                }
                index = nodes[index][a];
            }
            // For a separation, use data1[0]. This uniquely this pair since values shouldn't repeat.

            if (nodes[index][data1[0]] == -1) {
                found = false;
                nodes[index][data1[0]] = nodes.size();
                nodes.emplace_back(n,-1);
            }
            index = nodes[index][data1[0]];

            for (int a : data2) {
                if (nodes[index][a] == -1) {
                    found = false;
                    nodes[index][a] = nodes.size();
                    nodes.emplace_back(n,-1);
                }
                index = nodes[index][a];
            }
            return found;
        }

        // Adds fork and array to trie. If it was already added, return true. Otherwise return false.
        bool check_and_add(const std::vector<int> (&data1)[3], const std::vector<int>& data2) {
            bool found = true;
            int index = 0;
            for (int a : data2) {
                if (nodes[index][a] == -1) {
                    found = false;
                    nodes[index][a] = nodes.size();
                    nodes.emplace_back(n,-1);
                }
                index = nodes[index][a];
            }
            for (auto& branch : data1) for (int a : branch) {
                if (nodes[index][a] == -1) {
                    found = false;
                    nodes[index][a] = nodes.size();
                    nodes.emplace_back(n,-1);
                }
                index = nodes[index][a];
            }
            return found;
        }

        void reset(int N) {
            n = N;
            nodes.resize(1);
            nodes[0].assign(n,-1);
        }
    };

    /*
    Implements a doubly linked list representing a singularity that allows blow downs and blow ups while remembering both the blown down curves and the order in which blow ups must be done.
    To do this, each curve keeps a stack of curves blown down at each side. Whenever an intersection is blown up, if the stack is non empty, the top of the stack represents the curve that was blown down at that point originally, so it is recovered.
    Otherwise, a new curve with a new id is created, with id's starting with n, which is the amount of curves originally in the chain.

    It doesn't remember the begining or the end of the chain. Because of this, it could potentially be used for multiple chains. with some modification to reset.
    */
    struct BlowDownLinkedList{
        std::vector<std::stack<int>> ChildrenRight;
        std::vector<std::stack<int>> ChildrenLeft;
        std::vector<int> parentRight;
        std::vector<int> parentLeft;
        std::vector<int> next_in_chain;
        std::vector<int> prev_in_chain;
        std::vector<int> self_int;
        std::vector<int> location;

        // size acts a next id.
        int size;

        void reset(int n, const std::vector<int>& chain, const std::vector<int>& self_int) {
            size = n;
            ChildrenLeft.resize(n);
            ChildrenRight.resize(n);
            next_in_chain.assign(n,-1);
            prev_in_chain.assign(n,-1);
            location.assign(n,-1);
            this->self_int = self_int;
            for(auto& x : ChildrenLeft) while(!x.empty()) x.pop();
            for(auto& x : ChildrenRight) while(!x.empty()) x.pop();
            parentLeft.assign(n,-1);
            parentRight.assign(n,-1);
            for (int i = 0; i < chain.size(); ++i) {
                if(i != 0){
                    prev_in_chain[chain[i]] = chain[i-1];
                }
                if(i != chain.size()-1){
                    next_in_chain[chain[i]] = chain[i+1];
                }
            }
        }

    // Following functions private because of too many assumptions.
    private:
        // Blows down curve with the given index. Assumes that the cure has both parents, is a (-1), etc.
        inline void blowdown(int index) {
            int pLeft = prev_in_chain[index];
            int pRight = next_in_chain[index];
            ChildrenLeft[pRight].push(index);
            ChildrenRight[pLeft].push(index);
            parentLeft[index] = pLeft;
            parentRight[index] = pRight;
            next_in_chain[pLeft] = pRight;
            prev_in_chain[pRight] = pLeft;
            self_int[index] = INT_MAX;
            self_int[pLeft] += 1;
            self_int[pRight] += 1;
        }

        // Blows up an intersection while checking if there was already a curve at that point. Assumes the curves actually intersect.
        // Returns the index of the exceptional curve.
        inline int blowup(int pLeft, int pRight){
            if(ChildrenLeft[pRight].empty()){
                int new_index = size++;
                next_in_chain[pLeft] = new_index;
                prev_in_chain[pRight] = new_index;
                ChildrenLeft.emplace_back();
                ChildrenRight.emplace_back();
                next_in_chain.emplace_back(pRight);
                prev_in_chain.emplace_back(pLeft);
                location.emplace_back(-1);
                self_int.emplace_back(-1);
                self_int[pLeft] -= 1;
                self_int[pRight] -= 1;
                return new_index;
            }
            int a = ChildrenLeft[pRight].top();
            ChildrenRight[pLeft].pop();
            ChildrenLeft[pRight].pop();
            next_in_chain[pLeft] = a;
            prev_in_chain[pRight] = a;
            self_int[a] = -1;
            self_int[pLeft] -= 1;
            self_int[pRight] -= 1;
            return a;
        }

    public:

        // Reduces the chain as with algs::reduce starting from 'start'.
        // Returns true if successful or false if the reduction fails.
        // It remembers all data as explained above.
        bool reduce(int start) {
            while(start != -1){
                int a = - self_int[start];
                int prev_curve = prev_in_chain[start];
                int next_curve = next_in_chain[start];
                if(a >= 2){
                    start = next_curve;
                    continue;
                }
                else if(a <= 0){
                    return false;
                }
                else{
                    if(prev_curve == -1 or next_curve == -1) return false;
                    blowdown(start);
                    while(self_int[prev_curve] == -1){
                        int prev_prev_curve = prev_in_chain[prev_curve];
                        if(prev_prev_curve == -1) return false;
                        blowdown(prev_curve);
                        prev_curve = prev_prev_curve;
                    }
                    start = next_curve;
                }
            }
            return true;
        }

        // Receives a second chain and does the necessary blowups in the original chain so that a subchain starting with 'start' matches the second chain.
        // It fails if there are not enough curves in the original chain, or if the necessary self intersection is lower than what we need.
        // Receives the value 'chain_index' so afterwards we know where each curve belongs.
        bool compare_forward(int start, const std::vector<int>& chain, int chain_index) {
            for(int value : chain){
                location[start] = chain_index;
                if(start == -1 or value > self_int[start]) return false;
                while(value < self_int[start]){
                    int next_curve = next_in_chain[start];
                    if(next_curve == -1) return false;
                    blowup(start,next_curve);
                }
                start = next_in_chain[start];
            }
            return true;
        }

        // The same as before, but the comparison is done backwards in the original chain.
        bool compare_backward(int start, const std::vector<int>& chain, int chain_index) {
            for(int value : chain){
                location[start] = chain_index;
                if(start == -1 or value > self_int[start]) return false;
                while(value < self_int[start]){
                    int prev_curve = prev_in_chain[start];
                    if(prev_curve == -1) return false;
                    blowup(prev_curve,start);
                }
                start = prev_in_chain[start];
            }
            return true;
        }

        // Adds curves to the right of the curve 'index'. Later only used to add tails of (-2)-curves. Assumes that the curve 'index' is the last curve. Returns index of the curve.
        int add_curve_end(int index, int curve_selfint) {
            int new_index = size++;
            next_in_chain[index] = new_index;
            ChildrenLeft.resize(new_index+1);
            ChildrenRight.resize(new_index+1);
            prev_in_chain.emplace_back(index);
            next_in_chain.emplace_back(-1);
            self_int.emplace_back(curve_selfint);
            location.emplace_back(-1);
            return new_index;
        }

        void get_chain_forward(int start, std::vector<int>& chain, int chain_index) {
            chain.resize(0);
            while (start != -1 and location[start] == chain_index) {
                chain.emplace_back(start);
                start = next_in_chain[start];
            }
        }

        void get_chain_backward(int start, std::vector<int>& chain, int chain_index) {
            chain.resize(0);
            while (start != -1 and location[start] == chain_index) {
                chain.emplace_back(start);
                start = prev_in_chain[start];
            }
        }
    };

    static constexpr long long nCr(int n, int r) {
        long long c = 1;
        int k = std::min(n,n-r);
        for (int i = 1; i <= k; ++i) {
            c = c*(n-k+i)/i;
        }
        return c;
    }

    /*
    Adaptation of python itertools nth_combination.
    https://docs.python.org/3/library/itertools.html
    Assumes index between 0 and nCr(n,r)-1.
    Appends results to the vector result
    */
    static void ith_combination(int n, int r, long long index, std::vector<int>& result) {
        long long c = nCr(n,r);
        int N = n;
        while (r) {
            c = c*r/n;
            --n;
            --r;
            while (index >= c) {
                index -= c;
                c = c*(n-r)/n;
                --n;
            }
            result.emplace_back(N-1-n);
        }
    }
}

#endif
