#include "lut_fit.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <vector>

namespace otlut {
namespace {

size_t index3d(int r, int g, int b, int size) {
  return (static_cast<size_t>(r) * size * size +
          static_cast<size_t>(g) * size + b);
}

}  // namespace

double FitReport::coveragePercent() const {
  if (totalCells == 0) return 0.0;
  return 100.0 * static_cast<double>(directlySampledCells) /
         static_cast<double>(totalCells);
}

Lut3D makeIdentityLut(int size) {
  if (size < 2 || size > 129) {
    throw std::invalid_argument("LUT size must be between 2 and 129.");
  }

  Lut3D lut;
  lut.size = size;
  lut.rgb.resize(static_cast<size_t>(size) * size * size * 3);

  const float denom = static_cast<float>(size - 1);
  for (int r = 0; r < size; ++r) {
    for (int g = 0; g < size; ++g) {
      for (int b = 0; b < size; ++b) {
        const size_t cell = index3d(r, g, b, size);
        lut.rgb[cell * 3 + 0] = r / denom;
        lut.rgb[cell * 3 + 1] = g / denom;
        lut.rgb[cell * 3 + 2] = b / denom;
      }
    }
  }
  return lut;
}

Lut3D fitLutFromImagePair(const Image& source, const Image& target, int size,
                          FitReport* report) {
  if (source.width != target.width || source.height != target.height) {
    throw std::invalid_argument(
        "Source and target images must have identical dimensions.");
  }
  if (source.width <= 0 || source.height <= 0 ||
      source.rgb.size() != target.rgb.size()) {
    throw std::invalid_argument("Source and target images are invalid.");
  }

  Lut3D lut = makeIdentityLut(size);
  const size_t cells = static_cast<size_t>(size) * size * size;

  std::vector<double> sums(cells * 3, 0.0);
  std::vector<double> weights(cells, 0.0);

  const float scale = static_cast<float>(size - 1);
  const size_t pixelCount =
      static_cast<size_t>(source.width) * source.height;

  for (size_t p = 0; p < pixelCount; ++p) {
    const float sr = std::clamp(source.rgb[p * 3 + 0], 0.0f, 1.0f);
    const float sg = std::clamp(source.rgb[p * 3 + 1], 0.0f, 1.0f);
    const float sb = std::clamp(source.rgb[p * 3 + 2], 0.0f, 1.0f);

    const float tr = std::clamp(target.rgb[p * 3 + 0], 0.0f, 1.0f);
    const float tg = std::clamp(target.rgb[p * 3 + 1], 0.0f, 1.0f);
    const float tb = std::clamp(target.rgb[p * 3 + 2], 0.0f, 1.0f);

    const float fr = sr * scale;
    const float fg = sg * scale;
    const float fb = sb * scale;

    const int r0 = static_cast<int>(std::floor(fr));
    const int g0 = static_cast<int>(std::floor(fg));
    const int b0 = static_cast<int>(std::floor(fb));
    const int r1 = std::min(r0 + 1, size - 1);
    const int g1 = std::min(g0 + 1, size - 1);
    const int b1 = std::min(b0 + 1, size - 1);

    const float ar = fr - r0;
    const float ag = fg - g0;
    const float ab = fb - b0;

    for (int ri = 0; ri < 2; ++ri) {
      const int r = ri ? r1 : r0;
      const double wr = ri ? ar : (1.0 - ar);
      for (int gi = 0; gi < 2; ++gi) {
        const int g = gi ? g1 : g0;
        const double wg = gi ? ag : (1.0 - ag);
        for (int bi = 0; bi < 2; ++bi) {
          const int b = bi ? b1 : b0;
          const double wb = bi ? ab : (1.0 - ab);
          const double w = wr * wg * wb;
          if (w <= 0.0) continue;

          const size_t cell = index3d(r, g, b, size);
          weights[cell] += w;
          sums[cell * 3 + 0] += tr * w;
          sums[cell * 3 + 1] += tg * w;
          sums[cell * 3 + 2] += tb * w;
        }
      }
    }
  }

  size_t sampled = 0;
  for (size_t cell = 0; cell < cells; ++cell) {
    if (weights[cell] <= 0.0) continue;
    ++sampled;
    lut.rgb[cell * 3 + 0] =
        static_cast<float>(sums[cell * 3 + 0] / weights[cell]);
    lut.rgb[cell * 3 + 1] =
        static_cast<float>(sums[cell * 3 + 1] / weights[cell]);
    lut.rgb[cell * 3 + 2] =
        static_cast<float>(sums[cell * 3 + 2] / weights[cell]);
  }

  if (report) {
    report->pixelsExamined = pixelCount;
    report->directlySampledCells = sampled;
    report->totalCells = cells;
  }

  return lut;
}

}  // namespace otlut
