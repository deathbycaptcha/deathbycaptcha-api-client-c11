# Build Guide

This project uses CMake as its only supported build system.

## Requirements

- CMake 3.21 or newer
- A C11-capable compiler
- Ninja recommended
- libcurl recommended (enables HTTPS transport)

Platform notes:

- Linux: GCC or Clang
- macOS: Xcode Command Line Tools or LLVM/Clang
- Windows: MSVC with Ninja, or another supported CMake toolchain

## Development Build

```bash
cmake --preset default
cmake --build --preset default
```

To explicitly disable HTTPS support (socket-only build):

```bash
cmake --preset default -DDBC_ENABLE_HTTPS=OFF
cmake --build --preset default
```

Runtime note: the library uses socket transport by default; callers must opt in
to HTTPS explicitly with `dbc_set_transport(client, DBC_TRANSPORT_HTTPS)`.

Artifacts are generated under:

```text
build/default/bin/
build/default/lib/
```

## Release Build

```bash
cmake --preset release
cmake --build --preset release
```

Artifacts are generated under:

```text
build/release/bin/
build/release/lib/
```

## Windows MSVC Build

```powershell
cmake --preset windows-msvc
cmake --build --preset windows-msvc
```

Artifacts are generated under:

```text
build/windows-msvc/bin/
build/windows-msvc/lib/
```

## Main Outputs

- Shared library
  - Linux: `libdeathbycaptcha.so`
  - macOS: `libdeathbycaptcha.dylib`
  - Windows: `deathbycaptcha.dll`
- CLI executable
  - Linux/macOS: `deathbycaptcha`
  - Windows: `deathbycaptcha.exe`

CLI build and usage instructions are documented in `doc/CLI.md`.

## Example Loader

Dynamic loading samples are available at:

- `doc/lib_usage_socket_example.c`
- `doc/lib_usage_https_example.c`

Both are built by default in the CMake workflow. See `doc/EXAMPLES.md` for
runtime usage instructions.

## Unit Tests

Unit tests are enabled by default (`DBC_BUILD_TESTS=ON`) and built as
`deathbycaptcha_tests`.

Build tests:

```bash
cmake --preset default
cmake --build --preset default --target deathbycaptcha_tests
```

Run tests:

```bash
ctest --test-dir build/default --output-on-failure
```

## Live Integration Tests (CLI)

Integration tests use real DeathByCaptcha credentials from a local `.env` file and
exercise both transports (`--socket` and `--https`) using the CLI (which uses the
library internally).

Current coverage scope:
- `--socket`: balance + image solve + reCAPTCHA v2 solve.
- `--https`: balance + reCAPTCHA v2 solve.

Important:
- HTTPS must be available in the build. If libcurl is missing, this test is
  skipped with: `integration test skipped: HTTPS transport unavailable in current build`.
- For visible progress and skip reason details, run CTest in verbose mode (`-V`).

Required `.env` variables:

```bash
DBC_USERNAME=your_username
DBC_PASSWORD=your_password
```

Build with integration tests enabled:

```bash
rm -rf build/integration
cmake -S . -B build/integration -G Ninja -DCMAKE_BUILD_TYPE=Debug -DDBC_BUILD_TESTS=ON -DDBC_BUILD_INTEGRATION_TESTS=ON -DDBC_ENABLE_HTTPS=ON
cmake --build build/integration --target deathbycaptcha
```

On Linux, install libcurl development headers first if needed:

```bash
# debian based
sudo apt-get update && sudo apt-get install -y libcurl4-openssl-dev

# RHEL based
sudo dnf install -y libcurl-devel
```

Run only the integration test:

```bash
ctest --test-dir build/integration -V -R deathbycaptcha_cli_integration
```

## Coverage Check

For coverage-oriented verification (main library + socket/http harness + CLI harness):

```bash
rm -rf build/coverage
cmake -S . -B build/coverage -G Ninja -DCMAKE_BUILD_TYPE=Debug -DDBC_BUILD_TESTS=ON -DDBC_ENABLE_COVERAGE=ON -DDBC_ENABLE_HTTPS=OFF
cmake --build build/coverage
ctest --test-dir build/coverage --output-on-failure
```

`ctest` only runs the tests; it does not print a coverage percentage. To see the
coverage summary after tests pass, run (requires `gcovr`):

```bash
gcovr --root . --filter 'src/' --exclude 'src/cJSON' build/coverage --print-summary
```

Notes:
- Pass `build/coverage` as a positional argument, not `--object-directory`.
  This scopes the search to only that build directory and avoids picking up stale
  `.gcno` files from other build trees.
- `src/cJSON.c` is vendored third-party code and is excluded from the report.
- The `rm -rf` above is required — stale `.gcda` files from a previous build
  will cause a stamp-mismatch error.

## GitHub Release Binaries (Linux, macOS, and Windows)

The workflow `.github/workflows/release-cli.yml` builds CLI binaries for:

- Linux x86_64
- macOS x86_64
- Windows x86_64

and uploads them as assets to the GitHub Release matching the pushed tag.

Trigger it by creating and pushing a version tag:

```bash
git tag v4.7.0
git push origin v4.7.0
```

Expected release assets:

- `deathbycaptcha-linux-x86_64`
- `deathbycaptcha-macos-x86_64`
- `deathbycaptcha-windows-x86_64.exe`

## Coverage Badge (No External Service)

Workflow `.github/workflows/coverage-badge.yml` runs coverage and publishes
`coverage.svg` to the repository `badges` branch at `.github/badges/coverage.svg`.

Design constraints satisfied:

- No external badge provider is used.
- No bot commits are pushed back to `master`.
- Coverage percentage is rendered directly into an SVG generated in CI.