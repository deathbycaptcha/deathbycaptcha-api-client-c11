#include "deathbycaptcha.h"

void app_main(void)
{
    dbc_client client;

    if (0 == dbc_init(&client, "dummy-user", "dummy-pass")) {
        dbc_close(&client);
    }
}
