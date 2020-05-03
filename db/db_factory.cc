//
//  basic_db.cc
//  YCSB-C
//
//  Created by Jinglei Ren on 12/17/14.
//  Copyright (c) 2014 Jinglei Ren <jinglei@ren.systems>.
//

#include "core/db_wrapper.h"
#include "core/measurements.h"
#include "core/properties.h"
#include "db/db_factory.h"

#include <string>
#include "db/basic_db.h"
#include "db/lock_stl_db.h"
#include "db/redis_db.h"
#include "db/rocksdb_db.h"
#include "db/tbb_rand_db.h"
#include "db/tbb_scan_db.h"

using namespace std;
using ycsbc::DB;
using ycsbc::DBFactory;

DB* DBFactory::CreateDB(utils::Properties &props) {
  utils::Properties p;
  p.SetProperty("measurement.histogram.verbose", "true");
  p.SetProperty("hdrhistogram.fileoutput", "true");
  p.SetProperty("hdrhistogram.output.path", "./");
  Measurements::set_properties(p);
  if (props["dbname"] == "basic") {
    return new BasicDB;
  } else if (props["dbname"] == "lock_stl") {
    return new LockStlDB;
  } else if (props["dbname"] == "redis") {
    int port = stoi(props["port"]);
    int slaves = stoi(props["slaves"]);
    return new DBWrapper(std::shared_ptr<DB>(new RedisDB(props["host"].c_str(), port, slaves)));
    // return new RedisDB(props["host"].c_str(), port, slaves);
  } else if (props["dbname"] == "rocksdb") {
    rocksdb::Options options;
    options.OptimizeLevelStyleCompaction();
    options.create_if_missing = true;
    return new DBWrapper(std::shared_ptr<DB>(new RocksdbDB(options, "/tmp/YCSB-C_rocksdb/")));
  } else if (props["dbname"] == "tbb_rand") {
    return new TbbRandDB;
  } else if (props["dbname"] == "tbb_scan") {
    return new TbbScanDB;
  } else return NULL;
}

