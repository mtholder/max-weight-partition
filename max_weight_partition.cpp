#include "error.h"
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
using namespace std;
using otc::OTCError;


void read_labels(string & fp, vector<string> & idx2name, map<string, size_t> & name2idx) {
    ifstream inp{fp};
    if (!inp.good()) {
        throw OTCError() << "file \"" << fp << "\" is not a readable file.";
    }
    string next_line;
    int num_subsets = -1;
    while (getline(inp, next_line)) {
        cout << next_line << endl;

    }
}


void run(std::string &fp) {
    vector<string> idx2name;
    map<string, size_t> name2idx;
    read_labels(fp, idx2name, name2idx);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Expecting 1 argument: a filepath to a subset frequencies file.\n";
        return 1;
    }
    std::string fp{argv[1]};
    try {
        run(fp);
    } catch (std::exception & e) {
        std::cerr << "max-weight-partition exception: " << e.what() << std::endl;
        return 2;
    }
    return 0;
}