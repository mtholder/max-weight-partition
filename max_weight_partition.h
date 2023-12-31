#ifndef MAX_WEIGHT_PARTITION_H
#define MAX_WEIGHT_PARTITION_H
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <set>
#include <vector>
#include <list>
#include <tuple>
#include <utility>
#include <cassert>
#include <limits>

#include <cstdlib> 
#include "error.h"
using namespace std;
using otc::OTCError;
using str_list = std::list<std::string>;
using subset_t = set<size_t>;

class LightSubset {
public:
    LightSubset(const subset_t & sset)
        :real_set(sset) {
    }
    subset_t::const_iterator begin() const {
        return real_set.begin();
    }
    subset_t::const_iterator end() const {
        return real_set.end();
    }
    bool operator<(const LightSubset &other) const {
        return real_set < other.real_set;
    }
    bool operator==(const LightSubset &other) const {
        return real_set == other.real_set;
    }
    bool operator!=(const LightSubset &other) const {
        return real_set != other.real_set;
    }
    size_t count(size_t el_idx) const {
        return real_set.count(el_idx);
    }
    size_t size() const {
        return real_set.size();
    }
    const subset_t & stored_set() const {
        return real_set;
    }
private:
    const subset_t & real_set; 
};

using subset2wt_t = map<LightSubset, double>;
using subset_vec_t = vector<LightSubset>;

class Resolution {
public:
    Resolution()
        :score(std::numeric_limits<double>::lowest()) {

    }
    Resolution(const subset_vec_t & subsets_vec, double res_score)
      :subsets(subsets_vec),
      score(res_score) {
      }
    subset_vec_t subsets;
    double score;
};
using res_by_size_t = map<size_t, Resolution>;

class ConnectedComponent {
public:
    void fill_resolutions(unsigned num_greedy_steps);

    subset2wt_t subsets_to_wts;
    subset_t label_set;
    res_by_size_t resolutions;
    unsigned int level = 0;
private:
    void add_resolution(subset_vec_t &v, double res_score);
    size_t choose_one_label_index() const;
    unsigned int num_subsets_with_label(size_t curr_idx) const;
    LightSubset get_highest_weight_subset(const set<LightSubset> & taboo_set) const;
    void finish_from_subset(const LightSubset & curr_subset,
                            const vector<LightSubset> & others,
                            unsigned num_greedy_steps);

    
};

class Data {
public:
    void write(ostream & out) const;

    vector<string> idx2name;
    map<string, size_t> name2idx;
    ConnectedComponent cc;
    list<subset_t> stable_subsets;
    // fields that are helpful for debuggin
    map<LightSubset, size_t> subsets_to_subset_idx;
    subset_vec_t input_sub_order;
    size_t num_subsets;
    list<subset_t> tmp_subsets; // for clique-detection
    bool as_log_prob = true;
};

using broken_line_parser = void (*)(const str_list &, Data & );
void subset_encoder(const str_list & broken_line, Data & data);
void grouped_names_detection_loader(const str_list & broken_line, Data & data);
void detect_grouped_names(Data & data);
void name_parser(const str_list & broken_line, Data & data);
void read_labels(string & fp, Data & data, broken_line_parser blp);
void validate_data(Data & data);

// the following are from otcetera

template<typename T>
inline std::set<T> set_intersection_as_set(const std::set<T> & fir, const std::set<T> & sec) {
    std::set<T> d;
    set_intersection(begin(fir), end(fir), begin(sec), end(sec), std::inserter(d, d.end()));
    return d;
}


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

inline std::size_t find_first_graph_index(const std::string & s) {
    std::size_t pos = 0U;
    for (const auto & c : s) {
        if (isgraph(c)) {
            return pos;
        }
        ++pos;
    }
    return std::string::npos;
}

inline std::size_t find_last_graph_index(const std::string & s) {
    auto pos = s.length();
    while (pos > 0) {
        --pos;
        if (isgraph(s[pos])) {
            return pos;
        }
    }
    return std::string::npos;
}

inline std::string strip_surrounding_whitespace(const std::string &n) {
    const auto s = find_first_graph_index(n);
    if (s == std::string::npos) {
        return std::string();
    }
    const auto e = find_last_graph_index(n);
    assert(e != std::string::npos);
    return n.substr(s, 1 + e - s);
}

#endif