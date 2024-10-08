# Weighted LOWESS for C++

![Unit tests](https://github.com/LTLA/CppWeightedLowess/actions/workflows/run-tests.yaml/badge.svg)
![Documentation](https://github.com/LTLA/CppWeightedLowess/actions/workflows/doxygenate.yaml/badge.svg)
![Limma comparison](https://github.com/LTLA/CppWeightedLowess/actions/workflows/compare-limma.yaml/badge.svg)
[![Codecov](https://codecov.io/gh/LTLA/CppWeightedLowess/branch/master/graph/badge.svg?token=GBHWVK9MFY)](https://codecov.io/gh/LTLA/CppWeightedLowess)

## Overview

This C++ library implements the Locally Weighted Scatterplot Smoothing (LOWESS) method described by Cleveland (1979, 1981).
LOWESS is a non-parametric smoothing algorithm that is simple and computationally efficient yet can accommodate a wide variety of curves.
The libary itself is header-only and thus can be easily used in any C++ project by adding the relevant `#include` directives.
This implementation is derived from the [**limma**](https://bioconductor.org/packages/limma/) package and contains some modifications from Cleveland's original code.
Of particular interest is the ability to treat the weights as frequencies such that they are involved in the window span calculations for each anchor point - hence the name.

## Algorithm details

Cleveland's original Fortran implementation is quite simple:

1. Define a set of anchor points, more-or-less evenly spaced throughout the range of x-values.
2. Perform a local linear regression around each anchor, using only points within a window centered at each anchor.
In this regression, points inside the window are weighted based on their distances from the anchor on the x-axis.
(If additional sets of weights are provided, the product of weights is used for each point.)
Each anchor's window must be wide enough to contain a given proportion of all points.
4. Interpolate to obtain smoothed values for all points in between two anchors.
5. Compute a robustness weight for each point based on the absolute value of its residual from the fitted value.
Points with very large residuals are considered outliers and are assigned zero weight.
6. Repeat steps 1-5 using the computed weights in each anchor's local linear regression.
This is iterated several times to eliminate the effect of outliers on the fit.

In `limma::weightedLowess`, we implemented some (optional) modifications that are also available in this library.
See the [`?weightedLowess` documentation](https://rdrr.io/bioc/limma/man/weightedLowess.html) for more details.

- If additional weights are specified, we allow them to be interpreted as frequencies.
As a result, the weights are used in determining the width of the smoothing window around each anchor, in addition to their usual role in the local linear regression.
- The `delta` value can be automatically determined from a pre-specified number of anchor points.
This provides a convenient way of controlling the approximation fidelity.

In this library, we implement some further modifications from `limma::weightedLowess`:

- The number of robustness iterations in this library refers to additional iterations beyond the first fit,
while the number of robustness iterations in `limma::weightedLowess` includes the first fit.
So 3 iterations here are equivalent to 4 iterations in `limma::weightedLowess`.
- We omit the early termination condition when the MAD of the residuals is much lower than the mean.
This avoids inappropriate termination of the robustness iterations in pathological scenarios.
- We provide extra protection for the edge case where the robustness weighting causes the sum of weights in a window to be zero.
In such cases, we simply ignore the robustness weights when computing the smoothed value.

## Usage

Using this library is as simple as including the header file in your source code:

```cpp
#include "WeightedLowess/WeightedLowess.hpp"

// ... standard boilerplate here...

WeightedLowess::Options opt;
auto results = WeightedLowess::compute(num_points, x, y, opt);
results.fitted;
```

We can set options via the `WeightedLowess::Options` object:

```cpp
opt.span = 0.5;
opt.anchors = 100;
auto results2 = WeightedLowess::compute(num_points, x, y, opt);
```

If users already have an appropriate buffer for the fitted values and robustness weights, they can be filled directly with the results:

```cpp
std::vector<double> fitted(num_points), rweights(num_points);
WeightedLowess::compute(num_points, x, y, fitted.data(), rweights.data(), opt);
```

We can also pre-compute the window locations from `x` to re-use them with different `y`, e.g., for smoothing different dimensions with a common covariate.

```cpp
auto xwindows = WeightedLowess::define_windows(num_points, x, opt);
WeightedLowess::compute(num_points, x, xwindows, y, fitted.data(), resids.data(), opt);
WeightedLowess::compute(num_points, x, xwindows, y2, fitted2.data(), resids2.data(), opt);
// etc.
```

The `compute()` function assumes that the input x-coordinates are already sorted.
If this is not the case, we can use the `SortBy` class to sort the input and unsort the output:

```cpp
// Permuting the x/y pairs so that x is sorted:
WeightedLowess::SortBy sorter(num_points, x);
std::vector<uint8_t> workspace;
sorter.permute(x, workspace);
sorter.permute(y, workspace);

auto res_unsrt = WeightedLowess::compute(num_points, x, y, opt);

// Unpermuting the fitted values and robustness weights to match
// the original ordering of x/y pairs.
sorter.unpermute(res_unsrt.fitted.data(), workspace);
sorter.unpermute(res_unsrt.robust_weights.data(), workspace);
```

See the [reference documentation](https://ltla.github.io/CppWeightedLowess) for more details.

## Building projects

### CMake with `FetchContent`

If you're already using CMake, you can add something like this to your `CMakeLists.txt`:

```cmake
include(FetchContent)

FetchContent_Declare(
  WeightedLowess 
  GIT_REPOSITORY https://github.com/LTLA/CppWeightedLowess
  GIT_TAG master # or any version of interest
)

FetchContent_MakeAvailable(tatami)
```

And then:

```cmake
# For executables:
target_link_libraries(myexe WeightedLowess)

# For libaries
target_link_libraries(mylib INTERFACE WeightedLowess)
```

### CMake with `find_package()`

You can install the library by cloning a suitable version of this repository and running the following commands:

```sh
mkdir build && cd build
cmake .. -DWEIGHTEDLOWESS_TESTS=OFF
cmake --build . --target install
```

Then you can use `find_package()` as usual:

```cmake
find_package(ltla_WeightedLowess CONFIG REQUIRED)
target_link_libraries(mylib INTERFACE ltla::WeightedLowess)
```

By default, this will use `FetchContent` to fetch all external dependencies.
If you want to install them manually, use `-DWEIGHTEDLOWESS_FETCH_EXTERN=OFF`.
See [`extern/CMakeLists.txt`](extern/CMakeLists.txt) to find compatible versions of each dependency.

### Manual

If you're not using CMake, the simple approach is to just copy the files in the [`include/`](include) subdirectory - 
either directly or with Git submodules - and include their path during compilation with, e.g., GCC's `-I`.
This requires the external dependencies listed in [`extern/CMakeLists.txt`](extern/CMakeLists.txt), which also need to be made available during compilation.

## References 

Cleveland, W.S. (1979).
Robust locally weighted regression and smoothing scatterplots. 
_Journal of the American Statistical Association_ 74(368), 829-836.

Cleveland, W.S. (1981). 
LOWESS: A program for smoothing scatterplots by robust locally weighted regression. 
_The American Statistician_ 35(1), 54.
