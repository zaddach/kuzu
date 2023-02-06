#pragma once

#include <unordered_map>

#include "expression.h"

namespace kuzu {
namespace binder {

class NodeOrRelExpression : public Expression {
public:
    NodeOrRelExpression(common::DataTypeID dataTypeID, const std::string& uniqueName,
        std::vector<common::table_id_t> tableIDs)
        : Expression{common::VARIABLE, dataTypeID, uniqueName}, tableIDs{std::move(tableIDs)} {}
    ~NodeOrRelExpression() override = default;

    inline void addTableIDs(const std::vector<common::table_id_t>& tableIDsToAdd) {
        auto tableIDsMap = std::unordered_set<common::table_id_t>(tableIDs.begin(), tableIDs.end());
        for (auto tableID : tableIDsToAdd) {
            if (!tableIDsMap.contains(tableID)) {
                tableIDs.push_back(tableID);
            }
        }
    }
    inline bool isMultiLabeled() const { return tableIDs.size() > 1; }
    inline std::vector<common::table_id_t> getTableIDs() const { return tableIDs; }
    inline common::table_id_t getSingleTableID() const {
        assert(tableIDs.size() == 1);
        return tableIDs[0];
    }

    inline void addPropertyExpression(
        const std::string& propertyName, std::unique_ptr<Expression> property) {
        assert(!propertyNameToIdx.contains(propertyName));
        propertyNameToIdx.insert({propertyName, properties.size()});
        properties.push_back(std::move(property));
    }
    inline bool hasPropertyExpression(const std::string& propertyName) const {
        return propertyNameToIdx.contains(propertyName);
    }
    inline std::shared_ptr<Expression> getPropertyExpression(
        const std::string& propertyName) const {
        assert(propertyNameToIdx.contains(propertyName));
        return properties[propertyNameToIdx.at(propertyName)]->copy();
    }
    inline const std::vector<std::unique_ptr<Expression>>& getPropertyExpressions() const {
        return properties;
    }

protected:
    std::vector<common::table_id_t> tableIDs;
    std::unordered_map<std::string, size_t> propertyNameToIdx;
    std::vector<std::unique_ptr<Expression>> properties;
};

} // namespace binder
} // namespace kuzu
