FILE(GLOB ycsb_core_files "*.c")
FILE(GLOB ycsb_core_headers "*.h")

add_library(ycsb_core_static STATIC ${ycsb_core_files} ${ycsb_core_headers})