#pragma once
#ifndef WRITER_HPP
#define WRITER_HPP

#include"config.hpp"
#include"Searcher.hpp" // Example, std::tuple
#include"Reader.hpp" // Reader, std::string, std::vector, std::map
#include<algorithm> // std::min, std::max
#include<fstream> // ofstream
#include<climits> // INT_MAX

using std::get;

namespace Writer {

    // Returns {n[0],n[1],a[0],a[1],len(chain[0]),len(chain[1])} after swapping so that (n[0],a[0]) >= (n[1],a[1])
    static std::tuple<long long, long long, long long, long long,int,int>
    uniformize_double_by_n(const long long n[2], const long long a[2], int length0 = 0, int length1 = 0) {
        long long n0 = n[0];
        long long n1 = n[1];
        long long a0 = a[0];
        long long a1 = a[1];
        a0 = std::min(a0,n0 - a0);
        a1 = std::min(a1,n1 - a1);
        if (n0 < n1) {
            std::swap(n0,n1);
            std::swap(a0,a1);
            std::swap(length0,length1);
        }
        if (n0 == n1 and a0 < a1) {
            std::swap(a0,a1);
            std::swap(length0,length1);
        }
        return {n0,n1,a0,a1,length0,length1};
    }
    /*
    Sort in the order:
        1. Chain amount (single or double)
        3. K^2
        2. non QHD < QHD
        If all coincide and single chain:
            4. (n,a)
        If all coincide and single QHD:
            4. Type of QHD
            5. invariants of QHD
        If all coincide and both are double or p-extremal:
            Suppose that (n[0],a[0]) >= (n[1],a[1])
            4. (n[0],n[1],a[0],a[1])
            5. non p-extremal < p-extremal
        If all coincide and double QHD or QHD-extremal:
            4. Type of QHD
            5. Invariants of QHD
            6. (n[1],a[1])
            7. non p-extremal < p-extremal
    Meant to be used with stable_sort and where the examples are already sorted by test

    TODO: when p-extremal for QHD this will be inconsistent.
    */
    static bool less_by_n(const Example& a, const Example& b) {
        int amount_a = a.type <= Example::QHD_single_j_ ? 1 : 2;
        int amount_b = b.type <= Example::QHD_single_j_ ? 1 : 2;
        if (amount_a != amount_b) return amount_a < amount_b;
        if (a.K2 != b.K2) return a.K2 < b.K2;
        if (a.type == Example::single_) {
            if (b.type != Example::single_) return true;
            if (a.n[0] != b.n[0]) return a.n[0] < b.n[0];
            long long min_a_a = std::min(a.a[0], a.n[0] - a.a[0]);
            long long min_b_a = std::min(b.a[0], b.n[0] - b.a[0]);
            if (min_a_a != min_b_a) return min_a_a < min_b_a;
            return a.test < b.test;
        }
        else if (b.type == Example::single_) return false;
        else if (a.type <= Example::QHD_single_j_) {
            if (a.type != b.type) return a.type < b.type;
            auto inv_a = std::make_tuple(a.p,a.q,a.r);
            auto inv_b = std::make_tuple(b.p,b.q,b.r);
            if (inv_a != inv_b) return inv_a < inv_b;
            return a.test < b.test;
        }
        else if (a.type <= Example::p_extremal_) {
            if (b.type > Example::p_extremal_) return true;
            auto inv_a = uniformize_double_by_n(a.n,a.a);
            auto inv_b = uniformize_double_by_n(b.n,b.a);
            if (inv_a != inv_b) return inv_a < inv_b;
            if (a.type != b.type) return a.type < b.type;
            return a.test < b.test;
        }
        else if (b.type <= Example::p_extremal_) return false;
        else {
            if (a.type != b.type) return a.type < b.type;
            auto inv_a = std::make_tuple(a.p,a.q,a.r);
            auto inv_b = std::make_tuple(b.p,b.q,b.r);
            if (inv_a != inv_b) return inv_a < inv_b;
            auto inv_chain_a = std::make_tuple(a.n[1],std::min(a.a[1],a.n[1]-a.a[1]));
            auto inv_chain_b = std::make_tuple(b.n[1],std::min(b.a[1],b.n[1]-b.a[1]));
            if (inv_chain_a != inv_chain_b) return inv_chain_a < inv_chain_b;
            return a.test < b.test;
        }
    }

    // Returns {len(chain[0]),len(chain[1]),n[0],n[1],a[0],a[1]} after swapping so that (len(chain[0],n[0],a[0]) >= (len(chain[1],n[1],a[1])
    static std::tuple<int,int,long long, long long, long long, long long>
    uniformize_double_by_length(const long long n[2], const long long a[2],int length0, int length1) {
        long long n0 = n[0];
        long long n1 = n[1];
        long long a0 = a[0];
        long long a1 = a[1];
        a0 = std::min(a0,n0 - a0);
        a1 = std::min(a1,n1 - a1);
        if (length0 < length1) {
            std::swap(length0,length1);
            std::swap(n0,n1);
            std::swap(a0,a1);
        }
        if (length0 == length1 and n0 < n1) {
            std::swap(n0,n1);
            std::swap(a0,a1);
        }
        if (length0 == length1 and n1 == n0 and a0 < a1) {
            std::swap(a0,a1);
        }
        return {length0,length1,n0,n1,a0,a1};
    }

