#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "deathbycaptcha.h"

static int g_failures = 0;

#define EXPECT_TRUE(cond, msg)                                                   \
    do {                                                                          \
        if (!(cond)) {                                                            \
            fprintf(stderr, "[FAIL] %s\n", msg);                                \
            g_failures++;                                                         \
        } else {                                                                  \
            fprintf(stdout, "[ OK ] %s\n", msg);                                \
        }                                                                         \
    } while (0)

static void test_poll_intervals(void)
{
    EXPECT_TRUE(dbc_get_poll_interval(0) == DBC_INTERVALS[0],
                "poll interval idx 0 matches table");
    EXPECT_TRUE(dbc_get_poll_interval(DBC_INTERVALS_LEN - 1) == DBC_INTERVALS[DBC_INTERVALS_LEN - 1],
                "poll interval last idx matches table");
    EXPECT_TRUE(dbc_get_poll_interval(DBC_INTERVALS_LEN + 7) == DBC_DFLT_INTERVAL,
                "poll interval out-of-range uses default");
}

static void test_captcha_lifecycle(void)
{
    dbc_captcha captcha;
    captcha.id = 123;
    captcha.is_correct = 0;
    captcha.text = (char *)malloc(8);
    strcpy(captcha.text, "token");

    dbc_close_captcha(&captcha);
    EXPECT_TRUE(captcha.id == 0, "dbc_close_captcha resets id");
    EXPECT_TRUE(captcha.is_correct == 1, "dbc_close_captcha resets correctness flag");
    EXPECT_TRUE(captcha.text == NULL, "dbc_close_captcha frees text pointer");

    EXPECT_TRUE(dbc_init_captcha(&captcha) == 0, "dbc_init_captcha succeeds with valid pointer");
    EXPECT_TRUE(captcha.id == 0, "dbc_init_captcha sets id to zero");
    EXPECT_TRUE(captcha.text == NULL, "dbc_init_captcha initializes text pointer");
    EXPECT_TRUE(dbc_init_captcha(NULL) == -1, "dbc_init_captcha rejects NULL pointer");
}

static void test_transport_selector(void)
{
    dbc_client client;
    memset(&client, 0, sizeof(client));

    EXPECT_TRUE(dbc_set_transport(NULL, DBC_TRANSPORT_SOCKET) == -1,
                "dbc_set_transport rejects NULL client");
    EXPECT_TRUE(dbc_set_transport(&client, DBC_TRANSPORT_SOCKET) == 0,
                "dbc_set_transport accepts socket transport");
    EXPECT_TRUE(dbc_set_transport(&client, 999U) == -1,
                "dbc_set_transport rejects invalid transport id");

#if defined(DBC_HAS_CURL) && DBC_HAS_CURL
    EXPECT_TRUE(dbc_set_transport(&client, DBC_TRANSPORT_HTTPS) == 0,
                "dbc_set_transport accepts HTTPS when curl is enabled");
#else
    EXPECT_TRUE(dbc_set_transport(&client, DBC_TRANSPORT_HTTPS) == -1,
                "dbc_set_transport rejects HTTPS when curl is unavailable");
#endif
}

static void test_recaptcha_parameter_validation(void)
{
    dbc_captcha captcha;
    dbc_init_captcha(&captcha);

    EXPECT_TRUE(dbc_decode_recaptcha_v2(NULL, &captcha, NULL, "https://example.com", NULL, NULL, 30) == -1,
                "recaptcha v2 rejects missing sitekey");
    EXPECT_TRUE(dbc_decode_recaptcha_v2(NULL, &captcha, "sitekey", NULL, NULL, NULL, 30) == -1,
                "recaptcha v2 rejects missing pageurl");

    EXPECT_TRUE(dbc_decode_recaptcha_v3(NULL, &captcha, NULL, "https://example.com", "login", 0.3, NULL, NULL, 30) == -1,
                "recaptcha v3 rejects missing sitekey");
    EXPECT_TRUE(dbc_decode_recaptcha_v3(NULL, &captcha, "sitekey", NULL, "login", 0.3, NULL, NULL, 30) == -1,
                "recaptcha v3 rejects missing pageurl");

    EXPECT_TRUE(dbc_decode_recaptcha_enterprise(NULL, &captcha, NULL, "https://example.com", NULL, NULL, 30) == -1,
                "recaptcha enterprise rejects missing sitekey");
    EXPECT_TRUE(dbc_decode_recaptcha_enterprise(NULL, &captcha, "sitekey", NULL, NULL, NULL, 30) == -1,
                "recaptcha enterprise rejects missing pageurl");

    dbc_close_captcha(&captcha);
}

/* Verify the default-timeout constants and that dbc_decode_token rejects bad
 * parameters before attempting any I/O.  These checks are intentionally
 * independent of any mock infrastructure so they remain fast pure-unit tests.
 *
 * Covered per-type contract:
 *   type  0 (image)                   → DBC_TIMEOUT        (60 s)
 *   types 1, 4, 5, 6, 7, 10, 12, 13  → DBC_TOKEN_TIMEOUT (120 s)
 */
static void test_default_timeout_constants(void)
{
    EXPECT_TRUE(DBC_TIMEOUT == 60,
                "DBC_TIMEOUT (image / type-0 default) is 60 s");
    EXPECT_TRUE(DBC_TOKEN_TIMEOUT == 120,
                "DBC_TOKEN_TIMEOUT (all token-based types default) is 120 s");
    EXPECT_TRUE(DBC_TOKEN_TIMEOUT > DBC_TIMEOUT,
                "token-based default timeout exceeds image default");

    /* dbc_decode_token must reject bad params before any network I/O. */
    dbc_captcha cap;
    dbc_init_captcha(&cap);
    EXPECT_TRUE(dbc_decode_token(NULL, &cap, 4, NULL,
                                 "{\"sitekey\":\"sk\",\"pageurl\":\"https://x\"}",
                                 0) == -1,
                "dbc_decode_token rejects NULL params_field");
    EXPECT_TRUE(dbc_decode_token(NULL, &cap, 4, "hcaptcha_params",
                                 NULL, 0) == -1,
                "dbc_decode_token rejects NULL params_json");
    EXPECT_TRUE(dbc_decode_token(NULL, &cap, 4, "hcaptcha_params",
                                 "not-valid-json", 0) == -1,
                "dbc_decode_token rejects invalid JSON params");
    dbc_close_captcha(&cap);
}

int main(void)
{
    fprintf(stdout, "Running deathbycaptcha unit tests...\n");

    test_poll_intervals();
    test_captcha_lifecycle();
    test_transport_selector();
    test_recaptcha_parameter_validation();
    test_default_timeout_constants();

    if (g_failures > 0) {
        fprintf(stderr, "Unit tests failed: %d\n", g_failures);
        return EXIT_FAILURE;
    }

    fprintf(stdout, "All unit tests passed.\n");
    return EXIT_SUCCESS;
}
