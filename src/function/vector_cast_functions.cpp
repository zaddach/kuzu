#include "function/cast/vector_cast_functions.h"

#include "function/cast/cast_functions.h"

using namespace kuzu::common;

namespace kuzu {
namespace function {

bool VectorCastFunction::hasImplicitCast(const LogicalType& srcType, const LogicalType& dstType) {
    // We allow cast between any numerical types
    if (LogicalTypeUtils::isNumerical(srcType) && LogicalTypeUtils::isNumerical(dstType)) {
        return true;
    }
    switch (srcType.getLogicalTypeID()) {
    case LogicalTypeID::DATE: {
        switch (dstType.getLogicalTypeID()) {
        case LogicalTypeID::TIMESTAMP:
            return true;
        default:
            return false;
        }
    }
    case LogicalTypeID::STRING: {
        switch (dstType.getLogicalTypeID()) {
        case LogicalTypeID::DATE:
        case LogicalTypeID::TIMESTAMP:
        case LogicalTypeID::INTERVAL:
            return true;
        default:
            return false;
        }
    }
    default:
        return false;
    }
}

std::string VectorCastFunction::bindImplicitCastFuncName(const LogicalType& dstType) {
    switch (dstType.getLogicalTypeID()) {
    case LogicalTypeID::SERIAL:
        return CAST_TO_SERIAL_FUNC_NAME;
    case LogicalTypeID::INT64:
        return CAST_TO_INT64_FUNC_NAME;
    case LogicalTypeID::INT32:
        return CAST_TO_INT32_FUNC_NAME;
    case LogicalTypeID::INT16:
        return CAST_TO_INT16_FUNC_NAME;
    case LogicalTypeID::INT8:
        return CAST_TO_INT8_FUNC_NAME;
    case LogicalTypeID::UINT64:
        return CAST_TO_UINT64_FUNC_NAME;
    case LogicalTypeID::UINT32:
        return CAST_TO_UINT32_FUNC_NAME;
    case LogicalTypeID::UINT16:
        return CAST_TO_UINT16_FUNC_NAME;
    case LogicalTypeID::UINT8:
        return CAST_TO_UINT8_FUNC_NAME;
    case LogicalTypeID::FLOAT:
        return CAST_TO_FLOAT_FUNC_NAME;
    case LogicalTypeID::DOUBLE:
        return CAST_TO_DOUBLE_FUNC_NAME;
    case LogicalTypeID::DATE:
        return CAST_TO_DATE_FUNC_NAME;
    case LogicalTypeID::TIMESTAMP:
        return CAST_TO_TIMESTAMP_FUNC_NAME;
    case LogicalTypeID::INTERVAL:
        return CAST_TO_INTERVAL_FUNC_NAME;
    case LogicalTypeID::STRING:
        return CAST_TO_STRING_FUNC_NAME;
    default:
        throw NotImplementedException("bindImplicitCastFuncName()");
    }
}

void VectorCastFunction::bindImplicitCastFunc(
    LogicalTypeID sourceTypeID, LogicalTypeID targetTypeID, scalar_exec_func& func) {
    switch (targetTypeID) {
    case LogicalTypeID::INT8: {
        bindImplicitNumericalCastFunc<int8_t, CastToInt8>(sourceTypeID, func);
        return;
    }
    case LogicalTypeID::INT16: {
        bindImplicitNumericalCastFunc<int16_t, CastToInt16>(sourceTypeID, func);
        return;
    }
    case LogicalTypeID::INT32: {
        bindImplicitNumericalCastFunc<int32_t, CastToInt32>(sourceTypeID, func);
        return;
    }
    case LogicalTypeID::SERIAL:
    case LogicalTypeID::INT64: {
        bindImplicitNumericalCastFunc<int64_t, CastToInt64>(sourceTypeID, func);
        return;
    }
    case LogicalTypeID::UINT8: {
        bindImplicitNumericalCastFunc<uint8_t, CastToUInt8>(sourceTypeID, func);
        return;
    }
    case LogicalTypeID::UINT16: {
        bindImplicitNumericalCastFunc<uint16_t, CastToUInt16>(sourceTypeID, func);
        return;
    }
    case LogicalTypeID::UINT32: {
        bindImplicitNumericalCastFunc<uint32_t, CastToUInt32>(sourceTypeID, func);
        return;
    }
    case LogicalTypeID::UINT64: {
        bindImplicitNumericalCastFunc<uint64_t, CastToUInt64>(sourceTypeID, func);
        return;
    }
    case LogicalTypeID::FLOAT: {
        bindImplicitNumericalCastFunc<float_t, CastToFloat>(sourceTypeID, func);
        return;
    }
    case LogicalTypeID::DOUBLE: {
        bindImplicitNumericalCastFunc<double_t, CastToDouble>(sourceTypeID, func);
        return;
    }
    case LogicalTypeID::DATE: {
        assert(sourceTypeID == LogicalTypeID::STRING);
        func = &UnaryExecFunction<ku_string_t, date_t, CastStringToDate>;
        return;
    }
    case LogicalTypeID::TIMESTAMP: {
        assert(sourceTypeID == LogicalTypeID::STRING || sourceTypeID == LogicalTypeID::DATE);
        func = sourceTypeID == LogicalTypeID::STRING ?
                   &UnaryExecFunction<ku_string_t, timestamp_t, CastStringToTimestamp> :
                   &UnaryExecFunction<date_t, timestamp_t, CastDateToTimestamp>;
        return;
    }
    case LogicalTypeID::INTERVAL: {
        assert(sourceTypeID == LogicalTypeID::STRING);
        func = &UnaryExecFunction<ku_string_t, interval_t, CastStringToInterval>;
        return;
    }
    default:
        throw NotImplementedException("Unimplemented casting operation from " +
                                      LogicalTypeUtils::dataTypeToString(sourceTypeID) + " to " +
                                      LogicalTypeUtils::dataTypeToString(targetTypeID) + ".");
    }
}

vector_function_definitions CastToDateVectorFunction::getDefinitions() {
    vector_function_definitions result;
    result.push_back(make_unique<VectorFunctionDefinition>(CAST_TO_DATE_FUNC_NAME,
        std::vector<LogicalTypeID>{LogicalTypeID::STRING}, LogicalTypeID::DATE,
        UnaryExecFunction<ku_string_t, date_t, CastStringToDate>));
    return result;
}

vector_function_definitions CastToTimestampVectorFunction::getDefinitions() {
    vector_function_definitions result;
    result.push_back(std::make_unique<VectorFunctionDefinition>(CAST_TO_TIMESTAMP_FUNC_NAME,
        std::vector<LogicalTypeID>{LogicalTypeID::STRING}, LogicalTypeID::TIMESTAMP,
        UnaryExecFunction<ku_string_t, timestamp_t, CastStringToTimestamp>));
    return result;
}

vector_function_definitions CastToIntervalVectorFunction::getDefinitions() {
    vector_function_definitions result;
    result.push_back(make_unique<VectorFunctionDefinition>(CAST_TO_INTERVAL_FUNC_NAME,
        std::vector<LogicalTypeID>{LogicalTypeID::STRING}, LogicalTypeID::INTERVAL,
        UnaryExecFunction<ku_string_t, interval_t, CastStringToInterval>));
    return result;
}

vector_function_definitions CastToStringVectorFunction::getDefinitions() {
    vector_function_definitions result;
    result.push_back(make_unique<VectorFunctionDefinition>(CAST_TO_STRING_FUNC_NAME,
        std::vector<LogicalTypeID>{LogicalTypeID::BOOL}, LogicalTypeID::STRING,
        UnaryCastExecFunction<bool, ku_string_t, CastToString>));
    result.push_back(make_unique<VectorFunctionDefinition>(CAST_TO_STRING_FUNC_NAME,
        std::vector<LogicalTypeID>{LogicalTypeID::INT64}, LogicalTypeID::STRING,
        UnaryCastExecFunction<int64_t, ku_string_t, CastToString>));
    result.push_back(make_unique<VectorFunctionDefinition>(CAST_TO_STRING_FUNC_NAME,
        std::vector<LogicalTypeID>{LogicalTypeID::INT32}, LogicalTypeID::STRING,
        UnaryCastExecFunction<int32_t, ku_string_t, CastToString>));
    result.push_back(make_unique<VectorFunctionDefinition>(CAST_TO_STRING_FUNC_NAME,
        std::vector<LogicalTypeID>{LogicalTypeID::INT16}, LogicalTypeID::STRING,
        UnaryCastExecFunction<int16_t, ku_string_t, CastToString>));
    result.push_back(make_unique<VectorFunctionDefinition>(CAST_TO_STRING_FUNC_NAME,
        std::vector<LogicalTypeID>{LogicalTypeID::INT8}, LogicalTypeID::STRING,
        UnaryCastExecFunction<int8_t, ku_string_t, CastToString>));
    result.push_back(make_unique<VectorFunctionDefinition>(CAST_TO_STRING_FUNC_NAME,
        std::vector<LogicalTypeID>{LogicalTypeID::UINT64}, LogicalTypeID::STRING,
        UnaryCastExecFunction<uint64_t, ku_string_t, CastToString>));
    result.push_back(make_unique<VectorFunctionDefinition>(CAST_TO_STRING_FUNC_NAME,
        std::vector<LogicalTypeID>{LogicalTypeID::UINT32}, LogicalTypeID::STRING,
        UnaryCastExecFunction<uint32_t, ku_string_t, CastToString>));
    result.push_back(make_unique<VectorFunctionDefinition>(CAST_TO_STRING_FUNC_NAME,
        std::vector<LogicalTypeID>{LogicalTypeID::UINT16}, LogicalTypeID::STRING,
        UnaryCastExecFunction<uint16_t, ku_string_t, CastToString>));
    result.push_back(make_unique<VectorFunctionDefinition>(CAST_TO_STRING_FUNC_NAME,
        std::vector<LogicalTypeID>{LogicalTypeID::UINT8}, LogicalTypeID::STRING,
        UnaryCastExecFunction<uint8_t, ku_string_t, CastToString>));
    result.push_back(make_unique<VectorFunctionDefinition>(CAST_TO_STRING_FUNC_NAME,
        std::vector<LogicalTypeID>{LogicalTypeID::DOUBLE}, LogicalTypeID::STRING,
        UnaryCastExecFunction<double_t, ku_string_t, CastToString>));
    result.push_back(make_unique<VectorFunctionDefinition>(CAST_TO_STRING_FUNC_NAME,
        std::vector<LogicalTypeID>{LogicalTypeID::FLOAT}, LogicalTypeID::STRING,
        UnaryCastExecFunction<float_t, ku_string_t, CastToString>));
    result.push_back(make_unique<VectorFunctionDefinition>(CAST_TO_STRING_FUNC_NAME,
        std::vector<LogicalTypeID>{LogicalTypeID::DATE}, LogicalTypeID::STRING,
        UnaryCastExecFunction<date_t, ku_string_t, CastToString>));
    result.push_back(make_unique<VectorFunctionDefinition>(CAST_TO_STRING_FUNC_NAME,
        std::vector<LogicalTypeID>{LogicalTypeID::TIMESTAMP}, LogicalTypeID::STRING,
        UnaryCastExecFunction<timestamp_t, ku_string_t, CastToString>));
    result.push_back(make_unique<VectorFunctionDefinition>(CAST_TO_STRING_FUNC_NAME,
        std::vector<LogicalTypeID>{LogicalTypeID::INTERVAL}, LogicalTypeID::STRING,
        UnaryCastExecFunction<interval_t, ku_string_t, CastToString>));
    result.push_back(make_unique<VectorFunctionDefinition>(CAST_TO_STRING_FUNC_NAME,
        std::vector<LogicalTypeID>{LogicalTypeID::INTERNAL_ID}, LogicalTypeID::STRING,
        UnaryCastExecFunction<internalID_t, ku_string_t, CastToString>));
    result.push_back(make_unique<VectorFunctionDefinition>(CAST_TO_STRING_FUNC_NAME,
        std::vector<LogicalTypeID>{LogicalTypeID::STRING}, LogicalTypeID::STRING,
        UnaryCastExecFunction<ku_string_t, ku_string_t, CastToString>));
    result.push_back(make_unique<VectorFunctionDefinition>(CAST_TO_STRING_FUNC_NAME,
        std::vector<LogicalTypeID>{LogicalTypeID::VAR_LIST}, LogicalTypeID::STRING,
        UnaryCastExecFunction<list_entry_t, ku_string_t, CastToString>));
    result.push_back(make_unique<VectorFunctionDefinition>(CAST_TO_STRING_FUNC_NAME,
        std::vector<LogicalTypeID>{LogicalTypeID::STRUCT}, LogicalTypeID::STRING,
        UnaryCastExecFunction<struct_entry_t, ku_string_t, CastToString>));
    return result;
}

vector_function_definitions CastToBlobVectorFunction::getDefinitions() {
    vector_function_definitions result;
    result.push_back(make_unique<VectorFunctionDefinition>(CAST_TO_BLOB_FUNC_NAME,
        std::vector<LogicalTypeID>{LogicalTypeID::STRING}, LogicalTypeID::BLOB,
        UnaryCastExecFunction<ku_string_t, blob_t, CastToBlob>));
    return result;
}

vector_function_definitions CastToDoubleVectorFunction::getDefinitions() {
    vector_function_definitions result;
    result.push_back(bindVectorFunction<int8_t, double_t, CastToDouble>(
        CAST_TO_DOUBLE_FUNC_NAME, LogicalTypeID::INT8, LogicalTypeID::DOUBLE));
    result.push_back(bindVectorFunction<int16_t, double_t, CastToDouble>(
        CAST_TO_DOUBLE_FUNC_NAME, LogicalTypeID::INT16, LogicalTypeID::DOUBLE));
    result.push_back(bindVectorFunction<int32_t, double_t, CastToDouble>(
        CAST_TO_DOUBLE_FUNC_NAME, LogicalTypeID::INT32, LogicalTypeID::DOUBLE));
    result.push_back(bindVectorFunction<int64_t, double_t, CastToDouble>(
        CAST_TO_DOUBLE_FUNC_NAME, LogicalTypeID::INT64, LogicalTypeID::DOUBLE));
    result.push_back(bindVectorFunction<uint64_t, double_t, CastToDouble>(
        CAST_TO_DOUBLE_FUNC_NAME, LogicalTypeID::UINT64, LogicalTypeID::DOUBLE));
    result.push_back(bindVectorFunction<uint32_t, double_t, CastToDouble>(
        CAST_TO_DOUBLE_FUNC_NAME, LogicalTypeID::UINT32, LogicalTypeID::DOUBLE));
    result.push_back(bindVectorFunction<uint16_t, double_t, CastToDouble>(
        CAST_TO_DOUBLE_FUNC_NAME, LogicalTypeID::UINT16, LogicalTypeID::DOUBLE));
    result.push_back(bindVectorFunction<uint8_t, double_t, CastToDouble>(
        CAST_TO_DOUBLE_FUNC_NAME, LogicalTypeID::UINT8, LogicalTypeID::DOUBLE));
    result.push_back(bindVectorFunction<float_t, double_t, CastToDouble>(
        CAST_TO_DOUBLE_FUNC_NAME, LogicalTypeID::FLOAT, LogicalTypeID::DOUBLE));
    result.push_back(bindVectorFunction<ku_string_t, double_t, CastToDouble>(
        CAST_TO_DOUBLE_FUNC_NAME, LogicalTypeID::STRING, LogicalTypeID::DOUBLE));
    return result;
}

vector_function_definitions CastToFloatVectorFunction::getDefinitions() {
    vector_function_definitions result;
    result.push_back(bindVectorFunction<int8_t, float_t, CastToFloat>(
        CAST_TO_FLOAT_FUNC_NAME, LogicalTypeID::INT8, LogicalTypeID::FLOAT));
    result.push_back(bindVectorFunction<int16_t, float_t, CastToFloat>(
        CAST_TO_FLOAT_FUNC_NAME, LogicalTypeID::INT16, LogicalTypeID::FLOAT));
    result.push_back(bindVectorFunction<int32_t, float_t, CastToFloat>(
        CAST_TO_FLOAT_FUNC_NAME, LogicalTypeID::INT32, LogicalTypeID::FLOAT));
    result.push_back(bindVectorFunction<int64_t, float_t, CastToFloat>(
        CAST_TO_FLOAT_FUNC_NAME, LogicalTypeID::INT64, LogicalTypeID::FLOAT));
    result.push_back(bindVectorFunction<uint64_t, float_t, CastToFloat>(
        CAST_TO_FLOAT_FUNC_NAME, LogicalTypeID::UINT64, LogicalTypeID::FLOAT));
    result.push_back(bindVectorFunction<uint32_t, float_t, CastToFloat>(
        CAST_TO_FLOAT_FUNC_NAME, LogicalTypeID::UINT32, LogicalTypeID::FLOAT));
    result.push_back(bindVectorFunction<uint16_t, float_t, CastToFloat>(
        CAST_TO_FLOAT_FUNC_NAME, LogicalTypeID::UINT16, LogicalTypeID::FLOAT));
    result.push_back(bindVectorFunction<uint8_t, float_t, CastToFloat>(
        CAST_TO_FLOAT_FUNC_NAME, LogicalTypeID::UINT8, LogicalTypeID::FLOAT));
    result.push_back(bindVectorFunction<ku_string_t, float_t, CastToFloat>(
        CAST_TO_FLOAT_FUNC_NAME, LogicalTypeID::STRING, LogicalTypeID::FLOAT));
    // down cast
    result.push_back(bindVectorFunction<double_t, float_t, CastToFloat>(
        CAST_TO_FLOAT_FUNC_NAME, LogicalTypeID::DOUBLE, LogicalTypeID::FLOAT));
    return result;
}

vector_function_definitions CastToSerialVectorFunction::getDefinitions() {
    vector_function_definitions result;
    result.push_back(bindVectorFunction<int8_t, int64_t, CastToSerial>(
        CAST_TO_SERIAL_FUNC_NAME, LogicalTypeID::INT8, LogicalTypeID::SERIAL));
    result.push_back(bindVectorFunction<int16_t, int64_t, CastToSerial>(
        CAST_TO_SERIAL_FUNC_NAME, LogicalTypeID::INT16, LogicalTypeID::SERIAL));
    result.push_back(bindVectorFunction<int32_t, int64_t, CastToSerial>(
        CAST_TO_SERIAL_FUNC_NAME, LogicalTypeID::INT32, LogicalTypeID::SERIAL));
    result.push_back(bindVectorFunction<uint64_t, int64_t, CastToSerial>(
        CAST_TO_SERIAL_FUNC_NAME, LogicalTypeID::UINT64, LogicalTypeID::SERIAL));
    result.push_back(bindVectorFunction<uint32_t, int64_t, CastToSerial>(
        CAST_TO_SERIAL_FUNC_NAME, LogicalTypeID::UINT32, LogicalTypeID::SERIAL));
    result.push_back(bindVectorFunction<uint16_t, int64_t, CastToSerial>(
        CAST_TO_SERIAL_FUNC_NAME, LogicalTypeID::UINT16, LogicalTypeID::SERIAL));
    result.push_back(bindVectorFunction<uint8_t, int64_t, CastToSerial>(
        CAST_TO_SERIAL_FUNC_NAME, LogicalTypeID::UINT8, LogicalTypeID::SERIAL));
    // down cast
    result.push_back(bindVectorFunction<float_t, int64_t, CastToSerial>(
        CAST_TO_SERIAL_FUNC_NAME, LogicalTypeID::FLOAT, LogicalTypeID::SERIAL));
    result.push_back(bindVectorFunction<double_t, int64_t, CastToSerial>(
        CAST_TO_SERIAL_FUNC_NAME, LogicalTypeID::DOUBLE, LogicalTypeID::SERIAL));
    return result;
}

vector_function_definitions CastToInt64VectorFunction::getDefinitions() {
    vector_function_definitions result;
    result.push_back(bindVectorFunction<int8_t, int64_t, CastToInt64>(
        CAST_TO_INT64_FUNC_NAME, LogicalTypeID::INT8, LogicalTypeID::INT64));
    result.push_back(bindVectorFunction<int16_t, int64_t, CastToInt64>(
        CAST_TO_INT64_FUNC_NAME, LogicalTypeID::INT16, LogicalTypeID::INT64));
    result.push_back(bindVectorFunction<int32_t, int64_t, CastToInt64>(
        CAST_TO_INT64_FUNC_NAME, LogicalTypeID::INT32, LogicalTypeID::INT64));
    result.push_back(bindVectorFunction<uint64_t, int64_t, CastToInt64>(
        CAST_TO_INT64_FUNC_NAME, LogicalTypeID::UINT64, LogicalTypeID::INT64));
    result.push_back(bindVectorFunction<float_t, int64_t, CastToInt64>(
        CAST_TO_INT64_FUNC_NAME, LogicalTypeID::FLOAT, LogicalTypeID::INT64));
    result.push_back(bindVectorFunction<double_t, int64_t, CastToInt64>(
        CAST_TO_INT64_FUNC_NAME, LogicalTypeID::DOUBLE, LogicalTypeID::INT64));
    // down cast end
    result.push_back(bindVectorFunction<uint32_t, int64_t, CastToInt64>(
        CAST_TO_INT64_FUNC_NAME, LogicalTypeID::UINT32, LogicalTypeID::INT64));
    result.push_back(bindVectorFunction<uint16_t, int64_t, CastToInt64>(
        CAST_TO_INT64_FUNC_NAME, LogicalTypeID::UINT16, LogicalTypeID::INT64));
    result.push_back(bindVectorFunction<uint8_t, int64_t, CastToInt64>(
        CAST_TO_INT64_FUNC_NAME, LogicalTypeID::UINT8, LogicalTypeID::INT64));
    result.push_back(bindVectorFunction<ku_string_t, int64_t, CastToInt64>(
        CAST_TO_INT64_FUNC_NAME, LogicalTypeID::STRING, LogicalTypeID::INT64));
    return result;
}

vector_function_definitions CastToInt32VectorFunction::getDefinitions() {
    vector_function_definitions result;
    result.push_back(bindVectorFunction<int8_t, int32_t, CastToInt32>(
        CAST_TO_INT32_FUNC_NAME, LogicalTypeID::INT8, LogicalTypeID::INT32));
    result.push_back(bindVectorFunction<int16_t, int32_t, CastToInt32>(
        CAST_TO_INT32_FUNC_NAME, LogicalTypeID::INT16, LogicalTypeID::INT32));
    // down cast
    result.push_back(bindVectorFunction<int64_t, int32_t, CastToInt32>(
        CAST_TO_INT32_FUNC_NAME, LogicalTypeID::INT64, LogicalTypeID::INT32));
    result.push_back(bindVectorFunction<uint64_t, int32_t, CastToInt32>(
        CAST_TO_INT32_FUNC_NAME, LogicalTypeID::UINT64, LogicalTypeID::INT32));
    result.push_back(bindVectorFunction<uint32_t, int32_t, CastToInt32>(
        CAST_TO_INT32_FUNC_NAME, LogicalTypeID::UINT32, LogicalTypeID::INT32));
    result.push_back(bindVectorFunction<float_t, int32_t, CastToInt32>(
        CAST_TO_INT32_FUNC_NAME, LogicalTypeID::FLOAT, LogicalTypeID::INT32));
    result.push_back(bindVectorFunction<double_t, int32_t, CastToInt32>(
        CAST_TO_INT32_FUNC_NAME, LogicalTypeID::DOUBLE, LogicalTypeID::INT32));
    // down cast end
    result.push_back(bindVectorFunction<uint16_t, int32_t, CastToInt32>(
        CAST_TO_INT32_FUNC_NAME, LogicalTypeID::UINT16, LogicalTypeID::INT32));
    result.push_back(bindVectorFunction<uint8_t, int32_t, CastToInt32>(
        CAST_TO_INT32_FUNC_NAME, LogicalTypeID::UINT8, LogicalTypeID::INT32));
    result.push_back(bindVectorFunction<ku_string_t, int32_t, CastToInt32>(
        CAST_TO_INT32_FUNC_NAME, LogicalTypeID::STRING, LogicalTypeID::INT32));
    return result;
}

vector_function_definitions CastToInt16VectorFunction::getDefinitions() {
    vector_function_definitions result;
    result.push_back(bindVectorFunction<int8_t, int16_t, CastToInt16>(
        CAST_TO_INT16_FUNC_NAME, LogicalTypeID::INT8, LogicalTypeID::INT16));
    // down cast
    result.push_back(bindVectorFunction<int32_t, int16_t, CastToInt16>(
        CAST_TO_INT16_FUNC_NAME, LogicalTypeID::INT32, LogicalTypeID::INT16));
    result.push_back(bindVectorFunction<int64_t, int16_t, CastToInt16>(
        CAST_TO_INT16_FUNC_NAME, LogicalTypeID::INT64, LogicalTypeID::INT16));
    result.push_back(bindVectorFunction<uint64_t, int16_t, CastToInt16>(
        CAST_TO_INT16_FUNC_NAME, LogicalTypeID::UINT64, LogicalTypeID::INT16));
    result.push_back(bindVectorFunction<uint32_t, int16_t, CastToInt16>(
        CAST_TO_INT16_FUNC_NAME, LogicalTypeID::UINT32, LogicalTypeID::INT16));
    result.push_back(bindVectorFunction<uint16_t, int16_t, CastToInt16>(
        CAST_TO_INT16_FUNC_NAME, LogicalTypeID::UINT16, LogicalTypeID::INT16));
    result.push_back(bindVectorFunction<float_t, int16_t, CastToInt16>(
        CAST_TO_INT16_FUNC_NAME, LogicalTypeID::FLOAT, LogicalTypeID::INT16));
    result.push_back(bindVectorFunction<double_t, int16_t, CastToInt16>(
        CAST_TO_INT16_FUNC_NAME, LogicalTypeID::DOUBLE, LogicalTypeID::INT16));
    // down cast end
    result.push_back(bindVectorFunction<uint8_t, int16_t, CastToInt16>(
        CAST_TO_INT16_FUNC_NAME, LogicalTypeID::UINT8, LogicalTypeID::INT16));
    result.push_back(bindVectorFunction<ku_string_t, int16_t, CastToInt16>(
        CAST_TO_INT16_FUNC_NAME, LogicalTypeID::STRING, LogicalTypeID::INT16));
    return result;
}

vector_function_definitions CastToInt8VectorFunction::getDefinitions() {
    vector_function_definitions result;
    // down cast
    result.push_back(bindVectorFunction<int16_t, int8_t, CastToInt8>(
        CAST_TO_INT8_FUNC_NAME, LogicalTypeID::INT16, LogicalTypeID::INT8));
    result.push_back(bindVectorFunction<int32_t, int8_t, CastToInt8>(
        CAST_TO_INT8_FUNC_NAME, LogicalTypeID::INT32, LogicalTypeID::INT8));
    result.push_back(bindVectorFunction<int64_t, int8_t, CastToInt8>(
        CAST_TO_INT8_FUNC_NAME, LogicalTypeID::INT64, LogicalTypeID::INT8));
    result.push_back(bindVectorFunction<uint64_t, int8_t, CastToInt8>(
        CAST_TO_INT8_FUNC_NAME, LogicalTypeID::UINT64, LogicalTypeID::INT8));
    result.push_back(bindVectorFunction<uint32_t, int8_t, CastToInt8>(
        CAST_TO_INT8_FUNC_NAME, LogicalTypeID::UINT32, LogicalTypeID::INT8));
    result.push_back(bindVectorFunction<uint16_t, int8_t, CastToInt8>(
        CAST_TO_INT8_FUNC_NAME, LogicalTypeID::UINT16, LogicalTypeID::INT8));
    result.push_back(bindVectorFunction<uint8_t, int8_t, CastToInt8>(
        CAST_TO_INT8_FUNC_NAME, LogicalTypeID::UINT8, LogicalTypeID::INT8));
    result.push_back(bindVectorFunction<float_t, int8_t, CastToInt8>(
        CAST_TO_INT8_FUNC_NAME, LogicalTypeID::FLOAT, LogicalTypeID::INT8));
    result.push_back(bindVectorFunction<uint64_t, int8_t, CastToInt8>(
        CAST_TO_INT8_FUNC_NAME, LogicalTypeID::UINT64, LogicalTypeID::INT8));
    result.push_back(bindVectorFunction<uint32_t, int8_t, CastToInt8>(
        CAST_TO_INT8_FUNC_NAME, LogicalTypeID::UINT32, LogicalTypeID::INT8));
    result.push_back(bindVectorFunction<uint16_t, int8_t, CastToInt8>(
        CAST_TO_INT8_FUNC_NAME, LogicalTypeID::UINT16, LogicalTypeID::INT8));
    result.push_back(bindVectorFunction<uint8_t, int8_t, CastToInt8>(
        CAST_TO_INT8_FUNC_NAME, LogicalTypeID::UINT8, LogicalTypeID::INT8));
    result.push_back(bindVectorFunction<double_t, int8_t, CastToInt8>(
        CAST_TO_INT8_FUNC_NAME, LogicalTypeID::DOUBLE, LogicalTypeID::INT8));
    result.push_back(bindVectorFunction<float_t, int8_t, CastToInt8>(
        CAST_TO_INT8_FUNC_NAME, LogicalTypeID::FLOAT, LogicalTypeID::INT8));
    result.push_back(bindVectorFunction<ku_string_t, int8_t, CastToInt8>(
        CAST_TO_INT8_FUNC_NAME, LogicalTypeID::STRING, LogicalTypeID::INT8));
    return result;
}

vector_function_definitions CastToUInt64VectorFunction::getDefinitions() {
    vector_function_definitions result;
    result.push_back(bindVectorFunction<uint8_t, uint64_t, CastToUInt64>(
        CAST_TO_UINT64_FUNC_NAME, LogicalTypeID::UINT8, LogicalTypeID::UINT64));
    result.push_back(bindVectorFunction<uint16_t, uint64_t, CastToUInt64>(
        CAST_TO_UINT64_FUNC_NAME, LogicalTypeID::UINT16, LogicalTypeID::UINT64));
    result.push_back(bindVectorFunction<uint32_t, uint64_t, CastToUInt64>(
        CAST_TO_UINT64_FUNC_NAME, LogicalTypeID::UINT32, LogicalTypeID::UINT64));
    result.push_back(bindVectorFunction<int8_t, uint64_t, CastToUInt64>(
        CAST_TO_UINT64_FUNC_NAME, LogicalTypeID::INT8, LogicalTypeID::UINT64));
    result.push_back(bindVectorFunction<int16_t, uint64_t, CastToUInt64>(
        CAST_TO_UINT64_FUNC_NAME, LogicalTypeID::INT16, LogicalTypeID::UINT64));
    result.push_back(bindVectorFunction<int32_t, uint64_t, CastToUInt64>(
        CAST_TO_UINT64_FUNC_NAME, LogicalTypeID::INT32, LogicalTypeID::UINT64));
    result.push_back(bindVectorFunction<int64_t, uint64_t, CastToUInt64>(
        CAST_TO_UINT64_FUNC_NAME, LogicalTypeID::INT64, LogicalTypeID::UINT64));
    // down cast
    result.push_back(bindVectorFunction<float_t, uint64_t, CastToUInt64>(
        CAST_TO_UINT64_FUNC_NAME, LogicalTypeID::FLOAT, LogicalTypeID::UINT64));
    result.push_back(bindVectorFunction<double_t, uint64_t, CastToUInt64>(
        CAST_TO_UINT64_FUNC_NAME, LogicalTypeID::DOUBLE, LogicalTypeID::UINT64));
    result.push_back(bindVectorFunction<ku_string_t, uint64_t, CastToUInt64>(
        CAST_TO_UINT64_FUNC_NAME, LogicalTypeID::STRING, LogicalTypeID::UINT64));
    return result;
}

vector_function_definitions CastToUInt32VectorFunction::getDefinitions() {
    vector_function_definitions result;
    result.push_back(bindVectorFunction<uint8_t, uint32_t, CastToUInt32>(
        CAST_TO_UINT32_FUNC_NAME, LogicalTypeID::UINT8, LogicalTypeID::UINT32));
    result.push_back(bindVectorFunction<uint16_t, uint32_t, CastToUInt32>(
        CAST_TO_UINT32_FUNC_NAME, LogicalTypeID::UINT16, LogicalTypeID::UINT32));
    // down cast
    result.push_back(bindVectorFunction<uint64_t, uint32_t, CastToUInt32>(
        CAST_TO_UINT32_FUNC_NAME, LogicalTypeID::UINT64, LogicalTypeID::UINT32));
    result.push_back(bindVectorFunction<int8_t, uint32_t, CastToUInt32>(
        CAST_TO_UINT32_FUNC_NAME, LogicalTypeID::INT8, LogicalTypeID::UINT32));
    result.push_back(bindVectorFunction<int16_t, uint32_t, CastToUInt32>(
        CAST_TO_UINT32_FUNC_NAME, LogicalTypeID::INT16, LogicalTypeID::UINT32));
    result.push_back(bindVectorFunction<int32_t, uint32_t, CastToUInt32>(
        CAST_TO_UINT32_FUNC_NAME, LogicalTypeID::INT32, LogicalTypeID::UINT32));
    result.push_back(bindVectorFunction<int64_t, uint32_t, CastToUInt32>(
        CAST_TO_UINT32_FUNC_NAME, LogicalTypeID::INT64, LogicalTypeID::UINT32));
    result.push_back(bindVectorFunction<float_t, uint32_t, CastToUInt32>(
        CAST_TO_UINT32_FUNC_NAME, LogicalTypeID::FLOAT, LogicalTypeID::UINT32));
    result.push_back(bindVectorFunction<double_t, uint32_t, CastToUInt32>(
        CAST_TO_UINT32_FUNC_NAME, LogicalTypeID::DOUBLE, LogicalTypeID::UINT32));
    result.push_back(bindVectorFunction<ku_string_t, uint32_t, CastToUInt32>(
        CAST_TO_UINT32_FUNC_NAME, LogicalTypeID::STRING, LogicalTypeID::UINT32));
    return result;
}

vector_function_definitions CastToUInt16VectorFunction::getDefinitions() {
    vector_function_definitions result;
    result.push_back(bindVectorFunction<uint8_t, uint16_t, CastToUInt16>(
        CAST_TO_UINT16_FUNC_NAME, LogicalTypeID::UINT8, LogicalTypeID::UINT16));
    // down cast
    result.push_back(bindVectorFunction<uint32_t, uint16_t, CastToUInt16>(
        CAST_TO_UINT16_FUNC_NAME, LogicalTypeID::UINT32, LogicalTypeID::UINT16));
    result.push_back(bindVectorFunction<uint64_t, uint16_t, CastToUInt16>(
        CAST_TO_UINT16_FUNC_NAME, LogicalTypeID::UINT64, LogicalTypeID::UINT16));
    result.push_back(bindVectorFunction<int8_t, uint16_t, CastToUInt16>(
        CAST_TO_UINT16_FUNC_NAME, LogicalTypeID::INT8, LogicalTypeID::UINT16));
    result.push_back(bindVectorFunction<int16_t, uint16_t, CastToUInt16>(
        CAST_TO_UINT16_FUNC_NAME, LogicalTypeID::INT16, LogicalTypeID::UINT16));
    result.push_back(bindVectorFunction<int32_t, uint16_t, CastToUInt16>(
        CAST_TO_UINT16_FUNC_NAME, LogicalTypeID::INT32, LogicalTypeID::UINT16));
    result.push_back(bindVectorFunction<int64_t, uint16_t, CastToUInt16>(
        CAST_TO_UINT16_FUNC_NAME, LogicalTypeID::INT64, LogicalTypeID::UINT16));
    result.push_back(bindVectorFunction<float_t, uint16_t, CastToUInt16>(
        CAST_TO_UINT16_FUNC_NAME, LogicalTypeID::FLOAT, LogicalTypeID::UINT16));
    result.push_back(bindVectorFunction<double_t, uint16_t, CastToUInt16>(
        CAST_TO_UINT16_FUNC_NAME, LogicalTypeID::DOUBLE, LogicalTypeID::UINT16));
    result.push_back(bindVectorFunction<ku_string_t, uint16_t, CastToUInt16>(
        CAST_TO_UINT16_FUNC_NAME, LogicalTypeID::STRING, LogicalTypeID::UINT16));
    return result;
}

vector_function_definitions CastToUInt8VectorFunction::getDefinitions() {
    vector_function_definitions result;
    // down cast
    result.push_back(bindVectorFunction<uint16_t, uint8_t, CastToUInt8>(
        CAST_TO_UINT8_FUNC_NAME, LogicalTypeID::UINT16, LogicalTypeID::UINT8));
    result.push_back(bindVectorFunction<uint32_t, uint8_t, CastToUInt8>(
        CAST_TO_UINT8_FUNC_NAME, LogicalTypeID::UINT32, LogicalTypeID::UINT8));
    result.push_back(bindVectorFunction<uint64_t, uint8_t, CastToUInt8>(
        CAST_TO_UINT8_FUNC_NAME, LogicalTypeID::UINT64, LogicalTypeID::UINT8));
    result.push_back(bindVectorFunction<int8_t, uint8_t, CastToUInt8>(
        CAST_TO_UINT8_FUNC_NAME, LogicalTypeID::INT8, LogicalTypeID::UINT8));
    result.push_back(bindVectorFunction<int16_t, uint8_t, CastToUInt8>(
        CAST_TO_UINT8_FUNC_NAME, LogicalTypeID::INT16, LogicalTypeID::UINT8));
    result.push_back(bindVectorFunction<int32_t, uint8_t, CastToUInt8>(
        CAST_TO_UINT8_FUNC_NAME, LogicalTypeID::INT32, LogicalTypeID::UINT8));
    result.push_back(bindVectorFunction<int64_t, uint8_t, CastToUInt8>(
        CAST_TO_UINT8_FUNC_NAME, LogicalTypeID::INT64, LogicalTypeID::UINT8));
    result.push_back(bindVectorFunction<float_t, uint8_t, CastToUInt8>(
        CAST_TO_UINT8_FUNC_NAME, LogicalTypeID::FLOAT, LogicalTypeID::UINT8));
    result.push_back(bindVectorFunction<double_t, uint8_t, CastToUInt8>(
        CAST_TO_UINT8_FUNC_NAME, LogicalTypeID::DOUBLE, LogicalTypeID::UINT8));
    result.push_back(bindVectorFunction<ku_string_t, uint8_t, CastToUInt8>(
        CAST_TO_UINT8_FUNC_NAME, LogicalTypeID::STRING, LogicalTypeID::UINT8));
    return result;
}

} // namespace function
} // namespace kuzu
