#pragma once

#include <cstdint>
#include <string>

#include "src/common/include/string.h"
#include "src/common/include/types.h"

using namespace std;

namespace graphflow {
namespace common {

class Value {
public:
    Value() = default;

    explicit Value(DataType dataType) : dataType(dataType) {}

    explicit Value(uint8_t value) : dataType(BOOL) { this->primitive.booleanVal = value; }

    explicit Value(int32_t value) : dataType(INT32) { this->primitive.int32Val = value; }

    explicit Value(double value) : dataType(DOUBLE) { this->primitive.doubleVal = value; }

    explicit Value(const string& value) : dataType(STRING) { this->strVal.set(value); }

    Value& operator=(const Value& other);

    void castToString();

    string toString() const;

public:
    union PrimitiveValue {
        uint8_t booleanVal;
        int32_t int32Val;
        double doubleVal;
    } primitive{};

    gf_string_t strVal{};
    nodeID_t nodeID{};

    DataType dataType;
};

} // namespace common
} // namespace graphflow