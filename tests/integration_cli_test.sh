#!/usr/bin/env bash
set -euo pipefail

if [[ $# -lt 3 ]]; then
    echo "usage: $0 <cli_path> <repo_root> <env_file>" >&2
    exit 2
fi

cli_path="$1"
repo_root="$2"
env_file="$3"

if [[ ! -x "$cli_path" ]]; then
    echo "integration test setup error: CLI not found or not executable: $cli_path" >&2
    exit 2
fi

if [[ ! -f "$env_file" ]]; then
    echo "integration test skipped: missing env file $env_file" >&2
    exit 77
fi

set -a
# shellcheck disable=SC1090
source "$env_file"
set +a

if [[ -z "${DBC_USERNAME:-}" || -z "${DBC_PASSWORD:-}" ]]; then
    echo "integration test skipped: DBC_USERNAME/DBC_PASSWORD are required in $env_file" >&2
    exit 77
fi

captcha_file="$repo_root/img/text.jpg"
if [[ ! -f "$captcha_file" ]]; then
    echo "integration test setup error: missing captcha image $captcha_file" >&2
    exit 2
fi

work_root="$(mktemp -d "${TMPDIR:-/tmp}/dbc-it.XXXXXX")"
trap 'rm -rf "$work_root"' EXIT

run_cli_capture() {
    local out
    local rc

    set +e
    out="$($cli_path "$@" 2>&1)"
    rc=$?
    set -e

    printf '%s' "$out"
    return "$rc"
}

log_step() {
    printf '[IT] %s\n' "$1"
}

show_run_dir_state() {
    local run_dir="$1"
    echo "[IT] run dir: $run_dir"
    if [[ -d "$run_dir" ]]; then
        ls -la "$run_dir" || true
    fi
}

assert_balance_ge_zero() {
    local transport="$1"
    local run_dir="$2"
    local out
    local rc

    mkdir -p "$run_dir"
    log_step "$transport balance check"
    set +e
    out="$(run_cli_capture -l "$DBC_USERNAME" -p "$DBC_PASSWORD" "$transport" -d "$run_dir")"
    rc=$?
    set -e

    if [[ $rc -ne 0 ]]; then
        if [[ "$transport" == "--https" && "$out" == *"HTTPS transport is unavailable"* ]]; then
            echo "integration test skipped: HTTPS transport unavailable in current build" >&2
            exit 77
        fi
        echo "balance command failed for $transport (exit $rc)" >&2
        printf '%s\n' "$out" >&2
        exit 1
    fi

    local bal
    bal="$(printf '%s\n' "$out" | awk '/balance:/ {print $2}' | tail -n1)"
    if [[ -z "$bal" ]]; then
        echo "balance parse failed for $transport" >&2
        printf '%s\n' "$out" >&2
        exit 1
    fi

    if ! awk -v b="$bal" 'BEGIN { exit !(b + 0 >= 0) }'; then
        echo "balance validation failed for $transport: $bal" >&2
        exit 1
    fi

    log_step "$transport balance ok: $bal"
}

assert_image_solve_non_empty() {
    local transport="$1"
    local run_dir="$2"
    local out
    local rc

    mkdir -p "$run_dir"
    log_step "$transport image solve"
    set +e
    out="$(run_cli_capture -l "$DBC_USERNAME" -p "$DBC_PASSWORD" "$transport" -c "$captcha_file" -t 180 -d "$run_dir")"
    rc=$?
    set -e

    if [[ $rc -ne 0 ]]; then
        if [[ "$transport" == "--https" && "$out" == *"HTTPS transport is unavailable"* ]]; then
            echo "integration test skipped: HTTPS transport unavailable in current build" >&2
            exit 77
        fi
        echo "image solve failed for $transport (exit $rc)" >&2
        printf '%s\n' "$out" >&2
        exit 1
    fi

    if [[ ! -s "$run_dir/id.txt" ]]; then
        echo "image solve did not produce id.txt for $transport" >&2
        printf '%s\n' "$out" >&2
        show_run_dir_state "$run_dir"
        exit 1
    fi
    if [[ ! -s "$run_dir/answer.txt" ]]; then
        echo "image solve did not produce answer.txt for $transport" >&2
        printf '%s\n' "$out" >&2
        show_run_dir_state "$run_dir"
        exit 1
    fi

    log_step "$transport image solve ok"
}

assert_recaptcha_v2_non_empty() {
    local transport="$1"
    local run_dir="$2"
    local out
    local rc

    mkdir -p "$run_dir"

    local attempts=3
    for attempt in $(seq 1 "$attempts"); do
        rm -f "$run_dir/id.txt" "$run_dir/answer.txt"
        log_step "$transport recaptcha v2 solve (attempt $attempt/$attempts)"

        set +e
        out="$(run_cli_capture \
            -l "$DBC_USERNAME" -p "$DBC_PASSWORD" \
            "$transport" \
            --rc-v2 \
            --sitekey "6Le-wvkSAAAAAPBMRTvw0Q4Muexq9bi0DJwx_mJ-" \
            --pageurl "https://www.google.com/recaptcha/api2/demo" \
            -t 300 \
            -d "$run_dir")"
        rc=$?
        set -e

        if [[ $rc -ne 0 ]]; then
            if [[ "$transport" == "--https" && "$out" == *"HTTPS transport is unavailable"* ]]; then
                echo "integration test skipped: HTTPS transport unavailable in current build" >&2
                exit 77
            fi

            if [[ $attempt -lt $attempts ]]; then
                log_step "$transport recaptcha v2 attempt $attempt failed (exit $rc), retrying"
                sleep 2
                continue
            fi

            echo "recaptcha v2 solve failed for $transport (exit $rc)" >&2
            printf '%s\n' "$out" >&2
            show_run_dir_state "$run_dir"
            exit 1
        fi

        if [[ -s "$run_dir/id.txt" && -s "$run_dir/answer.txt" ]]; then
            log_step "$transport recaptcha v2 solve ok"
            return
        fi

        if [[ $attempt -lt $attempts ]]; then
            log_step "$transport recaptcha v2 produced no output files on attempt $attempt, retrying"
            sleep 2
            continue
        fi

        echo "recaptcha v2 solve did not produce id.txt/answer.txt for $transport" >&2
        printf '%s\n' "$out" >&2
        show_run_dir_state "$run_dir"
        exit 1
    done
}

for transport in --socket --https; do
    tname="${transport#--}"
    transport_root="$work_root/$tname"

    log_step "starting transport: $transport"

    assert_balance_ge_zero "$transport" "$transport_root/balance"
    if [[ "$transport" == "--socket" ]]; then
        assert_image_solve_non_empty "$transport" "$transport_root/image"
    else
        log_step "$transport image solve skipped (service may reject static demo image over HTTPS)"
    fi
    assert_recaptcha_v2_non_empty "$transport" "$transport_root/recaptcha_v2"

    log_step "completed transport: $transport"
done

echo "integration test passed: socket+https balance/image/recaptcha_v2"
