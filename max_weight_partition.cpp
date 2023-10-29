#include "max_weight_partition.h"
#include <cassert>
#include <cstddef>
#include <climits>
#include <algorithm>

using cache_map_t = std::map<subset_t, ConnectedComponent>;

cache_map_t SOLN_CACHE;
bool USE_MOST_FREQ = false;

list<ConnectedComponent> sub_cc_list;

ConnectedComponent * cc_for_subset(const subset_t & labels_needed,
                                  const vector<subset_t > &viable_others,
                                  const ConnectedComponent & par_cc) {
    assert(!viable_others.empty());
    subset_t labels_in_vo;
    bool have_labels_we_need = false;
    for (auto vop : viable_others) {
        labels_in_vo.insert(vop.begin(), vop.end());
        if (labels_in_vo == labels_needed) {
            have_labels_we_need = true;
            break;
        }
    }
    if (! have_labels_we_need) {
        return nullptr;
    }
    auto cache_it =  SOLN_CACHE.find(labels_needed);
    if (cache_it != SOLN_CACHE.end()) {
        return &(cache_it->second);
    }

    sub_cc_list.emplace_back();
    ConnectedComponent & new_cc = *(sub_cc_list.rbegin());
    for (auto vop : viable_others) {
        const subset_t & subset = vop;
        new_cc.subsets_to_wts[subset] = par_cc.subsets_to_wts.at(subset);
    }
    swap(new_cc.label_set, labels_in_vo);
    return &new_cc;
}

//////////////////////////////////////////////
// from: https://stackoverflow.com/a/1964252 by user 
// https://stackoverflow.com/users/127669/graphics-noob

template<class Set1, class Set2> 
inline bool is_disjoint(const Set1 &set1, const Set2 &set2)
{
    if(set1.empty() || set2.empty()) {
        return true;
    }
    typename Set1::const_iterator it1 = set1.begin();
    typename Set1::const_iterator it1End = set1.end();
    typename Set2::const_iterator it2 = set2.begin();
    typename Set2::const_iterator it2End = set2.end();
    if(*it1 > *set2.rbegin() || *it2 > *set1.rbegin()) {
        return true;  
    } 

    while(it1 != it1End && it2 != it2End) {
        if(*it1 == *it2) {
            return false;
        }
        if(*it1 < *it2) {
            it1++; 
        } else {
            it2++;
        }
    }

    return true;
}

///////////////////////////////////////////////
inline unsigned int ConnectedComponent::num_subsets_with_label(size_t curr_idx) const {
    unsigned int count_curr = 0;
    for (auto s2w_it : subsets_to_wts) {
        if (s2w_it.first.count(curr_idx) == 1) {
            count_curr++;
        }
    }
    return count_curr;
}

inline size_t ConnectedComponent::choose_one_label_index() const {
    if (USE_MOST_FREQ) {
        size_t commonest_label_index = SIZE_MAX;
        unsigned int count_for_commonest = 0;
        for (auto curr_idx : label_set) {
            auto count_curr = num_subsets_with_label(curr_idx);
            if (count_curr > count_for_commonest) {
                count_for_commonest = count_curr;
                commonest_label_index = curr_idx;
            }
        }
        assert(commonest_label_index != SIZE_MAX);
        return commonest_label_index;
    }
    size_t rarest_label_index = SIZE_MAX;
    unsigned int count_for_rarest = UINT_MAX;
    for (auto curr_idx : label_set) {
        auto count_curr = num_subsets_with_label(curr_idx);
        if (count_curr < count_for_rarest) {
            count_for_rarest = count_curr;
            rarest_label_index = curr_idx;
        }
    }
    assert(rarest_label_index != SIZE_MAX);
    return rarest_label_index;
}

void ConnectedComponent::fill_resolutions() {
    assert(!subsets_to_wts.empty());
    if (subsets_to_wts.size() == 1) {
        const auto first_it = subsets_to_wts.begin();
        subset_vec_t subsets_vec{1, first_it->first};
        add_resolution(subsets_vec, first_it->second);
        return;
    }
    size_t one_label_idx = choose_one_label_index();
    vector<subset_t> alternatives;
    vector<subset_t> others;
    alternatives.reserve(subsets_to_wts.size());
    others.reserve(subsets_to_wts.size());
    for (auto s2w_it : subsets_to_wts) {
        const subset_t & subset_ref = (s2w_it.first);
        if (subset_ref.count(one_label_idx) == 1) {
            alternatives.push_back(subset_ref);
        } else {
            others.push_back(subset_ref);
        }
    }
    assert(!alternatives.empty());
    vector<subset_t> viable_others;
    viable_others.reserve(subsets_to_wts.size());
    for (auto alt : alternatives) {
        const subset_t & curr_subset = alt;
        double curr_score = subsets_to_wts.at(curr_subset);
        subset_t leaves_needed;
        set_difference (label_set.begin(), label_set.end(),
                        curr_subset.begin(), curr_subset.end(),
                        inserter(leaves_needed, leaves_needed.begin()));
        if (leaves_needed.empty()) {
            subset_vec_t subsets_vec{1, curr_subset};
            add_resolution(subsets_vec, curr_score);
            continue;
        }
        viable_others.clear();
        for (auto other_set_ptr : others) {
            const subset_t & other_set = other_set_ptr;
            if (is_disjoint(other_set, curr_subset)) {
                viable_others.push_back(other_set_ptr);
            }
        }
        if (viable_others.empty()) {
            continue;
        }
        auto sub_cc = cc_for_subset(leaves_needed, viable_others, *this);
        if (sub_cc == nullptr) {
            continue;
        }
    }

       
}

void Data::write(ostream & out) const {
    out << cc.label_set.size() << " labels, " 
        << cc.subsets_to_wts.size() << " subsets.\n";
    for (auto size_res_pair : cc.resolutions) {
        const auto & res = size_res_pair.second;
        out << "  " << size_res_pair.first << " subsets, best_score=" << res.score << ":  [";
        unsigned is_first_sub = true;
        for (auto & s : res.subsets) {
            if (is_first_sub) {
                is_first_sub = false;
            } else {
                out << ", ";
            }
            out << "frozenset({";
            bool is_first_label = true;
            for (auto & label_idx : s) {
                if (is_first_label) {
                    is_first_label = false;
                } else {
                    out << ", ";
                }
                out << '\'' << idx2name[label_idx] << '\'';
            }
            out << "})";
        }
        out << "]\n";
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
    validate_data(data);
    data.cc.fill_resolutions();
    data.write(cout);
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