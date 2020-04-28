#include <iostream>

#include "core/measurements.h"
#include "core/properties.h"

using namespace ycsbc;

void test_OneMeasurementRaw() {
  utils::Properties p;
  p.SetProperty("measurement.raw.output_file", "measurement_raw_output");
  OneMeasurementRaw m("test_OneMeasurementRaw", p);
  TextMeasurementsExporter exporter;

  for (int i = 0; i < 100; i++)
    m.measure(i);
  std::cout << "OneMeasurementRaw get_summary" << std::endl;
  std::cout << m.get_summary() << std::endl << std::endl;

  m.export_measurements(&exporter);
  // std::cout << exporter.buf() << std::endl;
}

void test_OneMeasurementHistogram() {
  utils::Properties p;
  p.SetProperty("measurement.histogram.verbose", "true");
  OneMeasurementHistogram m("test_OneMeasurementHistogram", p);
  TextMeasurementsExporter exporter;

  for (int i = 0; i < 100; i++)
    m.measure(i);
  std::cout << "OneMeasurementHistogram get_summary" << std::endl;
  std::cout << m.get_summary() << std::endl << std::endl;

  m.export_measurements(&exporter);
  // std::cout << exporter.buf() << std::endl;
}

void test_OneMeasurementHdrHistogram() {
  utils::Properties p;
  p.SetProperty("measurement.histogram.verbose", "true");
  p.SetProperty("hdrhistogram.fileoutput", "true");
  p.SetProperty("hdrhistogram.output.path", "./");
  OneMeasurementHdrHistogram m("test_OneMeasurementHdrHistogram", p);
  TextMeasurementsExporter exporter;

  for (int i = 0; i < 100; i++)
    m.measure(i);
  std::cout << "OneMeasurementHdrHistogram get_summary" << std::endl;
  std::cout << m.get_summary() << std::endl << std::endl;

  m.export_measurements(&exporter);
  // std::cout << exporter.buf() << std::endl;
}

void test_Measurements() {
  utils::Properties p;
  p.SetProperty("measurement.histogram.verbose", "true");
  p.SetProperty("hdrhistogram.fileoutput", "true");
  p.SetProperty("hdrhistogram.output.path", "./");

  Measurements::set_properties(p);
  Measurements& m = Measurements::get_measurements();

  TextMeasurementsExporter exporter;

  for (int i = 0; i < 100; i++)
    m.measure("alec", i);
  std::cout << "Measurements get_summary" << std::endl;
  std::cout << m.get_summary() << std::endl << std::endl;

  m.export_measurements(&exporter);
  // std::cout << exporter.buf() << std::endl;
}

int main() {
  // test_OneMeasurementRaw();
  // test_OneMeasurementHistogram();
  // test_OneMeasurementHdrHistogram();
  test_Measurements();
}