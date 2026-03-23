# Death By Captcha C Client

This repository contains a C client for the Death By Captcha API.
It provides:

- A reusable shared library with HTTPS support via libcurl (when available).
- A command-line client for balance checks, CAPTCHA upload, polling and report.
- reCAPTCHA v2, v3, and Enterprise token-solving support in library, CLI, and samples.
- Vendored dependencies required to build the client.

The project uses a modern multiplatform CMake workflow for Linux, macOS and
Windows.

## CI Status Badges

| Workflow | Status |
|---|---|
| CI (unit + integration) | [![CI](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/actions/workflows/ci.yml/badge.svg?branch=master)](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/actions/workflows/ci.yml) |
| Tests Linux | [![Tests Linux](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/actions/workflows/test-linux.yml/badge.svg?branch=master)](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/actions/workflows/test-linux.yml) |
| Tests Windows | [![Tests Windows](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/actions/workflows/test-windows.yml/badge.svg?branch=master)](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/actions/workflows/test-windows.yml) |
| Tests macOS | [![Tests macOS](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/actions/workflows/test-macos.yml/badge.svg?branch=master)](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/actions/workflows/test-macos.yml) |
| CI 32-bit | [![CI 32-bit](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/actions/workflows/ci-32bit.yml/badge.svg?branch=master)](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/actions/workflows/ci-32bit.yml) |
| Release CLI Binaries | [![Release CLI Binaries](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/actions/workflows/release-cli.yml/badge.svg)](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/actions/workflows/release-cli.yml) |
| Coverage | ![Coverage %](https://raw.githubusercontent.com/deathbycaptcha/deathbycaptcha-api-client-c11/badges/.github/badges/coverage.svg) |

Coverage badge notes:
- No external service is used.
- No workflow commits back to `master`.
- The SVG is published to the `badges` branch under `.github/badges/coverage.svg`.

## Project Layout

- `src/`: core library and CLI source code.
- `doc/`: build and usage guides, plus dynamic loading samples.
- `CMakeLists.txt`, `CMakePresets.json`: modern cross-platform build system.

## Build

Recommended development workflow:

```bash
cmake --preset default
cmake --build --preset default
```

Optimized build:

```bash
cmake --preset release
cmake --build --preset release
```

## Main Components

- Library public API: `src/deathbycaptcha.h`
- HTTPS and socket protocol implementation: `src/deathbycaptcha.c`
- CLI client: `src/client.c`
- Dynamic loading socket example: `doc/lib_usage_socket_example.c`
- Dynamic loading HTTPS example: `doc/lib_usage_https_example.c`

## Documentation

- [Build guide](doc/BUILD.md)
- [CLI guide](doc/CLI.md)
- [Library guide](doc/LIBRARY.md)
- [Examples guide](doc/EXAMPLES.md)
- [Socket usage sample](doc/lib_usage_socket_example.c)
- [HTTPS usage sample](doc/lib_usage_https_example.c)
- [Agent API metadata overview](https://github.com/deathbycaptcha/deathbycaptcha-agent-api-metadata)
- [Socket API specification](https://github.com/deathbycaptcha/deathbycaptcha-agent-api-metadata/blob/main/spec/openapi/sockets.yaml)
- [HTTP API specification](https://github.com/deathbycaptcha/deathbycaptcha-agent-api-metadata/blob/main/spec/openapi/http.yaml)

## Notes

- The library defaults to socket transport (`api.dbcapi.me:8123`).
- Transport is selected explicitly by the caller via `dbc_set_transport(...)`.
- HTTPS mode (`https://api.dbcapi.me/api`) requires a build with libcurl
  support.

## Responsible Use

See [Responsible Use Agreement](RESPONSIBLE_USE.md).
- The POSIX code path uses pthread mutexes for Linux and macOS portability.