    /*
    Sort in the order:
        1. Chain amount (single or double)
        2. K^2
        3. non QHD < QHD.
        If all coincide and single chain:
            4. Length of chain
            5. (n,a)
        If all coincide and single QHD:
            4. Size of fork
            5. Type of QHD
            6. Invariants of QHD.
        If all coincide and both are double or p-extremal:
            Suppose that length(chain[0]) >= length(chain[1])
            4. length(chain[0])
            5. length(chain[1])
            Suppose that if length(chain[0]) == length(chain[1]) then (n[0],a[0]) >= (n[1],a[1])
            6. (n[0],n[1],a[0],a[1])
            7. non p-extremal < p-extremal
        If all coincide and both are double QHD or QHD-extremal:
            4. Size of fork
            5. length(chain[1])
            6. Type of QHD
            7. Invariants of QHD
            8. (n[1],a[1])
            9. non p-extremal < p-extremal
    Meant to be used with stable_sort and where the examples are already sorted by test

    TODO: when p-extremal for QHD this will be inconsistent.
    */
    static bool less_by_length(const Example& a, const Example& b) {
        int amount_a = a.type <= Example::QHD_single_j_ ? 1 : 2;
        int amount_b = b.type <= Example::QHD_single_j_ ? 1 : 2;
        if (amount_a != amount_b) return amount_a < amount_b;
        if (a.K2 != b.K2) return a.K2 < b.K2;
        if (a.type == Example::single_) {
            if (b.type != Example::single_) return true;
            if (a.chain[0].size() != b.chain[0].size()) return a.chain[0].size() < b.chain[0].size();
            if (a.n[0] != b.n[0]) return a.n[0] < b.n[0];
            long long min_a_a = std::min(a.a[0], a.n[0] - a.a[0]);
            long long min_b_a = std::min(b.a[0], b.n[0] - b.a[0]);
            if (min_a_a != min_b_a) return min_a_a < min_b_a;
            return a.test < b.test;
        }
        else if (b.type == Example::single_) return false;
        else if (a.type <= Example::QHD_single_j_) {
            if (b.type > Example::QHD_single_j_) return true;
            if (a.chain[0].size() != b.chain[0].size()) return a.chain[0].size() < b.chain[0].size();
            if (a.type != b.type) return a.type < b.type;
            auto inv_a = std::make_tuple(a.p,a.q,a.r);
            auto inv_b = std::make_tuple(b.p,b.q,b.r);
            if (inv_a != inv_b) return inv_a < inv_b;
            return a.test < b.test;
        }
        else if (a.type <= Example::p_extremal_) {
            if (b.type > Example::p_extremal_) return true;
            auto inv_a = uniformize_double_by_length(a.n,a.a,a.chain[0].size(),a.chain[1].size());
            auto inv_b = uniformize_double_by_length(b.n,b.a,b.chain[0].size(),b.chain[1].size());
            if (inv_a != inv_b) return inv_a < inv_b;
            if (a.type != b.type) return a.type < b.type;
            return a.test < b.test;
        }
        else if (b.type <= Example::p_extremal_) return false;
        else {
            if (a.chain[0].size() != b.chain[0].size()) return a.chain[0].size() < b.chain[0].size();
            if (a.chain[1].size() != b.chain[1].size()) return a.chain[1].size() < b.chain[1].size();
            if (a.type != b.type) return a.type < b.type;
            auto inv_a = std::make_tuple(a.p,a.q,a.r);
            auto inv_b = std::make_tuple(b.p,b.q,b.r);
            if (inv_a != inv_b) return inv_a < inv_b;
            auto inv_chain_a = std::make_tuple(a.n[1],std::min(a.a[1],a.n[1]-a.a[1]));
            auto inv_chain_b = std::make_tuple(b.n[1],std::min(b.a[1],b.n[1]-b.a[1]));
            if (inv_chain_a != inv_chain_b) return inv_chain_a < inv_chain_b;
            return a.test < b.test;
        }
    }


    static void json_export_bool(std::ostream& f, const char* name, bool value) {
        f << '\"' << name << "\":" << (value?"true":"false");
    }

    template<class T>
    static void json_export_number(std::ostream& f, const char* name, T value) {
        f << '\"' << name << "\":" << value;
    }

    static void json_export_char(std::ostream& f, const char* name, char value) {
        f << '\"' << name << "\":\"" << value << '\"';
    }

    static void json_export_map(std::ostream& f, const char* name, const std::map<std::string,int>& var) {
        f << '\"' << name << "\":{";
        for (auto iter = var.begin(); iter != var.end(); ++iter) {
            f << '\"' << iter->first << "\":" << iter->second;
            if (std::next(iter) != var.end()) f << ',';
        }
        f << '}';
    }

    template<size_t n>
    static void json_export_array(std::ostream& f, const char* name, const int var[n]) {
        f << '\"' << name << "\":[";
        for (int i = 0; i < n; ++i) {
            f << var[i];
            if (i != n-1) f << ',';
        }
        f << ']';
    }

