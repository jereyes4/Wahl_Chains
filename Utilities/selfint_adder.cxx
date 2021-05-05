#include"../src/Reader.cpp"
#include<fstream>
#include<cstdio>

/*
Temporary utility to add self intersection to existing json file using a test file since it was missing in previous builds.

Usage: ./selfint_adder <TestFile> <jsonlFile>
*/

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Error: Must include test file and jsonl file.\n";
        return 1;
    }
    std::ifstream fin;
    fin.open(argv[1]);
    if (fin.fail()) {
        std::cout << "Error while opening file \"" << argv[1] << "\". (Does it exist?)." << std::endl;
        return 1;
    }
    Reader reader;
    reader.parse(fin);
    fin.close();
    std::ifstream fjson;
    fjson.open(argv[2]);
    if (fjson.fail()) {
        std::cout << "Error while opening file \"" << argv[2] << "\". (Does it exist?)." << std::endl;
        return 1;
    }
    std::ofstream fout;
    std::string outname = std::string(argv[2]) + "_temp";
    fout.open(outname);
    if (fout.fail()) {
        std::cout << "Error while opening file \"" << outname << "\"" << std::endl;
        return 1;
    }
    std::string line;
    std::getline(fjson,line);
    fout << line.substr(0,line.size()-1);
    fout << ",\"selfint\":[";
    for (int i = 0; i < reader.self_int.size(); ++i) {
        fout << reader.self_int[i];
        if (i != reader.self_int.size() - 1) fout << ',';
    }
    fout << "]}\n";
    while(std::getline(fjson,line)) fout << line << '\n';
    fjson.close();
    fout.close();
    remove(argv[2]);
    rename(outname.c_str(),argv[2]);
    return 0;
}