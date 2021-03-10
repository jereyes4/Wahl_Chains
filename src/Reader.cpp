#include"Reader.hpp"
using std::string;
using std::vector;
using std::map;
using std::cin;
using std::cout;
using std::endl;
using std::istream;
using std::getline;
using std::stoi;
using std::to_string;


const map<string,vector<vector<int>>> singular_fibers {
    {"I1", {
        {0}
    }},
    {"I2", {
        {1,1},
        {0,0}
    }},
    {"I3", {
        {2,1},
        {0,2},
        {1,0}
    }},
    {"I4", {
        {3,1},
        {0,2},
        {1,3},
        {2,0}
    }},
    {"I5", {
        {4,1},
        {0,2},
        {1,3},
        {2,4},
        {3,0}
    }},
    {"I6", {
        {5,1},
        {0,2},
        {1,3},
        {2,4},
        {3,5},
        {4,0}
    }},
    {"I7", {
        {6,1},
        {0,2},
        {1,3},
        {2,4},
        {3,5},
        {4,6},
        {5,0}
    }},
    {"I8", {
        {7,1},
        {0,2},
        {1,3},
        {2,4},
        {3,5},
        {4,6},
        {5,7},
        {6,0}
    }},
    {"I9", {
        {8,1},
        {0,2},
        {1,3},
        {2,4},
        {3,5},
        {4,6},
        {5,7},
        {6,8},
        {7,0}
    }},
    {"I0*", {
        {1,2,3,4},
        {0},
        {0},
        {0},
        {0}
    }},
    {"I1*", {
        {1,2,3},
        {0,4,5},
        {0},
        {0},
        {1},
        {1}
    }},
    {"I2*", {
        {1,3,4},
        {0,2},
        {1,5,6},
        {0},
        {0},
        {2},
        {2}
    }},
    {"I3*", {
        {1,4,5},
        {0,2},
        {1,3},
        {2,6,7},
        {0},
        {0},
        {3},
        {3}
    }},
    {"I4*", {
        {1,5,6},
        {0,2},
        {1,3},
        {2,4},
        {3,7,8},
        {0},
        {0},
        {4},
        {4}
    }},
    {"IV*", {
        {1,3,5},
        {0,2},
        {1},
        {0,4},
        {3},
        {0,6},
        {5}
    }},
    {"III*", {
        {1,2,5},
        {0},
        {0,3},
        {2,4},
        {3},
        {0,6},
        {5,7},
        {6}
    }},
    {"II*", {
        {1,2,4},
        {0},
        {0,3},
        {2},
        {0,5},
        {4,6},
        {5,7},
        {6,8},
        {7}
    }}
};

//python-like split yay
vector<string> split(const string& s, char c = '\0') {
    vector<string> result;
    int last_whitespace = -1;
    for (int i = 0; i < s.size(); ++i) {
        if ((c != 0 and s[i] == c) or
        (c == 0 and (s[i] == ' ' or s[i] == '\n' or s[i] == '\r' or s[i] == '\t'))) {
            if (i - last_whitespace > 1) {
                result.push_back(s.substr(last_whitespace+1, i - last_whitespace - 1));
            }
            last_whitespace = i;
        }
    }
    if (last_whitespace != s.size() - 1) {
        result.push_back(s.substr(last_whitespace+1));
    }
    return result;
}

bool safe_stoi(const string& s, int& result) {
    try {
        size_t z;
        result = stoi(s,&z);
        if (z != s.size()) {
            return 0;
        }
    }
    catch (std::exception& e) {
        return 0;
    }
    return 1;
}

Reader::Reader() {
    summary_style = plain_text_;
    summary_sort = sort_by_n_;
    nef_check = skip_;
    effective_check = skip_;
    obstruction_check = skip_;
    keep_first = keep_global_;
    section_input_mode = by_self_intersection_;
    only_do_pretest = false;
    search_single_chain = true;
    search_double_chain = false;
    search_single_QHD = false;
    search_double_QHD = false;
    summary_include_gcd = false;
    latex_include_subsection = true;
    line_no = 0;
    curve_no = 0;
    tests_no = 1;
    tests_start_index = 0;
    K.self_int = 0;
    fixed_curves.resize(1);
    try_curves.resize(1);
    choose_curves.resize(1);
    ignored_curves.resize(1);
#ifdef WAHL_MULTITHREAD
    threads = 1;
#endif
    output_filename = DEFAULT_OUTPUT_NAME;
    summary_filename = DEFAULT_SUMMARY_NAME;
    search_for.insert(1);
    search_for.insert(2);
    search_for.insert(3);
    search_for.insert(4);
    error_stream = &std::cerr;
}

