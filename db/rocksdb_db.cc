#include "db/rocksdb_db.h"

namespace ycsbc {

int RocksdbDB::Read(const std::string &table, const std::string &key,
           const std::vector<std::string> *fields,
           std::vector<KVPair> &result);

int RocksdbDB::Scan(const std::string &table, const std::string &key,
         int len, const std::vector<std::string> *fields,
         std::vector<std::vector<KVPair>> &result);

int RocksdbDB::Update(const std::string &table, const std::string &key,
           std::vector<KVPair> &values);

int RocksdbDB::Insert(const std::string &table, const std::string &key,
           std::vector<KVPair> &values) {
  return Update(table, key, values);
}

int RocksdbDB::Delete(const std::string &table, const std::string &key);

}