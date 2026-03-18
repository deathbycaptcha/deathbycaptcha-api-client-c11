#define socket mock_socket
#define connect mock_connect
#define fcntl mock_fcntl
#define select mock_select
#define send mock_send
#define recv mock_recv
#define shutdown mock_shutdown
#define close mock_close
#define getaddrinfo mock_getaddrinfo
#define freeaddrinfo mock_freeaddrinfo
#define time mock_time
#define sleep mock_sleep

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>
#include <unistd.h>

#include "../src/deathbycaptcha.c"

#undef socket
#undef connect
#undef fcntl
#undef select
#undef send
#undef recv
#undef shutdown
#undef close
#undef getaddrinfo
#undef freeaddrinfo
#undef time
#undef sleep

typedef size_t (*write_cb_t)(void *, size_t, size_t, void *);

static const char *g_socket_responses[64];
static int g_socket_resp_count = 0;
static int g_socket_resp_idx = 0;
static int g_socket_resp_off = 0;
static int g_select_phase = 0;

static const char *g_http_responses[64];
static int g_http_resp_count = 0;
static int g_http_resp_idx = 0;

time_t g_now = 1000;

static char *test_strdup(const char *s)
{
    size_t n = strlen(s) + 1;
    char *out = (char *)calloc(n, sizeof(char));
    if (NULL != out) {
        memcpy(out, s, n);
    }
    return out;
}

struct CURL {
    char *url;
    char *postfields;
    write_cb_t write_cb;
    void *write_data;
    struct curl_slist *headers;
};

static void queue_socket(const char *json_with_crlf)
{
    g_socket_responses[g_socket_resp_count++] = json_with_crlf;
}

static void reset_socket_queue(void)
{
    g_socket_resp_count = 0;
    g_socket_resp_idx = 0;
    g_socket_resp_off = 0;
    g_select_phase = 0;
}

static void queue_http(const char *json)
{
    g_http_responses[g_http_resp_count++] = json;
}

static void reset_http_queue(void)
{
    g_http_resp_count = 0;
    g_http_resp_idx = 0;
}

/* --- POSIX/socket mocks --- */
int mock_getaddrinfo(const char *node,
                     const char *service,
                     const struct addrinfo *hints,
                     struct addrinfo **res)
{
    (void)node;
    (void)service;
    (void)hints;

    struct addrinfo *ai = (struct addrinfo *)calloc(1, sizeof(struct addrinfo));
    struct sockaddr_in *sin = (struct sockaddr_in *)calloc(1, sizeof(struct sockaddr_in));
    ai->ai_family = AF_INET;
    ai->ai_socktype = SOCK_STREAM;
    ai->ai_protocol = 0;
    ai->ai_addrlen = sizeof(struct sockaddr_in);
    ai->ai_addr = (struct sockaddr *)sin;
    ai->ai_next = NULL;

    *res = ai;
    return 0;
}

void mock_freeaddrinfo(struct addrinfo *res)
{
    while (NULL != res) {
        struct addrinfo *next = res->ai_next;
        free(res->ai_addr);
        free(res);
        res = next;
    }
}

int mock_socket(int domain, int type, int protocol)
{
    (void)domain;
    (void)type;
    (void)protocol;
    return 3;
}

int mock_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    (void)sockfd;
    (void)addr;
    (void)addrlen;
    errno = EINPROGRESS;
    return -1;
}

int mock_fcntl(int fd, int cmd, ...)
{
    (void)fd;
    (void)cmd;
    return 0;
}

int mock_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout)
{
    (void)nfds;
    (void)timeout;

    if (NULL != exceptfds) {
        FD_ZERO(exceptfds);
    }

    if (0 == g_select_phase) {
        if (NULL != readfds) {
            FD_ZERO(readfds);
        }
        if (NULL != writefds) {
            FD_ZERO(writefds);
            FD_SET(3, writefds);
        }
        g_select_phase = 1;
        return 1;
    }

    if (NULL != readfds) {
        FD_ZERO(readfds);
        FD_SET(3, readfds);
    }
    if (NULL != writefds) {
        FD_ZERO(writefds);
    }

    g_select_phase = 0;
    return 1;
}

ssize_t mock_send(int sockfd, const void *buf, size_t len, int flags)
{
    (void)sockfd;
    (void)buf;
    (void)flags;
    return (ssize_t)len;
}