void Reader::parse(istream& input) {
    string line;
    string next_line;
    vector<string> tokens;
    vector<string> next_tokens;
    State previous_state = start_;
    State state = start_;
    while (getline(input,line)) {
        line_no++;
        tokens = split(line);
        if (tokens.size() == 0) continue;
        if (tokens[0][0] == '#') continue;
        if (tokens[0].size() > 1 and tokens[0][0] == '/' and tokens[0][1] == '/') continue;
        if (tokens[0].size() > 1 and tokens[0][0] == '/' and tokens[0][1] == '*') {
            previous_state = state;
            state = comment_;
            continue;
        }
        if (state == comment_) {
            if (tokens[0].size() > 1 and tokens[0][0] == '*' and tokens[0][1] == '/') {
                state = previous_state;
                continue;
            }
            continue;
        }
        if (tokens.size() == 1) {
            if (tokens[0] == "Fibers:") {
                state = fibers_;
                continue;
            }
            if (tokens[0] == "DoubleSections:") {
                if (section_input_mode == by_self_intersection_) {
                    current_section_argument = 0;
                }
                else {
                    current_section_argument = -2;
                } 
                state = sections_;
                continue;
            }
            if (tokens[0] == "Sections:") {
                current_section_argument = -1;
                state = sections_;
                continue;
            }
            if (tokens[0] == "Name:" or tokens[0] == "Merge:") {
                state = merge_;
                continue;
            }
            if (tokens[0].substr(0,9) == "Sections(" and
                tokens[0].substr(tokens[0].size() - 2) == "):") {
                int value;
                string temp = tokens[0].substr(9,tokens[0].size() - 11);
                if (safe_stoi(temp,value)) {
                    current_section_argument = value;
                    state = sections_;
                    continue;
                }
                else {
                    error("Invalid self intersection for Sections: " + temp);
                }
            }
        }
        switch (state) {
        case start_:
            parse_option(tokens);
            break;
        case fibers_:
            if (input.eof()) {
                error("Unexpected EOF after Fiber definition.");
            }
            getline(input,next_line);
            line_no++;
            next_tokens = split(next_line);
            parse_fiber(tokens,next_tokens);
            break;
        case sections_:
            if (input.eof()) {
                error("Unexpected EOF after Section definition.");
            }
            getline(input,next_line);
            line_no++;
            next_tokens = split(next_line);
            parse_section(tokens,next_tokens);
            break;
        case merge_:
            if (input.eof()) {
                error("Unexpected EOF after Name/Merge definition.");
            }
            getline(input,next_line);
            line_no++;
            next_tokens = split(next_line);
            parse_merge(tokens,next_tokens);
            break;
        default:;
        }
    }
    // assert(curve_no == self_int.size());
    for (int i = 0; i < curve_no; ++i) {
        if (contains(adj_list[i],i)) {
            error("Curve \'" + curve_name[i] + "\' still has singularities.");
        }
    }
}

