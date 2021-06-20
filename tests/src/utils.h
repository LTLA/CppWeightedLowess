#ifndef UTILS_H
#define UTILS_H

#include <vector>

extern std::vector<double> x;

extern std::vector<double> y;

void compare_almost_equal(const std::vector<double>&, const std::vector<double>&);

double sum_abs_diff(const std::vector<double>&, const std::vector<double>&);

#endif
