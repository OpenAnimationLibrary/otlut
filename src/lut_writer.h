#pragma once

#include <iosfwd>
#include <string>

namespace otlut {

enum class LutFormat {
  Cube,
  ThreeDl,
};

struct IdentityLutOptions {
  int size = 33;
  int outputBitDepth = 12;
  std::string title = "otlut identity LUT";
};

bool isSupportedCubeSize(int size);
bool isSupportedThreeDlSize(int size);
int threeDlInputBitDepthForSize(int size);

void writeIdentityCube(std::ostream& output, const IdentityLutOptions& options);
void writeIdentityThreeDl(std::ostream& output,
                          const IdentityLutOptions& options);

LutFormat formatFromPath(const std::string& path);

}  // namespace otlut
