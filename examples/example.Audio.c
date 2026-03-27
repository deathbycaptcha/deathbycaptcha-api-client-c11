/*
 * Death By Captcha — C client example: Audio CAPTCHA (type 13).
 *
 * Submit a base64-encoded audio clip; the solver returns the spoken text.
 * Replace USERNAME and PASSWORD with your values.
 */

#include <stdio.h>
#include <stdlib.h>

#include "../src/deathbycaptcha.h"

#define USERNAME "your_username"
#define PASSWORD "your_password"

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
        "\"audio\":\"base64:<audio_base64>\","
        "\"language\":\"en\""
        "}";

    if (dbc_decode_token(&client, &captcha, 13, "audio",
                          params, DBC_TOKEN_TIMEOUT) == 0) {
        printf("Transcription: %s\n", captcha.text);
        dbc_close_captcha(&captcha);
    } else {
        printf("Failed (timeout or error)\n");
    }

    dbc_close(&client);
    return EXIT_SUCCESS;
}
