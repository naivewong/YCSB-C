#include "core/measurements.h"

namespace ycsbc {

OneMeasurementRaw::OneMeasurementRaw(const std::string& name, const utils::Properties& props): OneMeasurement(name) {
  std::string output_filepath = props.GetProperty(OUTPUT_FILE_PATH, OUTPUT_FILE_PATH_DEFAULT);
  if (!output_filepath.empty()) {
    std::cout << "Raw data measurement: will output to result file: " << output_filepath << std::endl;
    f_ = std::ofstream(output_filepath);
    if (!f_.is_open())
      throw "Failed to open raw data output file";
  }
  else
    std::cout << "Raw data measurement: will output to stdout." << std::endl;
}

void OneMeasurementRaw::measure(int latency) {
  total_latency += latency;
  window_total_latency_ += latency;
  window_operations_++;

  measurements_.emplace_back(latency);
}

void OneMeasurementRaw::export_measurements(MeasurementsExporter* exporter) {
  if (f_.is_open()) {
    f_ << get_name() << " latency raw data: op, timestamp(ms), latency(us)" << std::endl;
    for (auto& point : measurements_)
      f_ << get_name() << "," << point.timestamp() << "," << point.value() << std::endl;
    f_.close();
  }
  else {
    std::cout << get_name() << " latency raw data: op, timestamp(ms), latency(us)" << std::endl;
    for (auto& point : measurements_)
      std::cout << get_name() << "," << point.timestamp() << "," << point.value() << std::endl;
  }

  int total_ops = measurements_.size();
  exporter->write(get_name(), "Total Opeartions", total_ops);
  if (total_ops > 0 && !no_summary_stats_) {
    exporter->write(get_name(), "Below is a summary of latency in microseconds:", -1);
    exporter->write(get_name(), "Average", (double)(total_latency) / (double)(total_ops));

    std::sort(measurements_.begin(), measurements_.end(),
        [](const RawDataPoint& l, const RawDataPoint& r) -> bool {
          return l.value_ < r.value_;
        });

    exporter->write(get_name(), "Min", measurements_.front().value());
    exporter->write(get_name(), "Max", measurements_.back().value());
    exporter->write(get_name(), "p1", measurements_[(int)((double)(total_ops) * 0.01)].value());
    exporter->write(get_name(), "p5", measurements_[(int)((double)(total_ops) * 0.05)].value());
    exporter->write(get_name(), "p50", measurements_[(int)((double)(total_ops) * 0.5)].value());
    exporter->write(get_name(), "p90", measurements_[(int)((double)(total_ops) * 0.9)].value());
    exporter->write(get_name(), "p95", measurements_[(int)((double)(total_ops) * 0.95)].value());
    exporter->write(get_name(), "p99", measurements_[(int)((double)(total_ops) * 0.99)].value());
    exporter->write(get_name(), "p99.9", measurements_[(int)((double)(total_ops) * 0.999)].value());
    exporter->write(get_name(), "p99.99", measurements_[(int)((double)(total_ops) * 0.9999)].value());
  }

  export_status_counts(exporter);
}

std::string OneMeasurementRaw::get_summary() {
  if (window_operations_ == 0)
    return "";

  char buf[100];
  sprintf(buf, "%s count: %d, average latency(us): %.2f",
      get_name().c_str(),
      window_operations_,
      (double)(window_total_latency_) / (double)(window_operations_));
  window_operations_ = 0;
  window_total_latency_ = 0;
  std::string s = buf;
  return s;
}

OneMeasurementHistogram::OneMeasurementHistogram(const std::string& name, const utils::Properties& props):
    OneMeasurement(name), histogram_overflow_(0), operations_(0),
    total_latency_(0), total_squared_latency_(0),
    window_operations_(0), window_total_latency_(0),
    min_(-1), max_(-1) {
  buckets_ = stoi(props.GetProperty(BUCKETS, BUCKETS_DEFAULT));
  verbose_ = false;
  if (props.GetProperty(VERBOSE_PROPERTY, "false") == "true")
    verbose_ = true;
  histogram_ = std::vector<uint64_t>(buckets_);
}

void OneMeasurementHistogram::measure(int latency) {
  if (latency / 1000 >= buckets_)
    histogram_overflow_++;
  else
    histogram_[latency / 1000]++;
  operations_++;
  total_latency_ += latency;
  total_squared_latency_ += (double)(latency) * (double)(latency);
  window_operations_++;
  window_total_latency_ += latency;

  if (min_ < 0 || latency < min_)
    min_ = latency;
  if (max_ < 0 || latency > max_)
    max_ = latency;
}

void OneMeasurementHistogram::export_measurements(MeasurementsExporter* exporter) {
  double mean = total_latency_ / ((double)(operations_));
  double variance = total_squared_latency_ / ((double)(operations_)) - (mean * mean);

  exporter->write(get_name(), "Operations", operations_);
  exporter->write(get_name(), "AverageLatency(us)", mean);
  exporter->write(get_name(), "LatencyVariance(us)", variance);
  exporter->write(get_name(), "MinLatency(us)", min_);
  exporter->write(get_name(), "MaxLatency(us)", max_);

  uint64_t opcounter = 0;
  bool done_95th = false;
  for (int i = 0; i < buckets_; i++) {
    opcounter += histogram_[i];
    if (!done_95th && (double)(opcounter) / (double)(operations_) >= 0.95) {
      exporter->write(get_name(), "95thPercentileLatency(us)", i * 1000);
      done_95th = true;
    }
    if ((double)(opcounter) / (double)(operations_) >= 0.99) {
      exporter->write(get_name(), "99thPercentileLatency(us)", i * 1000);
      break;
    }
  }

  export_status_counts(exporter);

  if (verbose_) {
    for (int i = 0; i < buckets_; i++)
      exporter->write(get_name(), std::to_string(i), histogram_[i]);

    exporter->write(get_name(), ">" + std::to_string(buckets_), histogram_overflow_);
  }
}

std::string OneMeasurementHistogram::get_summary() {
  if (window_operations_ == 0)
    return "";

  char buf[100];
  sprintf(buf, "%s count: %lu, average latency(us): %.2f",
      get_name().c_str(),
      window_operations_,
      (double)(window_total_latency_) / (double)(window_operations_));
  window_operations_ = 0;
  window_total_latency_ = 0;
  std::string s = buf;
  return s;
}

OneMeasurementHdrHistogram::OneMeasurementHdrHistogram(const std::string& name, const utils::Properties& props):
    OneMeasurement(name), log_(nullptr), total_histogram_(nullptr) {
  percentiles_ = get_percentile_values(props.GetProperty(PERCENTILES_PROPERTY, PERCENTILES_PROPERTY_DEFAULT));
  verbose_ = false;
  if (props.GetProperty(VERBOSE_PROPERTY, "false") == "true")
    verbose_ = true;
  bool should_log = false;
  if (props.GetProperty("hdrhistogram.fileoutput", "false") == "true")
    should_log = true;
  if (should_log) {
    std::string hdr_output_filename = props.GetProperty("hdrhistogram.output.path", "") + name + ".hdr";
    log_ = fopen(hdr_output_filename.c_str(), "w+");

    hdr_log_writer_init(&histogram_log_writer_);
    std::string header("Logging for: " + name);
    struct timeval tp;
    gettimeofday(&tp, NULL);
    base_time_.tv_sec = tp.tv_sec;
    base_time_.tv_nsec = tp.tv_usec;
    hdr_log_write_header(&histogram_log_writer_, log_, header.c_str(), &base_time_);

    hdr_interval_recorder_init_all(&histogram_, 1, 1000000, 3);
  }
}

OneMeasurementHdrHistogram::~OneMeasurementHdrHistogram() {
  if (log_)
    fclose(log_);
  if (&histogram_)
    hdr_interval_recorder_destroy(&histogram_);
  if (total_histogram_)
    hdr_close(total_histogram_);
}

void OneMeasurementHdrHistogram::measure(int latency) {
  hdr_interval_recorder_record_value_atomic(&histogram_, latency);
}

void OneMeasurementHdrHistogram::export_measurements(MeasurementsExporter* exporter) {
  struct hdr_histogram* interval_histogram = get_interval_histogram_and_accumulate();
  if (log_) {
    struct timeval tp;
    gettimeofday(&tp, NULL);
    hdr_timespec now;
    now.tv_sec = tp.tv_sec;
    now.tv_nsec = tp.tv_usec;
    // TODO(alec), start time and end time.
    hdr_log_write(&histogram_log_writer_, log_, &base_time_, &now, interval_histogram);
  }

  exporter->write(get_name(), "Operations", (uint64_t)(total_histogram_->total_count));
  exporter->write(get_name(), "AverageLatency(us)", hdr_mean(total_histogram_));
  exporter->write(get_name(), "MinLatency(us)", (uint64_t)(hdr_min(total_histogram_)));
  exporter->write(get_name(), "MaxLatency(us)", (uint64_t)(hdr_max(total_histogram_)));

  for (double percentile : percentiles_)
    exporter->write(get_name(), ordinal(percentile) + "PercentileLatency(us)",
        (uint64_t)(hdr_value_at_percentile(total_histogram_, percentile)));

  export_status_counts(exporter);

  if (verbose_) {
    struct hdr_iter it;
    hdr_iter_recorded_init(&it, total_histogram_);
    while (hdr_iter_next(&it)) {
      exporter->write(get_name(), std::to_string(it.value), (double)(it.count));
    } 
  }
}

std::string OneMeasurementHdrHistogram::get_summary() {
  struct hdr_histogram* interval_histogram = get_interval_histogram_and_accumulate();
  if (log_) {
    struct timeval tp;
    gettimeofday(&tp, NULL);
    hdr_timespec now;
    now.tv_sec = tp.tv_sec;
    now.tv_nsec = tp.tv_usec;
    // TODO(alec), start time and end time.
    hdr_log_write(&histogram_log_writer_, log_, &base_time_, &now, interval_histogram);
  }

  std::stringstream ss(std::stringstream::in | std::stringstream::out);
  ss << std::setprecision(2) << "[" << get_name() << ": Count=" << interval_histogram->total_count
     << ", Max=" << hdr_max(interval_histogram) << ", Min=" << hdr_min(interval_histogram)
     << ", Avg=" << hdr_mean(interval_histogram)
     << ", 90=" << hdr_value_at_percentile(interval_histogram, 90)
     << ", 99=" << hdr_value_at_percentile(interval_histogram, 99)
     << ", 99.9=" << hdr_value_at_percentile(interval_histogram, 99.9)
     << ", 99.99=" << hdr_value_at_percentile(interval_histogram, 99.99) << "]";
  return ss.str();
}

std::vector<double> OneMeasurementHdrHistogram::get_percentile_values(const std::string& s) {
  std::vector<double> vals;
  size_t pos1 = 0;
  size_t pos2 = 0;
  std::string token;
  std::string delimiter = ",";
  while ((pos2 = s.find(delimiter, pos1)) != std::string::npos) {
    token = s.substr(pos1, pos2 - pos1);
    vals.push_back(std::stod(token));
    pos1 = pos2 + delimiter.length();
  }
  if (pos1 != s.size())
    vals.push_back(std::stod(s.substr(pos1, s.size() - pos1)));
  return vals;
}

struct hdr_histogram* OneMeasurementHdrHistogram::get_interval_histogram_and_accumulate() {
  struct hdr_histogram* interval_histogram = hdr_interval_recorder_sample(&histogram_);
  if (!total_histogram_)
    total_histogram_ = copy(interval_histogram);
  else
    hdr_add(total_histogram_, interval_histogram);
  return interval_histogram;
}

std::string OneMeasurementHdrHistogram::ordinal(double i) {
  std::string suffixes[] = {"th", "st", "nd", "rd", "th", "th", "th", "th", "th", "th"};
  if ((double)((int)(i)) == i) {
    switch ((int)(i) % 100) {
    case 11:
    case 12:
    case 13:
      return std::to_string((int)(i)) + "th";
    default:
      return std::to_string((int)(i)) + suffixes[(int)(i) % 10];
    }
  }
  else
    return std::to_string(i);
}

hdr_histogram* OneMeasurementHdrHistogram::copy(hdr_histogram* hdr) {
  hdr_histogram* histogram = (struct hdr_histogram*) calloc(1, sizeof(struct hdr_histogram));
  histogram->lowest_trackable_value = hdr->lowest_trackable_value;
  histogram->highest_trackable_value = hdr->highest_trackable_value;
  histogram->unit_magnitude = hdr->unit_magnitude;
  histogram->significant_figures = hdr->significant_figures;
  histogram->sub_bucket_half_count_magnitude = hdr->sub_bucket_half_count_magnitude;
  histogram->sub_bucket_half_count = hdr->sub_bucket_half_count;
  histogram->sub_bucket_mask = hdr->sub_bucket_mask;
  histogram->sub_bucket_count = hdr->sub_bucket_count;
  histogram->bucket_count = hdr->bucket_count;
  histogram->min_value = hdr->min_value;
  histogram->max_value = hdr->max_value;
  histogram->normalizing_index_offset = hdr->normalizing_index_offset;
  histogram->conversion_ratio = hdr->conversion_ratio;
  histogram->counts_len = hdr->counts_len;
  histogram->total_count = hdr->total_count;
  int64_t* counts = (int64_t*) calloc((size_t) hdr->counts_len, sizeof(int64_t));
  histogram->counts = counts;
  memcpy(histogram->counts, hdr->counts, (size_t) hdr->counts_len * sizeof(int64_t));
  return histogram;
}

thread_local Measurements::StartTimerHolder Measurements::intended_start_time_;
utils::Properties Measurements::props_;

}