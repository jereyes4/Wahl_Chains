#include<iostream>
#include<string>
#include<vector>
#include<fstream>
#include"../src/Reader.hpp"
#include"../src/Searcher.hpp"


#ifdef _WIN32
    #define POPEN _popen
    #define PCLOSE _pclose
#else
    #define POPEN popen
    #define PCLOSE pclose
#endif

class gnuplot{
    FILE* file;
public:
    gnuplot() {
        file = POPEN("gnuplot --persist","w");
    }
    ~gnuplot() {
        if (!file) {
            fprintf(file,"exit\n");
            fflush(file);
            PCLOSE(file);
        }
    }
    gnuplot& operator<<(const char* const s) {
        fprintf(file, s);
        return *this;
    }
    gnuplot& operator<<(const std::string& s) {
        fprintf(file, s.c_str());
        return *this;
    }
    gnuplot& operator<<(char c) {
        fputc(c,file);
        return *this;
    }
    template <class T>
    gnuplot& operator<<(const T& i) {
        std::stringstream ss;
        ss << i;
        fprintf(file, ss.str().c_str());
        return *this;
    }
    void flush() {
        fflush(file);
    }
    bool enabled() const {
        return !!file;
    }

};

int main(int argc, char** argv) {
    gnuplot gp;
    if (!gp.enabled()) {
        std::cout << "gnuplot required: http://www.gnuplot.info/" << std::endl;
        return 1;
    }
    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " <Configuration file> <Pretests data file>" << std::endl;
        return 1;
    }
    std::ifstream f, g;
    f.open(argv[1]);
    if (f.fail()) {
        std::cout << "Error while opening file \"" << argv[1] << "\". (Does it exist?)." << std::endl;
        return 1;
    }
    Searcher searcher;
    searcher.reader_copy.parse(f);
    f.close();
    searcher.init();

    g.open(argv[2]);
    if (g.fail()) {
        std::cout << "Error while opening file \"" << argv[2] << "\". (Does it exist?)." << std::endl;
        return 1;
    }

    // Second Chern class of the original surface
    int Original_Chern_2;
    std::cout << "Input the Euler characteristic of the original surface: ";
    std::cin >> Original_Chern_2;

    std::vector<std::pair<int,double>> sizeAndChern;


    int max_size = 0;
    int min_size = 1000;
    double max_Chern = 0;
    double min_Chern = 10;
    long long real_test;

    std::vector<long long> number_tests;
    searcher.reader_copy.get_test_numbers(number_tests);

    while(g >> real_test) {
        searcher.current_test = real_test;

        while (number_tests[searcher.test_index] + searcher.test_start <= real_test) {
            searcher.test_start += number_tests[searcher.test_index];
            searcher.test_index++;
        }

        long long mask = real_test - searcher.test_start;

        searcher.K2 = searcher.reader_copy.K.self_int;

        if (searcher.reader_copy.curves_used_exactly == -1) {
            // cout << "before get curves mask = " << mask << endl;
            searcher.get_curves_from_mask(mask);
            // cout << "after get curves" << endl;
        }
        else {
            searcher.get_curves_from_mask_exact_curves(mask);
        }
        searcher.contract_exceptional();

        int used_base = 0;

        int euler_config = 0;

        int double_intersections = 0;
        for (auto& fiber : searcher.reader_copy.fibers) {
            for (auto& curve : fiber) {
                if (contains(searcher.temp_included_curves,curve)) {
                    used_base++;
                }
            }
        }
        for (int section : searcher.reader_copy.sections) {
            if (contains(searcher.temp_included_curves, section)) {
                used_base++;
            }
        }
        int sum_self_int = 0;
        for (auto& curve_data : searcher.temp_included_curves) {
            sum_self_int += searcher.temp_self_int[curve_data.first];
            double_intersections += curve_data.second.size();
        }

        int exceptionals = searcher.temp_included_curves.size() - used_base;

        int Chern_1 = searcher.K2 - 4*(exceptionals + used_base) - sum_self_int + double_intersections;

        int Chern_2 = Original_Chern_2 + exceptionals + double_intersections/2 - 2*(exceptionals + used_base);

        double Chern_slope = double(Chern_1)/double(Chern_2);
        min_Chern = std::min(min_Chern, Chern_slope);
        max_Chern = std::max(max_Chern, Chern_slope);
        min_size = std::min(min_size, used_base);
        max_size = std::max(max_size, used_base);
        sizeAndChern.emplace_back(used_base,double(Chern_1)/double(Chern_2));
    }
    gp << "set xrange [" << min_size - 1 << ':' << max_size + 1 << "]\n";
    gp << "set yrange [" << min_Chern - 1 << ':' << max_Chern + 1 << "]\n";
    gp << "set xlabel \"Number of Curves\"\n";
    gp << "set ylabel \"Chern Slope\"\n";
    gp << "plot '-' title \"Pretests\" linetype 7 linecolor 7\n";
    for (auto data : sizeAndChern) {
        gp << data.first << ' ' << data.second << '\n';
    }
    gp << "e\n";
    gp.flush();
    return 0;
}