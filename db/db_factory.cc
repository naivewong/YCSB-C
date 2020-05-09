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
#include "db/rocksdb_cloud_db.h"
// #include "db/tbb_rand_db.h"
// #include "db/tbb_scan_db.h"
#include "rocksdb-cloud/include/rocksdb/cloud/cloud_env_options.h"

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
  } else if (props["dbname"] == "rocksdb-cloud") {
    std::string dbpath = "/tmp/YCSB-C_rocksdb-cloud";
    std::string region = "ap-northeast-1";
    std::string bucket_prefix = "rockset.";
    std::string bucket_suffix = "cloud-db-examples.alec";

    rocksdb::CloudEnvOptions cloud_env_options;
    std::unique_ptr<rocksdb::CloudEnv> cloud_env;
    cloud_env_options.src_bucket.SetBucketName(bucket_suffix,bucket_prefix);
    cloud_env_options.dest_bucket.SetBucketName(bucket_suffix,bucket_prefix);
    cloud_env_options.keep_local_sst_files = true;
    rocksdb::CloudEnv* cenv;
    const std::string bucketName = bucket_suffix + bucket_prefix;
    rocksdb::Status s = rocksdb::CloudEnv::NewAwsEnv(rocksdb::Env::Default(),
          bucket_suffix, dbpath, region,
          bucket_suffix, dbpath, region,
          cloud_env_options, nullptr, &cenv);
          // NewLRUCache(256*1024*1024));
    if (!s.ok()) {
      printf("Error open AwsEnv: %s\n", s.ToString().c_str());
      exit(-1);
    }
    cloud_env.reset(cenv);

    rocksdb::Options options;
    options.create_if_missing = true;
    options.compaction_style = rocksdb::kCompactionStyleLevel;
    options.write_buffer_size = 110 << 10;  // 110KB
    options.arena_block_size = 4 << 10;
    options.level0_file_num_compaction_trigger = 2;
    options.max_bytes_for_level_base = 100 << 10; // 100KB
    options.env = cloud_env.get();
    return new DBWrapper(std::shared_ptr<DB>(new RocksdbCloudDB(options, "/tmp/YCSB-C_rocksdb-cloud", std::move(cloud_env))));
  // } else if (props["dbname"] == "tbb_rand") {
  //   return new TbbRandDB;
  // } else if (props["dbname"] == "tbb_scan") {
  //   return new TbbScanDB;
  } else return NULL;
}

