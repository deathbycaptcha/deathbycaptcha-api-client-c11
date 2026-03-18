#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/deathbycaptcha.h"

static int g_fail_init = 0;
static int g_fail_init_token = 0;
static int g_fail_transport = 0;
static int g_balance_user_zero = 0;
static int g_decode_file_fail = 0;
static int g_decode_file_incorrect = 0;

static int mock_dbc_init(dbc_client *client, const char *username, const char *password)
{
    (void)username;
    (void)password;
    if (g_fail_init) {
        g_fail_init = 0;
        return -1;
    }
    memset(client, 0, sizeof(*client));
    client->user_id = g_balance_user_zero ? 0 : 777;
    client->balance = 12.34;
    return 0;
}

static int mock_dbc_init_token(dbc_client *client, const char *token)
{
    (void)token;
    if (g_fail_init_token) {
        g_fail_init_token = 0;
        return -1;
    }
    memset(client, 0, sizeof(*client));
    client->user_id = g_balance_user_zero ? 0 : 888;
    client->balance = 43.21;
    return 0;
}

static int mock_dbc_set_transport(dbc_client *client, unsigned int transport)
{
    if (g_fail_transport) {
        g_fail_transport = 0;
        return -1;
    }
    client->use_https = (DBC_TRANSPORT_HTTPS == transport) ? 1U : 0U;
    return 0;
}

static void mock_dbc_close(dbc_client *client)
{
    (void)client;
}

static double mock_dbc_get_balance(dbc_client *client)
{
    if (g_balance_user_zero) {
        client->user_id = 0;
    }
    return client->balance;
}

static int mock_dbc_init_captcha(dbc_captcha *captcha)
{
    memset(captcha, 0, sizeof(*captcha));
    captcha->is_correct = 1;
    return 0;
}

static void mock_dbc_close_captcha(dbc_captcha *captcha)
{
    if (NULL != captcha && NULL != captcha->text) {
        free(captcha->text);
        captcha->text = NULL;
    }
}

static int mock_dbc_decode_file(dbc_client *client, dbc_captcha *captcha, FILE *f, unsigned int timeout)
{
    (void)client;
    (void)f;
    (void)timeout;
    if (g_decode_file_fail) {
        g_decode_file_fail = 0;
        return -1;
    }
    captcha->id = 1001;
    captcha->is_correct = g_decode_file_incorrect ? 0U : 1U;
    captcha->text = (char *)calloc(16, sizeof(char));
    strcpy(captcha->text, "img-token");
    return 0;
}

static int mock_dbc_report(dbc_client *client, dbc_captcha *captcha)
{
    (void)client;
    captcha->is_correct = 0;
    return 0;
}

static int mock_dbc_decode_recaptcha_v2(dbc_client *client,
                                        dbc_captcha *captcha,
                                        const char *sitekey,
                                        const char *pageurl,
                                        const char *proxy,
                                        const char *proxytype,
                                        unsigned int timeout)
{
    (void)client;
    (void)sitekey;
    (void)pageurl;
    (void)proxy;
    (void)proxytype;
    (void)timeout;
    captcha->id = 2001;
    captcha->is_correct = 1;
    captcha->text = (char *)calloc(16, sizeof(char));
    strcpy(captcha->text, "rc-v2-token");
    return 0;
}

static int mock_dbc_decode_recaptcha_v3(dbc_client *client,
                                        dbc_captcha *captcha,
                                        const char *sitekey,
                                        const char *pageurl,
                                        const char *action,
                                        double min_score,
                                        const char *proxy,
                                        const char *proxytype,
                                        unsigned int timeout)
{
    (void)client;
    (void)sitekey;
    (void)pageurl;
    (void)action;
    (void)min_score;
    (void)proxy;
    (void)proxytype;
    (void)timeout;
    captcha->id = 2002;
    captcha->is_correct = 1;
    captcha->text = (char *)calloc(16, sizeof(char));
    strcpy(captcha->text, "rc-v3-token");
    return 0;
}

static int mock_dbc_decode_recaptcha_enterprise(dbc_client *client,
                                                dbc_captcha *captcha,
                                                const char *sitekey,
                                                const char *pageurl,
                                                const char *proxy,
                                                const char *proxytype,
                                                unsigned int timeout)
{
    (void)client;
    (void)sitekey;
    (void)pageurl;
    (void)proxy;
    (void)proxytype;
    (void)timeout;
    captcha->id = 2003;
    captcha->is_correct = 1;
    captcha->text = (char *)calloc(20, sizeof(char));
    strcpy(captcha->text, "rc-enterprise-token");
    return 0;
}

#define dbc_init mock_dbc_init
#define dbc_init_token mock_dbc_init_token
#define dbc_set_transport mock_dbc_set_transport
#define dbc_close mock_dbc_close
#define dbc_get_balance mock_dbc_get_balance
#define dbc_init_captcha mock_dbc_init_captcha
#define dbc_close_captcha mock_dbc_close_captcha
#define dbc_decode_file mock_dbc_decode_file
#define dbc_report mock_dbc_report
#define dbc_decode_recaptcha_v2 mock_dbc_decode_recaptcha_v2
#define dbc_decode_recaptcha_v3 mock_dbc_decode_recaptcha_v3
#define dbc_decode_recaptcha_enterprise mock_dbc_decode_recaptcha_enterprise
#define main dbc_cli_main
#include "../src/client.c"
#undef main

