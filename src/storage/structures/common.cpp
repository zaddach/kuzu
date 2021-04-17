#include "src/storage/include/structures/common.h"

namespace graphflow {
namespace storage {

bool ListSyncState::hasNewRangeToRead() {
    if (!hasValidRangeToRead()) {
        return false;
    }
    if (startIdx + size == numElements) {
        reset();
        return false;
    }
    return true;
}

void ListSyncState::reset() {
    startIdx = -1u;
    size = -1u;
    numElements = -1u;
}

bool ColumnOrListsHandle::hasMoreToRead() {
    if (isAdjListsHandle) {
        return listSyncState->hasNewRangeToRead();
    } else {
        return listSyncState->hasValidRangeToRead();
    }
}

BaseColumnOrLists::BaseColumnOrLists(const string& fname, const DataType& dataType,
    const size_t& elementSize, BufferManager& bufferManager)
    : logger{spdlog::get("storage")}, dataType{dataType}, elementSize{elementSize},
      numElementsPerPage{(uint32_t)(PAGE_SIZE / elementSize)}, fileHandle{fname},
      bufferManager(bufferManager){};

void BaseColumnOrLists::reclaim(const unique_ptr<ColumnOrListsHandle>& handle) {
    if (handle->hasPageIdx()) {
        bufferManager.unpin(fileHandle, handle->getPageIdx());
        handle->resetPageIdx();
    }
}

void BaseColumnOrLists::readBySettingFrame(const shared_ptr<ValueVector>& valueVector,
    const unique_ptr<ColumnOrListsHandle>& handle, uint64_t pageIdx, uint64_t pageOffset) {
    const uint8_t* frame;
    if (handle->getPageIdx() != pageIdx) {
        reclaim(handle);
        handle->setPageIdx(pageIdx);
        frame = bufferManager.pin(fileHandle, pageIdx);
    } else {
        frame = bufferManager.get(fileHandle, pageIdx);
    }
    valueVector->values = (uint8_t*)frame + pageOffset;
}

void BaseColumnOrLists::readBySequentialCopy(const shared_ptr<ValueVector>& valueVector,
    const unique_ptr<ColumnOrListsHandle>& handle, uint64_t sizeLeftToCopy, uint64_t pageIdx,
    uint64_t pageOffset, unique_ptr<LogicalToPhysicalPageIdxMapper> mapper) {
    reclaim(handle);
    valueVector->reset();
    auto values = valueVector->values;
    while (sizeLeftToCopy) {
        uint64_t physicalPageIdx = nullptr == mapper ? pageIdx : mapper->getPageIdx(pageIdx);
        auto sizeToCopyInPage = min(PAGE_SIZE - pageOffset, sizeLeftToCopy);
        auto frame = bufferManager.pin(fileHandle, physicalPageIdx);
        memcpy(values, frame + pageOffset, sizeToCopyInPage);
        bufferManager.unpin(fileHandle, physicalPageIdx);
        values += sizeToCopyInPage;
        sizeLeftToCopy -= sizeToCopyInPage;
        pageOffset = 0;
        pageIdx++;
    }
}

} // namespace storage
} // namespace graphflow
