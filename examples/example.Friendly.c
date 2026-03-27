/*
 * Death By Captcha — C client example: Friendly Captcha (type 20).
 *
 * Replace USERNAME, PASSWORD, SITEKEY, and PAGEURL with your values.
 */

#include <stdio.h>
#include <stdlib.h>

#include "../src/deathbycaptcha.h"

#define USERNAME "your_username"
#define PASSWORD "your_password"
#define SITEKEY  "FCMG_your_sitekey"
#define PAGEURL  "https://www.example.com/page"

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
        "\"pageurl\":\"" PAGEURL "\""
        "}";

    if (dbc_decode_token(&client, &captcha, 20, "friendly_params",
                          params, DBC_TOKEN_TIMEOUT) == 0) {
        printf("Solved token: %s\n", captcha.text);
        dbc_close_captcha(&captcha);
    } else {
        printf("Failed (timeout or error)\n");
    }

    dbc_close(&client);
    return EXIT_SUCCESS;
}
