#ifndef WEIGHTEDLOWESS_UTILS_HPP
#define WEIGHTEDLOWESS_UTILS_HPP

#include <type_traits>

namespace WeightedLowess {

// Identity function, for use in safe decltype'ing.
// Strip references and const'ness from Input_.
template<typename Input_>
using I = typename std::remove_cv<typename std::remove_reference<Input_>::type>::type;

}

#endif
