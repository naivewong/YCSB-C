//
//  rocksdb_db.h
//  YCSB-C
//

#ifndef YCSB_C_ROCKSDB_DB_H_
#define YCSB_C_ROCKSDB_DB_H_

#include "core/db.h"

#include <iostream>
#include <string>
#include "core/properties.h"
#include "rocksdb-cloud/rocksdb/db.h"
#include "rocksdb-cloud/rocksdb/options.h"

using std::cout;
using std::endl;

namespace ycsbc {

Status DBImpl::Open(const DBOptions& db_options, const std::string& dbname,
                    const std::vector<ColumnFamilyDescriptor>& column_families,
                    std::vector<ColumnFamilyHandle*>* handles, DB** dbptr,
                    const bool seq_per_batch, const bool batch_per_txn)

class RocksdbDB : public DB {
 public:
  RocksdbDB(const rocksdb::DBOptions& db_options, const std::string& dbpath) {
    rocksdb::DB::Open(db_options, dbpath, &rocksdb_);
  }
  RocksdbDB(const rocksdb::DBOptions& db_options, const std::string& dbpath,
            const std::vector<rocksdb::ColumnFamilyDescriptor>& column_families,
            std::vector<rocksdb::ColumnFamilyHandle*>* handles) {
    rocksdb::DB::Open(db_options, dbpath, column_families, handles, &rocksdb_);
  }

  int Read(const std::string &table, const std::string &key,
           const std::vector<std::string> *fields,
           std::vector<KVPair> &result);

  int Scan(const std::string &table, const std::string &key,
           int len, const std::vector<std::string> *fields,
           std::vector<std::vector<KVPair>> &result);

  int Update(const std::string &table, const std::string &key,
             std::vector<KVPair> &values);

  int Insert(const std::string &table, const std::string &key,
             std::vector<KVPair> &values) {
    return Update(table, key, values);
  }

  int Delete(const std::string &table, const std::string &key);

 private:
  rocksdb::DB* rocksdb_;
};

} // ycsbc

#endif // YCSB_C_REDIS_DB_H_