static int run_cli(char **argv)
{
    int argc = 0;
    while (NULL != argv[argc]) {
        argc++;
    }
    return dbc_cli_main(argc, argv);
}

int main(void)
{
    int rc = 0;

    FILE *tmp = fopen("cli_test_captcha.bin", "wb");
    fwrite("abc", 1, 3, tmp);
    fclose(tmp);

    /* Help/usage and validation branches. */
    char *args_help[] = {"deathbycaptcha", NULL};
    rc |= run_cli(args_help);

    char *args_missing_auth[] = {"deathbycaptcha", "-c", "cli_test_captcha.bin", NULL};
    rc |= run_cli(args_missing_auth);

    char *args_missing_token_fields[] = {"deathbycaptcha", "-l", "u", "-p", "p", "--rc-v2", NULL};
    rc |= run_cli(args_missing_token_fields);

    /* Balance + report normal flows. */
    char *args_balance[] = {"deathbycaptcha", "-l", "u", "-p", "p", "--socket", NULL};
    rc |= run_cli(args_balance);

    char *args_report_user[] = {"deathbycaptcha", "-l", "u", "-p", "p", "-n", "123", "--socket", NULL};
    rc |= run_cli(args_report_user);

    char *args_report_token[] = {"deathbycaptcha", "-a", "token", "-n", "123", "--socket", NULL};
    rc |= run_cli(args_report_token);

    /* Image decode branches, including timeout/id parse and work_dir rewrite. */
    char *args_image[] = {"deathbycaptcha", "-l", "u", "-p", "p", "-c", "cli_test_captcha.bin", "-d", "out", "-t", "0", "--https", NULL};
    rc |= run_cli(args_image);

    char *args_bad_parse[] = {"deathbycaptcha", "-l", "u", "-p", "p", "-c", "cli_test_captcha.bin", "-t", "bad", "-n", "badid", NULL};
    rc |= run_cli(args_bad_parse);

    g_decode_file_incorrect = 1;
    char *args_image_incorrect[] = {"deathbycaptcha", "-l", "u", "-p", "p", "-c", "cli_test_captcha.bin", NULL};
    rc |= run_cli(args_image_incorrect);
    g_decode_file_incorrect = 0;

    g_decode_file_fail = 1;
    char *args_image_fail[] = {"deathbycaptcha", "-l", "u", "-p", "p", "-c", "cli_test_captcha.bin", NULL};
    rc |= run_cli(args_image_fail);

    char *args_stdin_mode[] = {"deathbycaptcha", "-l", "u", "-p", "p", "-c", "-", NULL};
    rc |= run_cli(args_stdin_mode);

    char *args_missing_file[] = {"deathbycaptcha", "-l", "u", "-p", "p", "-c", "missing.bin", NULL};
    rc |= run_cli(args_missing_file);

    /* Token modes with proxy/proxytype and both transports. */
    char *args_v2[] = {"deathbycaptcha", "-l", "u", "-p", "p", "--rc-v2", "--sitekey", "k", "--pageurl", "https://x", "--proxy", "http://127.0.0.1:3128", "--proxytype", "HTTP", "--socket", NULL};
    rc |= run_cli(args_v2);

    char *args_v3[] = {"deathbycaptcha", "-l", "u", "-p", "p", "--rc-v3", "--sitekey", "k", "--pageurl", "https://x", "--action", "login", "--min-score", "0.3", "--proxy", "http://127.0.0.1:3128", "--proxytype", "HTTP", "--https", NULL};
    rc |= run_cli(args_v3);

    char *args_ent[] = {"deathbycaptcha", "-l", "u", "-p", "p", "--rc-enterprise", "--sitekey", "k", "--pageurl", "https://x", "--https", NULL};
    rc |= run_cli(args_ent);

    char *args_token_v2[] = {"deathbycaptcha", "-a", "token", "--rc-v2", "--sitekey", "k", "--pageurl", "https://x", "--https", NULL};
    rc |= run_cli(args_token_v2);

    char *args_token_image[] = {"deathbycaptcha", "-a", "token", "-c", "cli_test_captcha.bin", "--socket", NULL};
    rc |= run_cli(args_token_image);

    char *args_token_balance[] = {"deathbycaptcha", "-a", "token", NULL};
    rc |= run_cli(args_token_balance);

    /* Failure branches for init/set_transport and program failed path. */
    g_fail_init = 1;
    char *args_fail_init[] = {"deathbycaptcha", "-l", "u", "-p", "p", NULL};
    rc |= (run_cli(args_fail_init) == -1 ? 0 : 1);

    g_fail_init_token = 1;
    char *args_fail_init_token[] = {"deathbycaptcha", "-a", "token", NULL};
    rc |= run_cli(args_fail_init_token);

    g_fail_transport = 1;
    char *args_fail_transport_user[] = {"deathbycaptcha", "-l", "u", "-p", "p", "--https", NULL};
    rc |= (run_cli(args_fail_transport_user) == 1 ? 0 : 1);

    g_fail_transport = 1;
    char *args_fail_transport_token[] = {"deathbycaptcha", "-a", "token", "--https", NULL};
    rc |= (run_cli(args_fail_transport_token) == 1 ? 0 : 1);

    g_balance_user_zero = 1;
    char *args_program_failed[] = {"deathbycaptcha", "-l", "u", "-p", "p", NULL};
    rc |= (run_cli(args_program_failed) == 1 ? 0 : 1);
    g_balance_user_zero = 0;

    remove("cli_test_captcha.bin");

    return rc;
}
