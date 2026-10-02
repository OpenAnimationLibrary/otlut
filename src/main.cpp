#include "lut_writer.h"

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

void printUsage(std::ostream& output) {
  output
      << "otlut - OpenToonz-oriented LUT generator\n\n"
      << "Stage 1 supports deterministic identity LUT generation.\n\n"
      << "Usage:\n"
      << "  otlut --identity --output <file.cube|file.3dl> [options]\n\n"
      << "Options:\n"
      << "  --size <n>              LUT grid size (default: 33)\n"
      << "  --output-bit-depth <n>  .3dl integer output depth (default: 12)\n"
      << "  --title <text>          LUT title/comment metadata\n"
      << "  --help                   Show this help\n\n"
      << ".3dl sizes compatible with OpenToonz: 2, 3, 5, 9, 17, 33, 65, 129\n";
}

int parseInt(const std::string& text, const std::string& option) {
  std::size_t used = 0;
  int value = 0;
  try {
    value = std::stoi(text, &used);
  } catch (...) {
    throw std::invalid_argument(option + " requires an integer.");
  }
  if (used != text.size()) {
    throw std::invalid_argument(option + " requires an integer.");
  }
  return value;
}

}  // namespace

int main(int argc, char* argv[]) {
  try {
    bool identity = false;
    std::string outputPath;
    otlut::IdentityLutOptions options;

    for (int i = 1; i < argc; ++i) {
      const std::string arg = argv[i];
      if (arg == "--help" || arg == "-h") {
        printUsage(std::cout);
        return 0;
      }
      if (arg == "--identity") {
        identity = true;
        continue;
      }

      auto requireValue = [&](const std::string& name) -> std::string {
        if (i + 1 >= argc) {
          throw std::invalid_argument(name + " requires a value.");
        }
        return argv[++i];
      };

      if (arg == "--output" || arg == "-o") {
        outputPath = requireValue(arg);
      } else if (arg == "--size") {
        options.size = parseInt(requireValue(arg), arg);
      } else if (arg == "--output-bit-depth") {
        options.outputBitDepth = parseInt(requireValue(arg), arg);
      } else if (arg == "--title") {
        options.title = requireValue(arg);
      } else {
        throw std::invalid_argument("Unknown option: " + arg);
      }
    }

    if (!identity) {
      throw std::invalid_argument(
          "No conversion mode selected. Stage 1 requires --identity.");
    }
    if (outputPath.empty()) {
      throw std::invalid_argument("--output is required.");
    }

    const otlut::LutFormat format = otlut::formatFromPath(outputPath);
    if (format == otlut::LutFormat::Cube &&
        !otlut::isSupportedCubeSize(options.size)) {
      throw std::invalid_argument(".cube size must be from 2 to 129.");
    }
    if (format == otlut::LutFormat::ThreeDl &&
        !otlut::isSupportedThreeDlSize(options.size)) {
      throw std::invalid_argument(
          ".3dl size must be one of 2, 3, 5, 9, 17, 33, 65, or 129.");
    }

    std::ofstream output(outputPath, std::ios::out | std::ios::trunc);
    if (!output) {
      throw std::runtime_error("Could not open output file: " + outputPath);
    }

    if (format == otlut::LutFormat::Cube) {
      otlut::writeIdentityCube(output, options);
    } else {
      otlut::writeIdentityThreeDl(output, options);
    }

    if (!output) {
      throw std::runtime_error("Failed while writing output file.");
    }

    std::cout << "Wrote identity LUT: " << outputPath << "\n"
              << "Grid size: " << options.size << "\n";
    if (format == otlut::LutFormat::ThreeDl) {
      std::cout << "3DL output bit depth: " << options.outputBitDepth << "\n";
    }
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "otlut: " << e.what() << "\n";
    std::cerr << "Use --help for usage.\n";
    return 2;
  }
}
