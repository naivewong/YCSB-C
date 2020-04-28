//
//  db.h
//  YCSB-C
//
//  Created by Jinglei Ren on 12/10/14.
//  Copyright (c) 2014 Jinglei Ren <jinglei@ren.systems>.
//

#ifndef YCSB_C_DB_H_
#define YCSB_C_DB_H_

#include <vector>
#include <string>

#include "core/properties.h"

namespace ycsbc {

class DB {
 public:
  typedef std::pair<std::string, std::string> KVPair;
  static const int kOK = 0;
  static const int kErrorNoData = 1;
  static const int kErrorConflict = 2;
  // Added by Alec: keep consistent with YCSB.
  static const int kError = 3;               // The operation failed.
  static const int kNotFound = 4;            // The requested record was not found.
  static const int kNotImplemented = 5;
  static const int kUnexpectedState = 6;     // The operation reported success, but the result was not as expected.
  static const int kBadRequest = 7;          // The request was not valid.
  static const int kForbidden = 8;
  static const int kServiceUnavailable = 10; // Dependant service for the current binding is not available.
  static const int kBatchedOK = 11;          // The operation has been batched by the binding to be executed later.

  utils::Properties props_;
  void set_properties(const utils::Properties& props) { props_ = props; }
  utils::Properties get_properties() { return props_; }
  ///
  /// Initializes any state for accessing this DB.
  /// Called once per DB client (thread); there is a single DB instance globally.
  ///
  virtual void Init() { }
  ///
  /// Clears any state for accessing this DB.
  /// Called once per DB client (thread); there is a single DB instance globally.
  ///
  virtual void Close() { }
  ///
  /// Reads a record from the database.
  /// Field/value pairs from the result are stored in a vector.
  ///
  /// @param table The name of the table.
  /// @param key The key of the record to read.
  /// @param fields The list of fields to read, or NULL for all of them.
  /// @param result A vector of field/value pairs for the result.
  /// @return Zero on success, or a non-zero error code on error/record-miss.
  ///
  virtual int Read(const std::string &table, const std::string &key,
                   const std::vector<std::string> *fields,
                   std::vector<KVPair> &result) = 0;
  ///
  /// Performs a range scan for a set of records in the database.
  /// Field/value pairs from the result are stored in a vector.
  ///
  /// @param table The name of the table.
  /// @param key The key of the first record to read.
  /// @param record_count The number of records to read.
  /// @param fields The list of fields to read, or NULL for all of them.
  /// @param result A vector of vector, where each vector contains field/value
  ///        pairs for one record
  /// @return Zero on success, or a non-zero error code on error.
  ///
  virtual int Scan(const std::string &table, const std::string &key,
                   int record_count, const std::vector<std::string> *fields,
                   std::vector<std::vector<KVPair>> &result) = 0;
  ///
  /// Updates a record in the database.
  /// Field/value pairs in the specified vector are written to the record,
  /// overwriting any existing values with the same field names.
  ///
  /// @param table The name of the table.
  /// @param key The key of the record to write.
  /// @param values A vector of field/value pairs to update in the record.
  /// @return Zero on success, a non-zero error code on error.
  ///
  virtual int Update(const std::string &table, const std::string &key,
                     std::vector<KVPair> &values) = 0;
  ///
  /// Inserts a record into the database.
  /// Field/value pairs in the specified vector are written into the record.
  ///
  /// @param table The name of the table.
  /// @param key The key of the record to insert.
  /// @param values A vector of field/value pairs to insert in the record.
  /// @return Zero on success, a non-zero error code on error.
  ///
  virtual int Insert(const std::string &table, const std::string &key,
                     std::vector<KVPair> &values) = 0;
  ///
  /// Deletes a record from the database.
  ///
  /// @param table The name of the table.
  /// @param key The key of the record to delete.
  /// @return Zero on success, a non-zero error code on error.
  ///
  virtual int Delete(const std::string &table, const std::string &key) = 0;
  
  virtual ~DB() { }
};

} // ycsbc

#endif // YCSB_C_DB_H_