void Reader::parse_option(const vector<string>& tokens) {
    if (tokens[0] == "Tests:") {
        if (tokens.size() != 2 and !(tokens.size() == 4 and tokens[2] == "-")) {
            error("Argument for option \'Tests\' must be \'<number>\' or \'<number> - <number>\'.");
        }
        if (tokens.size() == 2) {
            int value;
            if (!safe_stoi(tokens[1],value) or value < 1) {
                error("Invalid number for option \'Tests\': " + tokens[1]);
            }
            tests_no = value;
            fixed_curves.resize(tests_no);
            try_curves.resize(tests_no);
            choose_curves.resize(tests_no);
            ignored_curves.resize(tests_no);
            return;
        }
        else {
            int start, finish;
            if (!safe_stoi(tokens[1],start) or start < 1) {
                error("Invalid number for option \'Tests\': " + tokens[1]);
            }
            if (!safe_stoi(tokens[3],finish) or finish < start) {
                error("Invalid number for option \'Tests\': " + tokens[3]);
            }
            tests_no = finish - start + 1;
            tests_start_index = start - 1;
            fixed_curves.resize(tests_no);
            try_curves.resize(tests_no);
            choose_curves.resize(tests_no);
            ignored_curves.resize(tests_no);
            return;
        }
    }
    else if (tokens[0] == "Output:") {
        if (tokens.size() != 2) {
            error("Option \'Output\' must take exactly one argument.");
        }
        output_filename = tokens[1];
        return;
    }
    else if (tokens[0] == "Summary_Output:") {
        if (tokens.size() != 2) {
            error("Option \'Summary_Output\' must take exactly one argument.");
        }
        summary_filename = tokens[1];
        return;
    }
    else if (tokens[0] == "Pretest_File:") {
        if (tokens.size() != 2) {
            error("Option \'Pretest_File\' must take exactly one argument.");
        }
        warning("Only pretesting not yet implemented. This option does nothing");
        pretest_filename = tokens[1];
        return;
    }
    else if (tokens[0] == "Summary_Style:") {
        if (tokens.size() != 2) {
            error("Option \'Summary_Style\' must take exactly one argument.");
        }
        if (tokens[1] == "LaTeX_Table") {
            summary_style = latex_table_;
        }
        else if (tokens[1] == "Plain_Text") {
            summary_style = plain_text_;
        }
        else {
            error("Invalid argument for \'Summary_Style\': " + tokens[1]);
        }
        return;
    }
    else if (tokens[0] == "Summary_Sort:") {
        if (tokens.size() != 2) {
            error("Option \'Summary_Sort\' must take exactly one argument.");
        }
        if (tokens[1] == "By_N") {
            summary_sort = sort_by_n_;
        }
        else if (tokens[1] == "By_Length") {
            summary_sort = sort_by_length_;
        }
        else {
            error("Invalid argument for \'Summary_Sort\': " + tokens[1]);
        }
        return;
    }
    else if (tokens[0] == "Nef_Check:") {
        if (tokens.size() != 2) {
            error("Option \'Nef_Check\' must take exactly one argument.");
        }
        if (tokens[1] == "Y" or tokens[1] == "skip") {
            nef_check = skip_;
        }
        else if (tokens[1] == "N") {
            nef_check = no_;
        }
        else if (tokens[1] == "print") {
            nef_check = print_;
        }
        else {
            error("Invalid argument for \'Nef_Check\': " + tokens[1]);
        }
        return;
    }
    else if (tokens[0] == "Effective_Check:") {
        if (tokens.size() != 2) {
            error("Option \'Effective_Check\' must take exactly one argument.");
        }
        if (tokens[1] == "Y" or tokens[1] == "skip") {
            effective_check = skip_;
        }
        else if (tokens[1] == "N") {
            effective_check = no_;
        }
        else if (tokens[1] == "print") {
            effective_check = print_;
        }
        else {
            error("Invalid argument for \'Effective_Check\': " + tokens[1]);
        }
        return;
    }
    else if (tokens[0] == "Obstruction_Check:") {
        if (tokens.size() != 2) {
            error("Option \'Obstruction_Check\' must take exactly one argument.");
        }
        if (tokens[1] == "Y" or tokens[1] == "skip") {
            obstruction_check = skip_;
        }
        else if (tokens[1] == "N") {
            obstruction_check = no_;
        }
        else if (tokens[1] == "print") {
            obstruction_check = print_;
        }
        else {
            error("Invalid argument for \'Obstruction_Check\': " + tokens[1]);
        }
        return;
    }
    else if (tokens[0] == "Single_Chain:") {
        if (tokens.size() != 2) {
            error("Option \'Single_Chain\' must take exactly one argument.");
        }
        if (tokens[1] == "Y") {
            search_single_chain = true;
        }
        else if (tokens[1] == "N") {
            search_single_chain = false;
        }
        else {
            error("Invalid argument for \'Single_Chain\': " + tokens[1]);
        }
        return;
    }
    else if (tokens[0] == "Double_Chain:") {
        if (tokens.size() != 2) {
            error("Option \'Double_Chain\' must take exactly one argument.");
        }
        if (tokens[1] == "Y") {
            search_double_chain = true;
        }
        else if (tokens[1] == "N") {
            search_double_chain = false;
        }
        else {
            error("Invalid argument for \'Double_Chain\': " + tokens[1]);
        }
        return;
    }
    else if (tokens[0] == "Single_QHD:") {
        if (tokens.size() != 2) {
            error("Option \'Single_QHD\' must take exactly one argument.");
        }
        if (tokens[1] == "Y") {
            search_single_QHD = true;
        }
        else if (tokens[1] == "N") {
            search_single_QHD = false;
        }
        else {
            error("Invalid argument for \'Single_QHD\': " + tokens[1]);
        }
        return;
    }
    else if (tokens[0] == "Double_QHD:") {
        if (tokens.size() != 2) {
            error("Option \'Double_QHD\' must take exactly one argument.");
        }
        if (tokens[1] == "Y") {
            search_double_QHD = true;
        }
        else if (tokens[1] == "N") {
            search_double_QHD = false;
        }
        else {
            error("Invalid argument for \'Double_QHD\': " + tokens[1]);
        }
        return;
    }
    else if (tokens[0] == "Search_For:") {
        if (tokens.size() == 1) {
            error("Option \'Search_For\' must take at least one argument.");
        }
        search_for.clear();
        int value;
        for (int i = 1; i < tokens.size(); ++i) {
            if(!safe_stoi(tokens[i],value)) {
                error("Invalid number for option \'Search_For\': " + tokens[i]);
            }
            search_for.insert(value);
        }
        return;
    }
    else if (tokens[0] == "K2:") {
        if (tokens.size() != 2) {
            error("Option \'K2\' must take exactly one argument.");
        }
        int value;
        if(!safe_stoi(tokens[1],value)) {
            error("Invalid number for option \'K2\': " + tokens[1]);
        }
        K.self_int = value;
        return;
    }
    else if (tokens[0] == "Keep_First:") {
        if (tokens.size() != 2) {
            error("Option \'Keep_First\' must take exactly one argument.");
        }
        if (tokens[1] == "local") {
            keep_first = keep_local_;
        }
        else if (tokens[1] == "global") {
            keep_first = keep_global_;
        }
        else if (tokens[1] == "N") {
            keep_first = no_;
            warning("Configurations are usually very symmetric. Expect tons of repeated results");

#ifndef NO_REPEATED_SEARCH
            warning("Build does not check for repeated examples within the same test case. Expect even more repeated results.");
#endif
        }
        else {
            error("Invalid argument for \'Keep_First\': " + tokens[1]);
        }
        return;
    }
    else if (tokens[0] == "Threads:") {
        if (tokens.size() != 2) {
            error("Option \'Threads\' must take exactly one argument.");
        }
        int value;
        if (!safe_stoi(tokens[1],value) or value < 1 or value > WAHL_MAX_THREADS) {
            error("Invalid number for option \'Threads\': " + tokens[1]);
        }
#ifdef WAHL_MULTITHREAD
        threads = value;
#else
        warning("Build does not allow threads. This option does nothing.");
#endif
        return;
    }
    else if (tokens[0] == "Sections_Input:") {
        if (tokens.size() != 2) {
            error("Option \'Sections_Input\' must take exactly one argument.");
        }
        if (tokens[1] == "By_Self_Intersection") {
            section_input_mode = by_self_intersection_;
        }
        else if (tokens[1] == "By_Canonical_Intersection") {
            section_input_mode = by_canonical_intersection_;
        }
        else {
            error("Invalid argument for \'Sections_Input\': " + tokens[1]);
        }
    }
    else if (tokens[0] == "Only_Pretest:") {
        if (tokens.size() != 2) {
            error("Option \'Only_Pretest\' must take exactly one argument.");
        }
        if (tokens[1] == "Y") {
            only_do_pretest = true;
        }
        else if (tokens[1] == "N") {
            only_do_pretest = false;
        }
        else {
            error("Invalid argument for \'Only_Pretest\': " + tokens[1]);
        }
        warning("Only pretesting not yet implemented. This option does nothing");
        return;
    }
    else {
        error("Invalid option: " + tokens[0]);
    }
}

