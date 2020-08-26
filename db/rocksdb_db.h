//
//  rocksdb_db.h
//  YCSB-C
//

#ifndef YCSB_C_ROCKSDB_DB_H_
#define YCSB_C_ROCKSDB_DB_H_

#include "core/db.h"
#include "lib/mutexlock.h"

#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "core/properties.h"
#include "rocksdb-cloud/include/rocksdb/db.h"
#include "rocksdb-cloud/include/rocksdb/options.h"

using std::cout;
using std::endl;

namespace ycsbc {

class RocksdbDB : public DB {
 public:
  RocksdbDB(const rocksdb::Options& db_options, const std::string& dbpath) {
    rocksdb::Status s = rocksdb::DB::Open(db_options, dbpath, &rocksdb_);
    if (!s.ok()) {
      printf("cannot open rocksdb: %s\n", s.ToString().c_str());
      exit(-1);
    }
    column_families_map_.emplace("default", 0);
    column_families_handles_.push_back(rocksdb_->DefaultColumnFamily());
    column_families_.push_back(rocksdb::ColumnFamilyDescriptor("default", rocksdb::ColumnFamilyOptions()));
  }
  RocksdbDB(const rocksdb::Options& db_options, const std::string& dbpath,
            const std::vector<rocksdb::ColumnFamilyDescriptor>& column_families) {
    rocksdb::Status s = rocksdb::DB::Open(db_options, dbpath, column_families, &column_families_handles_, &rocksdb_);
    if (!s.ok()) {
      printf("cannot open rocksdb: %s\n", s.ToString().c_str());
      exit(-1);
    }
    for (int i = 0; i < (int)(column_families.size()); i++) {
      column_families_map_.emplace(column_families[i].name, i);
      column_families_.push_back(column_families[i]);
    }
  }

  ~RocksdbDB() {
    delete rocksdb_;
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
             std::vector<KVPair> &values);

  int Delete(const std::string &table, const std::string &key);

 private:
  rocksdb::DB* rocksdb_;
  std::unordered_map<std::string, int> column_families_map_;
  std::vector<rocksdb::ColumnFamilyDescriptor> column_families_;
  std::vector<rocksdb::ColumnFamilyHandle*> column_families_handles_;
  RWMutex cf_lock_;

  std::string serialize_values(const std::unordered_map<std::string, std::string>& values);
  std::string serialize_values(const std::vector<KVPair>& values);
  void deserialize_values(
        const std::string& values,
        const std::unordered_set<std::string>* fields,
        std::unordered_map<std::string, std::string>* result);
  void deserialize_values(
        const std::string& values,
        const std::unordered_set<std::string>* fields,
        std::vector<KVPair>* result);
  int create_columnfamily(const std::string& name);
};

} // ycsbc

#endif // YCSB_C_ROCKSDB_DB_H_

