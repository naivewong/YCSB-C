#include "core/utils.h"
#include "db/rocksdb_db.h"

namespace ycsbc {

int RocksdbDB::Read(const std::string &table, const std::string &key,
           const std::vector<std::string> *fields,
           std::vector<KVPair> &result) {
  if (column_families_map_.count(table) == 0 && create_columnfamily(table) != 0)
    return DB::kError;
  rocksdb::ColumnFamilyHandle* cfh;
  {
    ReadLock lock(&cf_lock_);
    int idx = column_families_map_[table];
    cfh = column_families_handles_[idx];
  }
  std::string value;
  rocksdb::Status s = rocksdb_->Get(rocksdb::ReadOptions(), cfh, rocksdb::Slice(key), &value);
  if (s.IsNotFound())
    return DB::kNotFound;
  else if (!s.ok())
    return DB::kError;
  std::unordered_set<std::string> fields_set;
  if (fields) {
    for (const std::string& f : *fields)
      fields_set.insert(f);
  }
  deserialize_values(value, &fields_set, &result);
  return DB::kOK;
}

int RocksdbDB::Scan(const std::string &table, const std::string &key,
                    int len, const std::vector<std::string> *fields,
                    std::vector<std::vector<KVPair>> &result) {
  if (column_families_map_.count(table) == 0 && create_columnfamily(table) != 0)
    return DB::kError;
  rocksdb::ColumnFamilyHandle* cfh;
  {
    ReadLock lock(&cf_lock_);
    int idx = column_families_map_[table];
    cfh = column_families_handles_[idx];
  }
  rocksdb::Iterator* it = rocksdb_->NewIterator(rocksdb::ReadOptions(), cfh);
  int iterations = 0;
  it->Seek(key);
  std::unordered_set<std::string> fields_set;
  if (fields) {
    for (const std::string& f : *fields)
      fields_set.insert(f);
  }
  while (it->Valid() && iterations < len) {
    std::vector<KVPair> r;
    deserialize_values(std::string(it->value().data(), it->value().size()), &fields_set, &r);
    result.push_back(std::move(r));
    it->Next();
    iterations++;
  }
  delete it;
  return DB::kOK;
}

int RocksdbDB::Update(const std::string &table, const std::string &key,
           std::vector<KVPair> &values) {
  if (column_families_map_.count(table) == 0 && create_columnfamily(table) != 0)
    return DB::kError;
  rocksdb::ColumnFamilyHandle* cfh;
  {
    ReadLock lock(&cf_lock_);
    int idx = column_families_map_[table];
    cfh = column_families_handles_[idx];
  }
  std::unordered_map<std::string, std::string> r;
  std::string value;
  rocksdb::Status s = rocksdb_->Get(rocksdb::ReadOptions(), cfh, key, &value);
  if (s.IsNotFound())
    return DB::kNotFound;
  deserialize_values(value, nullptr, &r);

  // update.
  for (auto& p : values)
    r[p.first] = p.second;

  // store.
  s = rocksdb_->Put(rocksdb::WriteOptions(), cfh, key, serialize_values(r));
  if (!s.ok())
    return DB::kError;
  return DB::kOK;
}

int RocksdbDB::Insert(const std::string &table, const std::string &key,
           std::vector<KVPair> &values) {
  if (column_families_map_.count(table) == 0 && create_columnfamily(table) != 0)
    return DB::kError;
  rocksdb::ColumnFamilyHandle* cfh;
  {
    ReadLock lock(&cf_lock_);
    int idx = column_families_map_[table];
    cfh = column_families_handles_[idx];
  }
  rocksdb::Status s = rocksdb_->Put(rocksdb::WriteOptions(), cfh, key, serialize_values(values));
  if (!s.ok())
    return DB::kError;
  return DB::kOK;
}

int RocksdbDB::Delete(const std::string &table, const std::string &key) {
  if (column_families_map_.count(table) == 0 && create_columnfamily(table) != 0)
    return DB::kError;
  rocksdb::ColumnFamilyHandle* cfh;
  {
    ReadLock lock(&cf_lock_);
    int idx = column_families_map_[table];
    cfh = column_families_handles_[idx];
  }
  rocksdb::Status s = rocksdb_->Delete(rocksdb::WriteOptions(), cfh, key);
  if (!s.ok())
    return DB::kError;
  return DB::kOK;
}

std::string RocksdbDB::serialize_values(const std::unordered_map<std::string, std::string>& values) {
  std::string str;
  char temp[4];
  for (auto& it : values) {
    utils::encode_int(temp, it.first.size());
    str.push_back(temp[0]);
    str.push_back(temp[1]);
    str.push_back(temp[2]);
    str.push_back(temp[3]);
    str.append(it.first);

    utils::encode_int(temp, it.second.size());
    str.push_back(temp[0]);
    str.push_back(temp[1]);
    str.push_back(temp[2]);
    str.push_back(temp[3]);
    str.append(it.second);
  }
  return str;
}

std::string RocksdbDB::serialize_values(const std::vector<KVPair>& values) {
  std::string str;
  char temp[4];
  for (auto& it : values) {
    utils::encode_int(temp, it.first.size());
    str.push_back(temp[0]);
    str.push_back(temp[1]);
    str.push_back(temp[2]);
    str.push_back(temp[3]);
    str.append(it.first);

    utils::encode_int(temp, it.second.size());
    str.push_back(temp[0]);
    str.push_back(temp[1]);
    str.push_back(temp[2]);
    str.push_back(temp[3]);
    str.append(it.second);
  }
  return str;
}

void RocksdbDB::deserialize_values(
    const std::string& values,
    const std::unordered_set<std::string>* fields,
    std::unordered_map<std::string, std::string>* result) {
  int off = 0;
  while (off < (int)(values.size())) {
    int key_len = utils::decode_int(values.c_str() + off);
    off += 4;
    std::string key = values.substr(off, key_len);
    off += key_len;

    int value_len = utils::decode_int(values.c_str() + off);
    off += 4;
    std::string value = values.substr(off, value_len);
    off += value_len;

    if (fields == nullptr || fields->count(key) > 0)
      result->emplace(key, value);
  }
}

void RocksdbDB::deserialize_values(
        const std::string& values,
        const std::unordered_set<std::string>* fields,
        std::vector<KVPair>* result) {
  int off = 0;
  while (off < (int)(values.size())) {
    int key_len = utils::decode_int(values.c_str() + off);
    off += 4;
    std::string key = values.substr(off, key_len);
    off += key_len;

    int value_len = utils::decode_int(values.c_str() + off);
    off += 4;
    std::string value = values.substr(off, value_len);
    off += value_len;

    if (fields == nullptr || fields->count(key) > 0)
      result->emplace_back(key, value);
  }
}

int RocksdbDB::create_columnfamily(const std::string& name) {
  WriteLock lock(&cf_lock_);
  if (column_families_map_.count(name) == 0) {
    rocksdb::ColumnFamilyOptions cfo;
    cfo.OptimizeLevelStyleCompaction();
    rocksdb::ColumnFamilyHandle* cfh;
    rocksdb::Status s = rocksdb_->CreateColumnFamily(cfo, name, &cfh);
    rocksdb::ColumnFamilyDescriptor cfd;
    if (s.ok() && cfh->GetDescriptor(&cfd).ok()) {
      column_families_map_.emplace(name, column_families_.size());
      column_families_.push_back(cfd);
      column_families_handles_.push_back(cfh);
    }
    else
      return -1;
  }
  return 0;
}

} // namespace ycsbc.