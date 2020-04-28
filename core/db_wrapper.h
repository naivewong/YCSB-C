#ifndef _CORE_DB_WRAPPER_H_
#define _CORE_DB_WRAPPER_H_

#include <memory>

#include "core/db.h"
#include "core/measurements.h"

namespace ycsbc {

class DBWrapper: public DB {
public:
  DBWrapper(const std::shared_ptr<DB>& db): db_(db) {}
  void set_properties(const utils::Properties& props) { db_->set_properties(props); }
  utils::Properties get_properties() { return db_->get_properties(); }

  void Init() { db_->Init(); }
  void Close() {
    uint64_t ist = Measurements::get_measurements().get_intended_start_time_ns();
    uint64_t st = std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::system_clock::now().time_since_epoch()).count();
    db_->Close();
    uint64_t en = std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::system_clock::now().time_since_epoch()).count();
    measure("CLEANUP", 0, ist, st, en);
  }

  virtual int Read(const std::string &table, const std::string &key,
                   const std::vector<std::string> *fields,
                   std::vector<KVPair> &result) override {
    uint64_t ist = Measurements::get_measurements().get_intended_start_time_ns();
    uint64_t st = std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::system_clock::now().time_since_epoch()).count();
    int res = db_->Read(table, key, fields, result);
    uint64_t en = std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::system_clock::now().time_since_epoch()).count();
    measure("READ", res, ist, st, en);
    Measurements::get_measurements().report_status("READ", res);
    return res;
  }

  virtual int Scan(const std::string &table, const std::string &key,
                   int record_count, const std::vector<std::string> *fields,
                   std::vector<std::vector<KVPair>> &result) override {
    uint64_t ist = Measurements::get_measurements().get_intended_start_time_ns();
    uint64_t st = std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::system_clock::now().time_since_epoch()).count();
    int res = db_->Scan(table, key, record_count, fields, result);
    uint64_t en = std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::system_clock::now().time_since_epoch()).count();
    measure("SCAN", res, ist, st, en);
    Measurements::get_measurements().report_status("SCAN", res);
    return res;
  }

  virtual int Update(const std::string &table, const std::string &key,
                     std::vector<KVPair> &values) override {
    uint64_t ist = Measurements::get_measurements().get_intended_start_time_ns();
    uint64_t st = std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::system_clock::now().time_since_epoch()).count();
    int res = db_->Update(table, key, values);
    uint64_t en = std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::system_clock::now().time_since_epoch()).count();
    measure("UPDATE", res, ist, st, en);
    Measurements::get_measurements().report_status("UPDATE", res);
    return res;
  }

  virtual int Insert(const std::string &table, const std::string &key,
                     std::vector<KVPair> &values) override {
    uint64_t ist = Measurements::get_measurements().get_intended_start_time_ns();
    uint64_t st = std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::system_clock::now().time_since_epoch()).count();
    int res = db_->Insert(table, key, values);
    uint64_t en = std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::system_clock::now().time_since_epoch()).count();
    measure("INSERT", res, ist, st, en);
    Measurements::get_measurements().report_status("INSERT", res);
    return res;
  }

  virtual int Delete(const std::string &table, const std::string &key) override {
    uint64_t ist = Measurements::get_measurements().get_intended_start_time_ns();
    uint64_t st = std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::system_clock::now().time_since_epoch()).count();
    int res = db_->Delete(table, key);
    uint64_t en = std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::system_clock::now().time_since_epoch()).count();
    measure("DELETE", res, ist, st, en);
    Measurements::get_measurements().report_status("DELETE", res);
    return res;
  }

private:
  std::shared_ptr<DB> db_;

  void measure(const std::string& op, int status, uint64_t intended_start_time_ns,
        uint64_t start_time_ns, uint64_t end_time_ns) {
    if (status != 0) {
      Measurements::get_measurements().measure(op + "-FAILED",
          (end_time_ns - start_time_ns) / 1000);
      Measurements::get_measurements().measure_intended(op + "-FAILED",
          (end_time_ns - intended_start_time_ns) / 1000);
    }
    else {
      Measurements::get_measurements().measure(op, (end_time_ns - start_time_ns) / 1000);
      Measurements::get_measurements().measure_intended(op, (end_time_ns - intended_start_time_ns) / 1000);
    }
  }
};

}

#endif