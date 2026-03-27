/*
 * Death By Captcha — C client example: Cutcaptcha (type 19).
 *
 * Replace USERNAME, PASSWORD, APIKEY, MISERY_KEY, and PAGEURL.
 */

#include <stdio.h>
#include <stdlib.h>

#include "../src/deathbycaptcha.h"

#define USERNAME    "your_username"
#define PASSWORD    "your_password"
#define APIKEY      "your_api_key"
#define MISERY_KEY  "your_misery_key"
#define PAGEURL     "https://www.example.com/page"

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
        "\"apikey\":\"" APIKEY "\","
        "\"misery_key\":\"" MISERY_KEY "\","
        "\"pageurl\":\"" PAGEURL "\""
        "}";

    if (dbc_decode_token(&client, &captcha, 19, "cutcaptcha_params",
                          params, DBC_TOKEN_TIMEOUT) == 0) {
        printf("Solved token: %s\n", captcha.text);
        dbc_close_captcha(&captcha);
    } else {
        printf("Failed (timeout or error)\n");
    }

    dbc_close(&client);
    return EXIT_SUCCESS;
}
