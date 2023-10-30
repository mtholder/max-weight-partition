#include "max_weight_partition.h"
#include <cassert>
#include <cstddef>
#include <climits>
#include <algorithm>
#include <sstream>

using cache_map_t = std::map<subset_t, const ConnectedComponent *>;

cache_map_t SOLN_CACHE;
bool USE_MOST_FREQ = false;

list<ConnectedComponent> sub_cc_list;

const Data *gData = nullptr;

inline void db_indent(unsigned int level) {
    for (unsigned int i=0; i < level; ++i) {
        cerr << "  ";
    }
}

void db_msg_pivot(unsigned int level, size_t label_idx) {
    if (gData == nullptr) {
        return;
    }
    db_indent(level);
    cerr << "db: pivot label " << label_idx << " \'" << gData->idx2name[label_idx] << "\'" << endl;
}

set<string> db_trans_idx_set(const subset_t & subset) {
    assert(gData != nullptr);
    set<string> ret;
    for (auto idx : subset) {
        ret.insert(gData->idx2name[idx]);
    }
    return ret;
}

inline void indented_msg(unsigned int level, std::string pref) {
    db_indent(level);
    cerr << "db: " << pref << endl;
}

inline void db_msg(unsigned int level, std::string pref) {
    if (gData == nullptr) {
        return;
    }
    indented_msg(level, pref);
}

inline void db_set_int_internal(const subset_t & subset) {
    bool first = true;
    for (auto idx : subset) {
        if (first) {
            first = false;
        } else {
            cerr << ", ";
        }
        cerr << idx;
    }
}

inline void db_set_str_internal(const subset_t & subset) {
    bool first = true;
    auto ss = db_trans_idx_set(subset);
    for (auto word : ss) {
        if (first) {
            first = false;
        } else {
            cerr << ", ";
        }
        cerr << '\'' << word << '\'';
    }
}

void db_msg_set(unsigned int level, std::string pref, const subset_t & subset, bool header=true) {
    if (gData == nullptr) {
        return;
    }
    db_indent(level);
    if (header) {
        cerr << "db: " ;
    }
    cerr << pref;
    auto s2si_it = gData->subsets_to_subset_idx.find(subset);
    if (s2si_it != gData->subsets_to_subset_idx.end()) {
        cerr << " INPUT-SUBSET idx=" << s2si_it->second << " (size="<< subset.size() << ")" << endl;
        return;
    }
    cerr << " for " << subset.size() << " indices: {";
    db_set_int_internal(subset);
    // cerr << endl;
    // db_indent(level);
    // if (header) {
    //     cerr << "db: " ;
    // }
    cerr << "} alpha-str: {";
    db_set_str_internal(subset);
    cerr << '}' << endl;
}

template <typename T>
void db_msg_set_container(unsigned int level, std::string pref, const T & subset_cont) {
    if (gData == nullptr) {
        return;
    }
    db_indent(level);
    cerr << "db: " << pref << " for container of size = " << subset_cont.size() << ": ";
    bool first = true;
    for (auto subset : subset_cont) {
        if (first) {
            first = false;
        } else {
            cerr << ", ";
        }
        cerr << gData->subsets_to_subset_idx.at(subset);
    }
    cerr << endl;
    // for (auto subset : subset_cont) {
    //     db_msg_set(level, "    ", subset, false);
    // }
}


void db_msg_resolution(unsigned int level, const std::string & pref, const subset_vec_t &subset_cont, double res_score) {
    if (gData == nullptr) {
        return;
    }
    db_indent(level);
    cerr << "db: " << pref <<  " size=" << subset_cont.size() << " score=" << res_score;
    cerr << " INPUT-SUBSETs: ";
    bool first = true;
    for (auto subset : subset_cont) {
        if (first) {
            first = false;
        } else {
            cerr << ", ";
        }
        cerr << gData->subsets_to_subset_idx.at(subset);
    }
    cerr << endl;
    // cerr << ": ints (";
    // for (auto subset : subset_cont) {
    //     db_set_int_internal(subset);
    //     cerr << " | ";
    // }
    // cerr << "), alpha-str (";
    // for (auto subset : subset_cont) {
    //     db_set_str_internal(subset);
    //     cerr << " | ";
    // }
    // cerr << ")" << endl;
    
}



template<typename T>
inline std::set<T> set_difference_as_set(const std::set<T> & fir, const std::set<T> & sec) {
    std::set<T> d;
    set_difference(begin(fir), end(fir), begin(sec), end(sec), std::inserter(d, d.end()));
    return d;
}

