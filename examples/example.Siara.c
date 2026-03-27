/*
 * Death By Captcha — C client example: Siara CAPTCHA (type 17).
 *
 * Replace USERNAME, PASSWORD, SLIDEURLID, PAGEURL, and USERAGENT.
 */

#include <stdio.h>
#include <stdlib.h>

#include "../src/deathbycaptcha.h"

#define USERNAME    "your_username"
#define PASSWORD    "your_password"
#define SLIDEURLID  "your_slide_master_url_id"
#define PAGEURL     "https://www.example.com/page"
#define USERAGENT   "Mozilla/5.0"

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
        "\"slideurlid\":\"" SLIDEURLID "\","
        "\"pageurl\":\"" PAGEURL "\","
        "\"useragent\":\"" USERAGENT "\""
        "}";

    if (dbc_decode_token(&client, &captcha, 17, "siara_params",
                          params, DBC_TOKEN_TIMEOUT) == 0) {
        printf("Solved: %s\n", captcha.text);
        dbc_close_captcha(&captcha);
    } else {
        printf("Failed (timeout or error)\n");
    }

    dbc_close(&client);
    return EXIT_SUCCESS;
}
