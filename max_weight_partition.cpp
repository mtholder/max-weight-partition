#include "error.h"
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <list>

#include <stdlib.h>     /* strtol */
using namespace std;
using otc::OTCError;

// also from otc
std::list<std::string> split_string(const std::string &s, const char delimiter);

std::list<std::string> split_string(const std::string &s, const char delimiter) {
    // FIXME - this preserves old behavior - I don't know if we actually need this
    if (s.empty()) {
        return {};
    }
    std::list<std::string> r;
    r.push_back({});
    for (const auto & c : s) {
        if (c == delimiter) {
            r.push_back({});
        } else {
            r.back().append(1, c);
        }
    }
    return r;
}

// end otc

void read_labels(string & fp, vector<string> & idx2name, map<string, size_t> & name2idx) {
    ifstream inp{fp};
    if (!inp.good()) {
        throw OTCError() << "file \"" << fp << "\" is not a readable file.";
    }
    string next_line;
    long num_subsets = -1;
    long subsets_read = 0;
    while (getline(inp, next_line)) {
        if (num_subsets < 0) {
            char * i_end;
            auto line_c = next_line.c_str();
            num_subsets = strtol(line_c, &i_end, 10);
            //double d = strtod(s.c_str(), nullptr);
            if (num_subsets < 1) {
                throw OTCError() << "at least on subset is required.";
            }
            if ((i_end - line_c) != next_line.size()) {
                throw OTCError() << "content after the number of subsets is not allowed on line 1";
            }
        } else {
            auto sub_split = split_string(next_line, ',');
            if (!sub_split.empty()) {
                subsets_read++;
            }
        }
    }
    if (subsets_read != num_subsets) {
        throw OTCError() << "Expecting " << num_subsets << " lines, but read " << subsets_read;
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