void Reader::parse_fiber(const vector<string>& def_tokens, const vector<string>& content_tokens) {
    if (!contains(singular_fibers,def_tokens[0])) {
        error("Unknown fiber type or invalid command: " + def_tokens[0] + ".",line_no-1);
    }
    fiber_type.emplace_back(def_tokens[0]);
    fibers.emplace_back();
    int initial_index = curve_no;
    int n = content_tokens.size();
    for (const string& curve : content_tokens) {
        if (contains(curve_id,curve)) {
            error("Curve \'" + curve + "\' is already defined.");
        }
        curve_id[curve] = curve_no;
        fibers.back().emplace_back(curve_no);
        curve_name.emplace_back(curve);
        adj_list.emplace_back();
        if (n == 1) {
            self_int.emplace_back(0);
        }
        else {
            self_int.emplace_back(-2);
        }
        curve_no++;
    }
    const vector<vector<int>> &fiber_graph = singular_fibers.at(def_tokens[0]);
    if (fiber_graph.size() != n) {
        error("Fiber " + def_tokens[0] + " requires " + to_string(n) + ", " + to_string(content_tokens.size()) + " found.");
    }
    for (int i = 0; i < n; ++i) {
        for (int neighbor : fiber_graph[i]) {
            adj_list[initial_index + i].insert(initial_index + neighbor);
        }
    }
    string option = def_tokens.size() == 1 ? "T" : def_tokens.back();
    for (int t = 0; t < tests_no; ++t) {
        if (def_tokens.size() > t + 1 + tests_start_index) {
            option = def_tokens[t + 1 + tests_start_index];
        }
        if (option == "Fix" or option == "F") {
            for (int curve : fibers.back()) {
                fixed_curves[t].emplace_back(curve);
            }
        }
        else if (option == "Try" or option == "T") {
            for (int curve : fibers.back()) {
                try_curves[t].emplace_back(curve);
            }
        }
        else if (option == "Dis" or option == "D") {
            if (n == 1){
                ignored_curves[t].emplace_back(fibers.back()[0]);
            }
            choose_curves[t].emplace_back(fibers.back());
        }
        else if (option == "Ign" or option == "I") {
            ignored_curves[t].emplace_back(fibers.back()[0]);
        }
        else {
            error("Invalid test option for Fiber " + def_tokens[0] + ".",line_no-1);
        }
    }
}

