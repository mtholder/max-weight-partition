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
using str_list = std::list<std::string>;
str_list split_string(const std::string &s, const char delimiter);

inline str_list split_string(const std::string &s, const char delimiter) {
    if (s.empty()) {
        return {};
    }
    str_list r;
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
class Data {
public:
    vector<string> idx2name;
    map<string, size_t> name2idx;
};

using broken_line_parser = void (*)(const str_list &, Data & );

void subset_encoder(const str_list & broken_line, Data & data) {
    cout << "subset_encoder" << endl;
}

void name_parser(const str_list & broken_line, Data & data) {
    cout << "name_parser" << endl;

}

void read_labels(string & fp, Data & data, broken_line_parser blp) {
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
            size_t num_read = static_cast<size_t>(i_end - line_c);
            if (num_read != next_line.size()) {
                throw OTCError() << "content after the number of subsets is not allowed on line 1";
            }
        } else {
            auto sub_split = split_string(next_line, ',');
            if (!sub_split.empty()) {
                subsets_read++;
                blp(sub_split, data);
            }
        }
    }
    if (subsets_read != num_subsets) {
        throw OTCError() << "Expecting " << num_subsets << " lines, but read " << subsets_read;
    }
}


void run(std::string &fp) {
    Data data;
    read_labels(fp, data, name_parser);
    read_labels(fp, data, subset_encoder);
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