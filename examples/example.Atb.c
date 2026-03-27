/*
 * Death By Captcha — C client example: ATB CAPTCHA (type 24).
 *
 * Replace USERNAME, PASSWORD, APP_ID, API_SERVER, and PAGEURL.
 */

#include <stdio.h>
#include <stdlib.h>

#include "../src/deathbycaptcha.h"

#define USERNAME    "your_username"
#define PASSWORD    "your_password"
#define APP_ID      "your_app_id"
#define API_SERVER  "https://cap.aisecurius.com"
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
        "\"appid\":\"" APP_ID "\","
        "\"apiserver\":\"" API_SERVER "\","
        "\"pageurl\":\"" PAGEURL "\""
        "}";

    if (dbc_decode_token(&client, &captcha, 24, "atb_params",
                          params, DBC_TOKEN_TIMEOUT) == 0) {
        printf("Solved: %s\n", captcha.text);
        dbc_close_captcha(&captcha);
    } else {
        printf("Failed (timeout or error)\n");
    }

    dbc_close(&client);
    return EXIT_SUCCESS;
}
