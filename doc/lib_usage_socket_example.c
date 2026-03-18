#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#ifdef _WIN32
    #include <windows.h>
#else
    #include <dlfcn.h>
#endif  /* _WIN32 */

#include "deathbycaptcha.h"

enum dbc_sample_token_mode {
    DBC_SAMPLE_TOKEN_NONE = 0,
    DBC_SAMPLE_TOKEN_V2,
    DBC_SAMPLE_TOKEN_V3,
    DBC_SAMPLE_TOKEN_ENTERPRISE
};

#ifndef _WIN32
static void *load_symbol(void *lib, const char *name)
{
    return dlsym(lib, name);
}
#endif

static void save_outputs(const char *prefix, dbc_captcha *captcha)
{
    FILE *f = fopen("id.txt", "w");
    if (NULL != f) {
        fprintf(f, "%u", captcha->id);
        fclose(f);
    }

    f = fopen("answer.txt", "w");
    if (NULL != f) {
        fprintf(f, "%s", (NULL != captcha->text) ? captcha->text : "");
        fclose(f);
    }

    printf("[%s] %u %s\n", prefix, captcha->id, (NULL != captcha->text) ? captcha->text : "");
}

int main(int argc, char *argv[])
{
    if (argc < 3) {
        printf("Usage (image): %s USERNAME PASSWORD CAPTCHA_FILE [... ]\n", argv[0]);
        printf("Usage (rc v2): %s USERNAME PASSWORD --rc-v2 --sitekey SITEKEY --pageurl PAGEURL [--proxy URL] [--proxytype TYPE]\n", argv[0]);
        printf("Usage (rc v3): %s USERNAME PASSWORD --rc-v3 --sitekey SITEKEY --pageurl PAGEURL [--action ACTION] [--min-score SCORE] [--proxy URL] [--proxytype TYPE]\n", argv[0]);
        printf("Usage (enterprise): %s USERNAME PASSWORD --rc-enterprise --sitekey SITEKEY --pageurl PAGEURL [--proxy URL] [--proxytype TYPE]\n", argv[0]);
        return EXIT_SUCCESS;
    }

#ifdef _WIN32
    HINSTANCE lib = LoadLibrary(".\\deathbycaptcha.dll");
    if (NULL == lib) {
        fprintf(stderr, "LoadLibrary(): %d\n", (int)GetLastError());
        return EXIT_FAILURE;
    }
#elif __APPLE__
    void *lib = dlopen("./libdeathbycaptcha.dylib", RTLD_LAZY);
    if (NULL == lib) {
        fprintf(stderr, "dlopen(): %s\n", dlerror());
        return EXIT_FAILURE;
    }
#else
    void *lib = dlopen("./libdeathbycaptcha.so", RTLD_LAZY);
    if (NULL == lib) {
        fprintf(stderr, "dlopen(): %s\n", dlerror());
        return EXIT_FAILURE;
    }
#endif  /* _WIN32 */

    int (*dbc_init_fn)(dbc_client *, const char *, const char *) = NULL;
    int (*dbc_set_transport_fn)(dbc_client *, unsigned int) = NULL;
    void (*dbc_close_fn)(dbc_client *) = NULL;
    double (*dbc_get_balance_fn)(dbc_client *) = NULL;
    int (*dbc_decode_file_fn)(dbc_client *, dbc_captcha *, FILE *, unsigned int) = NULL;
    int (*dbc_decode_recaptcha_v2_fn)(dbc_client *, dbc_captcha *, const char *, const char *, const char *, const char *, unsigned int) = NULL;
    int (*dbc_decode_recaptcha_v3_fn)(dbc_client *, dbc_captcha *, const char *, const char *, const char *, double, const char *, const char *, unsigned int) = NULL;
    int (*dbc_decode_recaptcha_enterprise_fn)(dbc_client *, dbc_captcha *, const char *, const char *, const char *, const char *, unsigned int) = NULL;
    void (*dbc_close_captcha_fn)(dbc_captcha *) = NULL;

#ifdef _WIN32
    dbc_init_fn = (void *)GetProcAddress(lib, "dbc_init");
    dbc_set_transport_fn = (void *)GetProcAddress(lib, "dbc_set_transport");
    dbc_close_fn = (void *)GetProcAddress(lib, "dbc_close");
    dbc_get_balance_fn = (void *)GetProcAddress(lib, "dbc_get_balance");
    dbc_decode_file_fn = (void *)GetProcAddress(lib, "dbc_decode_file");
    dbc_decode_recaptcha_v2_fn = (void *)GetProcAddress(lib, "dbc_decode_recaptcha_v2");
    dbc_decode_recaptcha_v3_fn = (void *)GetProcAddress(lib, "dbc_decode_recaptcha_v3");
    dbc_decode_recaptcha_enterprise_fn = (void *)GetProcAddress(lib, "dbc_decode_recaptcha_enterprise");
    dbc_close_captcha_fn = (void *)GetProcAddress(lib, "dbc_close_captcha");
#else
    *(void **)(&dbc_init_fn) = load_symbol(lib, "dbc_init");
    *(void **)(&dbc_set_transport_fn) = load_symbol(lib, "dbc_set_transport");
    *(void **)(&dbc_close_fn) = load_symbol(lib, "dbc_close");
    *(void **)(&dbc_get_balance_fn) = load_symbol(lib, "dbc_get_balance");
    *(void **)(&dbc_decode_file_fn) = load_symbol(lib, "dbc_decode_file");
    *(void **)(&dbc_decode_recaptcha_v2_fn) = load_symbol(lib, "dbc_decode_recaptcha_v2");
    *(void **)(&dbc_decode_recaptcha_v3_fn) = load_symbol(lib, "dbc_decode_recaptcha_v3");
    *(void **)(&dbc_decode_recaptcha_enterprise_fn) = load_symbol(lib, "dbc_decode_recaptcha_enterprise");
    *(void **)(&dbc_close_captcha_fn) = load_symbol(lib, "dbc_close_captcha");
#endif  /* _WIN32 */

    if (NULL == dbc_init_fn || NULL == dbc_set_transport_fn || NULL == dbc_close_fn
        || NULL == dbc_get_balance_fn || NULL == dbc_decode_file_fn
        || NULL == dbc_decode_recaptcha_v2_fn || NULL == dbc_decode_recaptcha_v3_fn
        || NULL == dbc_decode_recaptcha_enterprise_fn || NULL == dbc_close_captcha_fn) {
        fprintf(stderr, "Failed resolving required symbols from shared library\n");
#ifdef _WIN32
        FreeLibrary(lib);
#else
        dlclose(lib);
#endif  /* _WIN32 */
        return EXIT_FAILURE;
    }

    const char *username = argv[1];
    const char *password = argv[2];
    enum dbc_sample_token_mode token_mode = DBC_SAMPLE_TOKEN_NONE;
    const char *sitekey = NULL;
    const char *pageurl = NULL;
    const char *action = NULL;
    const char *proxy = NULL;
    const char *proxytype = NULL;
    double min_score = 0.0;

    for (int i = 3; i < argc; i++) {
        if (!strcmp("--rc-v2", argv[i])) {
            token_mode = DBC_SAMPLE_TOKEN_V2;
        } else if (!strcmp("--rc-v3", argv[i])) {
            token_mode = DBC_SAMPLE_TOKEN_V3;
        } else if (!strcmp("--rc-enterprise", argv[i])) {
            token_mode = DBC_SAMPLE_TOKEN_ENTERPRISE;
        } else if (!strcmp("--sitekey", argv[i]) && i + 1 < argc) {
            sitekey = argv[++i];
        } else if (!strcmp("--pageurl", argv[i]) && i + 1 < argc) {
            pageurl = argv[++i];
        } else if (!strcmp("--action", argv[i]) && i + 1 < argc) {
            action = argv[++i];
        } else if (!strcmp("--proxy", argv[i]) && i + 1 < argc) {
            proxy = argv[++i];
        } else if (!strcmp("--proxytype", argv[i]) && i + 1 < argc) {
            proxytype = argv[++i];
        } else if (!strcmp("--min-score", argv[i]) && i + 1 < argc) {
            min_score = atof(argv[++i]);
        }
    }

    dbc_client *client = calloc(1, sizeof(dbc_client));
    if (NULL == client) {
        return EXIT_FAILURE;
    }

    if (dbc_init_fn(client, username, password)) {
        fprintf(stderr, "dbc_init(%s, %s) failed\n", username, password);
        dbc_close_fn(client);
        free(client);
#ifdef _WIN32
        FreeLibrary(lib);
#else
        dlclose(lib);
#endif
        return EXIT_FAILURE;
    }

    if (dbc_set_transport_fn(client, DBC_TRANSPORT_SOCKET)) {
        fprintf(stderr, "Failed selecting socket transport\n");
        dbc_close_fn(client);
        free(client);
#ifdef _WIN32
        FreeLibrary(lib);
#else
        dlclose(lib);
#endif
        return EXIT_FAILURE;
    }

    dbc_get_balance_fn(client);
    printf("[socket] User ID: %lu\n[socket] Balance: %.3f US cents\n", client->user_id, client->balance);

    int rc = 0;
    if (DBC_SAMPLE_TOKEN_NONE != token_mode) {
        dbc_captcha *captcha = calloc(1, sizeof(dbc_captcha));
        if (NULL == captcha || NULL == sitekey || NULL == pageurl) {
            rc = -1;
        } else if (DBC_SAMPLE_TOKEN_V2 == token_mode) {
            rc = dbc_decode_recaptcha_v2_fn(client, captcha, sitekey, pageurl, proxy, proxytype, DBC_TIMEOUT);
        } else if (DBC_SAMPLE_TOKEN_V3 == token_mode) {
            rc = dbc_decode_recaptcha_v3_fn(client, captcha, sitekey, pageurl, action, min_score, proxy, proxytype, DBC_TIMEOUT);
        } else {
            rc = dbc_decode_recaptcha_enterprise_fn(client, captcha, sitekey, pageurl, proxy, proxytype, DBC_TIMEOUT);
        }

        if (!rc && NULL != captcha) {
            save_outputs("socket", captcha);
        } else {
            fprintf(stderr, "Failed solving token-based captcha in socket sample\n");
        }

        if (NULL != captcha) {
            dbc_close_captcha_fn(captcha);
            free(captcha);
        }
    } else {
        for (int i = 3; i < argc; i++) {
            if ('-' == argv[i][0] && '-' == argv[i][1]) {
                /* Skip option tokens when running image mode. */
                if ((!strcmp(argv[i], "--sitekey") || !strcmp(argv[i], "--pageurl") ||
                     !strcmp(argv[i], "--action") || !strcmp(argv[i], "--proxy") ||
                     !strcmp(argv[i], "--proxytype") || !strcmp(argv[i], "--min-score")) && i + 1 < argc) {
                    i++;
                }
                continue;
            }

            FILE *f = fopen(argv[i], "rb");
            if (NULL == f) {
                fprintf(stderr, "fopen(%s): %d\n", argv[i], errno);
                continue;
            }

            dbc_captcha *captcha = calloc(1, sizeof(dbc_captcha));
            if (NULL == captcha) {
                fclose(f);
                continue;
            }

            rc = dbc_decode_file_fn(client, captcha, f, DBC_TIMEOUT);
            if (!rc) {
                save_outputs("socket", captcha);
            } else {
                fprintf(stderr, "dbc_decode_file(%s) failed\n", argv[i]);
            }

            dbc_close_captcha_fn(captcha);
            free(captcha);
            fclose(f);
        }
    }

    dbc_close_fn(client);
    free(client);

#ifdef _WIN32
    FreeLibrary(lib);
#else
    dlclose(lib);
#endif  /* _WIN32 */

    return EXIT_SUCCESS;
}
