/*
 * Death By Captcha — C client example: balance check.
 *
 * Replace USERNAME and PASSWORD with your credentials.
 */

#include <stdio.h>
#include <stdlib.h>

#include "../src/deathbycaptcha.h"

#define USERNAME "your_username"
#define PASSWORD "your_password"

int main(void)
{
    dbc_client client;

    if (dbc_init(&client, USERNAME, PASSWORD) != 0) {
        fprintf(stderr, "Failed to initialize client\n");
        return EXIT_FAILURE;
    }

    double balance = dbc_get_balance(&client);
    printf("Balance: %.2f US cents\n", balance);

    dbc_close(&client);
    return EXIT_SUCCESS;
}