void Reader::parse_section(const vector<string>& def_tokens, const vector<string>& content_tokens) {
    if (contains(curve_id,def_tokens[0])) {
        error("Curve \'" + def_tokens[0] + "\' is already defined.",line_no-1);
    }
    int this_id = curve_no;
    curve_id[def_tokens[0]] = this_id;
    sections.insert(this_id);
    curve_name.emplace_back(def_tokens[0]);
    adj_list.emplace_back();
    curve_no++;
    // Count the intersections with other curves and use it to get the new self_intersection
    map<int,int> intersections;
    for (const string& curve : content_tokens) {
        if (!contains(curve_id,curve)) {
            error("Curve \'" + curve + "\' is undefined.");
        }
        int id = curve_id[curve];
        adj_list.back().insert(id);
        if (id != this_id) adj_list[id].insert(this_id);
        intersections[id]++;
    }
    if (section_input_mode == by_self_intersection_) {
        self_int.emplace_back(current_section_argument + K.blowup_curve_self_int_delta(intersections));
    }
    else {
        self_int.emplace_back(2*intersections[this_id] - 2 - current_section_argument - K.exceptional_intersection(intersections));
    }

    string option = def_tokens.size() == 1 ? "T" : def_tokens.back();
    for (int t = 0; t < tests_no; ++t) {
        if (def_tokens.size() > t + 1 + tests_start_index) {
            option = def_tokens[t + 1 + tests_start_index];
        }
        if (option == "Fix" or option == "F") {
            fixed_curves[t].emplace_back(this_id);
        }
        else if (option == "Try" or option == "T") {
            try_curves[t].emplace_back(this_id);
        }
        else if (option == "Ign" or option == "I") {
            ignored_curves[t].emplace_back(this_id);
        }
        else {
            error("Invalid test option for Setion \'" + def_tokens[0] + "\'.",line_no-1);
        }
    }
}

