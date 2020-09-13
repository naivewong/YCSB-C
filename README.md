# YCSB-C

Yahoo! Cloud Serving Benchmark in C++, a C++ version of YCSB (https://github.com/brianfrankcooper/YCSB/wiki)

## New Feature
Add load -
`$ ./ycsbc load -db rocksdb -threads 1 -P ./workloads/workloada.spec`
Add HDR histogram. For example -
```
# Loading records:	100000
[READ: Count=100000, Max=68, Min=1, Avg=4, 90=7, 99=13, 99.9=19, 99.99=38] [UPDATE: Count=49826, Max=8391, Min=8, Avg=13, 90=16, 99=26, 99.9=38, 99.99=66] [INSERT: Count=100000, Max=504, Min=5, Avg=9.2, 90=13, 99=21, 99.9=36, 99.99=66] [CLEANUP: Count=2, Max=0, Min=0, Avg=0, 90=0, 99=0, 99.9=0, 99.99=0] 
[OVERALL], RunTime(ms), 1367.115696
[OVERALL], Throughput(ops/sec), 73146.698771
[READ], Operations, 100000
[READ], AverageLatency(us), 3.969580
[READ], MinLatency(us), 1
[READ], MaxLatency(us), 68
[READ], 95thPercentileLatency(us), 9
[READ], 99thPercentileLatency(us), 13
[READ], Return=, 100000
[READ], 1, 4489.000000
[READ], 2, 32119.000000
[READ], 3, 18996.000000
[READ], 4, 13066.000000
.......
```

## RocksDB
Folder rocksdb-cloud contains a RocksDB v6.5.2. If you want to test the latest RocksDB, you can replace the folder with the latest RocksDB.  
Before compiling rocksdb-cloud, set the following environment variables -
```
$ export USE_AWS=1
$ export USE_RTTI=1
```
After that, execute the following commands -
```
$ cd rocksdb-cloud
$ make -j8 static-lib
$ cd ..
$ make ycsbc
```
Then, you can run the script `run_rocksdb.sh`.