# otlut Alpha - 2026-10-02

First public alpha of the standalone **otlut** command-line LUT utility.

## Included

- Windows x64 `otlut.exe`
- identity `.cube` LUT generation
- OpenToonz-compatible `3DMESH` `.3dl` LUT generation
- configurable LUT grid size
- configurable `.3dl` output bit depth
- sample 33 x 33 x 33 identity LUTs in both formats
- OpenToonz LUT format compatibility notes

## Example

```text
otlut --identity --output identity.cube --size 33
otlut --identity --output identity.3dl --size 33 --output-bit-depth 12
```

## OpenToonz .3dl sizes

`2, 3, 5, 9, 17, 33, 65, 129`

17, 33 and 65 are the main practical sizes targeted for testing.

## Current limitation

This alpha validates the executable and LUT writers. It does **not yet derive LUTs from image pairs or image sequences**. Paired-image fitting is the next development stage.

Use this alpha for LUT writer and OpenToonz compatibility testing rather than production color work.
