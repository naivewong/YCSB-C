#include <cstdio>
#include <string>

#include "rocksdb/db.h"
#include "rocksdb/slice.h"
#include "rocksdb/options.h"

using namespace rocksdb;

std::string kDBPath = "/tmp/rocksdb_scan_example";

int main() {
  DB* db;
  Options options;
  // Optimize RocksDB. This is the easiest way to get RocksDB to perform well
  options.IncreaseParallelism();
  options.OptimizeLevelStyleCompaction();
  // create the DB if it's not already present
  options.create_if_missing = true;

  // open DB
  Status s = DB::Open(options, kDBPath, &db);
  assert(s.ok());

  // Put key-value
  s = db->Put(WriteOptions(), "key1", "value1");
  assert(s.ok());
  s = db->Put(WriteOptions(), "key2", "value2");
  assert(s.ok());
  s = db->Put(WriteOptions(), "key3", "value3");
  assert(s.ok());
  s = db->Put(WriteOptions(), "key4", "value4");
  assert(s.ok());
  s = db->Put(WriteOptions(), "key5", "value5");
  assert(s.ok());
  s = db->Put(WriteOptions(), "key6", "value6");
  assert(s.ok());
  s = db->Put(WriteOptions(), "key7", "value7");
  assert(s.ok());
  s = db->Put(WriteOptions(), "key8", "value8");
  assert(s.ok());
  s = db->Put(WriteOptions(), "key9", "value9");
  assert(s.ok());
  s = db->Put(WriteOptions(), "key10", "value10");
  assert(s.ok());

  Iterator* it = db->NewIterator(ReadOptions(), db->DefaultColumnFamily());
  it->Seek("key1");
  while (it->Valid()) {
    printf("key:%s value:%s\n",
        std::string(it->key().data(), it->key().size()).c_str(),
        std::string(it->value().data(), it->value().size()).c_str());
    it->Next();
  }
  delete db;

  return 0;
}
