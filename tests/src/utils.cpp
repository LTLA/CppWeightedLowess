#include "utils.h"
#include <gtest/gtest.h>
#include <cmath>

std::vector<double> x = { 
   0.506, 0.570, 0.043, 0.304, 0.343, 0.518, 0.180, 0.964, 0.491,
   0.755, 0.587, 0.590, 0.641, 0.921, 0.707, 0.420, 0.268, 0.164,
   0.654, 0.293, 0.898, 0.752, 0.614, 0.194, 0.677, 0.683, 0.257,
   0.667, 0.946, 0.678, 0.414, 0.472, 0.691, 0.511, 0.816, 0.365,
   0.748, 0.552, 0.990, 0.116, 0.733, 0.970, 0.765, 0.826, 0.741,  
   0.341, 0.447, 0.927, 0.313, 0.409    
};

std::vector<double> y = {
     0.214,  0.661, -0.564,  0.545,  0.964,  0.068, -1.706,  0.234,  0.940,
     2.209, -0.509, -0.988,  1.414,  0.074,  0.476,  0.529,  1.185,  0.465,
     0.347, -0.611, -2.884, -0.134,  0.097, -0.020, -0.620, -2.652,  0.955, 
    -0.929,  0.309, -0.775, -0.056,  0.616, -0.072, -1.110,  0.301,  0.010,
     1.191,  0.423, -0.426, -1.317, -0.448,  1.846,  2.264,  0.212, -1.076,
     0.206,  0.232,  0.426,  1.113,  0.785
};

void compare_almost_equal(const std::vector<double>& first, const std::vector<double>& second) {
    ASSERT_EQ(first.size(), second.size());
    for (size_t i = 0; i < first.size(); ++i) {
        EXPECT_FLOAT_EQ(first[i], second[i]);
    }
    return;
}

double sum_abs_diff(const std::vector<double>& first, const std::vector<double>& second) {
    EXPECT_EQ(first.size(), second.size());
    if (first.size()==second.size()) {
        double sumdiff = 0;
        for (size_t i = 0; i < first.size(); ++i) {
            sumdiff += std::abs(first[i] - second[i]);
        }
        return sumdiff;
    } else {
        return 0;
    }
}