    static void json_export_vector(std::ostream& f, const char* name, const std::vector<int>& var) {
        f << '\"' << name << "\":[";
        for (auto iter = var.begin(); iter != var.end(); ++iter) {
            f << *iter;
            if (std::next(iter) != var.end()) f << ',';
        }
        f << ']';
    }

    static void json_export_vector(std::ostream& f, const char* name, const std::vector<long long>& var) {
        f << '\"' << name << "\":[";
        for (auto iter = var.begin(); iter != var.end(); ++iter) {
            f << *iter;
            if (std::next(iter) != var.end()) f << ',';
        }
        f << ']';
    }

    static void json_export_vector(std::ostream& f, const char* name, const std::vector<std::pair<int,int>>& var) {
        f << '\"' << name << "\":[";
        for (auto iter = var.begin(); iter != var.end(); ++iter) {
            f << '[' << iter->first << ',' << iter->second << ']';
            if (std::next(iter) != var.end()) f << ',';
        }
        f << ']';
    }

    static void json_export_set(std::ostream& f, const char* name, const std::set<int>& var) {
        f << '\"' << name << "\":[";
        for (auto iter = var.begin(); iter != var.end(); ++iter) {
            f << *iter;
            if (std::next(iter) != var.end()) f << ',';
        }
        f << ']';
    }

    static void json_export_vector(std::ostream& f, const char* name, const std::vector<std::string>& var) {
        f << '\"' << name << "\":[";
        for (auto iter = var.begin(); iter != var.end(); ++iter) {
            f << '\"' << *iter << '\"';
            if (std::next(iter) != var.end()) f << ',';
        }
        f << ']';
    }

    static void json_export_vector(std::ostream&f, const char* name, const std::vector<std::vector<int>>& var) {
        f << '\"' << name << "\":[";
        for (auto iter = var.begin(); iter != var.end(); ++iter) {
            f << '[';
            for (auto iter2 = iter->begin(); iter2 != iter->end(); ++iter2) {
                f << *iter2;
                if (std::next(iter2) != iter->end()) f << ',';
            }
            f << ']';
            if (std::next(iter) != var.end()) f << ',';
        }
        f << ']';
    }

    static void json_export_vector(std::ostream&f, const char* name, const std::vector<std::multiset<int>>& var) {
        f << '\"' << name << "\":[";
        for (auto iter = var.begin(); iter != var.end(); ++iter) {
            f << '[';
            for (auto iter2 = iter->begin(); iter2 != iter->end(); ++iter2) {
                f << *iter2;
                if (std::next(iter2) != iter->end()) f << ',';
            }
            f << ']';
            if (std::next(iter) != var.end()) f << ',';
        }
        f << ']';
    }

    static void export_graph_data(std::ofstream& f, Reader& reader) {
        f << '{';

        json_export_map(f,"id",reader.curve_id);
        f << ',';
        json_export_vector(f,"name",reader.curve_name);
        f << ',';
        json_export_vector(f,"Fibers",reader.fibers);
        f << ',';
        json_export_vector(f,"FiberType",reader.fiber_type);
        f << ',';
        json_export_vector(f,"graph",reader.adj_list);
        f << ',';
        json_export_vector(f,"selfint",reader.self_int);
        f << ',';
        json_export_number(f,"K2",reader.K.self_int);
        f << ',';

        std::vector<int> canonical_curves(reader.K.used_components.begin(), reader.K.used_components.end());
        std::sort(canonical_curves.begin(),canonical_curves.end());

        json_export_vector(f,"blps",canonical_curves);
        f << ',';
        json_export_number(f,"INT_MAX",INT_MAX);
        f << ',';
        json_export_bool(f,"nef_check",reader.nef_check != Reader::no_);
        f << ',';
        json_export_bool(f,"effective_check",reader.effective_check != Reader::no_);
        f << ',';
        json_export_bool(f,"obstruction_check",reader.obstruction_check != Reader::no_);

        f << "}\n";
    }

