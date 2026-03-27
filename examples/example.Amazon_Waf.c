/*
 * Death By Captcha — C client example: Amazon WAF (type 16).
 *
 * Replace USERNAME, PASSWORD, SITEKEY, PAGEURL, IV, and CONTEXT.
 */

#include <stdio.h>
#include <stdlib.h>

#include "../src/deathbycaptcha.h"

#define USERNAME "your_username"
#define PASSWORD "your_password"
#define SITEKEY  "your_sitekey_here"
#define PAGEURL  "https://www.example.com/page"
#define IV       "your_iv_value"
#define CONTEXT  "your_context_value"

int main(void)
{
    dbc_client  client;
    dbc_captcha captcha;

    if (dbc_init(&client, USERNAME, PASSWORD) != 0) {
        fprintf(stderr, "Failed to initialize client\n");
        return EXIT_FAILURE;
    }

    const char *params =
        "{"
        "\"sitekey\":\"" SITEKEY "\","
        "\"pageurl\":\"" PAGEURL "\","
        "\"iv\":\"" IV "\","
        "\"context\":\"" CONTEXT "\""
        "}";

    if (dbc_decode_token(&client, &captcha, 16, "waf_params",
                          params, DBC_TOKEN_TIMEOUT) == 0) {
        printf("Solved token: %s\n", captcha.text);
        dbc_close_captcha(&captcha);
    } else {
        printf("Failed (timeout or error)\n");
    }

    dbc_close(&client);
    return EXIT_SUCCESS;
}
