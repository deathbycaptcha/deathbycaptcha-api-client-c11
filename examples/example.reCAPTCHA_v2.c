/*
 * Death By Captcha — C client example: reCAPTCHA v2 token (type 4).
 *
 * Replace USERNAME, PASSWORD, SITEKEY, and PAGEURL with your values.
 * proxy and proxytype are optional — set to NULL if not needed.
 */

#include <stdio.h>
#include <stdlib.h>

#include "../src/deathbycaptcha.h"

#define USERNAME  "your_username"
#define PASSWORD  "your_password"
#define SITEKEY   "6Le-wvkSAAAAAPBMRTvw0Q4Muexq9bi0DJwx_mJ-"
#define PAGEURL   "https://www.example.com/page"

int main(void)
{
    dbc_client  client;
    dbc_captcha captcha;

    if (dbc_init(&client, USERNAME, PASSWORD) != 0) {
        fprintf(stderr, "Failed to initialize client\n");
        return EXIT_FAILURE;
    }

    if (dbc_decode_recaptcha_v2(&client, &captcha, SITEKEY, PAGEURL,
                                NULL, NULL, DBC_TOKEN_TIMEOUT) == 0) {
        printf("Solved token: %s\n", captcha.text);
        dbc_close_captcha(&captcha);
    } else {
        printf("Failed (timeout or error)\n");
    }

    dbc_close(&client);
    return EXIT_SUCCESS;
}