    static void export_example_data(std::ofstream& f, Reader& reader, Example& example) {
        if (example.type == Example::single_) {
            f << '{';
            json_export_number(f,"#",1);
            f << ',';
            json_export_number(f,"K2",example.K2);
            f << ',';
            json_export_number(f,"N",example.n[0]);
            f << ',';
            json_export_set(f,"used",example.used_curves);
            f << ',';
            json_export_set(f,"blds",example.blown_down_exceptionals);
            f << ',';
            json_export_vector(f,"blps",example.blowups);
            f << ',';
            json_export_number(f,"en",example.extra_n[0]);
            f << ',';
            json_export_number(f,"ea",example.extra_orig[0]);
            f << ',';
            json_export_number(f,"eb",example.extra_pos[0]);
            f << ',';
            json_export_vector(f,"chain",example.chain[0]);
            f << ',';
            json_export_vector(f,"selfint",example.self_int);
            f << ',';
            json_export_vector(f,"disc",example.discrepancies);
            if (reader.nef_check != Reader::no_) {
                f << ',';
                json_export_bool(f,"nef",example.nef);
                f << ',';
                json_export_bool(f,"nef_warn",example.nef_warning);
            }
            if (reader.obstruction_check != Reader::no_) {
                f << ',';
                json_export_bool(f,"obs",example.no_obstruction);
            }
            if (reader.effective_check != Reader::no_) {
                f << ',';
                json_export_bool(f,"Qef",example.effective);
            }
            f << "}\n";
        }
        else if (example.type == Example::double_) {
            f << '{';
            json_export_number(f,"#",2);
            f << ',';
            json_export_number(f,"WH",0);
            f << ',';
            json_export_number(f,"K2",example.K2);
            f << ',';
            json_export_number(f,"N0",example.n[0]);
            f << ',';
            json_export_number(f,"N1",example.n[1]);
            f << ',';
            json_export_set(f,"used",example.used_curves);
            f << ',';
            json_export_set(f,"blds",example.blown_down_exceptionals);
            f << ',';
            json_export_vector(f,"blps",example.blowups);
            f << ',';
            json_export_number(f,"en0",example.extra_n[0]);
            f << ',';
            json_export_number(f,"ea0",example.extra_orig[0]);
            f << ',';
            json_export_number(f,"eb0",example.extra_pos[0]);
            f << ',';
            json_export_number(f,"en1",example.extra_n[1]);
            f << ',';
            json_export_number(f,"ea1",example.extra_orig[1]);
            f << ',';
            json_export_number(f,"eb1",example.extra_pos[1]);
            f << ',';
            json_export_vector(f,"chain0",example.chain[0]);
            f << ',';
            json_export_vector(f,"chain1",example.chain[1]);
            f << ',';
            json_export_vector(f,"selfint",example.self_int);
            f << ',';
            json_export_vector(f,"disc",example.discrepancies);
            if (reader.nef_check != Reader::no_) {
                f << ',';
                json_export_bool(f,"nef",example.nef);
                f << ',';
                json_export_bool(f,"nef_warn",example.nef_warning);
            }
            if (reader.obstruction_check != Reader::no_) {
                f << ',';
                json_export_bool(f,"obs",example.no_obstruction);
            }
            if (reader.effective_check != Reader::no_) {
                f << ',';
                json_export_bool(f,"Qef",example.effective);
            }
            f << "}\n";
        }
        else if (example.type == Example::p_extremal_) {
            f << '{';
            json_export_number(f,"#",2);
            f << ',';
            json_export_number(f,"WH",example.worm_hole?2:1);
            f << ',';
            json_export_number(f,"WHid",example.worm_hole?example.worm_hole_id:0);
            f << ',';
            json_export_number(f,"K2",example.K2);
            f << ',';
            json_export_number(f,"N0",example.n[0]);
            f << ',';
            json_export_number(f,"N1",example.n[1]);
            f << ',';
            json_export_set(f,"used",example.used_curves);
            f << ',';
            json_export_set(f,"blds",example.blown_down_exceptionals);
            f << ',';
            json_export_vector(f,"blps",example.blowups);
            f << ',';
            json_export_number(f,"en0",example.extra_n[0]);
            f << ',';
            json_export_number(f,"ea0",example.extra_orig[0]);
            f << ',';
            json_export_number(f,"eb0",example.extra_pos[0]);
            f << ',';
            json_export_number(f,"en1",example.extra_n[1]);
            f << ',';
            json_export_number(f,"ea1",example.extra_orig[1]);
            f << ',';
            json_export_number(f,"eb1",example.extra_pos[1]);
            f << ',';
            json_export_vector(f,"chain_orig",example.chain_original);
            f << ',';
            json_export_vector(f,"selfint_orig",example.self_int_original);
            f << ',';
            json_export_vector(f,"chain0",example.chain[0]);
            f << ',';
            json_export_vector(f,"chain1",example.chain[1]);
            f << ',';
            json_export_vector(f,"selfint",example.self_int);
            f << ',';
            json_export_vector(f,"disc",example.discrepancies);
            f << ',';
            json_export_number(f,"Delta",example.Delta);
            f << ',';
            json_export_number(f,"Omega",example.Omega);
            f << ',';
            json_export_bool(f,"WH_CE",example.worm_hole_conjecture_counterexample);
            if (reader.nef_check != Reader::no_) {
                f << ',';
                json_export_bool(f,"nef",example.nef);
                f << ',';
                json_export_bool(f,"nef_warn",example.nef_warning);
            }
            if (reader.obstruction_check != Reader::no_) {
                f << ',';
                json_export_bool(f,"obs",example.no_obstruction);
            }
            if (reader.effective_check != Reader::no_) {
                f << ',';
                json_export_bool(f,"Qef",example.effective);
            }
            f << "}\n";
        }
        else if (example.type <= Example::QHD_single_j_) {
            f << '{';
            json_export_number(f,"#",1);
            f << ',';
            json_export_number(f,"K2",example.K2);
            f << ',';
            json_export_number(f,"N",example.n[0]);
            f << ',';
            json_export_set(f,"used",example.used_curves);
            f << ',';
            json_export_set(f,"blds",example.blown_down_exceptionals);
            f << ',';
            json_export_vector(f,"blps",example.blowups);
            f << ',';
            json_export_number(f,"en",example.extra_n[0]);
            f << ',';
            json_export_number(f,"ea",example.extra_orig[0]);
            f << ',';
            json_export_number(f,"eb",example.extra_pos[0]);
            f << ',';
            json_export_vector(f,"fork",example.chain[0]);
            f << ',';
            json_export_char(f,"type",example.type - Example::QHD_single_a_ + 'a');
            f << ',';
            json_export_number(f,"p",example.p);
            f << ',';
            json_export_number(f,"q",example.q);
            f << ',';
            json_export_number(f,"r",example.r);
            f << ',';
            json_export_array<3>(f,"perm",example.branch_permutation);
            f << ',';
            json_export_vector(f,"selfint",example.self_int);
            f << ',';
            json_export_vector(f,"disc",example.discrepancies);
            if (reader.nef_check != Reader::no_) {
                f << ',';
                json_export_bool(f,"nef",example.nef);
                f << ',';
                json_export_bool(f,"nef_warn",example.nef_warning);
            }
            if (reader.obstruction_check != Reader::no_) {
                f << ',';
                json_export_bool(f,"obs",example.no_obstruction);
            }
            if (reader.effective_check != Reader::no_) {
                f << ',';
                json_export_bool(f,"Qef",example.effective);
            }
            f << "}\n";
        }
        else if (example.type <= Example::QHD_partial_j_) {
            f << '{';
            json_export_number(f,"#",2);
            f << ',';
            json_export_number(f,"WH",0);
            f << ',';
            json_export_number(f,"K2",example.K2);
            f << ',';
            json_export_number(f,"N0",example.n[0]);
            f << ',';
            json_export_number(f,"N1",example.n[1]);
            f << ',';
            json_export_set(f,"used",example.used_curves);
            f << ',';
            json_export_set(f,"blds",example.blown_down_exceptionals);
            f << ',';
            json_export_vector(f,"blps",example.blowups);
            f << ',';
            json_export_number(f,"en0",example.extra_n[0]);
            f << ',';
            json_export_number(f,"ea0",example.extra_orig[0]);
            f << ',';
            json_export_number(f,"eb0",example.extra_pos[0]);
            f << ',';
            json_export_number(f,"en1",example.extra_n[1]);
            f << ',';
            json_export_number(f,"ea1",example.extra_orig[1]);
            f << ',';
            json_export_number(f,"eb1",example.extra_pos[1]);
            f << ',';
            json_export_vector(f,"fork",example.chain[0]);
            f << ',';
            if (example.type <= Example::QHD_double_j_) {
                json_export_char(f,"type",example.type - Example::QHD_double_a_ + 'a');
            }
            else {
                json_export_char(f,"type",example.type - Example::QHD_partial_a_ + 'a');
            }
            f << ',';
            json_export_number(f,"p",example.p);
            f << ',';
            json_export_number(f,"q",example.q);
            f << ',';
            json_export_number(f,"r",example.r);
            f << ',';
            json_export_array<3>(f,"perm",example.branch_permutation);
            f << ',';
            json_export_vector(f,"chain",example.chain[1]);
            f << ',';
            json_export_vector(f,"selfint",example.self_int);
            f << ',';
            json_export_vector(f,"disc",example.discrepancies);
            if (reader.nef_check != Reader::no_) {
                f << ',';
                json_export_bool(f,"nef",example.nef);
                f << ',';
                json_export_bool(f,"nef_warn",example.nef_warning);
            }
            if (reader.obstruction_check != Reader::no_) {
                f << ',';
                json_export_bool(f,"obs",example.no_obstruction);
            }
            if (reader.effective_check != Reader::no_) {
                f << ',';
                json_export_bool(f,"Qef",example.effective);
            }
            f << "}\n";
        }
        else {
            std::cout << "Unimplemented type" << std::endl;
            exit(0);
        }
    }