ssize_t mock_recv(int sockfd, void *buf, size_t len, int flags)
{
    (void)sockfd;
    (void)flags;

    if (g_socket_resp_idx >= g_socket_resp_count) {
        return 0;
    }

    const char *src = g_socket_responses[g_socket_resp_idx];
    size_t src_len = strlen(src);
    if ((size_t)g_socket_resp_off >= src_len) {
        g_socket_resp_idx++;
        g_socket_resp_off = 0;
        return 0;
    }

    size_t remain = src_len - (size_t)g_socket_resp_off;
    size_t chunk = (remain < len) ? remain : len;
    memcpy(buf, src + g_socket_resp_off, chunk);
    g_socket_resp_off += (int)chunk;

    if ((size_t)g_socket_resp_off >= src_len) {
        g_socket_resp_idx++;
        g_socket_resp_off = 0;
    }

    return (ssize_t)chunk;
}

int mock_shutdown(int sockfd, int how)
{
    (void)sockfd;
    (void)how;
    return 0;
}

int mock_close(int fd)
{
    (void)fd;
    return 0;
}

/* --- time/sleep mocks --- */
time_t mock_time(time_t *tloc)
{
    if (NULL != tloc) {
        *tloc = g_now;
    }
    return g_now;
}

unsigned int mock_sleep(unsigned int seconds)
{
    g_now += (time_t)seconds;
    return 0;
}

/* --- fake curl implementation --- */
CURL *curl_easy_init(void)
{
    return (CURL *)calloc(1, sizeof(CURL));
}

void curl_easy_cleanup(CURL *curl)
{
    if (NULL != curl) {
        free(curl->url);
        free(curl->postfields);
        free(curl);
    }
}

CURLcode curl_easy_setopt(CURL *curl, int option, ...)
{
    va_list ap;
    va_start(ap, option);

    switch (option) {
        case CURLOPT_URL: {
            const char *v = va_arg(ap, const char *);
            free(curl->url);
            curl->url = test_strdup(v);
            break;
        }
        case CURLOPT_POSTFIELDS: {
            const char *v = va_arg(ap, const char *);
            free(curl->postfields);
            curl->postfields = test_strdup(v);
            break;
        }
        case CURLOPT_WRITEFUNCTION:
            curl->write_cb = va_arg(ap, write_cb_t);
            break;
        case CURLOPT_WRITEDATA:
            curl->write_data = va_arg(ap, void *);
            break;
        case CURLOPT_HTTPHEADER:
            curl->headers = va_arg(ap, struct curl_slist *);
            break;
        default:
            (void)va_arg(ap, void *);
            break;
    }

    va_end(ap);
    return CURLE_OK;
}

CURLcode curl_easy_perform(CURL *curl)
{
    if (g_http_resp_idx >= g_http_resp_count || NULL == curl->write_cb) {
        return CURLE_OK;
    }

    const char *body = g_http_responses[g_http_resp_idx++];
    curl->write_cb((void *)body, 1, strlen(body), curl->write_data);
    return CURLE_OK;
}

char *curl_easy_escape(CURL *curl, const char *string, int length)
{
    (void)curl;
    if (NULL == string) {
        return NULL;
    }

    if (length < 0) {
        length = (int)strlen(string);
    }

    char *out = (char *)calloc((size_t)length + 1, sizeof(char));
    memcpy(out, string, (size_t)length);
    out[length] = '\0';
    return out;
}

void curl_free(void *ptr)
{
    free(ptr);
}

const char *curl_easy_strerror(CURLcode code)
{
    (void)code;
    return "OK";
}

struct curl_slist *curl_slist_append(struct curl_slist *list, const char *string)
{
    struct curl_slist *node = (struct curl_slist *)calloc(1, sizeof(struct curl_slist));
    node->data = test_strdup(string);
    node->next = NULL;

    if (NULL == list) {
        return node;
    }

    struct curl_slist *it = list;
    while (NULL != it->next) {
        it = it->next;
    }
    it->next = node;
    return list;
}

void curl_slist_free_all(struct curl_slist *list)
{
    while (NULL != list) {
        struct curl_slist *next = list->next;
        free(list->data);
        free(list);
        list = next;
    }
}

CURLcode curl_global_init(long flags)
{
    (void)flags;
    return CURLE_OK;
}

void curl_global_cleanup(void)
{
}

