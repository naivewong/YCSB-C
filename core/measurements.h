#ifndef YCSB_C_CORE_MEASUREMENT_H_
#define YCSB_C_CORE_MEASUREMENT_H_

#include <algorithm>
#include <atomic>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/time.h>
#include <unordered_map>
#include <vector>

#include "core/properties.h"
#include "lib/mutexlock.h"
#include "third-party/HdrHistogram_c/src/hdr_histogram.h"
#include "third-party/HdrHistogram_c/src/hdr_histogram_log.h"
#include "third-party/HdrHistogram_c/src/hdr_interval_recorder.h"

namespace ycsbc {

class MeasurementsExporter {
public:
  virtual void write(const std::string& metric, const std::string& measurement, int i)=0;
  virtual void write(const std::string& metric, const std::string& measurement, uint64_t i)=0;
  virtual void write(const std::string& metric, const std::string& measurement, double i)=0;

  virtual void print() {}
};

class TextMeasurementsExporter: public MeasurementsExporter {
public:
  virtual void write(const std::string& metric, const std::string& measurement, int i) override {
    buf_.append("[" + metric + "], " + measurement + ", " + std::to_string(i) + "\n");
  }
  virtual void write(const std::string& metric, const std::string& measurement, uint64_t i) override {
    buf_.append("[" + metric + "], " + measurement + ", " + std::to_string(i) + "\n");
  }
  virtual void write(const std::string& metric, const std::string& measurement, double i) override {
    buf_.append("[" + metric + "], " + measurement + ", " + std::to_string(i) + "\n");
  }

  virtual void print() override { std::cout << buf_ << std::endl; }

  std::string buf() { return buf_; }

private:
  std::string buf_;
};

class OneMeasurement {
public:
  OneMeasurement(const std::string& name): name_(name) {}

  std::string get_name() { return name_; }

  void report_status(int status) {
    WriteLock lock(&rwlock_);
    std::unordered_map<int, std::atomic<int>>::iterator counter = return_codes_.find(status);
    if (counter != return_codes_.end())
      counter->second.fetch_add(1);
    else
      return_codes_.emplace(status, 1);
  }

  void export_status_counts(MeasurementsExporter* exporter) {
    ReadLock lock(&rwlock_);
    for (auto& p : return_codes_)
      exporter->write(name_, "Return=" + std::to_string(p.first), p.second.load());
  }

  virtual void measure(int latency)=0;
  virtual std::string get_summary()=0;
  virtual void export_measurements(MeasurementsExporter* exporter)=0;

private:
  std::string name_;
  std::unordered_map<int, std::atomic<int>> return_codes_;
  mutable RWMutex rwlock_;
};

class OneMeasurementRaw: public OneMeasurement {
public:
  struct RawDataPoint {
    uint64_t timestamp_;
    int value_;
    uint64_t timestamp() {
      return timestamp_;
    }
    int value() {
      return value_;
    }
    RawDataPoint(int value):
        timestamp_(
          std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count()
        ), value_(value) {}
  };

  OneMeasurementRaw(const std::string& name, const utils::Properties& props);

  ~OneMeasurementRaw() {
    if (f_.is_open())
      f_.close();
  }

  virtual void measure(int latency) override;

  virtual void export_measurements(MeasurementsExporter* exporter) override;

  virtual std::string get_summary() override;

public:
  /**
   * Optionally, user can configure an output file to save the raw data points.
   * Default is none, raw results will be written to stdout.
   *
   */
  std::string OUTPUT_FILE_PATH = "measurement.raw.output_file";
  std::string OUTPUT_FILE_PATH_DEFAULT = "";

  /**
   * Optionally, user can request to not output summary stats. This is useful
   * if the user chains the raw measurement type behind the HdrHistogram type
   * which already outputs summary stats. But even in that case, the user may
   * still want this class to compute summary stats for them, especially if
   * they want accurate computation of percentiles (because percentils computed
   * by histogram classes are still approximations).
   */
  std::string NO_SUMMARY_STATS = "measurement.raw.no_summary";
  std::string NO_SUMMARY_STATS_DEFAULT = "false";

private:
  std::ofstream f_;
  bool no_summary_stats_ = false;
  std::vector<RawDataPoint> measurements_;
  uint64_t total_latency = 0;