    static void export_jsonl(Reader& reader) {
        std::string filename = reader.output_filename + ".jsonl";
        std::ofstream f(filename);
        if (f.fail()) {
            std::cout << "Error while opening file \"" << filename << "\"." << std::endl;
            filename = DEFAULT_OUTPUT_NAME ".jsonl";
            std::cout << "Attempting to open default file \"" << filename << "\"." << std::endl;
            f.clear();
            f.open(filename);
            if (f.fail()) {
                std::cout << "Also failed. Aborting jsonl export." << std::endl;
                return;
            }
        }
        export_graph_data(f,reader);
        f.close();
    }

    static void export_jsonl(Reader& reader, std::vector<Example*>& ptr_examples) {
        std::string filename = reader.output_filename + ".jsonl";
        std::ofstream f(filename);
        if (f.fail()) {
            std::cout << "Error while opening file \"" << filename << "\"." << std::endl;
            filename = DEFAULT_OUTPUT_NAME ".jsonl";
            std::cout << "Attempting to open default file \"" << filename << "\"." << std::endl;
            f.clear();
            f.open(filename);
            if (f.fail()) {
                std::cout << "Also failed. Aborting jsonl export." << std::endl;
                return;
            }
        }

        export_graph_data(f,reader);
        for (Example* example : ptr_examples) {
            export_example_data(f,reader,*example);
        }
        f.close();
    }

