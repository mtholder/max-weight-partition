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

#include <cstdlib> 
#include "error.h"
using namespace std;
using otc::OTCError;
using str_list = std::list<std::string>;
using subset_t = set<size_t>;
using subset2wt_t = map<subset_t, double>;
using subset_vec_t = vector<subset_t>;

class Resolution {
public:
    Resolution(const subset_vec_t & subsets_vec, double res_score)
      :subsets(subsets_vec),
      score(res_score) {
      }
    const subset_vec_t subsets;
    const double score;
};
using res_by_size_t = map<size_t, Resolution>;

class ConnectedComponent {
public:
    void fill_resolutions();

    subset2wt_t subsets_to_wts;
    subset_t label_set;
    res_by_size_t resolutions;
private:
    void add_resolution(const subset_vec_t &v, double res_score) {
        auto sz = v.size();
        auto res_it = resolutions.find(sz);

        // cerr << "Considering res of size " << sz << " with score = " << res_score << "\n";
        if (res_it == resolutions.end() || res_it->second.score < res_score) {
            // cerr << "Adding res of size " << sz << " with score = " << res_score << "\n";
            resolutions.emplace(std::piecewise_construct,
                                std::forward_as_tuple(sz),
                                std::forward_as_tuple(v, res_score));
        }
    }
    size_t choose_one_label_index() const;
    unsigned int num_subsets_with_label(size_t curr_idx) const;
    
};

class Data {
public:
    void write(ostream & out) const;

    vector<string> idx2name;
    map<string, size_t> name2idx;
    ConnectedComponent cc;
};

using broken_line_parser = void (*)(const str_list &, Data & );
void subset_encoder(const str_list & broken_line, Data & data);
void name_parser(const str_list & broken_line, Data & data);
void read_labels(string & fp, Data & data, broken_line_parser blp);
void validate_data(Data & data);


#endif