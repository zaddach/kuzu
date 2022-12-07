#pragma once

#include <cassert>
#include <cstring>

#include "common/types/ku_string.h"
#include "utf8proc.h"

using namespace std;
using namespace kuzu::common;

namespace kuzu {
namespace function {
namespace operation {

struct SubStr {
public:
    static inline void operation(ku_string_t& src, int64_t start, int64_t len, ku_string_t& result,
        ValueVector& resultValueVector) {
        string srcStr = src.getAsString();
        bool isAscii = true;
        auto startPos = start - 1;
        auto endPos = min(srcStr.size(), (size_t)(startPos + len));
        // 1 character more than length has to be scanned for diatrics case: y + ˘ = ў.
        for (auto i = 0; i < min(srcStr.size(), endPos + 1); i++) {
            // UTF-8 character encountered.
            if (srcStr[i] & 0x80) {
                isAscii = false;
                break;
            }
        }
        if (isAscii) {
            copySubstr(src, start, len, result, resultValueVector, true /* isAscii */);
        } else {
            int64_t characterCount = 0, startBytePos = 0, endBytePos = 0;
            kuzu::utf8proc::utf8proc_grapheme_callback(
                srcStr.c_str(), srcStr.size(), [&](int64_t gstart, int64_t gend) {
                    if (characterCount == startPos) {
                        startBytePos = gstart;
                    } else if (characterCount == endPos) {
                        endBytePos = gstart;
                        return false;
                    }
                    characterCount++;
                    return true;
                });
            if (endBytePos == 0 && len != 0) {
                endBytePos = src.len;
            }
            // In this case, the function gets the EXACT byte location to start copying from.
            copySubstr(src, startBytePos, endBytePos - startBytePos, result, resultValueVector,
                false /* isAscii */);
        }
    }

    static inline void copySubstr(ku_string_t& src, int64_t start, int64_t len, ku_string_t& result,
        ValueVector& resultValueVector, bool isAscii) {
        result.len = min(len, src.len - start + 1);
        if (!ku_string_t::isShortString(result.len)) {
            result.overflowPtr = reinterpret_cast<uint64_t>(
                resultValueVector.getOverflowBuffer().allocateSpace(result.len));
        }
        if (isAscii) {
            // For normal ASCII char case, we get to the proper byte position to copy from by doing
            // a -1 (since it is guaranteed each char is 1 byte).
            memcpy((uint8_t*)result.getData(), src.getData() + start - 1, result.len);
        } else {
            // For utf8 char copy, the function gets the exact starting byte position to copy from.
            memcpy((uint8_t*)result.getData(), src.getData() + start, result.len);
        }
        if (!ku_string_t::isShortString(result.len)) {
            memcpy(result.prefix, result.getData(), ku_string_t::PREFIX_LENGTH);
        }
    }
};

} // namespace operation
} // namespace function
} // namespace kuzu