const ConnectedComponent * cc_for_subset(const subset_t & labels_needed,
                                  const vector<LightSubset> &viable_others,
                                  const ConnectedComponent & par_cc) {
    auto cache_it =  SOLN_CACHE.find(labels_needed);
    if (cache_it != SOLN_CACHE.end()) {
        db_msg_set(par_cc.level, "Cache hit for", labels_needed);
        return cache_it->second;
    }

    assert(!viable_others.empty());
    subset_t labels_still_needed = labels_needed;
    subset_t labels_union;
    bool have_labels_we_need = false;
    for (auto vop : viable_others) {
        labels_still_needed = set_difference_as_set(labels_still_needed, vop.stored_set());
        labels_union.insert(vop.begin(), vop.end());
        if (labels_still_needed.empty()) {
            have_labels_we_need = true;
            break;
        }
    }
    if (! have_labels_we_need) {
        db_msg_set(par_cc.level, "Missing labels", labels_still_needed);
        SOLN_CACHE[labels_needed] = nullptr;
        return nullptr;
    }
    assert(labels_union == labels_needed);
    sub_cc_list.emplace_back();
    ConnectedComponent & new_cc = *(sub_cc_list.rbegin());
    for (auto vop : viable_others) {
        const auto & subset = vop;
        new_cc.subsets_to_wts[subset] = par_cc.subsets_to_wts.at(subset);
    }
    new_cc.label_set = labels_needed;
    new_cc.level = par_cc.level + 1;
    // cerr << "CALLING fill_resolutions on sub_cc" << endl;
    new_cc.fill_resolutions();
    SOLN_CACHE[labels_needed] = &(new_cc);
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

inline void ConnectedComponent::add_resolution(subset_vec_t &v, double res_score) {
    db_msg_resolution(level, " Considering Resolution", v, res_score);
    auto sz = v.size();
    Resolution & res = resolutions[sz];
    if (res_score > res.score) {
        swap(res.subsets, v);
        res.score = res_score;
        db_msg(level, "ADDED");
    } else {
        stringstream m;
        m << "NOT ADDED curr score=" << res.score;
        db_msg(level, m.str());
    }
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
    db_msg_pivot(level, one_label_idx);
    vector<LightSubset> alternatives;
    vector<LightSubset> others;
    alternatives.reserve(subsets_to_wts.size());
    others.reserve(subsets_to_wts.size());
    for (auto s2w_it : subsets_to_wts) {
        const LightSubset & subset_ref = (s2w_it.first);
        if (subset_ref.count(one_label_idx) == 1) {
            alternatives.push_back(subset_ref);
        } else {
            others.push_back(subset_ref);
        }
    }
    assert(!alternatives.empty());
    vector<LightSubset> viable_others;
    viable_others.reserve(subsets_to_wts.size());
    db_msg_set_container(level, "alternatives", alternatives);
    db_msg_set_container(level, "others", others);
    size_t alt_idx = 0;
    for (auto alt : alternatives) {
        if (level < 4) {
            stringstream x;
            x << alt_idx++ << "/" << alternatives.size() << " cache.size() = " << SOLN_CACHE.size();
            indented_msg(level, x.str());
        }
        const LightSubset & curr_subset = alt;
        db_msg_set(level, " NEXT alt", curr_subset.stored_set());
        double curr_score = subsets_to_wts.at(curr_subset);
        subset_t labels_needed;
        set_difference (label_set.begin(), label_set.end(),
                        curr_subset.begin(), curr_subset.end(),
                        inserter(labels_needed, labels_needed.begin()));
        db_msg_set(level, " NEXT labels_needed", labels_needed);;
        if (labels_needed.empty()) {
            subset_vec_t subsets_vec{1, curr_subset};
            add_resolution(subsets_vec, curr_score);
            db_msg(level, "  no leaves needed for this alt");
            continue;
        }
        viable_others.clear();
        for (auto other_set_ptr : others) {
            const LightSubset & other_set = other_set_ptr;
            if (is_disjoint(other_set.stored_set(), curr_subset.stored_set())) {
                viable_others.push_back(other_set);
            }
        }
        db_msg_set_container(level, "  viable_others", viable_others);
        if (viable_others.empty()) {
            db_msg(level, "  no viable_others for this alt");
            continue;
        }
        auto sub_cc = cc_for_subset(labels_needed, viable_others, *this);
        if (sub_cc == nullptr) {
            db_msg(level, "  no sub_cc returned for this alt");
            continue;
        }
        for (auto sres : sub_cc->resolutions) {
            const Resolution & res = sres.second;
            subset_vec_t subsets_vec;
            subsets_vec.reserve(1 + res.subsets.size());
            subsets_vec.push_back(curr_subset);
            for (auto s : res.subsets) {
                subsets_vec.push_back(s);
            }
            add_resolution(subsets_vec, res.score + curr_score);
        }
    }
}

void Data::write(ostream & out) const {
    out << cc.label_set.size() << " labels, " 
        << cc.subsets_to_wts.size() << " subsets.\n";
    for (auto size_res_pair : cc.resolutions) {
        const auto & res = size_res_pair.second;
        out << "  " << size_res_pair.first << " subsets, best score=" << res.score << ": [";
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
    bool QUIET_MODE = true;
    Data data;
    gData = &data;
    read_labels(fp, data, name_parser);
    cerr << "Before simplification " << data.idx2name.size() << " labels:\n";
    unsigned idx = 0;
    for (auto name : data.idx2name) {
        cerr << "  label " << idx++ << " \'" << name << "\'\n";
    }
    read_labels(fp, data, grouped_names_detection_loader);
    detect_grouped_names(data);
    cerr << "After simplification " << data.idx2name.size() << " labels:\n";
    idx = 0;
    for (auto name : data.idx2name) {
        cerr << "  label " << idx++ << " \'" << name << "\'\n";
    }
    
    read_labels(fp, data, subset_encoder);
    validate_data(data);
    if (QUIET_MODE) {
        gData = nullptr; // Sssshhh!
    } else {
        cerr << data.num_subsets << " subsets";
        for (size_t i = 0; i < data.num_subsets; ++i) {
            const auto & sub = data.input_sub_order.at(i);
            cerr << "  Subset-" << i << " as idx = {";
            db_set_int_internal(sub.stored_set());
            cerr << "} alpha-str = {";
            db_set_str_internal(sub.stored_set()); 
            cerr << "} weight = " << data.cc.subsets_to_wts.at(sub) << endl;
        }
    }
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