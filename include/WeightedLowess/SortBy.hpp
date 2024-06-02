#ifndef WEIGHTEDLOWESS_SORTBY_HPP
#define WEIGHTEDLOWESS_SORTBY_HPP

#include <algorithm>
#include <numeric>
#include <vector>
#include <cstdint>

/**
 * @file SortBy.hpp
 * @brief Utility for sorting x-coordinates.
 */

namespace WeightedLowess {

/**
 * @brief Utility class for sorting on a covariate.
 *
 * This is typically used to ensure that x-values are sorted prior to calling `compute()`.
 * The idea is to create a permutation vector from the x-values to sort them in ascending order;
 * use `permute()` to apply that permutation to the various arrays of x-values, y-values and weights (if applicable);
 * calculate the fitted values from the permuted arrays with `compute()`, now that the x-values are sorted;
 * and then use `unpermute()` on the results of the fit, to obtain fitted values for the points in their original (pre-sort) order.
 */
class SortBy {
private:
    std::vector<size_t> my_permutation;
    bool my_sorted = true;

public:
    /**
     * @tparam Data_ Floating-point type of the data.
     * @param num_points Number of points.
     * @param[in] x Pointer to an array of x-values for the dataset.
     */
    template<typename Data_>
    SortBy(size_t num_points, const Data_* x) : my_permutation(num_points) {
        if (num_points) {
            my_sorted = std::is_sorted(x, x + num_points);
            if (!my_sorted) {
                std::iota(my_permutation.begin(), my_permutation.end(), 0);
                std::sort(my_permutation.begin(), my_permutation.end(), [&](size_t left, size_t right) -> bool { return x[left] < x[right]; });
            }
        }
    }

public:
    /**
     * @param[in,out] data Pointer to an array of length `num_points` to be permuted in-place,
     * in the same manner that `x` (from the constructor) would be permuted for sorting.
     * @param work Workspace.
     * This can be empty and will be recycled across `permute()` and `unpermute()` calls.
     */
    template<typename Data_>
    void permute(Data_* data, std::vector<uint8_t>& work) const {
        permute({ data }, work);
    }

    /**
     * @param[in,out] data One or more pointers to arrays of length `num_points` to be permuted in-place,
     * in the same manner that `x` (from the constructor) would be permuted for sorting.
     * @param work Workspace.
     * This can be empty and will be recycled across `permute()` and `unpermute()` calls.
     */
    template<typename Data_>
    void permute(std::initializer_list<Data_*> data, std::vector<uint8_t>& work) const {
        if (my_sorted) {
            return;
        }

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
    /**
     * @param[in,out] data Pointer to an array of length `num_points` to be permuted in-place to reverse the effect of `permute()`.
     * @param work Workspace.
     * This can be empty and will be recycled across `permute()` and `unpermute()` calls.
     */
    template<typename Data_>
    void unpermute(Data_* data, std::vector<uint8_t>& work) const {
        unpermute({ data }, work);
    }

    /**
     * @param[in,out] data One or more pointers to arrays of length `num_points` to be permuted in-place to reverse the effect of `permute()`.
     * @param work Workspace.
     * This can be empty and will be recycled across `permute()` and `unpermute()` calls.
     */
    template<typename Data_>
    void unpermute(std::initializer_list<Data_*> data, std::vector<uint8_t>& work) const {
        if (my_sorted) {
            return;
        }

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
