/*
 * Death By Captcha — C client example: Standard Image CAPTCHA (type 0).
 *
 * Replace USERNAME, PASSWORD, and CAPTCHA_FILE path with your values.
 */

#include <stdio.h>
#include <stdlib.h>

#include "../src/deathbycaptcha.h"

#define USERNAME     "your_username"
#define PASSWORD     "your_password"
#define CAPTCHA_FILE "test.jpg"

int main(void)
{
    dbc_client  client;
    dbc_captcha captcha;

    if (dbc_init(&client, USERNAME, PASSWORD) != 0) {
        fprintf(stderr, "Failed to initialize client\n");
        return EXIT_FAILURE;
    }

    FILE *f = fopen(CAPTCHA_FILE, "rb");
    if (NULL == f) {
        fprintf(stderr, "Cannot open CAPTCHA file: %s\n", CAPTCHA_FILE);
        dbc_close(&client);
        return EXIT_FAILURE;
    }

    if (dbc_decode_file(&client, &captcha, f, DBC_TIMEOUT) == 0) {
        printf("Solved: %s\n", captcha.text);
        dbc_close_captcha(&captcha);
    } else {
        printf("Failed (timeout or error)\n");
    }

    fclose(f);
    dbc_close(&client);
    return EXIT_SUCCESS;
}
