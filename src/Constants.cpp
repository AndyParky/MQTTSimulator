#include "../include/Constants.h"

// This file is intentionally left almost empty since most constants
// are defined as constexpr in the header file and don't need implementation.
// This file exists primarily to maintain consistency with the project structure
// and to allow for any non-constexpr constants if needed in the future.

namespace mqtt {
    namespace constants {

        // If there are any constants that cannot be constexpr (e.g., complex strings, 
        // runtime-calculated values, etc.), they would be defined here.

        // Example of a non-constexpr constant:
        // const std::string VERSION = "1.0.0";

    } // namespace constants
} // namespace mqtt