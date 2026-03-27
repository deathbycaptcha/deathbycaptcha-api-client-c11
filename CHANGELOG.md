# Changelog

All notable changes to this project are documented in this file.

## [4.7.1] - 2026-03-27

### Added
- Added `dbc_decode_token()` public API function in `src/deathbycaptcha.h` and
  `src/deathbycaptcha.c` for generic token-type CAPTCHA submission (types 2,
  3, 8-24 etc.) without requiring dedicated per-type wrappers.
- Added `examples/` directory with 22 per-type example `.c` files covering
  all supported CAPTCHA types (get_balance + 21 types).
- Added `tests/fixtures/test.jpg` fixture image for integration tests.
- Added `coverage.yml` GitHub Actions workflow using GitHub Pages JSON endpoint
  (shields.io endpoint schema v1) replacing the previous SVG-on-badges-branch
  approach.
- Added `test_default_timeout_constants()` to `tests/deathbycaptcha_tests.c`:
  verifies `DBC_TIMEOUT == 60`, `DBC_TOKEN_TIMEOUT == 120`, and that
  `dbc_decode_token()` rejects NULL/invalid params before any I/O.
- Added deterministic per-type timeout suite to
  `tests/deathbycaptcha_coverage_harness.c`: uses `mock_time`/`mock_sleep` to
  assert that each decode function (`dbc_decode`, `dbc_decode_file`,
  `dbc_decode_recaptcha_v2/v3/enterprise`, `dbc_decode_token`) exhausts
  exactly the correct default timeout (60 s image, 120 s token) when called
  with `timeout = 0`.

### Changed
- Renamed `.github/workflows/coverage-badge.yml` to `.github/workflows/coverage.yml`.
- Rewrote `README.md` with a full structure mirroring the Python client:
  badge row with language indicator, indexed TOC, per-type C code snippets for
  all supported CAPTCHA types, extended reference sections (reCAPTCHA FAQ,
  Amazon WAF, Cloudflare Turnstile), tests/CI table, and project layout.
- Merged `Installation` sub-sections "Build from Source" and
  "From GitHub Repository" into a single flat section.
- Updated README Examples section with full type-to-file table and coverage
  badge URL pointing to the GitHub Pages JSON endpoint.
- Bumped project version from `4.7.0` to `4.7.1` in `CMakeLists.txt` and
  `DBC_API_VERSION` in `src/deathbycaptcha.h`.

### Fixed
- Fixed CLI default timeout in `src/client.c`: `timeout` now initialises to
  `0` instead of `DBC_TIMEOUT` (60 s), allowing the library to apply the
  correct per-type default (60 s for image CAPTCHAs, 120 s for all token-based
  types). Previously all CLI calls used a hard-coded 60 s regardless of type.
- Fixed README code snippets: corrected `dbc_captcha.id` type to `unsigned int`
  (was `unsigned long`), updated `printf` format specifiers to `%u`, and
  applied `DBC_TOKEN_TIMEOUT` (120 s) for types 11 (GeeTest) and 13 (audio).

## [4.7.0] - 2026-03-18

### Added
- Added GitHub Actions workflow at `.github/workflows/ci.yml`.
- Added CI unit test job for `deathbycaptcha_tests` on Ubuntu.
- Added CI integration test job gated by repository secrets `DBC_USERNAME` and `DBC_PASSWORD`.
- Added CI artifact upload for integration test logs on failure.
- Added `tests/fake_curl/curl/curl.h` and coverage harnesses for deeper branch testing.
- Added explicit progress logging in `tests/integration_cli_test.sh` (`[IT] ...`).
- Added retries for flaky live reCAPTCHA v2 integration calls.

### Changed
- Bumped project version from `4.6.0` to `4.7.0` in `CMakeLists.txt`.
- Bumped API version string from `DBC/C v4.6` to `DBC/C v4.7.0` in `src/deathbycaptcha.h`.
- Refined docs in `doc/BUILD.md` for:
  - coverage execution and `gcovr` usage,
  - integration prerequisites and verbose test execution,
  - current integration coverage scope by transport.
- Updated `.gitlab-ci.yml` coverage flow:
  - excluded vendored `src/cJSON.c` from coverage reporting,
  - improved `gcovr` invocation to avoid stale build-tree contamination,
  - hardened `lcov` handling for local runner compatibility.
- Improved `tests/integration_cli_test.sh` reliability and diagnostics:
  - clearer failure messages and directory state dumps,
  - transport-level stage logs,
  - robust reCAPTCHA output validation.
- Improved HTTP response parsing in `src/deathbycaptcha.c`:
  - added fallback support for `application/x-www-form-urlencoded` responses,
  - kept JSON parsing support intact.

### Fixed
- Fixed intermittent `deathbycaptcha_cli_coverage_harness` instability by making argv usage deterministic (NULL-terminated arrays and computed argc).
- Fixed CI/local pipeline failure modes around coverage artifact collection and filtering.
- Fixed integration behavior where HTTPS balance parsing could fail due to non-JSON API response payloads.

### Test & Coverage
- Unit, harness, and coverage test flow validated locally.
- Coverage target exceeded 80% and reached approximately 89% (excluding vendored `cJSON`).

### Housekeeping
- Expanded `.gitignore` to exclude generated coverage artifacts and local runtime output files (`*.gcov`, `*.gcov.json.gz`, `coverage.xml`, `coverage.info`, `coverage_html/`, `id.txt`, `answer.txt`, `balance.txt`).
- Updated git remote to `git@github.com:deathbycaptcha/deathbycaptcha-api-client-c11.git`.