static int expect_true(int cond, const char *msg)
{
    if (!cond) {
        fprintf(stderr, "[FAIL] %s\n", msg);
        return 0;
    }
    fprintf(stdout, "[ OK ] %s\n", msg);
    return 1;
}

int main(void)
{
    int ok = 1;
    dbc_client client;
    dbc_client token_client;
    dbc_captcha cap;

    FILE *tmpf = fopen("harness_image.bin", "wb");
    fwrite("abc", 1, 3, tmpf);
    fclose(tmpf);

    ok &= expect_true(0 == dbc_init(&client, "user", "pass"), "dbc_init works with mocked getaddrinfo");
    ok &= expect_true(0 == dbc_init_token(&token_client, "token"), "dbc_init_token works with mocked getaddrinfo");
    ok &= expect_true(-1 == dbc_set_transport(&client, 99U), "invalid transport rejected");

    /* Socket transport: image upload/decode. */
    reset_socket_queue();
    queue_socket("{\"captcha\":101,\"text\":null,\"is_correct\":1}\r\n");
    queue_socket("{\"captcha\":101,\"text\":\"img_socket\",\"is_correct\":1}\r\n");
    ok &= expect_true(0 == dbc_set_transport(&client, DBC_TRANSPORT_SOCKET), "set socket transport");
    ok &= expect_true(0 == dbc_decode(&client, &cap, "abc", 3, 5), "decode image over socket");
    dbc_close_captcha(&cap);

    /* Socket transport: upload from file + explicit get/report. */
    reset_socket_queue();
    queue_socket("{\"captcha\":102,\"text\":null,\"is_correct\":1}\r\n");
    tmpf = fopen("harness_image.bin", "rb");
    ok &= expect_true(0 == dbc_upload_file(&client, &cap, tmpf), "upload file over socket");
    fclose(tmpf);
    reset_socket_queue();
    queue_socket("{\"captcha\":102,\"text\":\"img_get\",\"is_correct\":1}\r\n");
    ok &= expect_true(0 == dbc_get_captcha(&client, &cap, 102), "get captcha over socket");
    reset_socket_queue();
    queue_socket("{\"captcha\":102,\"text\":\"img_get\",\"is_correct\":0}\r\n");
    ok &= expect_true(0 == dbc_report(&client, &cap), "report captcha over socket");
    dbc_close_captcha(&cap);

    /* Socket transport: recaptcha v2/v3/enterprise. */
    reset_socket_queue();
    queue_socket("{\"captcha\":111,\"text\":null,\"is_correct\":1}\r\n");
    queue_socket("{\"captcha\":111,\"text\":\"tokv2_socket\",\"is_correct\":1}\r\n");
    ok &= expect_true(0 == dbc_decode_recaptcha_v2(&client, &cap, "site", "https://example.com", NULL, NULL, 5),
                      "decode recaptcha v2 over socket");
    dbc_close_captcha(&cap);

    reset_socket_queue();
    queue_socket("{\"captcha\":112,\"text\":null,\"is_correct\":1}\r\n");
    queue_socket("{\"captcha\":112,\"text\":\"tokv3_socket\",\"is_correct\":1}\r\n");
    ok &= expect_true(0 == dbc_decode_recaptcha_v3(&client, &cap, "site", "https://example.com", "login", 0.3, NULL, NULL, 5),
                      "decode recaptcha v3 over socket");
    dbc_close_captcha(&cap);

    reset_socket_queue();
    queue_socket("{\"captcha\":113,\"text\":null,\"is_correct\":1}\r\n");
    queue_socket("{\"captcha\":113,\"text\":\"tokent_socket\",\"is_correct\":1}\r\n");
    ok &= expect_true(0 == dbc_decode_recaptcha_enterprise(&client, &cap, "site", "https://example.com", NULL, NULL, 5),
                      "decode recaptcha enterprise over socket");
    dbc_close_captcha(&cap);

    /* HTTP transport: balance/user call. */
    reset_http_queue();
    queue_http("{\"user\":321,\"balance\":9.9,\"is_banned\":0}");
    ok &= expect_true(0 == dbc_set_transport(&client, DBC_TRANSPORT_HTTPS), "set https transport");
    ok &= expect_true(dbc_get_balance(&client) > 0.0, "get balance over http");

    /* HTTP transport: image decode_file path. */
    reset_http_queue();
    queue_http("{\"captcha\":201,\"text\":null,\"is_correct\":1}");
    queue_http("{\"captcha\":201,\"text\":\"img_http\",\"is_correct\":1}");
    tmpf = fopen("harness_image.bin", "rb");
    ok &= expect_true(0 == dbc_decode_file(&client, &cap, tmpf, 5), "decode file over http");
    fclose(tmpf);
    dbc_close_captcha(&cap);

    /* HTTP transport: explicit get/report commands. */
    reset_http_queue();
    queue_http("{\"captcha\":202,\"text\":\"tok\",\"is_correct\":1}");
    ok &= expect_true(0 == dbc_get_captcha(&client, &cap, 202), "get captcha over http");
    reset_http_queue();
    queue_http("{\"captcha\":202,\"text\":\"tok\",\"is_correct\":0}");
    ok &= expect_true(0 == dbc_report(&client, &cap), "report captcha over http");
    dbc_close_captcha(&cap);

    /* HTTP transport: recaptcha v2/v3/enterprise. */
    reset_http_queue();
    queue_http("{\"captcha\":211,\"text\":null,\"is_correct\":1}");
    queue_http("{\"captcha\":211,\"text\":\"tokv2_http\",\"is_correct\":1}");
    ok &= expect_true(0 == dbc_decode_recaptcha_v2(&client, &cap, "site", "https://example.com", NULL, NULL, 5),
                      "decode recaptcha v2 over http");
    dbc_close_captcha(&cap);

    reset_http_queue();
    queue_http("{\"captcha\":212,\"text\":null,\"is_correct\":1}");
    queue_http("{\"captcha\":212,\"text\":\"tokv3_http\",\"is_correct\":1}");
    ok &= expect_true(0 == dbc_decode_recaptcha_v3(&client, &cap, "site", "https://example.com", "login", 0.3, NULL, NULL, 5),
                      "decode recaptcha v3 over http");
    dbc_close_captcha(&cap);

    reset_http_queue();
    queue_http("{\"captcha\":213,\"text\":null,\"is_correct\":1}");
    queue_http("{\"captcha\":213,\"text\":\"tokent_http\",\"is_correct\":1}");
    ok &= expect_true(0 == dbc_decode_recaptcha_enterprise(&client, &cap, "site", "https://example.com", NULL, NULL, 5),
                      "decode recaptcha enterprise over http");
    dbc_close_captcha(&cap);

    /* Exercise response error mapping for all known server errors. */
    {
        const char *errs[] = {
            "not-logged-in", "banned", "insufficient-funds",
            "invalid-captcha", "service-overload", "unknown"
        };
        int expected[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0xff};
        for (int i = 0; i < 6; i++) {
            cJSON *resp = cJSON_CreateObject();
            cJSON_AddStringToObject(resp, "error", errs[i]);
            ok &= expect_true(expected[i] == dbc_response_error_code(&client, resp), "error mapping branch covered");
            cJSON_Delete(resp);
        }
    }

    /* Verbose socket mode: exercises connect/disconnect log paths. */
    {
        dbc_client vclient;
        ok &= expect_true(0 == dbc_init(&vclient, "vuser", "vpass"), "verbose client init");
        vclient.is_verbose = 1;
        reset_socket_queue();
        queue_socket("{\"captcha\":301,\"text\":null,\"is_correct\":1}\r\n");
        queue_socket("{\"captcha\":301,\"text\":\"verbose_tok\",\"is_correct\":1}\r\n");
        ok &= expect_true(0 == dbc_decode(&vclient, &cap, "img", 3, 5), "decode with verbose socket");
        dbc_close_captcha(&cap);
        dbc_close(&vclient);
    }

    /* Verbose HTTP mode: exercises verbose log branch in dbc_http_call. */
    {
        dbc_client vhclient;
        ok &= expect_true(0 == dbc_init(&vhclient, "vhu", "vhp"), "verbose http client init");
        ok &= expect_true(0 == dbc_set_transport(&vhclient, DBC_TRANSPORT_HTTPS), "verbose http transport");
        vhclient.is_verbose = 1;
        reset_http_queue();
        queue_http("{\"user\":400,\"balance\":3.0,\"is_banned\":0}");
        ok &= expect_true(dbc_get_balance(&vhclient) > 0.0, "get balance verbose http");
        dbc_close(&vhclient);
    }

    /* authtoken client over HTTPS: exercises dbc_http_append_auth authtoken branch. */
    {
        dbc_client tkhttp;
        ok &= expect_true(0 == dbc_init_token(&tkhttp, "my-token"), "authtoken http client init");
        ok &= expect_true(0 == dbc_set_transport(&tkhttp, DBC_TRANSPORT_HTTPS), "authtoken http transport");
        reset_http_queue();
        queue_http("{\"user\":500,\"balance\":7.7,\"is_banned\":0}");
        ok &= expect_true(dbc_get_balance(&tkhttp) > 0.0, "get balance http authtoken");
        reset_http_queue();
        queue_http("{\"captcha\":501,\"text\":null,\"is_correct\":1}");
        queue_http("{\"captcha\":501,\"text\":\"tokv2_auth\",\"is_correct\":1}");
        ok &= expect_true(0 == dbc_decode_recaptcha_v2(&tkhttp, &cap, "sk", "https://x", NULL, NULL, 5),
                          "recaptcha v2 http with authtoken");
        dbc_close_captcha(&cap);
        dbc_close(&tkhttp);
    }

    /* Transport switch HTTPS -> SOCKET: exercises curl_global_cleanup path. */
    {
        dbc_client swclient;
        ok &= expect_true(0 == dbc_init(&swclient, "sw", "sw"), "switch transport client init");
        ok &= expect_true(0 == dbc_set_transport(&swclient, DBC_TRANSPORT_HTTPS), "switch to https");
        ok &= expect_true(0 == dbc_set_transport(&swclient, DBC_TRANSPORT_SOCKET), "switch back to socket");
        dbc_close(&swclient);
    }

    /* NULL guards for top-level API functions. */
    ok &= expect_true(0.0 == dbc_get_balance(NULL), "dbc_get_balance NULL returns 0.0");
    ok &= expect_true(0 == dbc_response_error_code(&client, NULL), "response_error_code NULL response");
    {
        dbc_captcha ncap;
        ok &= expect_true(-1 == dbc_decode_file(&client, &ncap, NULL, 5),
                          "decode_file NULL stream returns -1");
    }
    {
        dbc_captcha ncap;
        ok &= expect_true(-1 == dbc_decode_recaptcha_v2(NULL, &ncap, "sk", "https://x", NULL, NULL, 5),
                          "recaptcha v2 NULL client hits internal guard");
    }

    /* reCAPTCHA with proxy/proxytype: covers optional proxy branches in v2/v3/enterprise. */
    ok &= expect_true(0 == dbc_set_transport(&client, DBC_TRANSPORT_SOCKET), "reset to socket for proxy tests");

    reset_socket_queue();
    queue_socket("{\"captcha\":311,\"text\":null,\"is_correct\":1}\r\n");
    queue_socket("{\"captcha\":311,\"text\":\"v2proxy\",\"is_correct\":1}\r\n");
    ok &= expect_true(0 == dbc_decode_recaptcha_v2(&client, &cap, "sk", "https://example.com",
                                                   "http://p:3128", "HTTP", 5),
                      "recaptcha v2 with proxy over socket");
    dbc_close_captcha(&cap);

    reset_socket_queue();
    queue_socket("{\"captcha\":312,\"text\":null,\"is_correct\":1}\r\n");
    queue_socket("{\"captcha\":312,\"text\":\"v3proxy\",\"is_correct\":1}\r\n");
    ok &= expect_true(0 == dbc_decode_recaptcha_v3(&client, &cap, "sk", "https://example.com",
                                                   "login", 0.5, "http://p:3128", "HTTP", 5),
                      "recaptcha v3 with proxy over socket");
    dbc_close_captcha(&cap);

    reset_socket_queue();
    queue_socket("{\"captcha\":313,\"text\":null,\"is_correct\":1}\r\n");
    queue_socket("{\"captcha\":313,\"text\":\"entproxy\",\"is_correct\":1}\r\n");
    ok &= expect_true(0 == dbc_decode_recaptcha_enterprise(&client, &cap, "sk", "https://example.com",
                                                          "http://p:3128", "HTTP", 5),
                      "recaptcha enterprise with proxy over socket");
    dbc_close_captcha(&cap);

    dbc_close(&token_client);
    dbc_close(&client);
    remove("harness_image.bin");

    return ok ? 0 : 1;
}
