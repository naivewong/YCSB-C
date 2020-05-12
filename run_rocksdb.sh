#/bin/bash

trap 'kill $(jobs -p)' SIGINT

workloads="./workloads/workloada.spec ./workloads/workloadb.spec ./workloads/workloadd.spec ./workloads/workloadf.spec"

./ycsb load -db rocksdb -threads 1 -P ./workloads/workloada.spec
for file_name in $workloads; do
  echo "Running Rocksdb with for $file_name"
  ./ycsbc run -db rocksdb -threads 1 -P $file_name
done
