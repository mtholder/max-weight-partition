#include "max_weight_partition.h"


void run(std::string &fp) {
    Data data;
    read_labels(fp, data, name_parser);
    unsigned idx = 0;
    for (auto name : data.idx2name) {
        cerr << "  label " << idx++ << " \"" << name << "\"\n";
    }
    read_labels(fp, data, subset_encoder);
    validate_data(data);
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