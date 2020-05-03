#/bin/bash

trap 'kill $(jobs -p)' SIGINT

workloads="./workloads/workloada.spec ./workloads/workloadb.spec ./workloads/workloadd.spec ./workloads/workloadf.spec"

for file_name in $workloads; do
  echo "Running Rocksdb with for $file_name"
  ./ycsbc -db rocksdb -threads 1 -P $file_name
done

