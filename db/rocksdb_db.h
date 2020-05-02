//
//  rocksdb_db.h
//  YCSB-C
//

#ifndef YCSB_C_ROCKSDB_DB_H_
#define YCSB_C_ROCKSDB_DB_H_

#include "core/db.h"

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "core/properties.h"
#include "rocksdb-cloud/rocksdb/db.h"
#include "rocksdb-cloud/rocksdb/options.h"

using std::cout;
using std::endl;

namespace ycsbc {

class RocksdbDB : public DB {
 public:
  RocksdbDB(const rocksdb::DBOptions& db_options, const std::string& dbpath) {
    rocksdb::DB::Open(db_options, dbpath, &rocksdb_);
  }
  RocksdbDB(const rocksdb::DBOptions& db_options, const std::string& dbpath,
            const std::vector<rocksdb::ColumnFamilyDescriptor>& column_families) {
    rocksdb::DB::Open(db_options, dbpath, column_families, &column_families_handles_, &rocksdb_);
    for (int i = 0; i < column_families.size()) {
      column_families_map_.emplace(column_families[i].name, i);
      column_families_.push_back(column_families[i]);
    }
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
  std::unordered_map<std::string, int> column_families_map_;
  std::vector<rocksdb::ColumnFamilyDescriptor> column_families_;
  std::vector<rocksdb::ColumnFamilyHandle*> column_families_handles_;
};

} // ycsbc

#endif // YCSB_C_REDIS_DB_H_

