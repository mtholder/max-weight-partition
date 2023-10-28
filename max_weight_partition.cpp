#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <set>
#include <vector>
#include <list>

#include <cstdlib> 
#include "error.h"
using namespace std;
using otc::OTCError;

// also from otc
using str_list = std::list<std::string>;
using subset_t = set<size_t>;
using subset2wt_t = map<subset_t, double>;

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
    subset2wt_t subsets_to_wts;
};

using broken_line_parser = void (*)(const str_list &, Data & );

void subset_encoder(const str_list & broken_line, Data & data) {
    const auto & name2idx = data.name2idx;
    auto & subsets_to_wts = data.subsets_to_wts;
    double wt = -1.0;
    subset_t sub;
    for (auto word : broken_line) {
        if (wt <= 0) {
            char * w_end;
            wt = strtod(word.c_str(), &w_end);
            size_t num_read = static_cast<size_t>(w_end - word.c_str());
            if (num_read != word.size()) {
                cerr << "word = " << word << " num_read = " << num_read << " wt=" << wt << endl;
                throw OTCError() << "weight not entirely convertable to a floating point number";
            }
            if (wt <= 0.0) {
                throw OTCError() << "weights must be positive";
            }
            continue;
        }
        auto idx = name2idx.at(word);
        if (sub.find(idx) != sub.end()) {
            throw OTCError() << "label \"" << word << "\" repeated in a subset.\n";
        }
        sub.insert(idx);
    }
    if (subsets_to_wts.find(sub) != subsets_to_wts.end()) {
        throw OTCError() << "subset repeated in subset weighting lines";
    }
    subsets_to_wts[sub] = wt;
}

void name_parser(const str_list & broken_line, Data & data) {
    auto & idx2name = data.idx2name;
    auto & name2idx = data.name2idx;
    bool number_read = false;
    bool word_read = false;
    for (auto word : broken_line) {
        if (! number_read) {
            number_read = true;
            continue;
        }
        word_read = true;
        if (name2idx.find(word) == name2idx.end()) {
            name2idx[word] = idx2name.size();
            idx2name.push_back(word);
        }
    }
    if (!word_read) {
        throw OTCError() << "Line without subset element labels.";
    }
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
    unsigned idx = 0;
    for (auto name : data.idx2name) {
        cerr << "  label " << idx++ << " \"" << name << "\"\n";
    }
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