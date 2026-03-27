/*
 * Death By Captcha — C client example: GeeTest v3 (type 8).
 *
 * Replace USERNAME, PASSWORD, GT, CHALLENGE, and PAGEURL with your values.
 */

#include <stdio.h>
#include <stdlib.h>

#include "../src/deathbycaptcha.h"

#define USERNAME   "your_username"
#define PASSWORD   "your_password"
#define GT         "your_gt_value"
#define CHALLENGE  "your_challenge_value"
#define PAGEURL    "https://www.example.com/page"

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
        "\"gt\":\"" GT "\","
        "\"challenge\":\"" CHALLENGE "\","
        "\"pageurl\":\"" PAGEURL "\""
        "}";

    if (dbc_decode_token(&client, &captcha, 8, "geetest_params",
                          params, DBC_TOKEN_TIMEOUT) == 0) {
        printf("Solved: %s\n", captcha.text);
        dbc_close_captcha(&captcha);
    } else {
        printf("Failed (timeout or error)\n");
    }

    dbc_close(&client);
    return EXIT_SUCCESS;
}