  // A window of stats to print summary for at the next getSummary() call.
  // It's supposed to be a one line summary, so we will just print count and
  // average.
  int window_operations_ = 0;
  uint64_t window_total_latency_ = 0;
};

class OneMeasurementHistogram: public OneMeasurement {
public:
  OneMeasurementHistogram(const std::string& name, const utils::Properties& props);

  virtual void measure(int latency) override;

  virtual void export_measurements(MeasurementsExporter* exporter) override;

  virtual std::string get_summary() override;

public:
  std::string BUCKETS = "histogram.buckets";
  std::string BUCKETS_DEFAULT = "1000";
  std::string VERBOSE_PROPERTY = "measurement.histogram.verbose";

private:
  int buckets_;
  std::vector<uint64_t> histogram_;
  // Counts all operations outside the histogram's range.
  uint64_t histogram_overflow_;
  uint64_t operations_;
  uint64_t total_latency_;
  double total_squared_latency_;
  // Whether or not to emit the histogram buckets.
  bool verbose_;
  uint64_t window_operations_;
  uint64_t window_total_latency_;
  int min_;
  int max_;
};

class OneMeasurementHdrHistogram: public OneMeasurement {
public:
  OneMeasurementHdrHistogram(const std::string& name, const utils::Properties& props);

  ~OneMeasurementHdrHistogram();

  virtual void measure(int latency) override;

  virtual void export_measurements(MeasurementsExporter* exporter) override;

  virtual std::string get_summary() override;

private:
  std::vector<double> get_percentile_values(const std::string& s);

  struct hdr_histogram* get_interval_histogram_and_accumulate();

  std::string ordinal(double i);

  hdr_histogram* copy(hdr_histogram* hdr);

public:
  std::string PERCENTILES_PROPERTY = "hdrhistogram.percentiles";
  std::string PERCENTILES_PROPERTY_DEFAULT = "95,99";
  std::string VERBOSE_PROPERTY = "measurement.histogram.verbose";

private:
  FILE* log_;
  struct hdr_log_writer histogram_log_writer_;
  struct hdr_histogram* total_histogram_;
  struct hdr_interval_recorder histogram_;
  hdr_timespec base_time_;
  bool verbose_;
  std::vector<double> percentiles_;
};

class Measurements {
public:
  enum MeasurementType {
    HISTOGRAM,
    HDRHISTOGRAM,
    HDRHISTOGRAM_AND_HISTOGRAM,
    HDRHISTOGRAM_AND_RAW,
    TIMESERIES,
    RAW
  };

  std::string MEASUREMENT_TYPE_PROPERTY = "measurementtype";
  std::string MEASUREMENT_TYPE_PROPERTY_DEFAULT = "hdrhistogram";
  std::string MEASUREMENT_INTERVAL = "measurement.interval";
  std::string MEASUREMENT_INTERVAL_DEFAULT = "op";
  std::string MEASUREMENT_TRACK_JVM_PROPERTY = "measurement.trackjvm";
  std::string MEASUREMENT_TRACK_JVM_PROPERTY_DEFAULT = "false";

  static Measurements& get_measurements() { 
    static Measurements instance; // Guaranteed to be destroyed.
                                  // Instantiated on first use.
    return instance;
  }

  // Note: Scott Meyers mentions in his Effective Modern
  //       C++ book, that deleted functions should generally
  //       be public as it results in better error messages
  //       due to the compilers behavior to check accessibility
  //       before deleted status
  Measurements(Measurements const&) = delete;
  void operator=(Measurements const&) = delete;

  static void set_properties(const utils::Properties& props) { props_ = props; }
  void set_intended_start_time_ns(uint64_t time) {
    if (measurement_interval_ == 0)
      return;
    intended_start_time_.time_ = time;
  }
  uint64_t get_intended_start_time_ns() {
    if (measurement_interval_ == 0)
      return 0;
    return intended_start_time_.start_time();
  }

  void measure(const std::string& operation, int latency) {
    if (measurement_interval_ == 1)
      return;
    OneMeasurement* m = get_op_measurement(operation);
    m->measure(latency);
  }

  void measure_intended(const std::string& operation, int latency) {
    if (measurement_interval_ == 0)
      return;
    OneMeasurement* m = get_op_intended_measurement(operation);
    m->measure(latency);
  }

