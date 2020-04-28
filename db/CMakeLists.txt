FILE(GLOB ycsb_db_files "*.c")
FILE(GLOB ycsb_db_headers "*.h")

add_library(ycsb_db_static STATIC ${ycsb_db_files} ${ycsb_db_headers})