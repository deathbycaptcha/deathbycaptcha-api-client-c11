# Changelog

All notable changes to this project are documented in this file.

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
