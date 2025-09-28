#ifndef WEIGHTEDLOWESS_SORTBY_HPP
#define WEIGHTEDLOWESS_SORTBY_HPP

#include <algorithm>
#include <numeric>
#include <vector>
#include <cstdint>
#include <type_traits>
#include <initializer_list>
#include <cstddef>

#include "sanisizer/sanisizer.hpp"

#include "utils.hpp"

/**
 * @file SortBy.hpp
 * @brief Utility for sorting x-coordinates.
 */

namespace WeightedLowess {

/**
 * @brief Utility class for sorting on a covariate.
 *
 * This is typically used to ensure that x-coordinates are sorted prior to calling `compute()`.
 * The idea is to create a permutation vector from the x-coordinates to sort them in ascending order;
 * use `permute()` to apply that permutation to the various arrays of x-coordinates, y-coordinates and weights (if applicable);
 * calculate the fitted values from the permuted arrays with `compute()`, now that the x-coordinates are sorted;
 * and then use `unpermute()` on the results of the fit, to obtain fitted values for the points in their original (pre-sort) order.
 */
class SortBy {
private:
    std::vector<std::size_t> my_permutation;
    bool my_sorted = true;

public:
    /**
     * @tparam Sortable_ Sortable type.
     * @param num_points Number of points.
     * @param[in] x Pointer to an array of sortable values, typically x-coordinates for the dataset.
     */
    template<typename Sortable_>
    SortBy(const std::size_t num_points, const Sortable_* const x) {
        set(num_points, x);
    }

    /**
     * Default constructor.
     * The object should not be used until `set()` is called.
     */
    SortBy() = default;

    /**
     * @tparam Sortable_ Sortable type.
     * @param num_points Number of points.
     * @param[in] x Pointer to an array of sortable values, typically x-coordinates for the dataset.
     */
    template<typename Sortable_>
    void set(const std::size_t num_points, const Sortable_* const x) {
        if (num_points) {
            my_sorted = std::is_sorted(x, x + num_points);
            if (!my_sorted) {
                sanisizer::resize(my_permutation, num_points);
                std::iota(my_permutation.begin(), my_permutation.end(), static_cast<std::size_t>(0));
                std::sort(my_permutation.begin(), my_permutation.end(), [&](std::size_t left, std::size_t right) -> bool { return x[left] < x[right]; });
            }
        }
    }

private:
    template<typename AllData_, typename Used_>
    void permute_raw(AllData_& data, Used_* const work) const {
        if (my_sorted) {
            return;
        }

        const auto num_points = my_permutation.size();
        std::fill_n(work, num_points, 0);

        // Reordering values in place.
        for (I<decltype(num_points)> i = 0; i < num_points; ++i) {
            if (work[i]) {
                continue;
            }
            work[i] = 1;

            std::size_t current = i, replacement = my_permutation[i];
            while (replacement != i) {
                if constexpr(std::is_pointer<AllData_>::value) {
                    std::swap(data[current], data[replacement]);
                } else {
                    for (auto d : data) {
                        std::swap(d[current], d[replacement]);
                    }
                }
                current = replacement;
                work[replacement] = 1;
                replacement = my_permutation[replacement]; 
            } 
        }
    }

public:
    /**
     * @tparam Data_ Any data type.
     * @tparam Used_ Boolean type of the workspace.
     * @param[in,out] data Pointer to an array of length `num_points` to be permuted in-place,
     * in the same manner that `x` (from the constructor) would be permuted for sorting.
     * @param[in] work Pointer to an array of length `num_points`, to use as the workspace.
     * This can recycled across `permute()` and `unpermute()` calls.
     */
    template<typename Data_, typename Used_>
    void permute(Data_* const data, Used_* const work) const {
        permute_raw(data, work);
    }

    /**
     * @tparam Data_ Any data type.
     * @tparam Used_ Boolean type of the workspace.
     * @param[in,out] data One or more pointers to arrays of length `num_points` to be permuted in-place,
     * in the same manner that `x` (from the constructor) would be permuted for sorting.
     * @param[in] work Pointer to an array of length `num_points`, to use as the workspace.
     * This can recycled across `permute()` and `unpermute()` calls.
     */
    template<typename Data_, typename Used_>
    void permute(std::initializer_list<Data_* const> data, Used_* const work) const {
        permute_raw(data, work);
    }

    /**
     * @tparam DataPointers_ Iterable container of pointers.
     * @tparam Used_ Boolean type of the workspace.
     * @param[in,out] data One or more pointers to arrays of length `num_points` to be permuted in-place,
     * in the same manner that `x` (from the constructor) would be permuted for sorting.
     * @param[in] work Pointer to an array of length `num_points`, to use as the workspace.
     * This can recycled across `permute()` and `unpermute()` calls.
     */
    template<typename DataPointers_, typename Used_>
    void permute(DataPointers_ data, Used_* const work) const {
        permute_raw(data, work);
    }

    /**
     * @tparam Data_ Any data type.
     * @tparam Used_ Boolean type of the workspace.
     * @param[in,out] data Pointer to an array of length `num_points` to be permuted in-place,
     * in the same manner that `x` (from the constructor) would be permuted for sorting.
     * @param work Workspace.
     * This can be empty and will be recycled across `permute()` and `unpermute()` calls.
     */
    template<typename Data_, typename Used_>
    void permute(Data_* const data, std::vector<Used_>& work) const {
        const auto num_points = my_permutation.size();
        sanisizer::resize(work, num_points);
        permute(data, work.data());
    }

