/*
 * Death By Captcha — C client example: Text CAPTCHA (type 11).
 *
 * Submit a text question; the solver returns the answer as a string.
 * Replace USERNAME and PASSWORD with your values.
 */

#include <stdio.h>
#include <stdlib.h>

#include "../src/deathbycaptcha.h"

#define USERNAME  "your_username"
#define PASSWORD  "your_password"

int main(void)
{
    dbc_client  client;
    dbc_captcha captcha;

    if (dbc_init(&client, USERNAME, PASSWORD) != 0) {
        fprintf(stderr, "Failed to initialize client\n");
        return EXIT_FAILURE;
    }

    const char *params = "{\"textcaptcha\":\"What is two plus two?\"}";

    if (dbc_decode_token(&client, &captcha, 11, "textcaptcha",
                          params, DBC_TOKEN_TIMEOUT) == 0) {
        printf("Answer: %s\n", captcha.text);
        dbc_close_captcha(&captcha);
    } else {
        printf("Failed (timeout or error)\n");
    }

    dbc_close(&client);
    return EXIT_SUCCESS;
}
