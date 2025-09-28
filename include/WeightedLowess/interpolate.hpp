#ifndef WEIGHTEDLOWESS_INTERPOLATE_HPP
#define WEIGHTEDLOWESS_INTERPOLATE_HPP

#include <vector>
#include <cstddef>
#include <utility>

#include "sanisizer/sanisizer.hpp"

#include "window.hpp"
#include "utils.hpp"

/**
 * @file interpolate.hpp
 * @brief Interpolate from the fitted trend.
 */

namespace WeightedLowess {

/**
 * @brief Assigned segments.
 * 
 * Instances of this class are typically produced by `assign_to_segments()`.
 */
struct AssignedSegments {
    /**
     * @cond
     */
    std::vector<std::size_t> boundaries;
    /**
     * @endcond
     */
};

/**
 * Assign points-to-be-interpolated to their corresponding segments between anchors, for use in `interpolate()`.
 * Segments are defined as the line between two adjacent anchors, to be used for linear interpolation of any intervening points.
 *
 * @tparam Data_ Floating-point type of the data.
 *
 * @param[in] x_fit Pointer to an array of x-coordinates for the fitted trend from `compute()`.
 * This should be sorted in increasing order.
 * @param windows_fit Precomputed windows for anchor points, created by calling `define_windows()` on `x_fit`.
 * @param num_points_out Number of points to be interpolated.
 * @param[in] x_out Pointer to an array of x-coordinates of the points to be interpolated.
 * This should be sorted in increasing order and have length equal to `num_points_out`.
 *
 * @return Assignment of each point in `x_out` to its corresponding segment, where possible.
 * This can be re-used for multiple `interpolate()` calls with different `fitted` values.
 */
template<typename Data_>
AssignedSegments assign_to_segments(
    const Data_* const x_fit,
    const PrecomputedWindows<Data_>& windows_fit,
    const std::size_t num_points_out,
    const Data_* const x_out
) {
    const auto& anchors = windows_fit.anchors;
    const auto num_anchors = anchors.size();

    AssignedSegments output;
    sanisizer::resize(output.boundaries, num_anchors);

    std::size_t counter = 0;
    {
        const auto right = x_fit[anchors[0]];
        while (counter < num_points_out && x_out[counter] < right) {
            ++counter;
        }
    }

    for (I<decltype(num_anchors)> i = 1; i < num_anchors; ++i) {
        output.boundaries[i - 1] = counter;
        if (counter == num_points_out) {
            continue;
        }
        const auto right = x_fit[anchors[i]];
        while (counter < num_points_out && x_out[counter] < right) {
            ++counter;
        }
    }

    // Anyone equal to the last anchor get assigned to the last segment.
    {
        const auto right = x_fit[anchors[num_anchors - 1]];
        while (counter < num_points_out && x_out[counter] == right) {
            ++counter;
        }
        output.boundaries[num_anchors - 1] = counter;
    }

    return output;
}

/**
 * @param assigned_out Assignment of each point-to-be-interpolated to a inter-anchor segment, produced by `assign_to_segments()`.
 * @return Pair containing (i) the index of the first point that lies within a segment and (ii) the index of the first point that lies beyond the final segment.
 * This defines the subinterval of points in `x_out` that can be safely interpolated, i.e., do not lie beyond `x_fit`.
 */
inline std::pair<std::size_t, std::size_t> get_interpolation_boundaries(const AssignedSegments& assigned_out) {
    return std::make_pair(assigned_out.boundaries.front(), assigned_out.boundaries.back());
}

/**
 * Interpolate the fitted values for a set of points, based on a pre-existing trend fitted by `compute()`.
 *
 * In `compute()`, the LOWESS algorithm calculates fitted values exactly for anchor points and then interpolates the fitted values for all intervening points.
 * This function applies the same interpolation to a separate set of points based only on their x-coordinates.
 *
 * @tparam Data_ Floating-point type of the data.
 *
 * @param[in] x_fit Pointer to an array of x-coordinates for the fitted trend from `compute()`.
 * This should be sorted in increasing order.
 * @param windows_fit Precomputed windows for anchor points, created by calling `define_windows()` on `x_fit`.
 * @param[in] fitted_fit Pointer to an array of fitted values from calling `compute()` with `x_fit` and `windows_fit`.
 * This should have the same length as `x_fit`.
 * @param[in] x_out Pointer to an array of x-coordinates of the points to be interpolated.
 * This should be sorted in increasing order. 
 * @param[in] assigned_out Assignment of points-to-be-interpolated to each inter-anchor segment.
 * This should be created by calling `assign_to_segment()` on `x_fit`, `windows_fit` and `x_out`.
 * @param[out] fitted_out Pointer to an array of same length as `x_out`.
 * On output, this stores the interpolated fitted value for each entry of `x_out` that lies within the interpolation boundaries (see `get_interpolation_boundaries()`).
 * No value is stored for points that lie beyond the interpolation boundaries.
 * @param num_threads Number of threads to use.
 */
template<typename Data_>
void interpolate(
    const Data_* const x_fit,
    const PrecomputedWindows<Data_>& windows_fit,
    const Data_* const fitted_fit,
    const Data_* const x_out,
    const AssignedSegments& assigned_out,
    Data_* const fitted_out,
    int num_threads
) {
    const auto& anchors = windows_fit.anchors;
    const auto num_anchors = anchors.size();
    const auto num_anchors_m1 = num_anchors - 1;

    // One would think that we should parallelize across x_out instead of anchors,
    // as this has better worksharing when x_out is not evenly distributed across anchor segments.
    // However, if we did so, we'd have to store the slope and intercept for the anchor segments first,
    // then look up the slope and intercept for each element of x_out.
    // This involves an extra memory access and is not SIMD-able.
    parallelize(num_threads, num_anchors_m1, [&](const int, const I<decltype(num_anchors_m1)> start, const I<decltype(num_anchors_m1)> length) {
        for (I<decltype(start)> s = start, end = start + length; s < end; ++s) {
            const auto run_start = assigned_out.boundaries[s];
            const auto run_end = assigned_out.boundaries[s + 1];
            if (run_start == run_end) {
                continue;
            }

            const auto left_anchor = windows_fit.anchors[s];
            const auto right_anchor = windows_fit.anchors[s + 1];
            const Data_ xdiff = x_fit[right_anchor] - x_fit[left_anchor];
            const Data_ ydiff = fitted_fit[right_anchor] - fitted_fit[left_anchor];
            if (xdiff > 0) {
                const Data_ slope = ydiff / xdiff;
                const Data_ intercept = fitted_fit[right_anchor] - slope * x_fit[right_anchor];
                for (auto outpt = run_start; outpt < run_end; ++outpt) {
                    fitted_out[outpt] = slope * x_out[outpt] + intercept; 
                }
            } else {
                // Protect against infinite slopes by just taking the average.
                const Data_ ave = fitted_fit[left_anchor] + ydiff / 2;
                std::fill(fitted_out + run_start, fitted_out + run_end, ave);
            }
        }
    });
}

/**
 * Overload of `interpolate()` that calls `assign_to_segments()` automatically.
 *
 * @tparam Data_ Floating-point type of the data.
 *
 * @param[in] x_fit Pointer to an array of x-coordinates for the fitted trend from `compute()`.
 * This should be sorted in increasing order.
 * @param windows_fit Precomputed windows for anchor points, created by calling `define_windows()` on `x_fit`.
 * @param[in] fitted_fit Pointer to an array of fitted values from calling `compute()` with `x_fit` and `windows_fit`.
 * This should have the same length as `x_fit`.
 * @param num_points_out Number of points to be interpolated.
 * @param[in] x_out Pointer to an array of x-coordinates of the points to be interpolated.
 * This should be sorted in increasing order. 
 * @param[out] fitted_out Pointer to an array of same length as `x_out`.
 * On output, this stores the interpolated fitted value for each entry of `x_out` that lies within the interpolation boundaries.
 * No value is stored for points that lie beyond the interpolation boundaries.
 * @param num_threads Number of threads to use.
 *
 * @return Boundaries of the interpolation, see `get_interpolation_boundaries()` for details.
 */
template<typename Data_>
std::pair<std::size_t, std::size_t> interpolate(
    const Data_* const x_fit,
    const PrecomputedWindows<Data_>& windows_fit,
    const Data_* const fitted_fit,
    const std::size_t num_points_out,
    const Data_* const x_out,
    Data_* const fitted_out,
    int num_threads
) {
    const auto assigned_out = assign_to_segments(x_fit, windows_fit, num_points_out, x_out);
    interpolate(x_fit, windows_fit, fitted_fit, x_out, assigned_out, fitted_out, num_threads);
    return get_interpolation_boundaries(assigned_out);
}

}

#endif
