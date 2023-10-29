#ifndef MAX_WEIGHT_PARTITION_H
#define MAX_WEIGHT_PARTITION_H
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
using str_list = std::list<std::string>;
using subset_t = set<size_t>;
using subset2wt_t = map<subset_t, double>;

class ConnectedComponent {
public:
    subset2wt_t subsets_to_wts;
    subset_t label_set;
};

class Data {
public:
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