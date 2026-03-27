/*
 * Death By Captcha — C client example: reCAPTCHA Image Group (type 3).
 *
 * Submit a grid image along with a banner and challenge text.
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
        "\"captchafile\":\"base64:<grid_image_base64>\","
        "\"banner\":\"base64:<banner_base64>\","
        "\"banner_text\":\"select all images with a traffic light\""
        "}";

    if (dbc_decode_token(&client, &captcha, 3, "captchafile",
                          params, DBC_TOKEN_TIMEOUT) == 0) {
        printf("Selected indexes: %s\n", captcha.text);
        dbc_close_captcha(&captcha);
    } else {
        printf("Failed (timeout or error)\n");
    }

    dbc_close(&client);
    return EXIT_SUCCESS;
}
