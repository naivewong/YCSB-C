include ../../make_config.mk

ifndef DISABLE_JEMALLOC
	ifdef JEMALLOC
		PLATFORM_CXXFLAGS += "-DROCKSDB_JEMALLOC"
	endif
	EXEC_LDFLAGS := $(JEMALLOC_LIB) $(EXEC_LDFLAGS) -lpthread
	PLATFORM_CXXFLAGS += $(JEMALLOC_INCLUDE)
endif

.PHONY: clean librocksdb

all: cloud_durable_example clone_example cloud_dump

cloud_durable_example: librocksdb cloud_durable_example.cc
	@echo "----------------------------------------"
	@echo "OPT", $(OPT)
	@echo "----------------------------------------"
	@echo "CXXFLAGS", $(CXXFLAGS)
	@echo "----------------------------------------"
	@echo "PLATFORM_CXXFLAGS", $(PLATFORM_CXXFLAGS)
	@echo "----------------------------------------"
	@echo "PLATFORM_LDFLAGS", $(PLATFORM_LDFLAGS)
	@echo "----------------------------------------"
	@echo "EXEC_LDFLAGS", $(EXEC_LDFLAGS) 
	@echo "----------------------------------------"
	$(CXX) $(CXXFLAGS) $@.cc -o $@ ../../librocksdb.a -I../../include $(OPT) -std=c++11 $(PLATFORM_LDFLAGS) $(PLATFORM_CXXFLAGS) $(EXEC_LDFLAGS)

clone_example: librocksdb clone_example.cc
	$(CXX) $(CXXFLAGS) $@.cc -o $@ ../../librocksdb.a -I../../include $(OPT) -std=c++11 $(PLATFORM_LDFLAGS) $(PLATFORM_CXXFLAGS) $(EXEC_LDFLAGS)

cloud_dump: librocksdb cloud_dump.cc
	$(CXX) $(CXXFLAGS) $@.cc -o $@ ../../librocksdb.a -I../../include $(OPT) -std=c++11 $(PLATFORM_LDFLAGS) $(PLATFORM_CXXFLAGS) $(EXEC_LDFLAGS)

clean:
	rm -rf ./cloud_durable_example  ./clone_example ./cloud_dump

librocksdb:
	cd ../.. && $(MAKE) static_lib
