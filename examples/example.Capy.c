/*
 * Death By Captcha — C client example: Capy CAPTCHA (type 15).
 *
 * Replace USERNAME, PASSWORD, CAPTCHAKEY, and PAGEURL with your values.
 */

#include <stdio.h>
#include <stdlib.h>

#include "../src/deathbycaptcha.h"

#define USERNAME    "your_username"
#define PASSWORD    "your_password"
#define CAPTCHAKEY  "PUZZLE_your_captcha_key"
#define API_SERVER  "https://api.capy.me/"
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
        "\"captchakey\":\"" CAPTCHAKEY "\","
        "\"api_server\":\"" API_SERVER "\","
        "\"pageurl\":\"" PAGEURL "\""
        "}";

    if (dbc_decode_token(&client, &captcha, 15, "capy_params",
                          params, DBC_TOKEN_TIMEOUT) == 0) {
        printf("Solved: %s\n", captcha.text);
        dbc_close_captcha(&captcha);
    } else {
        printf("Failed (timeout or error)\n");
    }

    dbc_close(&client);
    return EXIT_SUCCESS;
}
