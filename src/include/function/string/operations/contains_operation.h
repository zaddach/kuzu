#pragma once

#include <cassert>

#include "common/types/ku_string.h"
#include "function/string/operations/find_operation.h"

using namespace kuzu::common;

namespace kuzu {
namespace function {
namespace operation {

struct Contains {
    static inline void operation(ku_string_t& left, ku_string_t& right, uint8_t& result) {
        auto lStr = left.getAsString();
        auto rStr = right.getAsString();
        int64_t pos;
        Find::operation(left, right, pos);
        result = (pos != 0);
    }
};

} // namespace operation
} // namespace function
} // namespace kuzu