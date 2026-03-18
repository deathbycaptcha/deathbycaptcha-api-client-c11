# Library API Guide

This document is focused on direct C API usage from your own code.
If you want to run the provided sample executables, use `doc/EXAMPLES.md`.

## Transport Selection

Select transport after `dbc_init(...)` or `dbc_init_token(...)`:

- `DBC_TRANSPORT_SOCKET`: TCP socket API (`api.dbcapi.me:8123`).
- `DBC_TRANSPORT_HTTPS`: HTTPS API (`https://api.dbcapi.me/api`).

```c
dbc_client client;

if (dbc_init(&client, username, password) == 0) {
    /* Choose one transport per client instance. */
    dbc_set_transport(&client, DBC_TRANSPORT_SOCKET);
    /* or: dbc_set_transport(&client, DBC_TRANSPORT_HTTPS); */
}
```

Note: HTTPS transport requires a libcurl-enabled build.

## Token API Surface

The library provides dedicated calls for token-based reCAPTCHA solving:

- `dbc_decode_recaptcha_v2(...)`
- `dbc_decode_recaptcha_v3(...)`
- `dbc_decode_recaptcha_enterprise(...)`

Required fields:

- `sitekey` (`googlekey` on API payload)
- `pageurl`

Optional fields:

- `proxy`
- `proxytype`
- `action` (v3)
- `min_score` (v3)

## Minimal Integration Flow

```c
dbc_client client;
dbc_captcha captcha;

if (dbc_init(&client, "USERNAME", "PASSWORD") == 0 &&
    dbc_set_transport(&client, DBC_TRANSPORT_SOCKET) == 0 &&
    dbc_decode_recaptcha_v2(&client,
                            &captcha,
                            "SITEKEY",
                            "https://example.com/form",
                            NULL,
                            NULL,
                            120) == 0) {
    printf("captcha_id=%u token=%s\n", captcha.id, captcha.text);
    dbc_close_captcha(&captcha);
}

dbc_close(&client);
```

To switch to HTTPS, only change:

```c
dbc_set_transport(&client, DBC_TRANSPORT_HTTPS);
```

## Function-Specific Examples

### reCAPTCHA v2

```c
dbc_decode_recaptcha_v2(&client,
                        &captcha,
                        "SITEKEY",
                        "https://example.com/form",
                        NULL,
                        NULL,
                        120);
```

### reCAPTCHA v3

```c
dbc_decode_recaptcha_v3(&client,
                        &captcha,
                        "SITEKEY",
                        "https://example.com/login",
                        "login",
                        0.3,
                        NULL,
                        NULL,
                        120);
```

### reCAPTCHA Enterprise

```c
dbc_decode_recaptcha_enterprise(&client,
                                &captcha,
                                "SITEKEY",
                                "https://example.com/enterprise",
                                NULL,
                                NULL,
                                120);
```

## Related Docs

- `doc/EXAMPLES.md`: running bundled socket/HTTPS sample executables.
- `doc/CLI.md`: command-line client usage.
