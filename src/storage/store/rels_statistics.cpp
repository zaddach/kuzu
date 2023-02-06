#include "storage/store/rels_statistics.h"

using namespace kuzu::common;

namespace kuzu {
namespace storage {

RelStatistics::RelStatistics(std::vector<std::pair<table_id_t, table_id_t>> srcDstTableIDs)
    : TableStatistics{0}, nextRelOffset{0} {
    numRelsPerDirectionBoundTable.resize(2);
    for (auto& [srcTableID, dstTableID] : srcDstTableIDs) {
        numRelsPerDirectionBoundTable[RelDirection::FWD].emplace(srcTableID, 0);
        numRelsPerDirectionBoundTable[RelDirection::BWD].emplace(dstTableID, 0);
    }
}

RelsStatistics::RelsStatistics(
    std::unordered_map<table_id_t, std::unique_ptr<RelStatistics>> relStatisticPerTable_)
    : TablesStatistics{} {
    initTableStatisticPerTableForWriteTrxIfNecessary();
    for (auto& relStatistic : relStatisticPerTable_) {
        tablesStatisticsContentForReadOnlyTrx->tableStatisticPerTable[relStatistic.first] =
            std::make_unique<RelStatistics>(*(RelStatistics*)relStatistic.second.get());
        tablesStatisticsContentForWriteTrx->tableStatisticPerTable[relStatistic.first] =
            std::make_unique<RelStatistics>(*(RelStatistics*)relStatistic.second.get());
    }
}

// We should only call this function after we call setNumRelsPerDirectionBoundTableID.
void RelsStatistics::setNumRelsForTable(table_id_t relTableID, uint64_t numRels) {
    lock_t lck{mtx};
    initTableStatisticPerTableForWriteTrxIfNecessary();
    assert(tablesStatisticsContentForWriteTrx->tableStatisticPerTable.contains(relTableID));
    auto relStatistics =
        (RelStatistics*)tablesStatisticsContentForWriteTrx->tableStatisticPerTable[relTableID]
            .get();
    increaseNextRelOffset(relTableID, numRels - relStatistics->getNumTuples());
    relStatistics->setNumTuples(numRels);
    assertNumRelsIsSound(relStatistics->numRelsPerDirectionBoundTable[FWD], numRels);
    assertNumRelsIsSound(relStatistics->numRelsPerDirectionBoundTable[BWD], numRels);
}

void RelsStatistics::assertNumRelsIsSound(
    std::unordered_map<table_id_t, uint64_t>& relsPerBoundTable, uint64_t numRels) {
    uint64_t sum = 0;
    for (auto tableIDNumRels : relsPerBoundTable) {
        sum += tableIDNumRels.second;
    }
    assert(sum == numRels);
}

void RelsStatistics::updateNumRelsByValue(
    table_id_t relTableID, table_id_t srcTableID, table_id_t dstTableID, int64_t value) {
    lock_t lck{mtx};
    initTableStatisticPerTableForWriteTrxIfNecessary();
    auto relStatistics =
        (RelStatistics*)tablesStatisticsContentForWriteTrx->tableStatisticPerTable[relTableID]
            .get();
    auto numRelsAfterUpdate = relStatistics->getNumTuples() + value;
    relStatistics->setNumTuples(numRelsAfterUpdate);
    for (auto relDirection : REL_DIRECTIONS) {
        auto relStatistics =
            (RelStatistics*)tablesStatisticsContentForWriteTrx->tableStatisticPerTable
                .at(relTableID)
                .get();
        relStatistics->numRelsPerDirectionBoundTable[relDirection].at(
            relDirection == FWD ? srcTableID : dstTableID) += value;
    }
    // Update the nextRelID only when we are inserting rels.
    if (value > 0) {
        increaseNextRelOffset(relTableID, value);
    }
    assertNumRelsIsSound(relStatistics->numRelsPerDirectionBoundTable[FWD], numRelsAfterUpdate);
    assertNumRelsIsSound(relStatistics->numRelsPerDirectionBoundTable[BWD], numRelsAfterUpdate);
}

void RelsStatistics::setNumRelsPerDirectionBoundTableID(table_id_t tableID,
    std::vector<std::map<table_id_t, std::atomic<uint64_t>>>& directionNumRelsPerTable) {
    lock_t lck{mtx};
    initTableStatisticPerTableForWriteTrxIfNecessary();
    for (auto relDirection : REL_DIRECTIONS) {
        for (auto const& tableIDNumRelPair : directionNumRelsPerTable[relDirection]) {
            ((RelStatistics*)tablesStatisticsContentForWriteTrx->tableStatisticPerTable[tableID]
                    .get())
                ->setNumRelsForDirectionBoundTable(
                    relDirection, tableIDNumRelPair.first, tableIDNumRelPair.second.load());
        }
    }
}

offset_t RelsStatistics::getNextRelOffset(
    transaction::Transaction* transaction, table_id_t tableID) {
    lock_t lck{mtx};
    auto& tableStatisticContent =
        (transaction->isReadOnly() || tablesStatisticsContentForWriteTrx == nullptr) ?
            tablesStatisticsContentForReadOnlyTrx :
            tablesStatisticsContentForWriteTrx;
    return ((RelStatistics*)tableStatisticContent->tableStatisticPerTable.at(tableID).get())
        ->getNextRelOffset();
}

std::unique_ptr<TableStatistics> RelsStatistics::deserializeTableStatistics(
    uint64_t numTuples, uint64_t& offset, FileInfo* fileInfo, uint64_t tableID) {
    std::vector<std::unordered_map<table_id_t, uint64_t>> numRelsPerDirectionBoundTable{2};
    offset_t nextRelOffset;
    offset = SerDeser::deserializeUnorderedMap(numRelsPerDirectionBoundTable[0], fileInfo, offset);
    offset = SerDeser::deserializeUnorderedMap(numRelsPerDirectionBoundTable[1], fileInfo, offset);
    offset = SerDeser::deserializeValue(nextRelOffset, fileInfo, offset);
    return make_unique<RelStatistics>(
        numTuples, std::move(numRelsPerDirectionBoundTable), nextRelOffset);
}

void RelsStatistics::serializeTableStatistics(
    TableStatistics* tableStatistics, uint64_t& offset, FileInfo* fileInfo) {
    auto relStatistic = (RelStatistics*)tableStatistics;
    offset = SerDeser::serializeUnorderedMap(
        relStatistic->numRelsPerDirectionBoundTable[0], fileInfo, offset);
    offset = SerDeser::serializeUnorderedMap(
        relStatistic->numRelsPerDirectionBoundTable[1], fileInfo, offset);
    offset = SerDeser::serializeValue(relStatistic->nextRelOffset, fileInfo, offset);
}

} // namespace storage
} // namespace kuzu
