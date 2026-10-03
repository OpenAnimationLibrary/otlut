#include "lut_fit.h"

#include <cmath>
#include <stdexcept>
#include <string>

namespace {

void expectFit(bool condition, const std::string& message) {
  if (!condition) throw std::runtime_error(message);
}

void testIdentityPairFit() {
  otlut::Image source;
  source.width = 2;
  source.height = 2;
  source.rgb = {
      0.0f, 0.0f, 0.0f,
      1.0f, 0.0f, 0.0f,
      0.0f, 1.0f, 0.0f,
      1.0f, 1.0f, 1.0f,
  };
  const otlut::Image target = source;

  otlut::FitReport report;
  const otlut::Lut3D lut =
      otlut::fitLutFromImagePair(source, target, 3, &report);

  expectFit(lut.size == 3, "fit LUT size");
  expectFit(report.pixelsExamined == 4, "fit pixel count");
  expectFit(report.directlySampledCells > 0, "fit coverage");
}

void testMismatchedDimensions() {
  otlut::Image a;
  a.width = 1;
  a.height = 1;
  a.rgb = {0.0f, 0.0f, 0.0f};

  otlut::Image b = a;
  b.width = 2;

  bool threw = false;
  try {
    (void)otlut::fitLutFromImagePair(a, b, 3);
  } catch (const std::invalid_argument&) {
    threw = true;
  }
  expectFit(threw, "mismatched dimensions should fail");
}

}  // namespace

void runFitTests() {
  testIdentityPairFit();
  testMismatchedDimensions();
}
