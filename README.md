# [DeathByCaptcha](https://deathbycaptcha.com/)


<p align="center">
  <a href="https://github.com/deathbycaptcha/deathbycaptcha-api-client-python"><img alt="Python" src="https://img.shields.io/badge/Python-3776AB?style=for-the-badge&logo=python&logoColor=white"></a>
  <a href="https://github.com/deathbycaptcha/deathbycaptcha-api-client-nodejs"><img alt="Node.js" src="https://img.shields.io/badge/Node.js-339933?style=for-the-badge&logo=nodedotjs&logoColor=white"></a>
  <a href="https://github.com/deathbycaptcha/deathbycaptcha-api-client-dotnet"><img alt=".NET" src="https://img.shields.io/badge/.NET-512BD4?style=for-the-badge&logo=dotnet&logoColor=white"></a>
  <a href="https://github.com/deathbycaptcha/deathbycaptcha-api-client-java"><img alt="Java" src="https://img.shields.io/badge/Java-ED8B00?style=for-the-badge&logo=openjdk&logoColor=white"></a>
  <a href="https://github.com/deathbycaptcha/deathbycaptcha-api-client-php"><img alt="PHP" src="https://img.shields.io/badge/PHP-777BB4?style=for-the-badge&logo=php&logoColor=white"></a>
  <a href="https://github.com/deathbycaptcha/deathbycaptcha-api-client-perl"><img alt="Perl" src="https://img.shields.io/badge/Perl-39457E?style=for-the-badge&logo=perl&logoColor=white"></a>
  <a href="https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11"><img alt="C" src="https://img.shields.io/badge/%3E-C-A8B9CC?style=for-the-badge&logo=c&logoColor=black&labelColor=555555"></a>
</p>


## 📖 Introduction