    // Writes a summary in a text file. (*) means that even though it says that the example is nef, it requires inspection. (**) means a counter example to the wormhole conjecture.
    static void export_summary_text(const Reader& reader, const std::vector<Example*>& ptr_examples) {

        std::string filename = reader.summary_filename + ".txt";
        std::ofstream f(filename);
        if (f.fail()) {
            std::cout << "Error while opening file \"" << filename << "\"." << std::endl;
            filename = DEFAULT_SUMMARY_NAME ".txt";
            std::cout << "Attempting to open default file \"" << filename << "\"." << std::endl;
            f.clear();
            f.open(filename);
            if (f.fail()) {
                std::cout << "Also failed. Aborting summary export." << std::endl;
                return;
            }
        }

        int prev_chain_amount = 0;
        int prevK = -559038737;

        // Here assume that the examples are sorted first by amount and second by K^2.

        for (Example* ptr : ptr_examples) {
            Example& ex = *ptr;
            int chain_amount = ex.type <= Example::QHD_single_j_ ? 1 : 2;
            if (ex.K2 != prevK) {
                f << "K^2 = " << ex.K2 << ":\n";
                prevK = ex.K2;
            }
            if (chain_amount != prev_chain_amount) {
                f << "  Examples with " << (chain_amount == 1 ? "1 chain" : "2 chains") << ":\n";
                prev_chain_amount = chain_amount;
                int prevK = -559038737;
            }
            if (chain_amount == 1) {
                if (reader.summary_sort == Reader::sort_by_n_) {
                    if (ex.type == Example::single_) {
                        f << "    (" << ex.n[0] << ',' << ex.a[0] << ") l = " << ex.chain[0].size() << '.';
                    }
                    else {
                        char c_type = ex.type - Example::QHD_single_a_ + 'a';
                        f << "    (" << c_type << ';' << ex.p << ',' << ex.q << ',' << ex.r << ") l = " << ex.chain[0].size()-2<< '.';
                    }
                }
                else if (reader.summary_sort == Reader::sort_by_length_) {
                    if (ex.type == Example::single_) {
                        f << "    l = " << ex.chain[0].size() << " (" << ex.n[0] << ',' << ex.a[0] << ").";
                    }
                    else {
                        char c_type = ex.type - Example::QHD_single_a_ + 'a';
                        f << "    l = " << ex.chain[0].size()-2 << " (" << c_type << ';' << ex.p << ',' << ex.q << ',' << ex.r << ';' << ex.n[0] << ").";
                    }
                }
                if (reader.nef_check == Reader::print_) {
                    f << " Nef: " << (ex.nef ? 'Y' : 'N') << '.';
                }
                if (reader.effective_check == Reader::print_) {
                    f << " Q-ef: " << (ex.effective ? 'Y' : 'N') << '.';
                }
                if (reader.obstruction_check == Reader::print_) {
                    f << " Obstruction 0: " << (ex.no_obstruction ? 'Y' : 'N');
                    if (!ex.no_obstruction) {
                        f << '(' << ex.complete_fibers << ')';
                    }
                    f << '.';
                }
                f << " Index: " << ex.export_id  + 1<< '.';
                if (reader.nef_check != Reader::no_ and ex.nef_warning) {
                    f << " (*)";
                }
                f << '\n';
            }
            else {
                if (reader.summary_sort == Reader::sort_by_n_) {
                    if (ex.type <= Example::p_extremal_) {
                        auto inv = uniformize_double_by_n(ex.n,ex.a,ex.chain[0].size(),ex.chain[1].size());
                        f << "    (" << get<0>(inv) << ',' << get<2>(inv) << ") l = " << get<4>(inv) << ", (" << get<1>(inv) << ',' << get<3>(inv) << ") l = " << get<5>(inv) << '.';
                    }
                    else {
                        char c_type;
                        if (ex.type <= Example::QHD_double_j_) {
                            c_type = ex.type - Example::QHD_double_a_ + 'a';
                        }
                        else {
                            c_type = ex.type - Example::QHD_partial_a_ + 'a';
                        }
                        f << "    (" << c_type << ';' << ex.p << ',' << ex.q << ',' << ex.r << ';' << ex.n[0] << ") l = " << ex.chain[0].size()-2 << ", (" << ex.n[1] << ',' << std::min(ex.a[1],ex.n[1] - ex.a[1]) << ") l = " << ex.chain[1].size() << '.';
                    }
                }
                else if (reader.summary_sort == Reader::sort_by_length_) {
                    if (ex.type <= Example::p_extremal_) {
                        auto inv = uniformize_double_by_length(ex.n,ex.a,ex.chain[0].size(),ex.chain[1].size());
                        f << "    l = " << get<0>(inv) << " (" << get<2>(inv) << ',' << get<4>(inv) << "), l = " << get<1>(inv) << " (" << get<3>(inv) << ',' << get<5>(inv) << "), ";
                    }
                    else {
                        char c_type;
                        if (ex.type <= Example::QHD_double_j_) {
                            c_type = ex.type - Example::QHD_double_a_ + 'a';
                        }
                        else {
                            c_type = ex.type - Example::QHD_partial_a_ + 'a';
                        }
                        f << "    l = " << ex.chain[0].size()-2 << " (" << c_type << ';' << ex.p << ',' << ex.q << ',' << ex.r << ';' << ex.n[0] << "), l = " << ex.chain[1].size() << " (" << ex.n[1] << ',' << std::min(ex.a[1],ex.n[1] - ex.a[1]) << "), ";
                    }
                }
                if (reader.summary_include_gcd) {
                    f << " GCD = " << algs::gcd(ex.n[0],ex.n[1]) << '.';
                }
                if (reader.nef_check == Reader::print_) {
                    f << " Nef: " << (ex.nef ? 'Y' : 'N') << '.';
                }
                if (reader.effective_check == Reader::print_) {
                    f << " Q-ef: " << (ex.effective ? 'Y' : 'N') << '.';
                }
                if (reader.obstruction_check == Reader::print_) {
                    f << " Obstruction 0: " << (ex.no_obstruction ? 'Y' : 'N');
                    if (!ex.no_obstruction) {
                        f << '(' << ex.complete_fibers << ')';
                    }
                    f << '.';
                }
                f << " WH: " << (ex.type == Example::p_extremal_ ? (ex.worm_hole ? std::to_string(ex.worm_hole_id + 1) : "N") : "-") << '.';
                f << " Index: " << ex.export_id << '.';
                if (reader.nef_check != Reader::no_ and ex.nef_warning) {
                    f << " (*)";
                }
                if (ex.type == Example::p_extremal_ and ex.worm_hole and ex.worm_hole_conjecture_counterexample) {
                    f << " (**)";
                }
                f << '\n';
            }
        }
    }


