# Examples Guide

This project provides two dynamic loading samples:

- `doc/lib_usage_socket_example.c`
- `doc/lib_usage_https_example.c`

Both load symbols from the built shared library at runtime.

## Prerequisites

- Build the project first.
- Place the shared library and sample executable in the same runtime folder.

Recommended build:

```bash
cmake --preset default
cmake --build --preset default
```

On Linux/macOS artifacts are under `build/default/bin` and `build/default/lib`.

## Sample 1: Socket API

Source: `doc/lib_usage_socket_example.c`

Behavior:

- Initializes a client.
- Explicitly selects socket transport with `dbc_set_transport(..., DBC_TRANSPORT_SOCKET)`.
- Supports image mode and token modes for reCAPTCHA v2/v3/Enterprise.

Run from build output (Linux/macOS):

```bash
cd build/default/bin
LD_LIBRARY_PATH=../lib ./deathbycaptcha_socket_example USERNAME PASSWORD captcha1.jpg
```

macOS variant:

```bash
cd build/default/bin
DYLD_LIBRARY_PATH=../lib ./deathbycaptcha_socket_example USERNAME PASSWORD captcha1.jpg
```

Token examples (socket):

```bash
LD_LIBRARY_PATH=../lib ./deathbycaptcha_socket_example USERNAME PASSWORD --rc-v2 --sitekey SITEKEY --pageurl https://example.com/form
LD_LIBRARY_PATH=../lib ./deathbycaptcha_socket_example USERNAME PASSWORD --rc-v3 --sitekey SITEKEY --pageurl https://example.com/login --action login --min-score 0.3
LD_LIBRARY_PATH=../lib ./deathbycaptcha_socket_example USERNAME PASSWORD --rc-enterprise --sitekey SITEKEY --pageurl https://example.com/enterprise
```

## Sample 2: HTTPS API

Source: `doc/lib_usage_https_example.c`

Behavior:

- Initializes a client.
- Explicitly selects HTTPS transport with `dbc_set_transport(..., DBC_TRANSPORT_HTTPS)`.
- Supports image mode and token modes for reCAPTCHA v2/v3/Enterprise.

Requirements:

- Build must include libcurl support (`DBC_ENABLE_HTTPS=ON` and `libcurl` found by CMake).

Run from build output (Linux/macOS):

```bash
cd build/default/bin
LD_LIBRARY_PATH=../lib ./deathbycaptcha_https_example USERNAME PASSWORD captcha1.jpg
```

macOS variant:

```bash
cd build/default/bin
DYLD_LIBRARY_PATH=../lib ./deathbycaptcha_https_example USERNAME PASSWORD captcha1.jpg
```

Token examples (https):

```bash
LD_LIBRARY_PATH=../lib ./deathbycaptcha_https_example USERNAME PASSWORD --rc-v2 --sitekey SITEKEY --pageurl https://example.com/form
LD_LIBRARY_PATH=../lib ./deathbycaptcha_https_example USERNAME PASSWORD --rc-v3 --sitekey SITEKEY --pageurl https://example.com/login --action login --min-score 0.3
LD_LIBRARY_PATH=../lib ./deathbycaptcha_https_example USERNAME PASSWORD --rc-enterprise --sitekey SITEKEY --pageurl https://example.com/enterprise
```

If HTTPS sample exits with an error about transport selection, rebuild with libcurl available to CMake.

## Windows Notes

- Executables are generated in `build/<preset>/bin`.
- Keep `deathbycaptcha.dll` in the same folder as the sample executable, or ensure it is on `PATH`.
- Run:

```powershell
.\deathbycaptcha_socket_example.exe USERNAME PASSWORD captcha1.jpg
.\deathbycaptcha_https_example.exe USERNAME PASSWORD captcha1.jpg
```