The [DeathByCaptcha](https://deathbycaptcha.com) C client is the official C11 library for the DeathByCaptcha **captcha solving service**. It provides a simple, well-documented interface for integrating CAPTCHA solving into automation workflows — a particularly common need when you use it as a **captcha solver for web scraping**, where CAPTCHAs block access to the pages you need to extract data from. It supports both the HTTPS API (encrypted transport via libcurl — recommended when security is a priority) and the socket-based API (faster and lower latency, recommended for high-throughput production workloads).

Key features:

- 🧩 Send image, audio and modern token-based CAPTCHA types (reCAPTCHA v2/v3, Turnstile, GeeTest, etc.).
- 🔄 Unified client API across HTTPS and socket transports — switching is a single `dbc_set_transport()` call.
- 🔐 Built-in support for proxies, timeouts and advanced token parameters for modern CAPTCHA flows.
- 🏗️ Multiplatform CMake build system for Linux, macOS and Windows.

Quick start example (socket transport):

```c
#include "deathbycaptcha.h"

dbc_client  client;
dbc_captcha captcha;

dbc_init(&client, "your_username", "your_password");

FILE *f = fopen("captcha.jpg", "rb");
if (dbc_decode_file(&client, &captcha, f, DBC_TIMEOUT) == 0)
    printf("Solved: %s\n", captcha.text);
fclose(f);

dbc_close(&client);
```

> **🚌 Transport options:** Use `DBC_TRANSPORT_HTTPS` for encrypted communication via libcurl — credentials and data travel over TLS. Use `DBC_TRANSPORT_SOCKET` (the default) for lower latency and higher throughput — it is faster but communicates over a plain TCP connection to `api.dbcapi.me` on port `8123`.

---

### Tests Status

[![CI](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/actions/workflows/ci.yml/badge.svg?branch=master)](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/actions/workflows/ci.yml)
[![Coverage](https://img.shields.io/endpoint?url=https://deathbycaptcha.github.io/deathbycaptcha-api-client-c11/coverage-badge.json)](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/actions/workflows/coverage.yml)

---

## 🗂️ Index

- [Installation](#installation)
- [How to Use DBC API Clients](#how-to-use-dbc-api-clients)
    - [Common Clients' Interface](#common-clients-interface)
    - [Available Methods](#captcha-methods)
- [Credentials & Configuration](#credentials--configuration)
    - [Quick Setup](#quick-setup)
- [CAPTCHA Types Quick Reference & Examples](#captcha-types-quick-reference--examples)
    - [Quick Start](#quick-start)
    - [Type Reference](#sample-index-by-captcha-type)
    - [Per-Type Code Snippets](#per-type-code-snippets)
- [CAPTCHA Types Extended Reference](#captcha-types-extended-reference)
    - [reCAPTCHA Image-Based API — Deprecated (Types 2 & 3)](#recaptcha-image-based-api--deprecated-types-2--3)
    - [reCAPTCHA Token API (v2 & v3)](#recaptcha-token-api-v2--v3)
    - [reCAPTCHA v2 API FAQ](#recaptcha-v2-api-faq)
    - [What is reCAPTCHA v3?](#what-is-recaptcha-v3)
    - [reCAPTCHA v3 API FAQ](#recaptcha-v3-api-faq)
    - [Amazon WAF API (Type 16)](#amazon-waf-api-type-16)
    - [Cloudflare Turnstile API (Type 12)](#cloudflare-turnstile-api-type-12)
- [Tests and CI](#tests-and-ci)
- [Project Layout](#project-layout)
- [Documentation](#documentation)
- [Responsible Use](#responsible-use)
- [Contributing](#contributing)
- [Changelog / Releases](#changelog--releases)
- [License](#license)
- [Badge Details](#badge-details)


<a id="installation"></a>
## 🛠️ Installation

Build requirements and platform notes are in [`doc/BUILD.md`](doc/BUILD.md).

```bash
# Clone the repository
git clone https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11.git
cd deathbycaptcha-api-client-c11

# Development build (debug symbols, assertions enabled)
cmake --preset default
cmake --build --preset default

# Optimised release build
cmake --preset release
cmake --build --preset release
```

<a id="how-to-use-dbc-api-clients"></a>
## 🚀 How to Use DBC API Clients

<a id="common-clients-interface"></a>
### 🔌 Common Clients' Interface

All clients must be initialised with your DeathByCaptcha credentials — either *username* and *password*, or an *authtoken* (available in the DBC user panel). Call `dbc_set_transport()` to switch between socket (default) and HTTPS transports.

```c
#include "deathbycaptcha.h"

dbc_client client;

/* Username + password — socket transport (default, faster, lower latency) */
dbc_init(&client, username, password);

/* Username + password — HTTPS transport (encrypted TLS, recommended when security matters) */
dbc_init(&client, username, password);
dbc_set_transport(&client, DBC_TRANSPORT_HTTPS);

/* Authtoken only */
dbc_init_token(&client, authtoken);

/* Always release resources when done */
dbc_close(&client);
```

| Transport | Constant | Best for |
|---|---|---|
| Socket | `DBC_TRANSPORT_SOCKET` | Plain TCP — faster and lower latency, recommended for high-throughput production workloads |
| HTTPS | `DBC_TRANSPORT_HTTPS` | Encrypted TLS via libcurl — safer for credential handling and network-sensitive environments |

All clients share the same interface. Below is a summary of every available function.

<a id="captcha-methods"></a>

| Function | Returns | Description |
|---|---|---|
| `dbc_get_balance()` | `double` | Current account balance in US cents. |
| `dbc_decode_file()` | `0` / `-1` | Upload an image CAPTCHA from a `FILE*` stream and poll until solved. `timeout` 0 → 60 s. |
| `dbc_decode()` | `0` / `-1` | Upload an image CAPTCHA from a memory buffer and poll until solved. `timeout` 0 → 60 s. |
| `dbc_decode_recaptcha_v2()` | `0` / `-1` | Solve reCAPTCHA v2 (type 4). Takes `sitekey`, `pageurl`, optional `proxy`/`proxytype`. `timeout` 0 → 120 s. |
| `dbc_decode_recaptcha_v3()` | `0` / `-1` | Solve reCAPTCHA v3 (type 5). Takes `sitekey`, `pageurl`, `action`, `min_score`, optional proxy. `timeout` 0 → 120 s. |
| `dbc_decode_recaptcha_enterprise()` | `0` / `-1` | Solve reCAPTCHA v2 Enterprise (type 25). Takes `sitekey`, `pageurl`, optional proxy. `timeout` 0 → 120 s. |
| `dbc_decode_token()` | `0` / `-1` | Generic token solver. Takes numeric `type`, `params_field` name, and `params_json` string. `timeout` 0 → 120 s. |
| `dbc_get_captcha()` | `0` / `-1` | Fetch the status and result of a previously uploaded CAPTCHA by numeric `id`. |
| `dbc_report()` | `0` / `-1` | Report a CAPTCHA as incorrectly solved to request a refund. Only report genuine errors. |

### 📬 CAPTCHA Result Structure

All solve functions populate a `dbc_captcha` struct when successful:

| Field | Type | Description |
|---|---|---|
| `.id` | `unsigned int` | Numeric CAPTCHA ID assigned by DBC |
| `.text` | `char[]` | Solved text or token (the value you inject into the page) |

```c
/* Example usage after a successful solve */
dbc_captcha captcha;
if (dbc_decode_recaptcha_v2(&client, &captcha, sitekey, pageurl,
                             NULL, NULL, DBC_TOKEN_TIMEOUT) == 0) {
    printf("ID: %u  Token: %s\n", captcha.id, captcha.text);
    dbc_close_captcha(&captcha);
}
```

### 💡 Full Usage Example

```c
#include <stdio.h>
#include <stdlib.h>
#include "deathbycaptcha.h"

int main(void)
{
    dbc_client  client;
    dbc_captcha captcha;

    if (dbc_init(&client, "your_username", "your_password") != 0) {
        fprintf(stderr, "Failed to initialize client\n");
        return EXIT_FAILURE;
    }

    printf("Balance: %.2f US cents\n", dbc_get_balance(&client));

    FILE *f = fopen("captcha.jpg", "rb");
    if (f && dbc_decode_file(&client, &captcha, f, DBC_TIMEOUT) == 0) {
        printf("Solved CAPTCHA %u: %s\n", captcha.id, captcha.text);

        /* Report only if you are certain the solution is wrong: */
        /* dbc_report(&client, &captcha); */

        dbc_close_captcha(&captcha);
    } else {
        printf("Failed (timeout or error)\n");
    }
    if (f) fclose(f);

    dbc_close(&client);
    return EXIT_SUCCESS;
}
```

<a id="credentials--configuration"></a>
## 🔑 Credentials & Configuration

Credentials are compiled directly into example programs via `#define` constants, or passed to `dbc_init()` at runtime. For CI environments, set `DBC_USERNAME` and `DBC_PASSWORD` as repository secrets and pass them at build or test time.

<a id="quick-setup"></a>
### ⚡ Quick Setup

```bash
# ① Build the library and CLI
cmake --preset default && cmake --build --preset default

# ② Edit an example with your credentials and compile it
#    (replace USERNAME / PASSWORD defines inside the .c file)
cd examples
gcc -o example.Normal_Captcha \
    example.Normal_Captcha.c \
    ../build/default/libdeathbycaptcha.so \
    -I../src
./example.Normal_Captcha

# ③ Run the full test suite
cd ..
cmake --build --preset default --target test

# ④ Push to repo for GitHub Actions CI
git push
```

<a id="captcha-types-quick-reference--examples"></a>
## 🧩 CAPTCHA Types Quick Reference & Examples

This section covers every supported CAPTCHA type, how to compile and run the corresponding example files, and ready-to-copy C code snippets. Start with the Quick Start below, then use the Type Reference to find the type you need.

<a id="quick-start"></a>
### 🏁 Quick Start

1. **🔨 Build the library** (see [Installation](#installation))
2. **📂 Navigate to the `examples/` directory** and compile the example for the type you need:

```bash
cd examples

# Balance check
gcc -o get_balance get_balance.c ../build/default/libdeathbycaptcha.so -I../src
./get_balance

# Standard image CAPTCHA (requires test.jpg in the examples/ directory)
gcc -o example.Normal_Captcha example.Normal_Captcha.c \
    ../build/default/libdeathbycaptcha.so -I../src
./example.Normal_Captcha
```

> ⚠️ Always compile examples from the `examples/` directory so relative paths (e.g. `test.jpg`) are resolved correctly.

Before compiling, add your DBC credentials inside the example file:

```c
#define USERNAME  "your_username"
#define PASSWORD  "your_password"
```

<a id="sample-index-by-captcha-type"></a>
### 📋 Type Reference

The table below maps every supported type to its use case, a code snippet, and the corresponding example file in `examples/`.

| Type ID | CAPTCHA Type | Use Case | Quick Use | C Sample |
| --- | --- | --- | --- | --- |
| 0 | Standard Image | Basic image CAPTCHA | [snippet](#sample-type-0-standard-image) | [open](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/blob/master/examples/example.Normal_Captcha.c) |
| 2 | ~~reCAPTCHA Coordinates~~ | Deprecated — do not use for new integrations | — | — |
| 3 | ~~reCAPTCHA Image Group~~ | Deprecated — do not use for new integrations | — | — |
| 4 | reCAPTCHA v2 Token | reCAPTCHA v2 token solving | [snippet](#sample-type-4-recaptcha-v2-token) | [open](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/blob/master/examples/example.reCAPTCHA_v2.c) |
| 5 | reCAPTCHA v3 Token | reCAPTCHA v3 with risk scoring | [snippet](#sample-type-5-recaptcha-v3-token) | [open](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/blob/master/examples/example.reCAPTCHA_v3.c) |
| 25 | reCAPTCHA v2 Enterprise | reCAPTCHA v2 Enterprise tokens | [snippet](#sample-type-25-recaptcha-v2-enterprise) | [open](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/blob/master/examples/example.reCAPTCHA_v2_Enterprise.c) |
| 8 | GeeTest v3 | Geetest v3 verification | [snippet](#sample-type-8-geetest-v3) | [open](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/blob/master/examples/example.Geetest_v3.c) |
| 9 | GeeTest v4 | Geetest v4 verification | [snippet](#sample-type-9-geetest-v4) | [open](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/blob/master/examples/example.Geetest_v4.c) |
| 11 | Text CAPTCHA | Text-based question solving | [snippet](#sample-type-11-text-captcha) | [open](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/blob/master/examples/example.Textcaptcha.c) |
| 12 | Cloudflare Turnstile | Cloudflare Turnstile token | [snippet](#sample-type-12-cloudflare-turnstile) | [open](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/blob/master/examples/example.Turnstile.c) |
| 13 | Audio CAPTCHA | Audio CAPTCHA solving | [snippet](#sample-type-13-audio-captcha) | [open](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/blob/master/examples/example.Audio.c) |
| 14 | Lemin | Lemin CAPTCHA | [snippet](#sample-type-14-lemin) | [open](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/blob/master/examples/example.Lemin.c) |
| 15 | Capy | Capy CAPTCHA | [snippet](#sample-type-15-capy) | [open](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/blob/master/examples/example.Capy.c) |
| 16 | Amazon WAF | Amazon WAF verification | [snippet](#sample-type-16-amazon-waf) | [open](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/blob/master/examples/example.Amazon_Waf.c) |
| 17 | Siara | Siara CAPTCHA | [snippet](#sample-type-17-siara) | [open](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/blob/master/examples/example.Siara.c) |
| 18 | MTCaptcha | Mtcaptcha CAPTCHA | [snippet](#sample-type-18-mtcaptcha) | [open](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/blob/master/examples/example.Mtcaptcha.c) |
| 19 | Cutcaptcha | Cutcaptcha CAPTCHA | [snippet](#sample-type-19-cutcaptcha) | [open](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/blob/master/examples/example.Cutcaptcha.c) |
| 20 | Friendly Captcha | Friendly Captcha | [snippet](#sample-type-20-friendly-captcha) | [open](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/blob/master/examples/example.Friendly.c) |
| 21 | DataDome | Datadome verification | [snippet](#sample-type-21-datadome) | [open](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/blob/master/examples/example.Datadome.c) |
| 23 | Tencent | Tencent CAPTCHA | [snippet](#sample-type-23-tencent) | [open](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/blob/master/examples/example.Tencent.c) |
| 24 | ATB | ATB CAPTCHA | [snippet](#sample-type-24-atb) | [open](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/blob/master/examples/example.Atb.c) |

<a id="per-type-code-snippets"></a>
### 📝 Per-Type Code Snippets

Minimal usage snippet for each supported type. Use these as a starting point and refer to the full example files in `examples/` for complete implementations.

<a id="sample-type-0-standard-image"></a>
#### 🖼️ Sample Type 0: Standard Image
Official description: [Supported CAPTCHAs](https://deathbycaptcha.com/api#supported_captchas)
Full sample: [example.Normal_Captcha.c](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/blob/master/examples/example.Normal_Captcha.c)

```c
FILE *f = fopen("captcha.jpg", "rb");
dbc_decode_file(&client, &captcha, f, DBC_TIMEOUT);
fclose(f);
```

---

<a id="sample-type-4-recaptcha-v2-token"></a>
#### 🤖 Sample Type 4: reCAPTCHA v2 Token
Official description: [reCAPTCHA Token API (v2)](https://deathbycaptcha.com/api/newtokenrecaptcha#token-v2)
Full sample: [example.reCAPTCHA_v2.c](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/blob/master/examples/example.reCAPTCHA_v2.c)

```c
dbc_decode_recaptcha_v2(&client, &captcha,
    "sitekey",
    "https://target",
    "http://user:pass@127.0.0.1:1234", "HTTP",
    DBC_TOKEN_TIMEOUT);
```

---

<a id="sample-type-5-recaptcha-v3-token"></a>
#### 🤖 Sample Type 5: reCAPTCHA v3 Token
Official description: [reCAPTCHA v3](https://deathbycaptcha.com/api/newtokenrecaptcha#reCAPTCHAv3)
Full sample: [example.reCAPTCHA_v3.c](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/blob/master/examples/example.reCAPTCHA_v3.c)

```c
dbc_decode_recaptcha_v3(&client, &captcha,
    "sitekey",
    "https://target",
    "verify", 0.3,
    "http://user:pass@127.0.0.1:1234", "HTTP",
    DBC_TOKEN_TIMEOUT);
```

---

<a id="sample-type-25-recaptcha-v2-enterprise"></a>
#### 🏢 Sample Type 25: reCAPTCHA v2 Enterprise
Official description: [reCAPTCHA v2 Enterprise](https://deathbycaptcha.com/api/newtokenrecaptcha#reCAPTCHAv2Enterprise)
Full sample: [example.reCAPTCHA_v2_Enterprise.c](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/blob/master/examples/example.reCAPTCHA_v2_Enterprise.c)

```c
dbc_decode_recaptcha_enterprise(&client, &captcha,
    "sitekey",
    "https://target",
    "http://user:pass@127.0.0.1:1234", "HTTP",
    DBC_TOKEN_TIMEOUT);
```

---

<a id="sample-type-8-geetest-v3"></a>
#### 🧩 Sample Type 8: GeeTest v3
Official description: [GeeTest](https://deathbycaptcha.com/api/geetest)
Full sample: [example.Geetest_v3.c](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/blob/master/examples/example.Geetest_v3.c)

```c
const char *params =
    "{\"proxy\":\"http://user:pass@127.0.0.1:1234\","
    "\"proxytype\":\"HTTP\","
    "\"gt\":\"gt_value\","
    "\"challenge\":\"challenge_value\","
    "\"pageurl\":\"https://target\"}";
dbc_decode_token(&client, &captcha, 8, "geetest_params", params, DBC_TOKEN_TIMEOUT);
```

---

<a id="sample-type-9-geetest-v4"></a>
#### 🧩 Sample Type 9: GeeTest v4
Official description: [GeeTest](https://deathbycaptcha.com/api/geetest)
Full sample: [example.Geetest_v4.c](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/blob/master/examples/example.Geetest_v4.c)

```c
const char *params =
    "{\"proxy\":\"http://user:pass@127.0.0.1:1234\","
    "\"proxytype\":\"HTTP\","
    "\"captcha_id\":\"captcha_id\","
    "\"pageurl\":\"https://target\"}";
dbc_decode_token(&client, &captcha, 9, "geetest_params", params, DBC_TOKEN_TIMEOUT);
```

---

<a id="sample-type-11-text-captcha"></a>
#### 💬 Sample Type 11: Text CAPTCHA
Official description: [Text CAPTCHA](https://deathbycaptcha.com/api/textcaptcha)
Full sample: [example.Textcaptcha.c](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/blob/master/examples/example.Textcaptcha.c)

```c
const char *params = "{\"textcaptcha\":\"What is two plus two?\"}";
dbc_decode_token(&client, &captcha, 11, "textcaptcha", params, DBC_TOKEN_TIMEOUT);
```

---

<a id="sample-type-12-cloudflare-turnstile"></a>
#### ☁️ Sample Type 12: Cloudflare Turnstile
Official description: [Cloudflare Turnstile](https://deathbycaptcha.com/api/turnstile)
Full sample: [example.Turnstile.c](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/blob/master/examples/example.Turnstile.c)

```c
const char *params =
    "{\"proxy\":\"http://user:pass@127.0.0.1:1234\","
    "\"proxytype\":\"HTTP\","
    "\"sitekey\":\"sitekey\","
    "\"pageurl\":\"https://target\"}";
dbc_decode_token(&client, &captcha, 12, "turnstile_params", params, DBC_TOKEN_TIMEOUT);
```

---

<a id="sample-type-13-audio-captcha"></a>
#### 🔊 Sample Type 13: Audio CAPTCHA
Official description: [Audio CAPTCHA](https://deathbycaptcha.com/api/audio)
Full sample: [example.Audio.c](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/blob/master/examples/example.Audio.c)

```c
/* audio value must be prefixed with "base64:"; language is e.g. "en" */
const char *params =
    "{\"audio\":\"base64:<audio_base64>\","
    "\"language\":\"en\"}";
dbc_decode_token(&client, &captcha, 13, "audio", params, DBC_TOKEN_TIMEOUT);
```

---

<a id="sample-type-14-lemin"></a>
#### 🔵 Sample Type 14: Lemin
Official description: [Lemin](https://deathbycaptcha.com/api/lemin)
Full sample: [example.Lemin.c](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/blob/master/examples/example.Lemin.c)

```c
const char *params =
    "{\"proxy\":\"http://user:pass@127.0.0.1:1234\","
    "\"proxytype\":\"HTTP\","
    "\"captchaid\":\"CROPPED_xxx\","
    "\"pageurl\":\"https://target\"}";
dbc_decode_token(&client, &captcha, 14, "lemin_params", params, DBC_TOKEN_TIMEOUT);
```

---

<a id="sample-type-15-capy"></a>
#### 🏴 Sample Type 15: Capy
Official description: [Capy](https://deathbycaptcha.com/api/capy)
Full sample: [example.Capy.c](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/blob/master/examples/example.Capy.c)

```c
const char *params =
    "{\"proxy\":\"http://user:pass@127.0.0.1:1234\","
    "\"proxytype\":\"HTTP\","
    "\"captchakey\":\"PUZZLE_xxx\","
    "\"api_server\":\"https://api.capy.me/\","
    "\"pageurl\":\"https://target\"}";
dbc_decode_token(&client, &captcha, 15, "capy_params", params, DBC_TOKEN_TIMEOUT);
```

---

<a id="sample-type-16-amazon-waf"></a>
#### 🛡️ Sample Type 16: Amazon WAF
Official description: [Amazon WAF](https://deathbycaptcha.com/api/amazonwaf)
Full sample: [example.Amazon_Waf.c](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/blob/master/examples/example.Amazon_Waf.c)

```c
const char *params =
    "{\"proxy\":\"http://user:pass@127.0.0.1:1234\","
    "\"proxytype\":\"HTTP\","
    "\"sitekey\":\"sitekey\","
    "\"pageurl\":\"https://target\","
    "\"iv\":\"iv_value\","
    "\"context\":\"context_value\"}";
dbc_decode_token(&client, &captcha, 16, "waf_params", params, DBC_TOKEN_TIMEOUT);
```

---

<a id="sample-type-17-siara"></a>
#### 🔍 Sample Type 17: Siara
Official description: [Siara](https://deathbycaptcha.com/api/siara)
Full sample: [example.Siara.c](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/blob/master/examples/example.Siara.c)

```c
const char *params =
    "{\"proxy\":\"http://user:pass@127.0.0.1:1234\","
    "\"proxytype\":\"HTTP\","
    "\"slideurlid\":\"slide_master_url_id\","
    "\"pageurl\":\"https://target\","
    "\"useragent\":\"Mozilla/5.0\"}";
dbc_decode_token(&client, &captcha, 17, "siara_params", params, DBC_TOKEN_TIMEOUT);
```

---

<a id="sample-type-18-mtcaptcha"></a>
#### 🔒 Sample Type 18: MTCaptcha
Official description: [MTCaptcha](https://deathbycaptcha.com/api/mtcaptcha)
Full sample: [example.Mtcaptcha.c](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/blob/master/examples/example.Mtcaptcha.c)

```c
const char *params =
    "{\"proxy\":\"http://user:pass@127.0.0.1:1234\","
    "\"proxytype\":\"HTTP\","
    "\"sitekey\":\"MTPublic-xxx\","
    "\"pageurl\":\"https://target\"}";
dbc_decode_token(&client, &captcha, 18, "mtcaptcha_params", params, DBC_TOKEN_TIMEOUT);
```

---

<a id="sample-type-19-cutcaptcha"></a>
#### ✂️ Sample Type 19: Cutcaptcha
Official description: [Cutcaptcha](https://deathbycaptcha.com/api/cutcaptcha)
Full sample: [example.Cutcaptcha.c](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/blob/master/examples/example.Cutcaptcha.c)

```c
const char *params =
    "{\"proxy\":\"http://user:pass@127.0.0.1:1234\","
    "\"proxytype\":\"HTTP\","
    "\"apikey\":\"api_key\","
    "\"miserykey\":\"misery_key\","
    "\"pageurl\":\"https://target\"}";
dbc_decode_token(&client, &captcha, 19, "cutcaptcha_params", params, DBC_TOKEN_TIMEOUT);
```

---

<a id="sample-type-20-friendly-captcha"></a>
#### 💚 Sample Type 20: Friendly Captcha
Official description: [Friendly Captcha](https://deathbycaptcha.com/api/friendly)
Full sample: [example.Friendly.c](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/blob/master/examples/example.Friendly.c)

```c
const char *params =
    "{\"proxy\":\"http://user:pass@127.0.0.1:1234\","
    "\"proxytype\":\"HTTP\","
    "\"sitekey\":\"FCMG...\","
    "\"pageurl\":\"https://target\"}";
dbc_decode_token(&client, &captcha, 20, "friendly_params", params, DBC_TOKEN_TIMEOUT);
```

---

<a id="sample-type-21-datadome"></a>
#### 🛡️ Sample Type 21: DataDome
Official description: [DataDome](https://deathbycaptcha.com/api/datadome)
Full sample: [example.Datadome.c](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/blob/master/examples/example.Datadome.c)

```c
const char *params =
    "{\"proxy\":\"http://user:pass@127.0.0.1:1234\","
    "\"proxytype\":\"HTTP\","
    "\"pageurl\":\"https://target\","
    "\"captcha_url\":\"https://target/captcha\"}";
dbc_decode_token(&client, &captcha, 21, "datadome_params", params, DBC_TOKEN_TIMEOUT);
```

---

<a id="sample-type-23-tencent"></a>
#### 🌐 Sample Type 23: Tencent
Official description: [Tencent](https://deathbycaptcha.com/api/tencent)
Full sample: [example.Tencent.c](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/blob/master/examples/example.Tencent.c)

```c
const char *params =
    "{\"proxy\":\"http://user:pass@127.0.0.1:1234\","
    "\"proxytype\":\"HTTP\","
    "\"appid\":\"appid\","
    "\"pageurl\":\"https://target\"}";
dbc_decode_token(&client, &captcha, 23, "tencent_params", params, DBC_TOKEN_TIMEOUT);
```

---

<a id="sample-type-24-atb"></a>
#### 🏷️ Sample Type 24: ATB
Official description: [ATB](https://deathbycaptcha.com/api/atb)
Full sample: [example.Atb.c](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/blob/master/examples/example.Atb.c)

```c
const char *params =
    "{\"proxy\":\"http://user:pass@127.0.0.1:1234\","
    "\"proxytype\":\"HTTP\","
    "\"appid\":\"appid\","
    "\"apiserver\":\"https://cap.aisecurius.com\","
    "\"pageurl\":\"https://target\"}";
dbc_decode_token(&client, &captcha, 24, "atb_params", params, DBC_TOKEN_TIMEOUT);
```

<a id="captcha-types-extended-reference"></a>
## 📚 CAPTCHA Types Extended Reference

Full API-level documentation for selected CAPTCHA types: parameter references, payload schemas, request/response formats, token lifespans, and integration notes.

---

<a id="recaptcha-image-based-api--deprecated-types-2--3"></a>
### ⛔ reCAPTCHA Image-Based API — Deprecated (Types 2 & 3)

> ⚠️ **Deprecated.** Types 2 (Coordinates) and 3 (Image Group) are legacy image-based reCAPTCHA challenge methods that are no longer used at captcha solving. Do not use them for new integrations — use the [reCAPTCHA Token API (v2 & v3)](#recaptcha-token-api-v2--v3) instead.

---

<a id="recaptcha-token-api-v2--v3"></a>
### 🔐 reCAPTCHA Token API (v2 & v3)

The Token-based API solves reCAPTCHA challenges by returning a token you inject directly into the page form, rather than clicking images. Given a site URL and site key, DBC solves the challenge on its side and returns a token valid for one submission.

- **Token Image API**: Provided a site URL and site key, the API returns a token that you use to submit the form on the page with the reCAPTCHA challenge.

---

<a id="recaptcha-v2-api-faq"></a>
### ❓ reCAPTCHA v2 API FAQ

**What's the Token Image API URL?**
To use the Token Image API you will have to send a HTTP POST Request to <http://api.dbcapi.me/api/captcha>
**What are the POST parameters for the Token image API?**
-   **`username`**: Your DBC account username
-   **`password`**: Your DBC account password
-   **`type`=4**: Type 4 specifies this is the reCAPTCHA v2 Token API
-   **`token_params`=json(payload)**: the data to access the recaptcha challenge
json payload structure:
    -   **`proxy`**: your proxy url and credentials (if any). Examples:
        -   <http://127.0.0.1:3128>
        -   <http://user:password@127.0.0.1:3128>

    -   **`proxytype`**: your proxy connection protocol. For supported proxy types refer to Which proxy types are supported?. Example:
        -   HTTP

    -   **`googlekey`**: the google recaptcha site key of the website with the recaptcha. For more details about the site key refer to What is a recaptcha site key?. Example:
        -   6Le-wvkSAAAAAPBMRTvw0Q4Muexq9bi0DJwx_mJ-

    -   **`pageurl`**: the url of the page with the recaptcha challenges. This url has to include the path in which the recaptcha is loaded. Example: if the recaptcha you want to solve is in <http://test.com/path1>, pageurl has to be <http://test.com/path1> and not <http://test.com>.
    -   **`data-s`**: This parameter is only required for solve the google search tokens, the ones visible, while google search trigger the robot protection. Use the data-s value inside the google search response html. For regulars tokens don't use this parameter.

The **`proxy`** parameter is optional, but we strongly recommend to use one to prevent token rejection by the provided page due to inconsistencies between the IP that solved the captcha (ours if no proxy is provided) and the IP that submitted the token for verification (yours).
**Note**: If **`proxy`** is provided, **`proxytype`** is a required parameter.
Full example of **`token_params`**:
```json
{
  "proxy": "http://127.0.0.1:3128",
  "proxytype": "HTTP",
  "googlekey": "6Le-wvkSAAAAAPBMRTvw0Q4Muexq9bi0DJwx_mJ-",
  "pageurl": "http://test.com/path_with_recaptcha"
}
```
Example of **`token_params`** for google search captchas:
```json
{
  "googlekey": "6Le-wvkSA...",
  "pageurl": "...",
  "data-s": "IUdfh4rh0sd..."
}
```
**What's the response from the Token image API?**
The token image API response has the same structure as regular captchas' response. Refer to Polling for uploaded CAPTCHA status for details about the response. The token will come in the `text` field of `dbc_captcha`. It's valid for one use and has a 2 minute lifespan. It will be a string like the following:
```
"03AOPBWq_RPO2vLzyk0h8gH0cA2X4v3tpYCPZR6Y4yxKy1s3Eo7CHZRQntxrdsaD2H0e6S3547xi1FlqJB4rob46J0-wfZMj6YpyVa0WGCfpWzBWcLn7tO_EYsvEC_3kfLNINWa5LnKrnJTDXTOz-JuCKvEXx0EQqzb0OU4z2np4uyu79lc_NdvL0IRFc3Cslu6UFV04CIfqXJBWCE5MY0Ag918r14b43ZdpwHSaVVrUqzCQMCybcGq0yxLQf9eSexFiAWmcWLI5nVNA81meTXhQlyCn5bbbI2IMSEErDqceZjf1mX3M67BhIb4"
```

---

<a id="what-is-recaptcha-v3"></a>
### 🔎 What is reCAPTCHA v3?
This API extends the reCAPTCHA v2 Token API with two additional parameters: `action` and **minimal score (`min_score`)**.
reCAPTCHA v3 returns a score from each user, that evaluate if user is a bot or human. Then the website uses the score value that could range from 0 to 1 to decide if will accept or not the requests. Lower scores near to 0 are identified as bot.
The `action` parameter at reCAPTCHA v3 is an additional data used to separate different captcha validations like for example **login**, **register**, **sales**, **etc**.

---

<a id="recaptcha-v3-api-faq"></a>
### ❓ reCAPTCHA v3 API FAQ

**What is `action` in reCAPTCHA v3?**
Is a new parameter that allows processing user actions on the website differently.
To find this we need to inspect the javascript code of the website looking for call of grecaptcha.execute function. Example:
```javascript
grecaptcha.execute('6Lc2fhwTAAAAAGatXTzFYfvlQMI2T7B6ji8UVV_f', {action: something})
```
Sometimes it's really hard to find it and we need to look through all javascript files. We may also try to find the value of action parameter inside ___grecaptcha_cfg configuration object. Also we can call grecaptcha.execute and inspect javascript code. The API will use "verify" default value it if we won't provide action in our request.
**What is `min-score` in reCAPTCHA v3 API?**
The minimal score needed for the captcha resolution. We recommend using the 0.3 min-score value, scores highers than 0.3 are hard to get.
**What are the POST parameters for the reCAPTCHA v3 API?**
-   **`username`**: Your DBC account username
-   **`password`**: Your DBC account password
-   **`type`=5**: Type 5 specifies this is reCAPTCHA v3 API
-   **`token_params`**=json(payload): the data to access the recaptcha challenge
json payload structure:
    -   **`proxy`**: your proxy url and credentials (if any). Examples:
        -   <http://127.0.0.1:3128>
        -   <http://user:password@127.0.0.1:3128>

    -   **`proxytype`**: your proxy connection protocol. For supported proxy types refer to Which proxy types are supported?. Example:
        -   HTTP

    -   **`googlekey`**: the google recaptcha site key of the website with the recaptcha. For more details about the site key refer to What is a recaptcha site key?. Example:
        -   6Le-wvkSAAAAAPBMRTvw0Q4Muexq9bi0DJwx_mJ-

    -   **`pageurl`**: the url of the page with the recaptcha challenges. This url has to include the path in which the recaptcha is loaded. Example: if the recaptcha you want to solve is in <http://test.com/path1>, pageurl has to be <http://test.com/path1> and not <http://test.com>.

    -   **`action`**: The action name.

    -   **`min_score`**: The minimal score, usually 0.3

The **`proxy`** parameter is optional, but we strongly recommend to use one to prevent rejection by the provided page due to inconsistencies between the IP that solved the captcha (ours if no proxy is provided) and the IP that submitted the solution for verification (yours).
**Note**: If **`proxy`** is provided, **`proxytype`** is a required parameter.
Full example of **`token_params`**:
```json
{
  "proxy": "http://127.0.0.1:3128",
  "proxytype": "HTTP",
  "googlekey": "6Le-wvkSAAAAAPBMRTvw0Q4Muexq9bi0DJwx_mJ-",
  "pageurl": "http://test.com/path_with_recaptcha",
  "action": "example/action",
  "min_score": 0.3
}
```
**What's the response from reCAPTCHA v3 API?**
The response has the same structure as regular captcha. Refer to [Polling for uploaded CAPTCHA status](https://deathbycaptcha.com/api#polling-captcha) for details about the response. The solution will come in the `text` field of `dbc_captcha`. It's valid for one use and has a 1 minute lifespan.

---

<a id="amazon-waf-api-type-16"></a>
### 🛡️ Amazon WAF API (Type 16)

Amazon WAF Captcha (also referred to as AWS WAF Captcha) is part of the Intelligent Threat Mitigation system within Amazon AWS. It presents image-alignment challenges that DBC solves by returning a token you set as the `aws-waf-token` cookie on the target page.

- **Official documentation:** [deathbycaptcha.com/api/amazonwaf](https://deathbycaptcha.com/api/amazonwaf)
- **C sample:** [examples/example.Amazon_Waf.c](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/blob/master/examples/example.Amazon_Waf.c)

**API URL:** Send a HTTP POST request to `http://api.dbcapi.me/api/captcha`

**POST parameters:**

-   **`username`**: Your DBC account username
-   **`password`**: Your DBC account password
-   **`type`=16**: Type 16 specifies this is the Amazon WAF API
-   **`waf_params`=json(payload)**: The data needed to access the Amazon WAF challenge

`waf_params` payload fields:

| Parameter | Required | Description |
|---|---|---|
| `proxy` | Optional\* | Proxy URL with credentials. E.g. `http://user:password@127.0.0.1:3128` |
| `proxytype` | Required if proxy set | Proxy protocol. Currently only `HTTP` is supported. |
| `sitekey` | Required | Amazon WAF site key found in the page's captcha script (value of the `key` parameter) |
| `pageurl` | Required | Full URL of the page showing the Amazon WAF challenge (must include the path) |
| `iv` | Required | Value of the `iv` parameter found in the captcha script on the page |
| `context` | Required | Value of the `context` parameter found in the captcha script on the page |
| `challengejs` | Optional | URL of the `challenge.js` script referenced on the page |
| `captchajs` | Optional | URL of the `captcha.js` script referenced on the page |

> The `proxy` parameter is optional but strongly recommended — using a proxy prevents token rejection caused by IP inconsistencies between the solving machine (DBC) and the submitting machine (yours).
> **📌 Note:** If `proxy` is provided, `proxytype` is required.

Full example of `waf_params`:

```json
{
  "proxy": "http://user:password@127.0.0.1:1234",
  "proxytype": "HTTP",
  "sitekey": "AQIDAHjcYu/GjX+QlghicBgQ/7bFaQZ+m5FKCMDnO+vTbNg96AHDh0IR5vgzHNceHYqZR+GO...",
  "pageurl": "https://efw47fpad9.execute-api.us-east-1.amazonaws.com/latest",
  "iv": "CgAFRjIw2vAAABSM",
  "context": "zPT0jOl1rQlUNaldX6LUpn4D6Tl9bJ8VUQ/NrWFxPii..."
}
```

**Response:** The API returns a token string valid for one use with a 1-minute lifespan. Once received, set it as the `aws-waf-token` cookie on the target page before submitting the form.

---

<a id="cloudflare-turnstile-api-type-12"></a>
### 🌐 Cloudflare Turnstile API (Type 12)

Cloudflare Turnstile is a CAPTCHA alternative that protects pages without requiring user interaction in most cases. DBC solves it by returning a token you inject into the target form or pass to the page's callback.

- **Official documentation:** [deathbycaptcha.com/api/turnstile](https://deathbycaptcha.com/api/turnstile)
- **C sample:** [examples/example.Turnstile.c](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/blob/master/examples/example.Turnstile.c)

**API URL:** Send a HTTP POST request to `http://api.dbcapi.me/api/captcha`

| POST Parameter | Description |
|---|---|
| `username` | Your DBC account username |
| `password` | Your DBC account password |
| `type` | `12` — specifies this is a Turnstile API request |
| `turnstile_params` | JSON-encoded payload (see fields below) |

**`turnstile_params` payload fields:**

| Field | Required | Description |
|---|---|---|
| `proxy` | Optional | Proxy URL with optional credentials. E.g. `http://user:password@127.0.0.1:3128` |
| `proxytype` | Required if `proxy` set | Proxy connection protocol. Currently only `HTTP` is supported. |
| `sitekey` | Required | The Turnstile site key found in `data-sitekey` attribute, the captcha iframe URL, or the `turnstile.render` call. E.g. `0x4AAAAAAAGlwMzq_9z6S9Mh` |
| `pageurl` | Required | Full URL of the page hosting the Turnstile challenge, including path. E.g. `https://testsite.com/xxx-test` |
| `action` | Optional | Value of the `data-action` attribute or the `action` option passed to `turnstile.render`. |

> **📌 Note:** The `proxy` parameter is optional but strongly recommended to avoid rejection due to IP inconsistency between the solver and the submitter. If `proxy` is provided, `proxytype` becomes required.

**Example `turnstile_params` (basic):**

```json
{
    "proxy": "http://user:password@127.0.0.1:1234",
    "proxytype": "HTTP",
    "sitekey": "0x4AAAAAAAGlwMzq_9z6S9Mh",
    "pageurl": "https://testsite.com/xxx-test"
}
```

**Response:** The API returns a token string valid for one use with a 2-minute lifespan. Submit it via the `input[name="cf-turnstile-response"]` field (or `input[name="g-recaptcha-response"]` when reCAPTCHA compatibility mode is enabled).

## Tests and CI

| Workflow | Status |
|---|---|
| CI (unit + integration) | [![CI](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/actions/workflows/ci.yml/badge.svg?branch=master)](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/actions/workflows/ci.yml) |
| Tests Linux | [![Tests Linux](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/actions/workflows/test-linux.yml/badge.svg?branch=master)](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/actions/workflows/test-linux.yml) |
| Tests Windows | [![Tests Windows](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/actions/workflows/test-windows.yml/badge.svg?branch=master)](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/actions/workflows/test-windows.yml) |
| Tests macOS | [![Tests macOS](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/actions/workflows/test-macos.yml/badge.svg?branch=master)](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/actions/workflows/test-macos.yml) |
| CI 32-bit | [![CI 32-bit](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/actions/workflows/ci-32bit.yml/badge.svg?branch=master)](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/actions/workflows/ci-32bit.yml) |
| Release CLI Binaries | [![Release CLI Binaries](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/actions/workflows/release-cli.yml/badge.svg)](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/actions/workflows/release-cli.yml) |
| Coverage | [![Coverage](https://img.shields.io/endpoint?url=https://deathbycaptcha.github.io/deathbycaptcha-api-client-c11/coverage-badge.json)](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/actions/workflows/coverage.yml) |

Coverage badge notes:
- Served via GitHub Pages as a shields.io endpoint JSON.
- No external service (Codecov, etc.) is used.
- No workflow commits back to `master`.

## Project Layout

- `src/`: core library (`deathbycaptcha.h`, `deathbycaptcha.c`) and CLI (`client.c`).
- `doc/`: build guide, CLI guide, library guide, and dynamic loading examples.
- `examples/`: standalone `.c` programs — one per CAPTCHA type.
- `tests/`: unit and integration test harnesses.
- `CMakeLists.txt`, `CMakePresets.json`: multiplatform build system (Linux, macOS, Windows).

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

## ⚖️ Responsible Use

See [Responsible Use Agreement](RESPONSIBLE_USE.md).

## Contributing

Contributions should keep API behavior compatible with existing official clients and include tests for new functionality.

## Changelog / Releases

See [CHANGELOG.md](CHANGELOG.md) and the
[Releases page](https://github.com/deathbycaptcha/deathbycaptcha-api-client-c11/releases).

## License

See [LICENSE](LICENSE).

## Badge Details

- `CI` indicates the main `ci.yml` workflow status on `master`. Runs build + unit + integration tests on Linux, macOS and Windows.
- `Coverage` reflects the latest coverage percentage published to GitHub Pages as a shields.io endpoint JSON. Updated on each push to `master`. No external service (Codecov, etc.) is used.
- `Release CLI Binaries` indicates packaging/release workflow status.
- OS-specific test badges in [Tests and CI](#tests-and-ci) show per-platform validation coverage for Linux, Windows, and macOS.