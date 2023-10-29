#include "max_weight_partition.h"

using cache_map_t = std::map<subset_t, ConnectedComponent>;

cache_map_t SOLN_CACHE;

void ConnectedComponent::fill_resolutions() {

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