  void report_status(const std::string& operation, int status) {
    OneMeasurement* m = (measurement_interval_ == 1?
        get_op_intended_measurement(operation) : get_op_measurement(operation));
    m->report_status(status);
  }

  void export_measurements(MeasurementsExporter* exporter) {
    {
      ReadLock lock(&lock1_);
      for (auto& it : op_to_measurement_map_)
        it.second->export_measurements(exporter);
    }
    {
      ReadLock lock(&lock2_);
      for (auto& it : op_to_intended_measurement_map_)
        it.second->export_measurements(exporter);
    }
  }

  std::string get_summary() {
    std::string ret;
    {
      ReadLock lock(&lock1_);
      for (auto& it : op_to_measurement_map_)
        ret.append(it.second->get_summary() + " ");
    }
    {
      ReadLock lock(&lock2_);
      for (auto& it : op_to_intended_measurement_map_)
        ret.append(it.second->get_summary() + " ");
    }
    return ret;
  }

private:
  Measurements() {
    std::string type_string = props_.GetProperty(MEASUREMENT_TYPE_PROPERTY, MEASUREMENT_TYPE_PROPERTY_DEFAULT);
    if (type_string == "histogram")
      measurement_type_ = HISTOGRAM;
    else if (type_string == "hdrhistogram")
      measurement_type_ = HDRHISTOGRAM;
    else if (type_string == "hdrhistogram+istogram")
      measurement_type_ = HDRHISTOGRAM_AND_HISTOGRAM;
    else if (type_string == "hdrhistogram+aw")
      measurement_type_ = HDRHISTOGRAM_AND_RAW;
    else if (type_string == "timeseries")
      measurement_type_ = TIMESERIES;
    else if (type_string == "raw")
      measurement_type_ = RAW;

    std::string m_interval_string = props_.GetProperty(MEASUREMENT_INTERVAL, MEASUREMENT_INTERVAL_DEFAULT);
    if (m_interval_string == "op")
      measurement_interval_ = 0;
    else if (m_interval_string == "intended")
      measurement_interval_ = 1;
    else if (m_interval_string == "both")
      measurement_interval_ = 2;
  }

  std::unique_ptr<OneMeasurement> construct_onemeasurement(const std::string& name) {
    if (measurement_type_ == HISTOGRAM)
      return std::unique_ptr<OneMeasurement>(new OneMeasurementHistogram(name, props_));
    if (measurement_type_ == HDRHISTOGRAM)
      return std::unique_ptr<OneMeasurement>(new OneMeasurementHdrHistogram(name, props_));
    if (measurement_type_ == RAW)
      return std::unique_ptr<OneMeasurement>(new OneMeasurementRaw(name, props_));

    return nullptr;
  }

  OneMeasurement* get_op_measurement(const std::string& operation) {
    WriteLock lock(&lock1_);
    auto it = op_to_measurement_map_.find(operation);
    if (it == op_to_measurement_map_.end()) {
      op_to_measurement_map_[operation] = construct_onemeasurement(operation);
    }
    return op_to_measurement_map_[operation].get();
  }

  OneMeasurement* get_op_intended_measurement(const std::string& operation) {
    WriteLock lock(&lock2_);
    auto it = op_to_intended_measurement_map_.find(operation);
    if (it == op_to_intended_measurement_map_.end()) {
      std::string name = (measurement_interval_ == 1? operation : "Intended-" + operation);
      op_to_intended_measurement_map_[operation] = construct_onemeasurement(name);
    }
    return op_to_intended_measurement_map_[operation].get();
  }

  struct StartTimerHolder {
    uint64_t time_ = 0;

    uint64_t start_time() {
      if (time_)
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
      else
        return time_;
    }
  };

  thread_local static StartTimerHolder intended_start_time_;

  std::unordered_map<std::string, std::unique_ptr<OneMeasurement>> op_to_measurement_map_;
  mutable RWMutex lock1_;
  std::unordered_map<std::string, std::unique_ptr<OneMeasurement>> op_to_intended_measurement_map_;
  mutable RWMutex lock2_;
  MeasurementType measurement_type_;
  int measurement_interval_;
  static utils::Properties props_;

};

} // namespace ycsbc.

#endif