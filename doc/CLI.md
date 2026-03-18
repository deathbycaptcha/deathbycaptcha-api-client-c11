# CLI Tool Guide

This document covers how to build and use the `deathbycaptcha` command-line tool on Linux, macOS, and Windows.

## What The CLI Does

The CLI can:

- Check account balance.
- Upload and solve a CAPTCHA image.
- Report an incorrectly solved CAPTCHA.
- Solve token-based reCAPTCHA v2, reCAPTCHA v3, and reCAPTCHA v2 Enterprise.

The executable is built from `src/client.c`.

## Build By OS

## Linux

```bash
cmake --preset default
cmake --build --preset default --target deathbycaptcha_cli
```

Binary path:

```text
build/default/bin/deathbycaptcha
```

## macOS

```bash
cmake --preset default
cmake --build --preset default --target deathbycaptcha_cli
```

Binary path:

```text
build/default/bin/deathbycaptcha
```

## Windows (MSVC)

```powershell
cmake --preset windows-msvc
cmake --build --preset windows-msvc --target deathbycaptcha_cli
```

Binary path:

```text
build/windows-msvc/bin/deathbycaptcha.exe
```

## CLI Arguments

- `-l USERNAME`: account username.
- `-p PASSWORD`: account password.
- `-a AUTHTOKEN`: account token (alternative to `-l/-p`).
- `-c CAPTCHA_FILE`: path to CAPTCHA image file, or `-` to read from stdin.
- `-n CAPTCHA_ID`: report an incorrectly solved CAPTCHA ID.
- `-t TIMEOUT`: solve timeout in seconds (default `60`).
- `-d WORK_DIR`: output directory for generated files (`balance.txt`, `id.txt`, `answer.txt`).
- `-v`: verbose mode.
- `--socket`: force socket transport (default).
- `--https`: force HTTPS transport (requires libcurl-enabled build).
- `--rc-v2`: token mode for reCAPTCHA v2.
- `--rc-v3`: token mode for reCAPTCHA v3.
- `--rc-enterprise`: token mode for reCAPTCHA v2 Enterprise.
- `--sitekey KEY`: reCAPTCHA site key (`googlekey`).
- `--pageurl URL`: page URL where challenge is rendered.
- `--action ACTION`: reCAPTCHA v3 action (optional).
- `--min-score SCORE`: reCAPTCHA v3 minimum score (optional).
- `--proxy URL`: optional proxy URL.
- `--proxytype TYPE`: optional proxy type.

Authentication rules:

- Use either `-l/-p` or `-a`.
- If both are provided, token path is used.

Token mode rules:

- `--rc-v2`, `--rc-v3`, and `--rc-enterprise` require `--sitekey` and `--pageurl`.
- If no token mode is provided, CLI uses image mode (`-c` file/stdin path).

## Usage Examples

## 1) Check Balance (username/password)

```bash
./build/default/bin/deathbycaptcha -l USERNAME -p PASSWORD
```

Writes `balance.txt` in current directory unless `-d` is provided.

## 2) Check Balance (token)

```bash
./build/default/bin/deathbycaptcha -a AUTHTOKEN
```

## 3) Solve CAPTCHA From File

```bash
./build/default/bin/deathbycaptcha -l USERNAME -p PASSWORD -c captcha.jpg -t 90 -d ./out
```

On success:

- `id.txt` contains the CAPTCHA id.
- `answer.txt` contains the solved text.

## 4) Solve CAPTCHA From stdin

```bash
cat captcha.jpg | ./build/default/bin/deathbycaptcha -l USERNAME -p PASSWORD -c -
```

## 5) Report Incorrect Solution

```bash
./build/default/bin/deathbycaptcha -l USERNAME -p PASSWORD -n 123456789
```

Or with token:

```bash
./build/default/bin/deathbycaptcha -a AUTHTOKEN -n 123456789
```

## 6) reCAPTCHA v2 (socket)

```bash
./build/default/bin/deathbycaptcha -l USERNAME -p PASSWORD --socket --rc-v2 --sitekey SITEKEY --pageurl https://example.com/form
```

## 7) reCAPTCHA v2 (https)

```bash
./build/default/bin/deathbycaptcha -l USERNAME -p PASSWORD --https --rc-v2 --sitekey SITEKEY --pageurl https://example.com/form
```

## 8) reCAPTCHA v3 (socket)

```bash
./build/default/bin/deathbycaptcha -l USERNAME -p PASSWORD --socket --rc-v3 --sitekey SITEKEY --pageurl https://example.com/login --action login --min-score 0.3
```

## 9) reCAPTCHA v3 (https)

```bash
./build/default/bin/deathbycaptcha -l USERNAME -p PASSWORD --https --rc-v3 --sitekey SITEKEY --pageurl https://example.com/login --action login --min-score 0.3
```

## 10) reCAPTCHA Enterprise (socket)

```bash
./build/default/bin/deathbycaptcha -l USERNAME -p PASSWORD --socket --rc-enterprise --sitekey SITEKEY --pageurl https://example.com/enterprise
```

## 11) reCAPTCHA Enterprise (https)

```bash
./build/default/bin/deathbycaptcha -l USERNAME -p PASSWORD --https --rc-enterprise --sitekey SITEKEY --pageurl https://example.com/enterprise
```

## Notes

- The CLI defaults to `--socket` if no transport flag is provided.
- `--https` requires a libcurl-enabled build.
- If credentials are missing or invalid, the tool prints usage and exits with an error.
- Keep credentials out of shell history when possible (prefer env vars or secure shell practices).
