#pragma once
#ifndef QHD_FUNCTIONS_HPP
#define QHD_FUNCTIONS_HPP

#include"config.hpp"
#include"Algorithms.hpp" // gcd
#include<vector> // vector
#include<algorithm> // swap

namespace algs {

    /*
    Here, some linear algebra to calculate discrepancies of QHD singularities.
    The implementations are somewhat inefficient, but I'm betting on compilers optimizing most of it.
    */

    template<std::size_t n>
    static constexpr long long det(const long long M[n][n]) {
        long long res = 0;
        for (int i = 0; i < n; ++i) {
            long long N[n-1][n-1];
            for (int j = 0; j < n-1; ++j) {
                for (int k = 0; k < n-1; ++k) {
                    N[j][k] = M[j+1][k + (k >= i)];
                }
            }
            res += (i%2?-1:1)*M[0][i]*det<n-1>(N);
        }
        return res;
    }

    template<>
    constexpr long long det<2>(const long long M[2][2]) {
        return M[0][0]*M[1][1] - M[1][0]*M[0][1];
    }

    template<>
    constexpr long long det<1>(const long long M[1][1]) {
        return M[0][0];
    }

    template<std::size_t n>
    static constexpr void adj(const long long M[n][n], long long (&R)[n][n]) {
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                long long N[n-1][n-1];
                for (int k = 0; k < n-1; ++k) {
                    for (int l = 0; l < n-1; ++l) {
                        N[k][l] = M[k + (k >= i)][l + (l >= j)];
                    }
                }
                R[j][i] = det<n-1>(N);
                if ((i+j)%2) R[j][i] *= -1;
            }
        }
    }

    template<>
    constexpr void adj<2>(const long long M[2][2], long long (&R)[2][2]) {
        R[0][0] = M[1][1];
        R[1][1] = M[0][0];
        R[1][0] = -M[1][0];
        R[0][1] = -M[0][1];
    }

    template<>
    constexpr void adj<1>(const long long M[1][1], long long (&R)[1][1]) {
        R[0][0] = 1;
    }

    template<size_t n>
    static constexpr void mul(const long long M[n][n], const long long V[n], long long (&R)[n]) {
        for (int i = 0; i < n; ++i) {
            R[i] = 0;
            for (int j = 0; j < n; ++j) {
                R[i] += M[i][j]*V[j];
            }
        }
    }

    // Given the system M.R = V, returns d.R and d, where d = det(M). Doesn't modify R if d == 0.
    template<std::size_t n>
    static constexpr long long solve(const long long M[n][n], const long long V[n], long long (&R)[n]) {
        long long d = det<n>(M);
        if (d == 0) return 0;
        long long Adj[n][n];
        adj<n>(M,Adj);
        mul<n>(Adj,V,R);
        return d;
    }


    // Follows notation of https://arxiv.org/pdf/0902.2277.pdf
    // The branches are ordered anticlockwise starting from the leftmost.

    // Order of this enum is very important! arithmetics are done with subjacent chars.
    enum QHD_Type : char {
        none,
        type_a,
        type_b,
        type_c,
        type_d,
        type_e,
        type_f,
        type_g,
        type_h,
        type_i,
        type_j
    };

    struct QHD_data {
        QHD_Type type;
        int p;
        int q;
        int r;
        int which_branch[3]; // A permutation of {0,1,2} indicating which branch is the first, second and third.
    };

    // For each branch B of fork, B[0] should be the central framing. As such, B is assumed to be of size at least 2.
    // Branches are assumed reduced.
    // Types a and c are symmetric. The invariants will be chosen so that (p,q,r) is lexicographically greatest.
    static QHD_data get_QHD_type(const std::vector<int> (&fork) [3], const std::vector<int>& self_int) {
        QHD_data data;
        int &p = data.p = 0;
        int &q = data.q = 0;
        int &r = data.r = 0;
        int (&perm)[3] = data.which_branch;

        // By default the test fails.
        data.type = none;
        perm[0] = 0;
        perm[1] = 1;
        perm[2] = 2;
        switch(self_int[fork[0][0]]) {
            case -4: {

                // type a.
                // fork[0] should have length q+2, fork[1] p+2, and fork[2] r+2;
                p = fork[1].size() - 2;
                q = fork[0].size() - 2;
                r = fork[2].size() - 2;
                for (int i = 1; i < q + 1; ++i) {
                    if (self_int[fork[0][i]] != -2) {
                        return data;
                    }
                }
                for (int i = 1; i < p + 1; ++i) {
                    if (self_int[fork[1][i]] != -2) {
                        return data;
                    }
                }
                for (int i = 1; i < r + 1; ++i) {
                    if (self_int[fork[2][i]] != -2) {
                        return data;
                    }
                }

                // Some swapping to compensate false negatives due to symmetry.
                if (self_int[fork[0][q+1]] != -p-3) {
                    if (self_int[fork[0][q+1]] != -r-3) {
                        // Now fail
                        return data;
                    }
                    std::swap(p,r);
                    std::swap(perm[1],perm[2]);
                }
                if (self_int[fork[perm[1]][p+1]] != -r-3) {
                    // If p == r, we could still swap branches 2 and 3
                    if (p != r or self_int[fork[perm[2]][r+1]] != -p-3) {
                        return data;
                    }
                    std::swap(p,r);
                    std::swap(perm[1],perm[2]);
                }
                if (self_int[fork[perm[2]][r+1]] != -q-3) {
                    return data;
                }
                // Fork is QHD type a.
                data.type = type_a;

                // Symmetry is cyclical generated by (p,q,r) -> (q,r,p).
                // Swap a little more to make (p,q,r) lexicographically greatest.
                if (p == q and q == r) return data;
                if (p >= q and p > r) return data;
                if (q >= r and q > p) {
                    std::swap(p,q);
                    std::swap(perm[1],perm[0]);
                    std::swap(q,r);
                    std::swap(perm[0],perm[2]);
                    return data;
                }
                else {
                    std::swap(p,q);
                    std::swap(perm[1],perm[0]);
                    std::swap(p,r);
                    std::swap(perm[1],perm[2]);
                    return data;
                }
            }
            case -3: {
                // type b, c.
                int _2_index; // index of a branch with a single -2
                for (_2_index = 0; _2_index < 3; ++_2_index) {
                    if (fork[_2_index].size() == 2 and self_int[fork[_2_index][1]] == -2) {
                        break;
                    }
                }
                if (_2_index != 3) {
                    // branch with single -2 found
                    std::swap(perm[_2_index],perm[1]);
                    q = fork[perm[0]].size() - 2;
                    r = fork[perm[2]].size() - 2;
                    for (int i = 1; i < q + 1; ++i) {
                        if (self_int[fork[perm[0]][i]] != -2) {
                            // Fail
                            return data;
                        }
                    }
                    for (int i = 1; i < r + 1; ++i) {
                        if (self_int[fork[perm[2]][i]] != -2) {
                            // Fail
                            return data;
                        }
                    }
                    if (self_int[fork[perm[0]][q+1]] != -r-4 or self_int[fork[perm[2]][r+1]] != -q-4) {
                        // Fail
                        return data;
                    }
                    // QHD of type c
                    data.type = type_c;

                    // Swap if r > q to make (q,r) lexicographically greatest.
                    if (r > q) {
                        std::swap(r,q);
                        std::swap(perm[0],perm[2]);
                    }
                    return data;
                }
                else {
                    // int r_index is the index of the branch with r
                    for (int r_index = 0; r_index < 3; ++r_index) {
                        if (fork[r_index].size() > 2) {
                            q = 0;
                            int index = 1;
                            while(index < fork[r_index].size() and self_int[fork[r_index][index]] == -2) {
                                index++;
                                q++;
                            }
                            if (index == fork[r_index].size() or index == fork[r_index].size() - 1) {
                                continue;
                            }
                            if (self_int[fork[r_index][index]] != -3) {
                                // Branch has a non -2 non -3 in its interior. Fail.
                                return data;
                            }
                            index++;
                            p = 0;
                            while(index < fork[r_index].size() and self_int[fork[r_index][index]] == -2) {
                                index++;
                                p++;
                            }
                            if (index != fork[r_index].size() - 1) {
                                continue;
                            }
                            r = - self_int[fork[r_index][index]] - 3;
                            if (r < 0) continue;
                            for (int p_index = 0; p_index < 3; ++p_index) {
                                if (p_index == r_index) continue;
                                if (fork[p_index].size() != 2 or self_int[fork[p_index][1]] != -p-3) continue;
                                int q_index = 3 - r_index - p_index; // The remaining index.
                                if (fork[q_index].size() != r + 2 or self_int[fork[q_index][r+1]] != -q-4) continue;

                                for (int i = 1; i < r + 1; ++i) {
                                    if (self_int[fork[q_index][i]] != -2) {
                                        // Here r > 0 and the branch has a non -2 in the interior. Fail.
                                        return data;
                                    }
                                }
                                // QHD of type b
                                perm[0] = r_index;
                                perm[1] = q_index;
                                perm[2] = p_index;
                                data.type = type_b;
                                return data;
                            }
                            // Fail
                            return data;
                        }
                    }
                    // Fail.
                    return data;
                }
            }
            case -2: {
                // type d,e,f,g,h,i,j
                int _2_index; // Check if a branch has a single -2.
                for (_2_index = 0; _2_index < 3; ++_2_index) {
                    if (fork[_2_index].size() == 2 and self_int[fork[_2_index][1]] == -2) {
                        break;
                    }
                }
                if (_2_index == 3) {
                    // No single -2.
                    // type e,g,h,i
                    // There is a unique > 2 branch: q_index
                    for (int q_index = 0; q_index < 3; ++q_index) {
                        if (fork[q_index].size() == 2) continue;

                        // check if the others have length 2
                        for (int temp = q_index + 1; temp < 3; ++temp) {
                            if (fork[temp].size() != 2) {
                                // Fail.
                                return data;
                            }
                        }

                        q = 0;
                        int index = 1;
                        while(index < fork[q_index].size() and self_int[fork[q_index][index]] == -2) {
                            index++;
                            q++;
                        }
                        if (index == fork[q_index].size()) {
                            // Only -2's. Fail.
                            return data;
                        }
                        if (index == fork[q_index].size() - 1) {
                            // Only -2's in the interior.
                            // type i.
                            q--;
                            if (q < 0) {
                                // Fail
                                return data;
                            }
                            if (self_int[fork[q_index][index]] != -q-3) {
                                // Fail.
                                return data;
                            }
                            // int _3_index is the index of the -3 branch.
                            for (int _3_index = 0; _3_index < 3; ++_3_index) {
                                if (_3_index == q_index) continue;
                                if (self_int[fork[_3_index][1]] == -3) {
                                    int _6_index = 3 - q_index - _3_index; // Remaining index
                                    if (self_int[fork[_6_index][1]] != -6) {
                                        // Fail.
                                        return data;
                                    }
                                    // QHD type i
                                    data.type = type_i;
                                    perm[0] = q_index;
                                    perm[1] = _3_index;
                                    perm[2] = _6_index;
                                    return data;
                                }
                            }
                            // Fail.
                            return data;
                        }
                        if (self_int[fork[q_index][index]] == -4) {
                            // type e
                            p = 0;
                            index++;
                            while(index < fork[q_index].size() and self_int[fork[q_index][index]] == -2) {
                                index++;
                                p++;
                            }
                            if (index != fork[q_index].size() - 1) {
                                // Fail
                                return data;
                            }
                            if (self_int[fork[q_index][index]] != -q-4) {
                                // Fail
                                return data;
                            }
                            for (int p_index = 0; p_index < 3; ++p_index) {
                                if (p_index == q_index) continue;
                                if (self_int[fork[p_index][1]] != -p-3) continue;
                                int _3_index = 3 - p_index - q_index; // The remaining index
                                if (self_int[fork[_3_index][1]] != -3) {
                                    // Fail
                                    return data;
                                }
                                // QHD of type e
                                data.type = type_e;
                                perm[0] = q_index;
                                perm[1] = _3_index;
                                perm[2] = p_index;
                                return data;
                            }
                            // Fail
                            return data;
                        }
                        // Here we can have type g,h.
                        if (self_int[fork[q_index][index]] != -3) {
                            // Fail
                            return data;
                        }
                        if (index == fork[q_index].size() - 2) {
                            // type h
                            if (q != - self_int[fork[q_index][index+1]] - 3) {
                                // Fail
                                return data;
                            }
                            for (int i = 0; i < 3; ++i) {
                                if (i == q_index) continue;
                                if (self_int[fork[i][1]] != -4) {
                                    // Fail
                                    return data;
                                }
                            }
                            // QHD of type h
                            std::swap(perm[0],perm[q_index]);
                            data.type = type_h;
                            return data;
                        }
                        // type g
                        r = 0;
                        index++;
                        while(index < fork[q_index].size() and self_int[fork[q_index][index]] == -2) {
                            index++;
                            r++;
                        }
                        if (index == fork[q_index].size() or index == fork[q_index].size() - 1) {
                            // Fail
                            return data;
                        }
                        if (self_int[fork[q_index][index]] != -3) {
                            // Fail
                            return data;
                        }
                        p = 0;
                        index++;
                        while(index < fork[q_index].size() and self_int[fork[q_index][index]] == -2) {
                            index++;
                            p++;
                        }
                        if (index != fork[q_index].size() - 1) {
                            // Fail
                            return data;
                        }
                        if (self_int[fork[q_index][index]] != -q-4) {
                            // Fail
                            return data;
                        }
                        for (int p_index = 0; p_index < 3; ++p_index) {
                            if (p_index == q_index) continue;
                            if (self_int[fork[p_index][1]] != -p-3) continue;
                            int r_index = 3 - p_index - q_index; // Remaining index
                            if (self_int[fork[r_index][1]] != -r-4) {
                                // Fail
                                return data;
                            }
                            // QHD of type g
                            data.type = type_g;
                            perm[0] = q_index;
                            perm[1] = p_index;
                            perm[2] = r_index;
                            return data;
                        }
                        // Fail
                        return data;
                    }
                    // There are only branches of length 2. Fail.
                    return data;
                }
                else {
                    // There is a single -2 at _2_index
                    // type d,f,j
                    // q branch is the unique branch of length > 2 unless type f and q = 0
                    int q_index;
                    for (q_index = 0; q_index < 3; ++q_index) {
                        if (q_index == _2_index) continue;
                        if (fork[q_index].size() != 2) {
                            break;
                        }
                    }
                    if (q_index == 3) {
                        // Only branches of length 2.
                        // type f with q = 0
                        for (q_index = 0; q_index < 3; ++q_index) {
                            if (q_index == _2_index) continue;
                            if (self_int[fork[q_index][1]] != -6) continue;
                            int _3_index = 3 - q_index - _2_index; // Remaining index
                            if (self_int[fork[_3_index][1]] != -3) {
                                // Fail
                                return data;
                            }
                            perm[0] = _2_index;
                            perm[1] = _3_index;
                            perm[2] = q_index;
                            q = 0;
                            data.type = type_f;
                            return data;
                        }
                        // Fail
                        return data;
                    }
                    else {

                        // check if the others have length 2
                        for (int temp = q_index + 1; temp < 3; ++temp) {
                            if (fork[temp].size() != 2) {
                                // Fail.
                                return data;
                            }
                        }

                        q = 0;
                        int index = 1;
                        while(index < fork[q_index].size() and self_int[fork[q_index][index]] == -2) {
                            index++;
                            q++;
                        }
                        if (index == fork[q_index].size()) {
                            // Fail
                            return data;
                        }
                        if (index == fork[q_index].size() - 1) {
                            // type f,j
                            if (q == -self_int[fork[q_index][index]] - 6) {
                                // type f
                                int _3_index = 3 - _2_index - q_index; // Remaining index
                                if (self_int[fork[_3_index][1]] != -3) {
                                    // Fail
                                    return data;
                                }
                                // QHD of type f
                                perm[0] = _2_index;
                                perm[1] = _3_index;
                                perm[2] = q_index;
                                data.type = type_f;
                                return data;
                            }
                            if (q == -self_int[fork[q_index][index]] - 3) {
                                // type j
                                q--;

                                // This never happens, because the length of this branch would be 2
                                // if (q < 0) {
                                //     // Fail
                                //     return data;
                                // }

                                int _6_index = 3 - _2_index - q_index; // Remaining index
                                if (self_int[fork[_6_index][1]] != -6) {
                                    // Fail
                                    return data;
                                }
                                // QHD of type j
                                perm[0] = _2_index;
                                perm[1] = q_index;
                                perm[2] = _6_index;
                                data.type = type_j;
                                return data;
                            }
                            // Fail
                            return data;
                        }
                        // type d
                        if (self_int[fork[q_index][index]] != -3) {
                            // Fail
                            return data;
                        }
                        r = 0;
                        index++;
                        while(index < fork[q_index].size() and self_int[fork[q_index][index]] == -2) {
                            index++;
                            r++;
                        }
                        if (index != fork[q_index].size() - 1) {
                            // Fail
                            return data;
                        }
                        if (q != -self_int[fork[q_index][index]] - 5) {
                            // Fail
                            return data;
                        }
                        int r_index = 3 - _2_index - q_index; // Remaining index
                        if (self_int[fork[r_index][1]] != -r-4) {
                            // Fail
                            return data;
                        }
                        // QHD of type d
                        perm[0] = q_index;
                        perm[1] = r_index;
                        perm[2] = _2_index;
                        data.type = type_d;
                        return data;
                    }
                }
            }
            default: {
                // Fail
                return data;
            }
        }
    }

    // Assigns the discrepancies of the QHD fork with invariants given by 'data'.
    // For each branch B of fork, B[0] should be the central framing. As such, B is assumed to be of size at least 2.
    // Branches are assumed reduced.
    // This takes vectors of references to self intersections. 'fork' is assumed to be consistent with 'data'. Modifies only discrepancies of the positions pointed at by fork. The size of discrepancies is assumed to be enough.
    // 'discrepancies' will actually contain the numerators of the corresponding discrepancies, thus, negative integers. The denominator is returned by this function.
    // Discrepancies don't seem to overflow for very big values: p,q,r ~ 60.
    static long long get_QHD_discrepancies(const std::vector<int> (&fork)[3], const std::vector<int>& self_int, const QHD_data& data, std::vector<long long>& discrepancies) {
        const int &p = data.p;
        const int &q = data.q;
        const int &r = data.r;
        const int (&perm)[3] = data.which_branch;
        switch(data.type) {
            case type_a: {
                long long M[4][4] = {
                    {-p-2,q-(p+3)*(q+1),0,0},
                    {-q-2,0,r-(q+3)*(r+1),0},
                    {-r-2,0,0,p-(r+3)*(p+1)},
                    {-1,1,1,1}
                };
                long long V[4] = {p+1,q+1,r+1,2};
                long long R[4];
                long long d = solve<4>(M,V,R);
                assert(d);

                {
                    if (d < 0) {
                        d = -d;
                        for (long long& x : R) x = -x;
                    }
                    long long g = d;
                    for (long long x : R) g = gcd(std::abs(x),g);
                    d /= g;
                    for (long long& x : R) x /= g;
                }

                long long C = R[0];
                long long a = R[1];
                long long b = R[2];
                long long c = R[3];
                discrepancies[fork[perm[0]][0]] = C;
                for (int i = 1; i < fork[perm[0]].size(); ++i) {
                    discrepancies[fork[perm[0]][i]] = discrepancies[fork[perm[0]][i-1]] + a;
                }
                for (int i = 1; i < fork[perm[1]].size(); ++i) {
                    discrepancies[fork[perm[1]][i]] = discrepancies[fork[perm[1]][i-1]] + c;
                }
                for (int i = 1; i < fork[perm[2]].size(); ++i) {
                    discrepancies[fork[perm[2]][i]] = discrepancies[fork[perm[2]][i-1]] + b;
                }
                return d;
            }
            case type_b: {
                long long M[4][4] = {
                    {-r-2, - (r+2) * (q+1), p - (p+1) * (r+3), 0},
                    {-1, -q-2, 1, 0},
                    {-q-3, 0, 0, r - (q+4) * (r+1)},
                    {-p-2, p+3, 0, p+3}
                };
                long long V[4] = {r+1, 1, q+2, 2*p+4};
                long long R[4];
                long long d = solve<4>(M,V,R);
                assert(d);

                {
                    if (d < 0) {
                        d = -d;
                        for (long long& x : R) x = -x;
                    }
                    long long g = d;
                    for (long long x : R) g = gcd(std::abs(x),g);
                    d /= g;
                    for (long long& x : R) x /= g;
                }

                long long C = R[0];
                long long a = R[1];
                long long b = R[2];
                long long c = R[3];
                discrepancies[fork[0][0]] = C;
                discrepancies[fork[perm[2]][1]] = C + d - a - c;
                for (int i = 1; i < fork[perm[1]].size(); ++i) {
                    discrepancies[fork[perm[1]][i]] = discrepancies[fork[perm[1]][i-1]] + c;
                }
                for (int i = 1; i < fork[perm[0]].size(); ++i) {
                    discrepancies[fork[perm[0]][i]] = discrepancies[fork[perm[0]][i-1]] + ((i <= q + 1)? a : b);
                }
                return d;
            }
            case type_c: {
                long long M[3][3] = {
                    {-1, 1, 1},
                    {-2 * r - 6, q - (r+4) * (q+1), 0},
                    {-2 * q - 6, 0, r - (q+4) * (r+1)}
                };
                long long V[3] = {1, r + 2, q + 2};
                long long R[3];
                long long d = solve<3>(M,V,R);
                assert(d);

                {
                    if (d < 0) {
                        d = -d;
                        for (long long& x : R) x = -x;
                    }
                    long long g = d;
                    for (long long x : R) g = gcd(std::abs(x),g);
                    d /= g;
                    for (long long& x : R) x /= g;
                }

                long long C = R[0];
                long long a = R[1];
                long long b = R[2];
                discrepancies[fork[0][0]] = 2ll*C;
                discrepancies[fork[perm[1]][1]] = C;
                for (int i = 1; i < fork[perm[0]].size(); ++i) {
                    discrepancies[fork[perm[0]][i]] = discrepancies[fork[perm[0]][i-1]] + a;
                }
                for (int i = 1; i < fork[perm[2]].size(); ++i) {
                    discrepancies[fork[perm[2]][i]] = discrepancies[fork[perm[2]][i-1]] + b;
                }
                return d;
            }
            case type_d: {
                long long M[3][3] = {
                    {-r-2, r+4, 0},
                    {-2, -q-2, 1},
                    {-2 * q - 8, -(q+4) * (q+1), r - (q+5) * (r+1)}
                };
                long long V[3] = {r + 2, 1, q + 3};
                long long R[3];
                long long d = solve<3>(M,V,R);
                assert(d);

                {
                    if (d < 0) {
                        d = -d;
                        for (long long& x : R) x = -x;
                    }
                    long long g = d;
                    for (long long x : R) g = gcd(std::abs(x),g);
                    d /= g;
                    for (long long& x : R) x /= g;
                }

                long long C = R[0];
                long long a = R[1];
                long long b = R[2];
                discrepancies[fork[0][0]] = 2ll*C;
                discrepancies[fork[perm[2]][1]] = C;
                discrepancies[fork[perm[1]][1]] = C - a;
                for (int i = 1; i < fork[perm[0]].size(); ++i) {
                    discrepancies[fork[perm[0]][i]] = discrepancies[fork[perm[0]][i-1]] + ((i <= q + 1) ? a : b);
                }
                return d;
            }
            case type_e: {
                long long M[3][3] = {
                    {-2 * p - 3, p + 3, 0},
                    {-6, -2*q-3, 1},
                    {-3 * q - 9, - (q+3) * (q+1), p - (q+4) * (p+1)}
                };
                long long V[3] = {2*p + 3, 4, 2*q + 5};
                long long R[3];
                long long d = solve<3>(M,V,R);
                assert(d);

                {
                    if (d < 0) {
                        d = -d;
                        for (long long& x : R) x = -x;
                    }
                    long long g = d;
                    for (long long x : R) g = gcd(std::abs(x),g);
                    d /= g;
                    for (long long& x : R) x /= g;
                }

                long long C = R[0];
                long long a = R[1];
                long long b = R[2];
                discrepancies[fork[0][0]] = d + 3ll*C;
                discrepancies[fork[perm[1]][1]] = C;
                discrepancies[fork[perm[2]][1]] = 2ll*C + d - a;
                for (int i = 1; i < fork[perm[0]].size(); ++i) {
                    discrepancies[fork[perm[0]][i]] = discrepancies[fork[perm[0]][i-1]] + ((i <= q + 1) ? a : b);
                }
                return d;
            }
            case type_f: {
                long long d = q + 6;
                long long central_disc = -2*d + 6;
                long long _2_disc = -d + 3;
                long long _3_disc = -d + 2;
                long long a = 1;
                discrepancies[fork[0][0]] = central_disc;
                discrepancies[fork[perm[0]][1]] = _2_disc;
                discrepancies[fork[perm[1]][1]] = _3_disc;
                for (int i = 1; i < fork[perm[2]].size(); ++i) {
                    discrepancies[fork[perm[2]][i]] = discrepancies[fork[perm[2]][i-1]] + a;
                }
                return d;
            }
            case type_g: {
                long long M[4][4] = {
                    {1 - (p + 2) * (r + 3), (p + 3) * (r + 4), 0, 0},
                    {-1, -q-2, 1, 0},
                    {-1, -q - 1, -r - 2, 1},
                    {-q - 3, -(q + 3) * (q + 1), -(q + 3) * (r + 1), p - (q + 4) * (p + 1)}
                };
                long long V[4] = {2 * ((p + 2) * (r + 3) - 1), 1, 1, q + 2};
                long long R[4];
                long long d = solve<4>(M,V,R);
                assert(d);
                {
                    if (d < 0) {
                        d = -d;
                        for (long long& x : R) x = -x;
                    }
                    long long g = d;
                    for (long long x : R) g = gcd(std::abs(x),g);
                    d /= g;
                    for (long long& x : R) x /= g;
                }

                // discrepancy at third branch is (C+2)/(r+4) - 1.
                long long mult = (r + 4)/gcd((long long )r + 4, d);
                // discrepancy at second branch is (C+2)/(p+3) - 1.
                mult *= (p + 3)/gcd((long long )p + 3, d);
                d *= mult;
                for (long long& x : R) x *= mult;
                long long p_disc = (R[0] + 2ll*d)/((long long) p + 3ll) - d;
                long long r_disc = (R[0] + 2ll*d)/((long long) r + 4ll) - d;

                {
                    long long g = d;
                    g = gcd(std::abs(p_disc),g);
                    g = gcd(std::abs(r_disc),g);
                    for (long long x : R) g = gcd(std::abs(x),g);
                    d /= g;
                    for (long long& x : R) x /= g;
                    p_disc /= g;
                    r_disc /= g;
                }

                long long C = R[0];
                long long a = R[1];
                long long b = R[2];
                long long c = R[3];
                discrepancies[fork[0][0]] = C;
                discrepancies[fork[perm[1]][1]] = p_disc;
                discrepancies[fork[perm[2]][1]] = r_disc;
                for (int i = 1; i < fork[perm[0]].size(); ++i) {
                    discrepancies[fork[perm[0]][i]] = discrepancies[fork[perm[0]][i-1]] + ((i <= q + 1) ? a : ((i <= q + r + 2) ? b : c));
                }
                return d;
            }
            case type_h: {
                // the (-3) almost at the end of the first branch has always discrepancy -1.
                // Because of this, discrepancies increase at a constant rate until the very end.
                // x is the discrepancy at a -4.
                long long d = 2*(q+3);
                long long x = -d + 1;
                long long a = 2;
                discrepancies[fork[0][0]] = 4 - 2*d;
                discrepancies[fork[perm[1]][1]] = x;
                discrepancies[fork[perm[2]][1]] = x;
                for (int i = 1; i < fork[perm[0]].size(); ++i) {
                    discrepancies[fork[perm[0]][i]] = discrepancies[fork[perm[0]][i-1]] + a;
                }
                return d;
            }
            case type_i: {
                long long d = 3*(q + 3);
                long long a = 3;
                discrepancies[fork[0][0]] = -2*d + 6;
                discrepancies[fork[perm[1]][1]] = -d + 2;
                discrepancies[fork[perm[2]][1]] = -d + 1;
                for (int i = 1; i < fork[perm[0]].size(); ++i) {
                    discrepancies[fork[perm[0]][i]] = discrepancies[fork[perm[0]][i-1]] + a;
                }
                return d;
            }
            case type_j: {
                long long d = 2*(q+4);
                long long a = 2;
                discrepancies[fork[0][0]] = -2*d + 6;
                discrepancies[fork[perm[0]][1]] = -d + 3;
                discrepancies[fork[perm[2]][1]] = -d + 1;
                for (int i = 1; i < fork[perm[1]].size(); ++i) {
                    discrepancies[fork[perm[1]][i]] = discrepancies[fork[perm[1]][i-1]] + a;
                }
                return d;
            }
            default: return 0;
        }
    }
}

#endif // QHD_FUNCTIONS_HPP