    // Writes a summary in a tex file as a longtable. \dagger means that even though it says that the example is nef, it requires inspection. \ddagger means a counter example to the wormhole conjecture.
    static void export_summary_latex(const Reader& reader, const std::vector<Example*>& ptr_examples) {

        std::string filename = reader.summary_filename + ".tex";
        std::ofstream f(filename);
        if (f.fail()) {
            std::cout << "Error while opening file \"" << filename << "\"." << std::endl;
            filename = DEFAULT_SUMMARY_NAME ".tex";
            std::cout << "Attempting to open default file \"" << filename << "\"." << std::endl;
            f.clear();
            f.open(filename);
            if (f.fail()) {
                std::cout << "Also failed. Aborting summary export." << std::endl;
                return;
            }
        }

        // Reminder to include this package
        f << "%\\usepackage{longtable}\n";

        int prev_chain_amount = 0;
        int prevK = -559038737;

        // every entry which is not first in the table must append a \\\\\n at the end of the previous line.
        bool is_first = false;

        for (Example* ptr : ptr_examples) {
            Example& ex = *ptr;
            int chain_amount = ex.type <= Example::QHD_single_j_ ? 1 : 2;
            if (chain_amount != prev_chain_amount or prevK != ex.K2) {
                if (prev_chain_amount != 0) {
                    // There was a table before that must be ended
                    f << "\n\\end{longtable}\n";
                }
                is_first = true;
                prev_chain_amount = chain_amount;
                prevK = ex.K2;

                int columns = 2*chain_amount + 1; // for (n,a) + length for each chain and index

                std::string chain_amount_text = (chain_amount == 1 ? "1 chain" : "2 chains");
                std::string header = "$(n,a)$ & Length & ";

                if (chain_amount == 2) {
                    header += "$(n,a)$ & Length & ";
                }
                if (chain_amount == 2 and reader.summary_include_gcd) {
                    header += "GCD & ";
                    columns++;
                }
                if (reader.nef_check == Reader::print_) {
                    header += "Nef & ";
                    columns++;
                }
                if (reader.effective_check == Reader::print_) {
                    header += "$\\mathbb Q$-ef & ";
                    columns++;
                }
                if (reader.obstruction_check == Reader::print_) {
                    header += "Obstruction 0 & ";
                    columns++;
                }
                if (chain_amount == 2) {
                    header += "WH & ";
                    columns++;
                }
                header += "Index";

                if (reader.latex_include_subsection) {
                    f << "\\subsection{" << chain_amount_text << ", \\(K^2 = " << ex.K2 << "\\)}\n";
                }

                f << "\\begin{longtable}{|";
                for (int i = 0; i < columns; ++i) f << "c|";
                f <<
                "}\n"
                "\\hline\n"
                "\\multicolumn{" << columns << "}{|c|}{" << chain_amount_text << ", $K^2 = " << ex.K2 << "$}\\\\\n"
                "\\hline\n"
                << header << "\\\\\n"
                "\\hline\n"
                "\\endfirsthead\n"
                "\n"
                "\\hline\n"
                << header << "\\\\\n"
                "\\hline\n"
                "\\endhead\n"
                "\\hline\n"
                "\\endfoot\n"
                "\n";
            }
            if (!is_first) {
                f << "\\\\\n";
            }
            else {
                is_first = false;
            }
            if (chain_amount == 1) {
                if (ex.type == Example::single_) {
                    f << "$(" << ex.n[0] << ", " << std::min(ex.a[0], ex.n[0] - ex.a[0]) << ")$ & " << ex.chain[0].size() << " & ";
                }
                else {
                    char c_type = ex.type - Example::QHD_single_a_ + 'a';
                    f << "$(" << c_type << "; " << ex.p << ", " << ex.q << ", " << ex.r << "; " << ex.n[0] << ")$ & " << ex.chain[0].size()-2 << " & ";
                }
                if (reader.nef_check == Reader::print_) {
                    f << (ex.nef ? "YES" : "NO") << " & ";
                }
                if (reader.effective_check == Reader::print_) {
                    f << (ex.effective ? "YES" : "NO") << " & ";
                }
                if (reader.obstruction_check == Reader::print_) {
                    f << (ex.no_obstruction ? "YES" : "NO");
                    if (!ex.no_obstruction) {
                        f << '(' << ex.complete_fibers << ')';
                    }
                    f << " & ";
                }
                f << ex.export_id + 1;
                if (reader.nef_check != Reader::no_ and ex.nef_warning) {
                    f << " ${}^\\dagger$";
                }
            }
            else {
                if (reader.summary_sort == Reader::sort_by_n_) {
                    if (ex.type <= Example::p_extremal_) {
                        auto inv = uniformize_double_by_n(ex.n,ex.a,ex.chain[0].size(),ex.chain[1].size());
                        f << "$(" << get<0>(inv) << ", " << get<2>(inv) << ")$ & " << get<4>(inv) << " & "
                             "$(" << get<1>(inv) << ", " << get<3>(inv) << ")$ & " << get<5>(inv) << " & ";
                    }
                    else {
                        char c_type;
                        if (ex.type <= Example::QHD_double_j_) {
                            c_type = ex.type - Example::QHD_double_a_ + 'a';
                        }
                        else {
                            c_type = ex.type - Example::QHD_partial_a_ + 'a';
                        }
                        f << "$(" << c_type << "; " << ex.p << ", " << ex.q << ", " << ex.r << "; " << ex.n[0] << ")$ & " << ex.chain[0].size()-2 << " & "
                             "$(" << ex.n[1] << ", " << std::min(ex.a[1], ex.n[1] - ex.a[1]) << ")$ & " << ex.chain[1].size() << " & ";
                    }
                }
                else if (reader.summary_sort == Reader::sort_by_length_) {
                    if (ex.type <= Example::p_extremal_) {
                        auto inv = uniformize_double_by_length(ex.n,ex.a,ex.chain[0].size(),ex.chain[1].size());
                        f << "$(" << get<2>(inv) << ", " << get<4>(inv) << ")$ & " << get<0>(inv) << " & "
                             "$(" << get<3>(inv) << ", " << get<5>(inv) << ")$ & " << get<1>(inv) << " & ";
                    }
                    else {
                        char c_type;
                        if (ex.type <= Example::QHD_double_j_) {
                            c_type = ex.type - Example::QHD_double_a_ + 'a';
                        }
                        else {
                            c_type = ex.type - Example::QHD_partial_a_ + 'a';
                        }
                        f << "$(" << c_type << "; " << ex.p << ", " << ex.q << ", " << ex.r << "; " << ex.n[0] << ")$ & " << ex.chain[0].size()-2 << " & "
                             "$(" << ex.n[1] << ", " << std::min(ex.a[1], ex.n[1] - ex.a[1]) << ")$ & " << ex.chain[1].size() << " & ";
                    }
                }
                if (reader.summary_include_gcd) {
                    f << algs::gcd(ex.n[0],ex.n[1]) << " & ";
                }
                if (reader.nef_check == Reader::print_) {
                    f << (ex.nef ? "YES" : "NO") << " & ";
                }
                if (reader.effective_check == Reader::print_) {
                    f << (ex.effective ? "YES" : "NO") << " & ";
                }
                if (reader.obstruction_check == Reader::print_) {
                    f << (ex.no_obstruction ? "YES" : "NO");
                    if (!ex.no_obstruction) {
                        f << '(' << ex.complete_fibers << ')';
                    }
                    f << " & ";
                }
                if (ex.type != Example::p_extremal_) f << "-- & ";
                else if (ex.worm_hole) {
                    if (ex.worm_hole_conjecture_counterexample) {
                        f << "CE ${}^\\ddagger$ & ";
                    }
                    else {
                        f << ex.worm_hole_id + 1 << " & ";
                    }
                }
                else {
                    f << "NO & ";
                }
                f << ex.export_id + 1;
                if (reader.nef_check != Reader::no_ and ex.nef_warning) {
                    f << " ${}^\\dagger$";
                }
            }
        }
        if (prev_chain_amount != 0) {
            f << "\n\\end{longtable}\n";
        }
        f.close();
    }

    #ifdef EXPORT_PRETEST_DATA
    static void export_pretest_data(const Reader& reader, const std::vector<long long> passed_pretest_list) {

        std::string filename = reader.pretest_filename;
        std::ofstream f(filename);
        if (f.fail()) {
            std::cout << "Error while opening file \"" << filename << "\"." << std::endl;
            filename = DEFAULT_PRETEST_NAME;
            std::cout << "Attempting to open default file \"" << filename << "\"." << std::endl;
            f.clear();
            f.open(filename);
            if (f.fail()) {
                std::cout << "Also failed. Aborting pretest data export." << std::endl;
                return;
            }
        }

        for (long long x : passed_pretest_list) {
            f << x << '\n';
        }

        f.close();
    }
    #endif //EXPORT_PRETEST_DATA
}

#endif
