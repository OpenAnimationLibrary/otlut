# LUT Format Compatibility

This document records the output requirements otlut should follow, with special attention to the LUT dialects OpenToonz actually accepts.

## OpenToonz source of truth

OpenToonz parses monitor/color-calibration LUT files in:

`toonz/sources/toonzqt/lutcalibrator.cpp`

The current parser accepts `.cube` and `.3dl` files. otlut compatibility tests should track this parser so changes are caught rather than silently producing incompatible files.

## .cube

OpenToonz accepts 3D `.cube` LUTs with `LUT_3D_SIZE`, optional `TITLE`, `DOMAIN_MIN` / `DOMAIN_MAX`, or `LUT_3D_INPUT_RANGE`.

OpenToonz rejects 1D, shaper and 2D `.cube` LUTs in this path. Grid size must currently be between 2 and 129 inclusive.

For otlut, normal output should use a three-channel domain of 0..1 unless the conversion explicitly requires another domain.

Example:

```text
TITLE "otlut generated LUT"
LUT_3D_SIZE 33
DOMAIN_MIN 0.0 0.0 0.0
DOMAIN_MAX 1.0 1.0 1.0
```

A writer test must verify exact RGB entry ordering against OpenToonz.

## .3dl

OpenToonz expects a textual 3DMESH form. The first non-comment line must be:

```text
3DMESH
```

The next data line must be:

```text
Mesh <inputBitDepth> <outputBitDepth>
```

OpenToonz calculates:

`meshSize = (1 << inputBitDepth) + 1`

Current parser constraints:

- input-bit-depth field: 0 through 7
- resulting mesh: no more than 129 points per axis
- output-bit-depth field: 1 through 30
- one input-grid line containing exactly `meshSize` fields
- exactly `meshSize^3` RGB entries after the grid
- RGB entries are integers

### Useful mesh sizes

| inputBitDepth | meshSize |
|---:|---:|
| 0 | 2 |
| 1 | 3 |
| 2 | 5 |
| 3 | 9 |
| 4 | 17 |
| 5 | 33 |
| 6 | 65 |
| 7 | 129 |

otlut's public UI/CLI should expose mesh size rather than requiring users to reason about the OpenToonz header field.

For example, `--format 3dl --size 33` maps to:

```text
3DMESH
Mesh 5 <outputBitDepth>
```

### Output quantization

OpenToonz normalizes each stored integer using:

```text
maxValue = 2^outputBitDepth - 1
normalized = storedInteger / maxValue
```

Therefore otlut must quantize fitted floating-point RGB values consistently into that integer range.

The default output bit depth should be chosen after round-trip tests and should remain configurable through an advanced option if needed.

### Input grid

The current OpenToonz parser validates the number of fields in the input-grid line but does not use those values when constructing the LUT texture.

otlut should nevertheless emit a valid monotonically increasing grid so generated `.3dl` files remain sensible outside OpenToonz. The exact convention should be fixed by compatibility tests against representative `.3dl` readers before the format is declared stable.

### Entry ordering

OpenToonz reads `.3dl` entries under nested R/G/B loops and rearranges them into its internal texture layout. Entry ordering is therefore a critical compatibility item.

Required test:

1. generate an identity `.3dl`
2. load it in OpenToonz
3. verify neutral colors, primaries, secondaries and ramps remain unchanged
4. generate a deliberately asymmetric channel transform
5. verify no axis/channel swaps occur

## Cross-format equivalence

For any LUT representable in both formats, `.cube` and `.3dl` exports should produce equivalent output within `.3dl` quantization error.

Recommended deterministic test vectors include black, white, 18% gray, primaries, secondaries, RGB ramps, random RGB samples and values near lattice boundaries.

## Compatibility policy

`.cube` is otlut's primary interchange format.

`.3dl` support means specifically:

> otlut can write a 3DMESH `.3dl` LUT that OpenToonz accepts and applies with the intended transform.

It does not initially imply compatibility with every historical or vendor-specific `.3dl` variant.