void Reader::parse_merge(const vector<string>& def_tokens, const vector<string>& content_tokens) {
    if (contains(curve_id,def_tokens[0])) {
        error("Curve \'" + def_tokens[0] + "\' is already defined.",line_no-1);
    }
    int this_id = curve_no;
    curve_id[def_tokens[0]] = this_id;
    curve_name.emplace_back(def_tokens[0]);
    self_int.emplace_back(-1);
    adj_list.emplace_back();
    curve_no++;
    map<int,int> intersections;
    for (const string& curve : content_tokens) {
        if (!contains(curve_id,curve)) {
            error("Curve \'" + curve + "\' is undefined.");
        }
        int id = curve_id[curve];
        if (id == this_id) {
            error("Exceptional curve \'" + def_tokens[0] + "\' must be smooth, cannot intersect itself.");
        }
        adj_list.back().insert(id);
        adj_list[id].insert(this_id);
        intersections[id]++;
    }
    for (auto iter = intersections.begin(); iter != intersections.end(); ++iter) {
        const int& A = iter->first;
        int& singA = iter->second;
        if (adj_list[A].count(A) < singA*(singA-1)/2) {
            error("Curve \'" + curve_name[A] + "\' does not have enough singularities/arithmetic genus to merge.");
        }
        for (int i = 0; i < singA*(singA-1)/2; ++i) {
            adj_list[A].erase(adj_list[A].find(A));
        }
        self_int[A] -= singA*singA;
        for (auto iter2 = next(iter); iter2 != intersections.end(); ++iter2) {
            const int& B = iter2->first;
            int& singB = iter2->second;
            int A_int_B = adj_list[A].count(B);
            if (A_int_B < singA*singB) {
                error("Curve \'" + curve_name[A] + "\' does not intersect curve \'" + curve_name[B] + "\' enough times.");
            }
            for (int i = 0; i < singA*singB; ++i) {
                adj_list[A].erase(adj_list[A].find(B));
                adj_list[B].erase(adj_list[B].find(A));
            }
        }
    }
    K.blowup(this_id, intersections);
    
    string option = def_tokens.size() == 1 ? "T" : def_tokens.back();
    for (int t = 0; t < tests_no; ++t) {
        if (def_tokens.size() > t + 1 + tests_start_index) {
            option = def_tokens[t + 1 + tests_start_index];
        }
        if (option == "Fix" or option == "F") {
            fixed_curves[t].emplace_back(this_id);
        }
        else if (option == "Try" or option == "T") {
            try_curves[t].emplace_back(this_id);
        }
        else if (option == "Ign" or option == "I") {
            ignored_curves[t].emplace_back(this_id);
        }
        else {
            error("Invalid test option for Name/Merge \'" + def_tokens[0] + "\'.",line_no-1);
        }
    }
}
int Canonical_Divisor::exceptional_intersection(map<int,int>& intersections) {
    int result = 0;
    for (auto& comp : components) {
        result += comp.multiplicity*intersections[comp.id];
    }
    return result;
}

int Canonical_Divisor::blowup_curve_self_int_delta(map<int,int>& intersections) {
    int ex_inters = exceptional_intersection(intersections);
    int delta_arithmetic_genus = 0;
    for (auto iter = components.rbegin(); iter != components.rend(); ++iter) {
        auto& comp = *iter;
        int inters = intersections[comp.id];
        delta_arithmetic_genus += inters*(inters-1)/2;
        if (comp.left_parent != -1) {
            intersections[comp.left_parent] += inters;
        }
        if (comp.right_parent != -1) {
            intersections[comp.right_parent] += inters;
        }
    }
    return -2*delta_arithmetic_genus - ex_inters;
}

void Canonical_Divisor::blowup(int id, const map<int,int>& intersections) {
    self_int--;
    int mult = 1;
    int left_p = -1;
    int right_p = -1;
    for (auto& comp : components) {
        if (contains(intersections,comp.id) and intersections.at(comp.id) > 0) {
            if (left_p == -1) {
                left_p = comp.id;
            }
            else {
                right_p = comp.id;
            }
            mult += comp.multiplicity;
        }
    }
    used_components.insert(id);
    components.emplace_back(id,mult,left_p,right_p);
}

long long Reader::get_test_numbers(vector<long long>& number_of_tests) {
    long long total_tests = 0;
    number_of_tests.resize(tests_no);
    for (int t = 0; t < tests_no; ++t) {
        int curves_to_test = try_curves[t].size();
        for (auto& choose_set : choose_curves[t]) {
            curves_to_test += choose_set.size();
        }
        if (curves_to_test >= 61) {
            error("Too many test cases.");
        }
        long long current_test_number = 1ll<<try_curves[t].size();
        for (auto& choose_set : choose_curves[t]) {
            current_test_number *= (1ll<<choose_set.size()) - 1ll;
        }
        total_tests += current_test_number;
        if (total_tests >= (1ll<<62)) {
            error("Too many test cases.");
        }
        number_of_tests[t] = current_test_number;
    }
    return total_tests;
}
