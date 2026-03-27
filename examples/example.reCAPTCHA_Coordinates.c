/*
 * Death By Captcha — C client example: reCAPTCHA Coordinates (type 2).
 *
 * Submit a screenshot; the solver returns click coordinates.
 * Replace USERNAME, PASSWORD, and CAPTCHA_FILE with your values.
 */

#include <stdio.h>
#include <stdlib.h>

#include "../src/deathbycaptcha.h"

#define USERNAME     "your_username"
#define PASSWORD     "your_password"
#define CAPTCHA_FILE "screenshot.png"

int main(void)
{
    dbc_client  client;
    dbc_captcha captcha;

    if (dbc_init(&client, USERNAME, PASSWORD) != 0) {
        fprintf(stderr, "Failed to initialize client\n");
        return EXIT_FAILURE;
    }

    /* Type 2 submits an image — use dbc_decode_file with captcha type 2
     * encoded inline via the generic token upload. */
    if (dbc_decode_token(&client, &captcha, 2, "captchafile",
                          "{\"captchafile\":\"base64:...\"}",
                          DBC_TOKEN_TIMEOUT) == 0) {
        printf("Coordinates: %s\n", captcha.text);
        dbc_close_captcha(&captcha);
    } else {
        printf("Failed (timeout or error)\n");
    }

    dbc_close(&client);
    return EXIT_SUCCESS;
}
