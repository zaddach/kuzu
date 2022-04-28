#pragma once

#include "client_context.h"
#include "prepared_statement.h"
#include "query_result.h"

#include "src/planner/logical_plan/include/logical_plan.h"
#include "src/transaction/include/transaction_manager.h"

using namespace graphflow::planner;
using namespace graphflow::transaction;
using lock_t = unique_lock<mutex>;

namespace graphflow {
namespace main {

class Database;
class Connection {

public:
    /**
     * If the connection is in AUTO_COMMIT mode any query over the connection will be wrapped around
     * a transaction and committed (even if the query is READ_ONLY).
     * If the connection is in MANUAL transaction mode, which happens only if an application
     * manually begins a transaction (see below), then an application has to manually commit or
     * rollback the transaction by calling commit() or rollback().
     *
     * AUTO_COMMIT is the default mode when a Connection is created. If an application calls
     * begin[ReadOnly/Write]Transaction at any point, the mode switches to MANUAL. This creates
     * an "active transaction" in the connection. When a connection is in MANUAL mode and the
     * active transaction is rolled back or committed, then the active transaction is removed (so
     * the connection no longer has an active transaction) and the mode automatically switches
     * back to AUTO_COMMIT.
     * Note: When a Connection object is deconstructed, if the connection has an active (manual)
     * transaction, then the active transaction is rolled back.
     */
    enum ConnectionTransactionMode : uint8_t { AUTO_COMMIT, MANUAL };

public:
    explicit Connection(Database* database);

    ~Connection();

    inline ConnectionTransactionMode getTransactionMode() {
        lock_t lck{mtx};
        return transactionMode;
    }

    inline void beginReadOnlyTransaction() {
        lock_t lck{mtx};
        setTransactionModeNoLock(MANUAL);
        beginTransactionNoLock(READ_ONLY);
    }

    inline void beginWriteTransaction() {
        lock_t lck{mtx};
        setTransactionModeNoLock(MANUAL);
        beginTransactionNoLock(WRITE);
    }

    inline void commit() {
        lock_t lck{mtx};
        commitOrRollbackNoLock(true /* isCommit */);
    }

    inline void rollback() {
        lock_t lck{mtx};
        commitOrRollbackNoLock(false /* is rollback */);
    }

    inline void setMaxNumThreadForExec(uint64_t numThreads) {
        clientContext->numThreadsForExecution = numThreads;
    }

    std::unique_ptr<QueryResult> query(const std::string& query);

    std::unique_ptr<PreparedStatement> prepare(const std::string& query) {
        lock_t lck{mtx};
        return prepareNoLock(query);
    }

    template<typename... Args>
    inline std::unique_ptr<QueryResult> execute(
        PreparedStatement* preparedStatement, pair<string, Args>... args) {
        unordered_map<string, shared_ptr<Literal>> inputParameters;
        return executeWithParams(preparedStatement, inputParameters, args...);
    }

    // Note: Any call that goes through executeWithParams acquires a lock in the end by calling
    // executeLock(...).
    std::unique_ptr<QueryResult> executeWithParams(PreparedStatement* preparedStatement,
        unordered_map<string, shared_ptr<Literal>>& inputParams);

    // Catalog utility interfaces
    inline string getBuiltInFunctionNames() {
        return getBuiltInScalarFunctionNames() + "\n" + getBuiltInAggregateFunctionNames();
    }
    string getBuiltInScalarFunctionNames();
    string getBuiltInAggregateFunctionNames();
    string getNodeLabelNames();
    string getRelLabelNames();
    string getNodePropertyNames(const string& nodeLabelName);
    string getRelPropertyNames(const string& relLabelName);

    // Used in test helper. Note: for our testing framework, we should not catch exception and
    // instead let IDE catch these exception
    std::vector<unique_ptr<planner::LogicalPlan>> enumeratePlans(const std::string& query);
    std::unique_ptr<QueryResult> executePlan(unique_ptr<planner::LogicalPlan> logicalPlan);
    // used in API test
    inline uint64_t getMaxNumThreadForExec() {
        lock_t lck{mtx};
        return clientContext->numThreadsForExecution;
    }

    inline uint64_t getActiveTransactionID() {
        lock_t lck{mtx};
        return activeTransaction ? activeTransaction->getID() : UINT64_MAX;
    }

    inline bool hasActiveTransaction() {
        lock_t lck{mtx};
        return activeTransaction != nullptr;
    }

private:
    void setTransactionModeNoLock(ConnectionTransactionMode newTransactionMode) {
        if (activeTransaction && transactionMode == MANUAL && newTransactionMode == AUTO_COMMIT) {
            throw ConnectionException(
                "Cannot change transaction mode from MANUAL to AUTO_COMMING when there is an "
                "active transaction. Need to first commit or rollback the active transaction.");
        }
        transactionMode = newTransactionMode;
    }

    bool isManualModeAndNoActiveTransactionNoLock() {
        return transactionMode == MANUAL && !activeTransaction;
    }

    void beginTransactionNoLock(TransactionType type);
    inline void commitNoLock() { commitOrRollbackNoLock(true /* is commit */); }
    inline void rollbackNoLock() { commitOrRollbackNoLock(false /* is rollback */); }
    void commitOrRollbackNoLock(bool isCommit);

    unique_ptr<QueryResult> queryResultWithError(std::string& errMsg);

    inline unique_ptr<QueryResult> queryResultWithErrorForNoActiveTransaction() {
        std::string errMsg("Transaction mode is manual but there is no active "
                           "transaction. Please begin a transaction"
                           "or set the transaction mode of the connection to AUTO_COMMIT");
        return queryResultWithError(errMsg);
    }
    std::unique_ptr<PreparedStatement> prepareNoLock(const std::string& query);

    // Note: Any call that goes through executeWithParams acquires a lock in the end by calling
    // executeLock(...).
    template<typename T, typename... Args>
    std::unique_ptr<QueryResult> executeWithParams(PreparedStatement* preparedStatement,
        unordered_map<string, shared_ptr<Literal>>& params, pair<string, T> arg,
        pair<string, Args>... args) {
        auto name = arg.first;
        auto val = make_shared<Literal>(Literal::createLiteral<T>(arg.second));
        params.insert({name, val});
        return executeWithParams(preparedStatement, params, args...);
    }

    void bindParametersNoLock(PreparedStatement* preparedStatement,
        unordered_map<string, shared_ptr<Literal>>& inputParams);

    std::unique_ptr<QueryResult> executeLock(PreparedStatement* preparedStatement) {
        lock_t lck{mtx};
        if (isManualModeAndNoActiveTransactionNoLock()) {
            return queryResultWithErrorForNoActiveTransaction();
        }
        return executeAndAutoCommitIfNecessaryNoLock(preparedStatement);
    }
    std::unique_ptr<QueryResult> executeAndAutoCommitIfNecessaryNoLock(
        PreparedStatement* preparedStatement);

private:
    Database* database;
    std::unique_ptr<ClientContext> clientContext;
    std::unique_ptr<Transaction> activeTransaction;
    ConnectionTransactionMode transactionMode;
    std::mutex mtx;
};

} // namespace main
} // namespace graphflow