    /**
     * @tparam Data_ Any data type.
     * @tparam Used_ Boolean type of the workspace.
     * @param[in,out] data One or more pointers to arrays of length `num_points` to be permuted in-place,
     * in the same manner that `x` (from the constructor) would be permuted for sorting.
     * @param work Workspace.
     * This can be empty and will be recycled across `permute()` and `unpermute()` calls.
     */
    template<typename Data_, typename Used_>
    void permute(std::initializer_list<Data_* const> data, std::vector<Used_>& work) const {
        const auto num_points = my_permutation.size();
        sanisizer::resize(work, num_points);
        permute(data, work.data());
    }

    /**
     * @tparam DataPointers_ Iterable container of pointers.
     * @tparam Used_ Boolean type of the workspace.
     * @param[in,out] data One or more pointers to arrays of length `num_points` to be permuted in-place,
     * in the same manner that `x` (from the constructor) would be permuted for sorting.
     * @param work Workspace.
     * This can be empty and will be recycled across `permute()` and `unpermute()` calls.
     */
    template<typename DataPointers_, typename Used_>
    void permute(DataPointers_ data, std::vector<Used_>& work) const {
        const auto num_points = my_permutation.size();
        sanisizer::resize(work, num_points);
        permute(data, work.data());
    }

private:
    template<typename AllData_, typename Used_>
    void unpermute_raw(AllData_& data, Used_* const work) const {
        if (my_sorted) {
            return;
        }

        const auto num_points = my_permutation.size();
        std::fill_n(work, num_points, 0);

        for (I<decltype(num_points)> i = 0; i < num_points; ++i) {
            if (work[i]) {
                continue;
            }
            work[i] = 1;

            auto replacement = my_permutation[i];
            while (replacement != i) {
                if constexpr(std::is_pointer<AllData_>::value) {
                    std::swap(data[i], data[replacement]);
                } else {
                    for (const auto d : data) {
                        std::swap(d[i], d[replacement]);
                    }
                }
                work[replacement] = 1;
                replacement = my_permutation[replacement]; 
            } 
        }
    }

public:
    /**
     * @tparam Data_ Any data type.
     * @tparam Used_ Boolean type of the workspace.
     * @param[in,out] data Pointer to an array of length `num_points` to be permuted in-place to reverse the effect of `permute()`.
     * @param[in] work Pointer to an array of length `num_points`.
     * This can be recycled across `permute()` and `unpermute()` calls.
     */
    template<typename Data_, typename Used_>
    void unpermute(Data_* const data, Used_* const work) const {
        unpermute_raw(data, work);
    }

    /**
     * @tparam Data_ Any data type.
     * @tparam Used_ Boolean type of the workspace.
     * @param[in,out] data One or more pointers to arrays of length `num_points` to be permuted in-place to reverse the effect of `permute()`.
     * @param[in] work Pointer to an array of length `num_points`.
     * This can be recycled across `permute()` and `unpermute()` calls.
     */
    template<typename Data_, typename Used_>
    void unpermute(std::initializer_list<Data_* const> data, Used_* const work) const {
        unpermute_raw(data, work);
    }

    /**
     * @tparam DataPointers_ Iterable container of pointers.
     * @tparam Used_ Boolean type of the workspace.
     * @param[in,out] data One or more pointers to arrays of length `num_points` to be permuted in-place to reverse the effect of `permute()`.
     * @param[in] work Pointer to an array of length `num_points`.
     * This can be recycled across `permute()` and `unpermute()` calls.
     */
    template<typename DataPointers_, typename Used_>
    void unpermute(DataPointers_ data, Used_* const work) const {
        unpermute_raw(data, work);
    }

    /**
     * @tparam Data_ Any data type.
     * @tparam Used_ Boolean type of the workspace.
     * @param[in,out] data Pointer to an array of length `num_points` to be permuted in-place to reverse the effect of `permute()`.
     * @param work Workspace.
     * This can be empty and will be recycled across `permute()` and `unpermute()` calls.
     */
    template<typename Data_, typename Used_>
    void unpermute(Data_* const data, std::vector<Used_>& work) const {
        const auto num_points = my_permutation.size();
        sanisizer::resize(work, num_points);
        unpermute(data, work.data());
    }

    /**
     * @tparam Data_ Any data type.
     * @tparam Used_ Boolean type of the workspace.
     * @param[in,out] data One or more pointers to arrays of length `num_points` to be permuted in-place to reverse the effect of `permute()`.
     * @param work Workspace.
     * This can be empty and will be recycled across `permute()` and `unpermute()` calls.
     */
    template<typename Data_, typename Used_>
    void unpermute(std::initializer_list<Data_* const> data, std::vector<Used_>& work) const {
        const auto num_points = my_permutation.size();
        sanisizer::resize(work, num_points);
        unpermute(data, work.data());
    }

    /**
     * @tparam DataPointers_ Iterable container of pointers.
     * @tparam Used_ Boolean type of the workspace.
     * @param[in,out] data One or more pointers to arrays of length `num_points` to be permuted in-place to reverse the effect of `permute()`.
     * @param work Workspace.
     * This can be empty and will be recycled across `permute()` and `unpermute()` calls.
     */
    template<typename DataPointers_, typename Used_>
    void unpermute(DataPointers_ data, std::vector<Used_>& work) const {
        const auto num_points = my_permutation.size();
        sanisizer::resize(work, num_points);
        unpermute(data, work.data());
    }
};

}

#endif
