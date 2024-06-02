#ifndef WEIGHTEDLOWESS_SORTBY_HPP
#define WEIGHTEDLOWESS_SORTBY_HPP

#include <algorithm>
#include <numeric>
#include <vector>
#include <cstdint>

namespace WeightedLowess {

class SortBy {
private:
    std::vector<size_t> my_permutation;

public:
    template<typename Data_>
    SortBy(size_t num_points, Data_* x) : my_permutation(num_points) {
        if (num_points) {
            std::iota(my_permutation.begin(), my_permutation.end(), 0);
            std::sort(my_permutation.begin(), my_permutation.end(), [&](size_t left, size_t right) -> bool { return x[left] < x[right]; });
        }
    }

public:
    template<typename Data_>
    void permute(Data_* data, std::vector<uint8_t>& work) const {
        permute({ data }, work);
    }

    template<typename Data_>
    void permute(std::initializer_list<Data_*> data, std::vector<uint8_t>& work) const {
        size_t num_points = my_permutation.size();
        work.clear();
        work.resize(num_points);

        // Reordering values in place.
        for (size_t i = 0; i < num_points; ++i) {
            if (work[i]) {
                continue;
            }
            work[i] = 1;

            size_t current = i, replacement = my_permutation[i];
            while (replacement != i) {
                for (auto d : data) {
                    std::swap(d[current], d[replacement]);
                }
                current = replacement;
                work[replacement] = 1;
                replacement = my_permutation[replacement]; 
            } 
        }
    }

public:
    template<typename Data_>
    void unpermute(Data_* data, std::vector<uint8_t>& work) const {
        unpermute({ data }, work);
    }

    template<typename Data_>
    void unpermute(std::initializer_list<Data_*> data, std::vector<uint8_t>& work) const {
        size_t num_points = my_permutation.size();
        work.clear();
        work.resize(num_points);

        for (size_t i = 0; i < num_points; ++i) {
            if (work[i]) {
                continue;
            }
            work[i] = 1;

            size_t replacement = my_permutation[i];
            while (replacement != i) {
                for (auto d : data) {
                    std::swap(d[i], d[replacement]);
                }
                work[replacement] = 1;
                replacement = my_permutation[replacement]; 
            } 
        }
    }
};

}

#endif
