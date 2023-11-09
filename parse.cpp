#include "max_weight_partition.h"
#include <cassert>
#include <cmath>



void subset_encoder(const str_list & broken_line, Data & data) {
    const auto & name2idx = data.name2idx;
    auto & subsets_to_wts = data.cc.subsets_to_wts;
    auto & subsets_to_subset_idx = data.subsets_to_subset_idx;
    double wt = -1.0; // at this point, weight (from the file will be positive)
                      //   log-transform happens after.
    data.stable_subsets.emplace_back();
    subset_t & sub = *(data.stable_subsets.rbegin());
    data.input_sub_order.reserve(data.num_subsets);
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
        // don't check for duplicates, because our grouped names will lead to dups.
        sub.insert(idx);
    }
    LightSubset lsub{sub};
    if (subsets_to_wts.find(lsub) != subsets_to_wts.end()) {
        throw OTCError() << "subset repeated in subset weighting lines";
    }
    subsets_to_wts[lsub] = wt;
    subsets_to_subset_idx[lsub] = data.input_sub_order.size();
    data.input_sub_order.push_back(lsub);
}

void grouped_names_detection_loader(const str_list & broken_line, Data & data) {
    const auto & name2idx = data.name2idx;
    subset_t sub;
    bool first = true;
    for (auto word : broken_line) {
        if (first) {
            first = false;
            continue;
        }
        auto idx = name2idx.at(word);
        if (sub.find(idx) != sub.end()) {
            throw OTCError() << "label \"" << word << "\" repeated in a subset.\n";
        }
        sub.insert(idx);
    }
    data.tmp_subsets.push_back(sub);
}

void detect_grouped_names(Data & data) {
    auto & idx2name = data.idx2name;
    
    set<size_t> ids_to_check;
    for (size_t i=0; i < idx2name.size(); ++i) {
        ids_to_check.insert(i);
    }
    set<size_t> remapped;
    list<subset_t> remapping_in_order;

    while (!ids_to_check.empty()) {
        auto next_to_check = ids_to_check.begin();
        auto raw_label_idx = *next_to_check;
        ids_to_check.erase(next_to_check);
        if (remapped.count(raw_label_idx) == 1) {
            continue;
        }
        auto scratch_id_set = ids_to_check;
        for (auto & sub : data.tmp_subsets) {
            if (sub.count(raw_label_idx) == 0) {
                continue;
            }
            scratch_id_set = set_intersection_as_set(scratch_id_set, sub);
            if (scratch_id_set.empty()) {
                break;
            }
        }
        scratch_id_set.insert(raw_label_idx);
        if (!scratch_id_set.empty()) {
            remapping_in_order.push_back(scratch_id_set);
            for (auto el : scratch_id_set) {
                if (el != raw_label_idx) {
                    ids_to_check.erase(el);
                }
                remapped.insert(el);
            }
        }
    }
    if (remapping_in_order.empty()) {
        assert(remapped.empty());
        return;
    }
    assert(!remapped.empty());
    vector<string> remapped_idx2name;
    remapped_idx2name.reserve(data.idx2name.size());
    for (auto rs : remapping_in_order) {
        stringstream nn;
        bool first = true;
        for (auto el : rs) {
            if (first) {
                first = false;
            } else {
                nn << "\", \"";
            }
            nn << idx2name.at(el);
        }
        remapped_idx2name.push_back(nn.str());
    }
    size_t old_idx = 0;
    for (auto old_name :  data.idx2name) {
        if (remapped.count(old_idx) == 1) {
            old_idx++;
            continue;
        }
        remapped_idx2name.push_back(idx2name.at(old_idx++));
    }
    map<string, size_t> complete_name2idx;
    auto ri2n_it = remapped_idx2name.begin();
    auto remapping_it = remapping_in_order.begin();
    for (size_t idx=0; idx < remapped_idx2name.size(); ++idx) {
        complete_name2idx[*ri2n_it++] = idx;
        if (idx < remapping_in_order.size()) {
            const auto & old_name_idx_set = *remapping_it++;
            for (auto old_id: old_name_idx_set) {
                auto old_name = idx2name.at(old_id);
                complete_name2idx[old_name] = idx;
            }
        }
    }
    assert(ri2n_it == remapped_idx2name.end());
    assert(remapping_it == remapping_in_order.end());
    swap(data.idx2name, remapped_idx2name);
    swap(data.name2idx, complete_name2idx);    
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
    data.num_subsets = static_cast<size_t>(num_subsets);
    
}

void validate_data(Data & data) {
    map<size_t, double> label2wtsum;
    for (size_t idx=0; idx < data.idx2name.size(); ++idx) {
        data.cc.label_set.insert(idx);
        label2wtsum[idx] = 0.0;
    }
    for (auto s2w : data.cc.subsets_to_wts) {
        const auto & subset = s2w.first;
        double wt = s2w.second;
        for (auto el : subset) {
            label2wtsum[el] += wt;
        }
    }
    cerr << data.cc.label_set.size() << " labels" << endl;
    double sum_wts = -1.0;
    for (auto l2w : label2wtsum) {
        if (sum_wts < 0.0) {
            sum_wts = l2w.second;
        } else if (abs(sum_wts - l2w.second) > 1e-6) {
            throw OTCError() << "label \"" << l2w.first << "\" had weights summing to " << l2w.second << " instead of " << sum_wts;
        }
    }
    cerr << "each label has a sum of weights = " << sum_wts << endl;

    if (data.as_log_prob) {
        const double negLnCount = -log(sum_wts);
        for (auto & s2w : data.cc.subsets_to_wts) {
            s2w.second = log(s2w.second) + negLnCount;
        }
    }
}

