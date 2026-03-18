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
| CI 32-bit | [![CI 32-bit](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/actions/workflows/ci-32bit.yml/badge.svg?branch=master)](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/actions/workflows/ci-32bit.yml) |
| Release CLI Binaries | [![Release CLI Binaries](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/actions/workflows/release-cli.yml/badge.svg)](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/actions/workflows/release-cli.yml) |
| Coverage Badge Pipeline | [![Coverage Badge](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/actions/workflows/coverage-badge.yml/badge.svg?branch=master)](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/actions/workflows/coverage-badge.yml) |
| Coverage (%) | ![Coverage %](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/releases/download/badges/coverage.svg) |

Coverage badge notes:
- No external service is used.
- No workflow commits back to `master`.
- The SVG is uploaded as a release asset under tag `badges`.

## Project Layout

- `src/`: core library and CLI source code.
- `doc/`: build and usage guides, plus dynamic loading samples.
- `deathbycaptcha-agent-api-metadata/`: API metadata submodule with formal HTTP
  and socket specifications.
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
- [Agent API metadata overview](deathbycaptcha-agent-api-metadata/README.md)
- [Socket API specification](deathbycaptcha-agent-api-metadata/spec/openapi/sockets.yaml)
- [HTTP API specification](deathbycaptcha-agent-api-metadata/spec/openapi/http.yaml)

## Notes

- The library defaults to socket transport (`api.dbcapi.me:8123`).
- Transport is selected explicitly by the caller via `dbc_set_transport(...)`.
- HTTPS mode (`https://api.dbcapi.me/api`) requires a build with libcurl
  support.
- The POSIX code path uses pthread mutexes for Linux and macOS portability.