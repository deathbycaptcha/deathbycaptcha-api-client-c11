/*
 * Death By Captcha — C client example: reCAPTCHA v3 token (type 5).
 *
 * Replace USERNAME, PASSWORD, SITEKEY, and PAGEURL with your values.
 * action and min_score are optional — set action to NULL or min_score to 0.0
 * to use API defaults.
 */

#include <stdio.h>
#include <stdlib.h>

#include "../src/deathbycaptcha.h"

#define USERNAME   "your_username"
#define PASSWORD   "your_password"
#define SITEKEY    "6Le-wvkSAAAAAPBMRTvw0Q4Muexq9bi0DJwx_mJ-"
#define PAGEURL    "https://www.example.com/page"
#define ACTION     "verify"
#define MIN_SCORE  0.3

int main(void)
{
    dbc_client  client;
    dbc_captcha captcha;

    if (dbc_init(&client, USERNAME, PASSWORD) != 0) {
        fprintf(stderr, "Failed to initialize client\n");
        return EXIT_FAILURE;
    }

    if (dbc_decode_recaptcha_v3(&client, &captcha, SITEKEY, PAGEURL,
                                ACTION, MIN_SCORE,
                                NULL, NULL, DBC_TOKEN_TIMEOUT) == 0) {
        printf("Solved token: %s\n", captcha.text);
        dbc_close_captcha(&captcha);
    } else {
        printf("Failed (timeout or error)\n");
    }

    dbc_close(&client);
    return EXIT_SUCCESS;
}
