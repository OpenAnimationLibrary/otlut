# otlut

**otlut** is a standalone command-line LUT generator intended to complement OpenToonz.

The initial goal is to derive a 3D color transform from paired source/graded images or image sequences and write that transform as a LUT that OpenToonz and other color tools can use.

Primary output is **.cube**. OpenToonz-compatible **.3dl** output is also a first-class requirement.

## Goals

- Run independently from OpenToonz as a small command-line executable.
- Convert a paired source image and graded/reference image into a 3D LUT.
- Accumulate samples from paired image sequences to improve RGB-space coverage.
- Export `.cube` LUTs as the primary format.
- Export `.3dl` LUTs in the form accepted by OpenToonz.
- Report LUT sampling/coverage so users can judge how well the imagery supports the generated transform.
- Keep the OpenToonz integration boundary simple: arguments/files in, LUT file and status out.
- Later support deterministic LUT-reference images for round-tripping a look through other applications.

## Conversion model

A single finished image does not uniquely identify the color transform that produced it. The first implementation therefore focuses on workflows where the transform can be derived defensibly.

### Paired image

`source image + graded image -> RGB correspondences -> fitted 3D LUT`

Each corresponding pixel supplies a `source RGB -> graded RGB` sample. Samples are accumulated in RGB space, filtered, fitted to a regular 3D lattice and written as a LUT.

### Paired image sequence

`source sequence + graded sequence -> combined RGB sample cloud -> one 3D LUT`

Sequences are not averaged. Samples from all matched frames contribute to the same transform fit, providing better RGB-space coverage than a single frame.

### LUT reference image

A later mode should generate a deterministic LUT reference image. The user grades that image externally and gives the graded result back to otlut:

`generated reference -> external grade -> graded reference -> .cube/.3dl`

Because the original RGB values are known exactly, this can provide a reliable way to capture an externally-created look.

### Single-image look extraction

Estimating a LUT from only one already-graded image is inherently approximate because the original colors are unknown. This should be a later experimental feature and clearly distinguished from paired-image fitting.

## Proposed CLI

```text
otlut --source original.png --target graded.png --output look.cube
```

Sequence example:

```text
otlut --source "original/shot.####.exr" --target "graded/shot.####.exr" --range 1-48 --output look.cube --size 33
```

OpenToonz-compatible 3DL:

```text
otlut --source original.png --target graded.png --output look.3dl --format 3dl --size 33
```

Planned options include `--source`, `--target`, `--range`, `--output`, `--format cube|3dl`, `--size`, color-space options, alpha handling and report output. The file extension should normally imply output format.

## Fitting pipeline

```text
Decode source and target
  -> normalize/transform into a defined RGB working space
  -> match corresponding pixels
  -> reject invalid/transparent samples
  -> accumulate RGB correspondences
  -> fit the 3D lattice
  -> fill weakly sampled/unsampled cells
  -> regularize/smooth
  -> validate
  -> write .cube or OpenToonz-compatible .3dl
```

The first implementation should remain deterministic and should not require machine learning.

## .cube output - primary

`.cube` is the preferred output because it is human-readable, widely supported and accepted by OpenToonz.

Initial target sizes: 17, 33 and 65.

Normal output should use a defined three-channel input domain, usually 0..1. Writer tests must verify exact entry count and channel ordering against OpenToonz.

OpenToonz's current color-calibration parser accepts 3D `.cube` LUTs and rejects 1D, shaper and 2D LUTs in this path.

## .3dl output - OpenToonz compatible

OpenToonz currently expects the textual 3DMESH form:

```text
3DMESH
Mesh <input bit depth> <output bit depth>
<input grid values...>
<R G B>
<R G B>
...
```

OpenToonz computes the mesh size as:

`meshSize = (1 << inputBitDepth) + 1`

The current parser permits a maximum of 129 points per axis, making these useful compatible sizes:

| Mesh size | OpenToonz input-bit-depth field |
|---:|---:|
| 17 | 4 |
| 33 | 5 |
| 65 | 6 |
| 129 | 7 |

The output-bit-depth field is accepted from 1 through 30 and LUT entries are integer RGB triplets. otlut should expose mesh size to users while handling this OpenToonz-specific header mapping internally.

`.3dl` support means specifically that otlut writes a 3DMESH LUT that OpenToonz accepts and applies with the intended transform. It does not initially imply support for every vendor-specific historical `.3dl` dialect.

See `docs/FORMAT_COMPATIBILITY.md` for the compatibility contract.

## Coverage reporting

otlut should report how much of the LUT lattice was directly supported by imagery and how much required interpolation.

```text
Frames examined:        48
Samples examined:       3,482,917
Usable samples:         3,197,402
LUT resolution:         33 x 33 x 33
Directly sampled cells: 78.4%
Interpolated cells:     21.6%
```

Coverage is not a guarantee of accuracy, but it communicates how strongly the generated transform is constrained by the supplied imagery.

## OpenToonz integration

Preferred architecture:

```text
OpenToonz -> source/target paths + options -> otlut executable -> .cube/.3dl + report + exit status -> OpenToonz LUT workflow
```

Keeping the fitting engine outside `OpenToonz.exe` avoids coupling OpenToonz to analysis dependencies and allows otlut to evolve and be tested independently.

A future OpenToonz dialog can expose current frame, selected frames, entire level/sequence, source and graded inputs, LUT size, output format, output path and an option to load/apply the generated LUT.

## Development stages

### Stage 1 - standalone foundation
- command-line executable
- image loading
- paired single-image validation
- internal floating-point RGB representation
- `.cube` writer
- identity-LUT generation
- writer/parser tests

### Stage 2 - paired-image LUT fitting
- source/target correspondence sampling
- alpha handling and outlier rejection
- lattice fitting and unsampled-cell interpolation
- error metrics and coverage reporting

### Stage 3 - sequence fitting
- numbered-sequence discovery
- explicit frame ranges and missing-frame handling
- accumulated sample fitting
- reproducible sampling for large sequences

### Stage 4 - OpenToonz-compatible .3dl
- `.3dl` writer
- supported mesh-size enforcement
- integer output quantization
- OpenToonz parser/application tests
- `.cube` versus `.3dl` transform comparison tests

### Stage 5 - LUT reference workflow
- generate reference image
- ingest graded reference image
- reconstruct LUT deterministically
- `.cube` and `.3dl` export

### Stage 6 - OpenToonz integration
- external-tool configuration or bundled helper
- Create 3D LUT dialog
- progress/error reporting
- automatic LUT loading/application

### Later / experimental
- single-image look estimation
- additional LUT formats
- GPU-accelerated fitting
- OpenColorIO-aware workflows
- diagnostic visualizations

## Validation priorities

Tests should cover identity round-trip, known analytic transforms, `.cube` and `.3dl` equivalence, channel ordering, black/white endpoints, saturated primaries/secondaries, RGB ramps, sequence consistency, transparency and high-bit-depth/EXR inputs.

## Project principle

otlut should remain useful as an independent color utility even when OpenToonz is not installed. OpenToonz integration should invoke a stable documented CLI rather than depend on the converter's internal implementation.
