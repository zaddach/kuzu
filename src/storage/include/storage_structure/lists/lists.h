#pragma once

#include <utility>

#include "src/common/types/include/literal.h"
#include "src/common/types/include/value.h"
#include "src/storage/include/storage_structure/lists/large_list_handle.h"
#include "src/storage/include/storage_structure/lists/list_headers.h"
#include "src/storage/include/storage_structure/lists/lists_metadata.h"
#include "src/storage/include/storage_structure/lists/utils.h"
#include "src/storage/include/storage_structure/overflow_pages.h"
#include "src/storage/include/storage_structure/storage_structure.h"

namespace graphflow {
namespace storage {

struct ListInfo {
    bool isLargeList{false};
    uint64_t listLen{-1u};
    std::function<uint32_t(uint32_t)> mapper;
    PageElementCursor cursor;
};

/**
 * A lists data structure holds a list of homogeneous values for each offset in it. Lists are used
 * for storing Adjacency List, Rel Property Lists and unstructured Node Property Lists.
 *
 * The offsets in the Lists are partitioned into fixed size. Hence, each offset, and its list,
 * belongs to a chunk. If the offset's list is small (less than the PAGE_SIZE) it is stored together
 * along with other lists in that chunk as in a CSR. However, large lists are stored out of their
 * regular chunks and span multiple pages. The nature, size and logical location of the list is
 * given by a 32-bit header value (explained in {@class ListHeaders}). Given the logical location of
 * a list, {@class ListsMetadata} contains information that maps logical location of the list to the
 * actual physical location in the Lists file on disk.
 * */
class Lists : public StorageStructure {

public:
    Lists(const string& fName, const DataType& dataType, const size_t& elementSize,
        shared_ptr<ListHeaders> headers, BufferManager& bufferManager, bool isInMemory)
        : Lists{fName, dataType, elementSize, headers, bufferManager, true /*hasNULLBytes*/,
              isInMemory} {};

    void readValues(node_offset_t nodeOffset, const shared_ptr<ValueVector>& valueVector,
        const unique_ptr<LargeListHandle>& largeListHandle);

    inline uint64_t getNumElementsInList(node_offset_t nodeOffset) {
        return getListInfo(nodeOffset).listLen;
    }

    ListInfo getListInfo(node_offset_t nodeOffset);

protected:
    Lists(const string& fName, const DataType& dataType, const size_t& elementSize,
        shared_ptr<ListHeaders> headers, BufferManager& bufferManager, bool hasNULLBytes,
        bool isInMemory)
        : StorageStructure{fName, dataType, elementSize, bufferManager, hasNULLBytes, isInMemory},
          metadata{fName}, headers(move(headers)){};

    virtual void readFromLargeList(const shared_ptr<ValueVector>& valueVector,
        const unique_ptr<LargeListHandle>& largeListHandle, ListInfo& info);

    virtual void readSmallList(const shared_ptr<ValueVector>& valueVector, ListInfo& info);

public:
    static constexpr char LISTS_SUFFIX[] = ".lists";

    // LIST_CHUNK_SIZE should strictly be a power of 2.
    constexpr static uint16_t LISTS_CHUNK_SIZE_LOG_2 = 9;
    constexpr static uint16_t LISTS_CHUNK_SIZE = 1 << LISTS_CHUNK_SIZE_LOG_2;

protected:
    ListsMetadata metadata;
    shared_ptr<ListHeaders> headers;
};

class StringPropertyLists : public Lists {

public:
    StringPropertyLists(const string& fName, shared_ptr<ListHeaders> headers,
        BufferManager& bufferManager, bool isInMemory)
        : Lists{fName, STRING, sizeof(gf_string_t), move(headers), bufferManager, isInMemory},
          stringOverflowPages{fName, bufferManager, isInMemory} {};

private:
    void readFromLargeList(const shared_ptr<ValueVector>& valueVector,
        const unique_ptr<LargeListHandle>& largeListHandle, ListInfo& info) override;

    void readSmallList(const shared_ptr<ValueVector>& valueVector, ListInfo& info) override;

private:
    OverflowPages stringOverflowPages;
};

class ListPropertyLists : public Lists {

public:
    ListPropertyLists(const string& fName, shared_ptr<ListHeaders> headers,
        BufferManager& bufferManager, bool isInMemory)
        : Lists{fName, LIST, sizeof(gf_list_t), move(headers), bufferManager, isInMemory},
          listOverflowPages{fName, bufferManager, isInMemory} {};

private:
    void readFromLargeList(const shared_ptr<ValueVector>& valueVector,
        const unique_ptr<LargeListHandle>& largeListHandle, ListInfo& info) override;

    void readSmallList(const shared_ptr<ValueVector>& valueVector, ListInfo& info) override;

private:
    OverflowPages listOverflowPages;
};

class AdjLists : public Lists {

public:
    AdjLists(const string& fName, BufferManager& bufferManager,
        NodeIDCompressionScheme nodeIDCompressionScheme, bool isInMemory)
        : Lists{fName, NODE, nodeIDCompressionScheme.getNumTotalBytes(),
              make_shared<ListHeaders>(fName), bufferManager, false, isInMemory},
          nodeIDCompressionScheme{nodeIDCompressionScheme} {};

    shared_ptr<ListHeaders> getHeaders() { return headers; };

    //    // Currently, used only in Loader tests.
    unique_ptr<vector<nodeID_t>> readAdjacencyListOfNode(node_offset_t nodeOffset);

private:
    void readFromLargeList(const shared_ptr<ValueVector>& valueVector,
        const unique_ptr<LargeListHandle>& largeListHandle, ListInfo& info) override;

    void readSmallList(const shared_ptr<ValueVector>& valueVector, ListInfo& info) override;

private:
    NodeIDCompressionScheme nodeIDCompressionScheme;
};

class ListsFactory {

public:
    static unique_ptr<Lists> getLists(const string& fName, const DataType& dataType,
        const shared_ptr<ListHeaders>& adjListsHeaders, BufferManager& bufferManager,
        bool isInMemory) {
        switch (dataType) {
        case INT64:
        case DOUBLE:
        case BOOL:
        case DATE:
        case TIMESTAMP:
        case INTERVAL:
            return make_unique<Lists>(fName, dataType, Types::getDataTypeSize(dataType),
                adjListsHeaders, bufferManager, isInMemory);
        case STRING:
            return make_unique<StringPropertyLists>(
                fName, adjListsHeaders, bufferManager, isInMemory);
        case LIST:
            return make_unique<ListPropertyLists>(
                fName, adjListsHeaders, bufferManager, isInMemory);
        default:
            throw invalid_argument("Invalid type for property list creation.");
        }
    }
};

} // namespace storage
} // namespace graphflow