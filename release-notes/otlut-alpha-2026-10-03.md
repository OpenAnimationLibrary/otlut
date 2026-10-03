# OTLUT Alpha - 2026-10-03

This alpha adds the paired-image LUT workflow required by the OpenToonz integration.

## New CLI support

```text
otlut --source original.png --target graded.png --output look.cube --size 33
```

Also supported:

```text
otlut --source original.png --target graded.png --output look.3dl --size 33 --output-bit-depth 12
```

## Included

- `--source` and `--target` paired-image fitting
- PNG/JPEG/BMP/TGA/PSD/GIF/HDR/PIC/PNM loading
- whole-lattice interpolation/filling for unsupported LUT regions
- direct vs inferred coverage reporting
- .cube and OpenToonz-compatible .3dl output
- Windows/Linux/macOS-tested core changes

## Important

This release supersedes `otlut-alpha-2026-10-02` for OpenToonz integration testing. The earlier executable only supported `--identity` and will fail with an "Unknown option: --source" error.
