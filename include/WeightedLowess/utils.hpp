#ifndef WEIGHTEDLOWESS_UTILS_HPP
#define WEIGHTEDLOWESS_UTILS_HPP

#include <type_traits>

namespace WeightedLowess {

namespace internal {

// Identity function, for use in safe decltype'ing.
// Takes advantage of type deduction to strip references and const'ness from Input_.
template<typename Input_>
std::remove_cv_t<std::remove_reference_t<Input_> > I(Input_ x) {
    return x;
}

}

}

#endif
