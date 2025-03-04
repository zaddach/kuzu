#pragma once

#include "binder/query/query_graph.h"
#include "bound_insert_info.h"
#include "bound_set_info.h"
#include "bound_updating_clause.h"

namespace kuzu {
namespace binder {

class BoundMergeClause : public BoundUpdatingClause {
public:
    BoundMergeClause(QueryGraphCollection queryGraphCollection,
        std::shared_ptr<Expression> predicate, std::vector<BoundInsertInfo> insertInfos,
        std::shared_ptr<Expression> distinctMark)
        : BoundUpdatingClause{common::ClauseType::MERGE},
          queryGraphCollection{std::move(queryGraphCollection)}, predicate{std::move(predicate)},
          insertInfos{std::move(insertInfos)}, distinctMark{std::move(distinctMark)} {}

    const QueryGraphCollection* getQueryGraphCollection() const { return &queryGraphCollection; }
    bool hasPredicate() const { return predicate != nullptr; }
    std::shared_ptr<Expression> getPredicate() const { return predicate; }

    const std::vector<BoundInsertInfo>& getInsertInfosRef() const { return insertInfos; }
    const std::vector<BoundSetPropertyInfo>& getOnMatchSetInfosRef() const {
        return onMatchSetPropertyInfos;
    }
    const std::vector<BoundSetPropertyInfo>& getOnCreateSetInfosRef() const {
        return onCreateSetPropertyInfos;
    }

    bool hasInsertNodeInfo() const {
        return hasInsertInfo(
            [](const BoundInsertInfo& info) { return info.tableType == common::TableType::NODE; });
    }
    std::vector<const BoundInsertInfo*> getInsertNodeInfos() const {
        return getInsertInfos(
            [](const BoundInsertInfo& info) { return info.tableType == common::TableType::NODE; });
    }
    bool hasInsertRelInfo() const {
        return hasInsertInfo(
            [](const BoundInsertInfo& info) { return info.tableType == common::TableType::REL; });
    }
    std::vector<const BoundInsertInfo*> getInsertRelInfos() const {
        return getInsertInfos(
            [](const BoundInsertInfo& info) { return info.tableType == common::TableType::REL; });
    }

    bool hasOnMatchSetNodeInfo() const {
        return hasOnMatchSetInfo([](const BoundSetPropertyInfo& info) {
            return info.updateTableType == UpdateTableType::NODE;
        });
    }
    std::vector<const BoundSetPropertyInfo*> getOnMatchSetNodeInfos() const {
        return getOnMatchSetInfos([](const BoundSetPropertyInfo& info) {
            return info.updateTableType == UpdateTableType::NODE;
        });
    }
    bool hasOnMatchSetRelInfo() const {
        return hasOnMatchSetInfo([](const BoundSetPropertyInfo& info) {
            return info.updateTableType == UpdateTableType::REL;
        });
    }
    std::vector<const BoundSetPropertyInfo*> getOnMatchSetRelInfos() const {
        return getOnMatchSetInfos([](const BoundSetPropertyInfo& info) {
            return info.updateTableType == UpdateTableType::REL;
        });
    }

    bool hasOnCreateSetNodeInfo() const {
        return hasOnCreateSetInfo([](const BoundSetPropertyInfo& info) {
            return info.updateTableType == UpdateTableType::NODE;
        });
    }
    std::vector<const BoundSetPropertyInfo*> getOnCreateSetNodeInfos() const {
        return getOnCreateSetInfos([](const BoundSetPropertyInfo& info) {
            return info.updateTableType == UpdateTableType::NODE;
        });
    }
    bool hasOnCreateSetRelInfo() const {
        return hasOnCreateSetInfo([](const BoundSetPropertyInfo& info) {
            return info.updateTableType == UpdateTableType::REL;
        });
    }
    std::vector<const BoundSetPropertyInfo*> getOnCreateSetRelInfos() const {
        return getOnCreateSetInfos([](const BoundSetPropertyInfo& info) {
            return info.updateTableType == UpdateTableType::REL;
        });
    }

    void addOnMatchSetPropertyInfo(BoundSetPropertyInfo setPropertyInfo) {
        onMatchSetPropertyInfos.push_back(std::move(setPropertyInfo));
    }
    void addOnCreateSetPropertyInfo(BoundSetPropertyInfo setPropertyInfo) {
        onCreateSetPropertyInfos.push_back(std::move(setPropertyInfo));
    }

    std::shared_ptr<Expression> getDistinctMark() const { return distinctMark; }

private:
    bool hasInsertInfo(const std::function<bool(const BoundInsertInfo& info)>& check) const;
    std::vector<const BoundInsertInfo*> getInsertInfos(
        const std::function<bool(const BoundInsertInfo& info)>& check) const;

    bool hasOnMatchSetInfo(
        const std::function<bool(const BoundSetPropertyInfo& info)>& check) const;
    std::vector<const BoundSetPropertyInfo*> getOnMatchSetInfos(
        const std::function<bool(const BoundSetPropertyInfo& info)>& check) const;

    bool hasOnCreateSetInfo(
        const std::function<bool(const BoundSetPropertyInfo& info)>& check) const;
    std::vector<const BoundSetPropertyInfo*> getOnCreateSetInfos(
        const std::function<bool(const BoundSetPropertyInfo& info)>& check) const;

private:
    // Pattern to match.
    QueryGraphCollection queryGraphCollection;
    std::shared_ptr<Expression> predicate;
    // Pattern to create on match failure.
    std::vector<BoundInsertInfo> insertInfos;
    // Update on match
    std::vector<BoundSetPropertyInfo> onMatchSetPropertyInfos;
    // Update on create
    std::vector<BoundSetPropertyInfo> onCreateSetPropertyInfos;
    std::shared_ptr<Expression> distinctMark;
};

} // namespace binder
} // namespace kuzu
