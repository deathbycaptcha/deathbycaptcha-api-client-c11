/**
 * Death By Captcha socket API client.
 * Feel free to use however you see fit.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <stdarg.h>
#include <stdint.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#ifdef _WIN32
    #define WINVER 0x0502
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif  /* !WIN32_LEAN_AND_MEAN */
    #include <windows.h>
    #include <windef.h>
    #include <winsock2.h>
    #include <ws2tcpip.h>
#else
    #include <pthread.h>
    #include <sys/socket.h>
    #include <sys/select.h>
    #include <netdb.h>
    #include <unistd.h>
#endif  /* _WIN32 */

#include "base64.h"
#include "cJSON.h"

#include "deathbycaptcha.h"

#if defined(DBC_HAS_CURL) && DBC_HAS_CURL
    #include <curl/curl.h>
#endif

const int DBC_INTERVALS[9] = {1, 1, 2, 3, 2, 2, 3, 2, 2};

#ifndef _WIN32
typedef ssize_t dbc_io_result_t;
#else
typedef int dbc_io_result_t;
#endif  /* _WIN32 */

/**
 * Load a file into a buffer.  Returns the number of characters loaded.
 */
size_t dbc_load_file(FILE *f, char **buf)
{
    size_t buflen = 0;
    if (NULL == f) {
        fprintf(stderr, "Invalid CAPTCHA image file stream\n");
    } else {
        char *pbuf = NULL;
        int r = 0, chunk_size = 2048;
        *buf = (char *)calloc(chunk_size, sizeof(char));
        while (0 < (r = fread(*buf + buflen, sizeof(char), chunk_size, f))) {
            buflen += r;
            if (NULL == (pbuf = (char *)realloc(*buf, (buflen + chunk_size) * sizeof(char)))) {
                fprintf(stderr, "realloc(): %d\n", errno);
                buflen = 0;
                break;
            } else {
                *buf = pbuf;
            }
        }
        if (0 < buflen) {
            if (NULL == (pbuf = (char *)realloc(*buf, buflen))) {
                fprintf(stderr, "realloc(): %d\n", errno);
                buflen = 0;
            } else {
                *buf = pbuf;
            }
        }
        if (0 == buflen && NULL != *buf) {
            free(*buf);
        }
    }
    return buflen;
}

/**
 * Choose a random socket API port.
 */
unsigned short dbc_get_random_port()
{
    srand(time(NULL));
    return DBC_FIRST_PORT + (int)(((float)rand() / RAND_MAX) * (DBC_LAST_PORT - DBC_FIRST_PORT + 1));
}


/**
 * Close opened socket API connection.
 */
int dbc_disconnect(dbc_client *client)
{
    if (client->is_verbose) {
        fprintf(stderr, "%d CLOSE\n", (int)time(NULL));
    }
#ifdef _WIN32
    if (DBC_INVALID_SOCKET != client->socket) {
        shutdown(client->socket, SD_BOTH);
        closesocket(client->socket);
        client->socket = DBC_INVALID_SOCKET;
    }
#else
    if (DBC_INVALID_SOCKET != client->socket) {
        shutdown(client->socket, SHUT_RDWR);
        close(client->socket);
        client->socket = DBC_INVALID_SOCKET;
    }
#endif  /* _WIN32 */
    return client->socket == DBC_INVALID_SOCKET ? -1 : 0;
}

/**
 * Open a socket connection to the API server.
 * Returns 0 on success, -1 otherwise.
 */
int dbc_connect(dbc_client *client)
{
    struct addrinfo *sa = client->server_addr;
#ifdef _WIN32
    for (; DBC_INVALID_SOCKET == client->socket && NULL != sa; sa = sa->ai_next) {
        if (client->is_verbose) {
            fprintf(stderr, "%d CONN\n", (int)time(NULL));
        }
        client->socket = socket(sa->ai_family, sa->ai_socktype, sa->ai_protocol);
        if (DBC_INVALID_SOCKET == client->socket) {
            fprintf(stderr, "socket(): %d\n", WSAGetLastError());
        } else {
            unsigned long nbio = 1;
            ioctlsocket(client->socket, FIONBIO, &nbio);
            ((struct sockaddr_in *)(sa->ai_addr))->sin_port = htons(dbc_get_random_port());
            if (SOCKET_ERROR == connect(client->socket, sa->ai_addr, sa->ai_addrlen)) {
                int wsaerr = WSAGetLastError();
                if (WSAEWOULDBLOCK != wsaerr && WSAEINPROGRESS != wsaerr) {
                    fprintf(stderr, "connect(): %d\n", wsaerr);
                    dbc_disconnect(client);
                }
            }
        }
    }
    return (DBC_INVALID_SOCKET == client->socket) ? -1 : 0;
#else
    for (; DBC_INVALID_SOCKET == client->socket && NULL != sa; sa = sa->ai_next) {
        if (client->is_verbose) {
            fprintf(stderr, "%d CONN\n", (int)time(NULL));
        }
        client->socket = socket(sa->ai_family, sa->ai_socktype, sa->ai_protocol);
        if (DBC_INVALID_SOCKET == client->socket) {
            fprintf(stderr, "socket(): %d\n", errno);
        } else {
            fcntl(client->socket, F_SETFL, fcntl(client->socket, F_GETFL) | O_NONBLOCK);
            ((struct sockaddr_in *)(sa->ai_addr))->sin_port = htons(dbc_get_random_port());
            if (connect(client->socket, sa->ai_addr, sa->ai_addrlen)) {
                if (EINPROGRESS != errno) {
                    fprintf(stderr, "connect(): %d\n", errno);
                    dbc_disconnect(client);
                }
            }
        }
    }
    return (DBC_INVALID_SOCKET == client->socket) ? -1 : 0;
#endif  /* _WIN32 */
}

int dbc_connected(dbc_client *client)
{
    return DBC_INVALID_SOCKET != client->socket ? 1 : 0;
}

/**
 * Update client structure from API response.
 */
void dbc_update_client(dbc_client *client, cJSON *response)
{
    if (NULL != response && NULL != client) {
        cJSON *tmp = NULL;
        tmp = cJSON_GetObjectItem(response, "user");
        client->user_id = (NULL != tmp)
            ? (long) tmp->valueint
            : 0;
        client->balance = 0.0;
        client->is_banned = 0;
        if (0 < client->user_id) {
            if (NULL != (tmp = cJSON_GetObjectItem(response, "balance"))) {
                client->balance = tmp->valuedouble;
            }
            if (NULL != (tmp = cJSON_GetObjectItem(response, "is_banned"))) {
                client->is_banned = tmp->valueint;
            }
        }
    }
}

/**
 * Update CAPTCHA structure from API response.
 */
void dbc_update_captcha(dbc_captcha *captcha, cJSON *response)
{
    dbc_close_captcha(captcha);
    if (NULL != response && NULL != captcha) {
        cJSON *tmp = NULL;
        if (NULL != (tmp = cJSON_GetObjectItem(response, "captcha"))) {
            captcha->id = tmp->valueint;
            if (0 < captcha->id) {
                if (NULL != (tmp = cJSON_GetObjectItem(response, "text"))) {
                    if (cJSON_NULL != tmp->type && 0 < strlen(tmp->valuestring)) {
                        captcha->text = (char *)calloc(strlen(tmp->valuestring) + 1, sizeof(char));
                        strcpy(captcha->text, tmp->valuestring);
                    }
                }
                if (NULL != (tmp = cJSON_GetObjectItem(response, "is_correct"))) {
                    captcha->is_correct = tmp->valueint;
                }
            }
        }
    }
}

int dbc_response_error_code(dbc_client *client, cJSON *response)
{
    int err = 0x00;

    if (NULL == response) {
        return err;
    }

    dbc_update_client(client, response);

    cJSON *tmp = cJSON_GetObjectItem(response, "error");
    if (NULL != tmp && NULL != tmp->valuestring) {
        const char *errstr = tmp->valuestring;
        if (!strcmp(errstr, "not-logged-in")) {
            fprintf(stderr, "Access denied, check your credentials.\n");
            err = 0x01;
        } else if (!strcmp(errstr, "banned")) {
            fprintf(stderr, "Access denied, account is suspended.\n");
            err = 0x02;
        } else if (!strcmp(errstr, "insufficient-funds")) {
            fprintf(stderr, "CAPTCHA was rejected due to low balance.\n");
            err = 0x03;
        } else if (!strcmp(errstr, "invalid-captcha")) {
            fprintf(stderr, "CAPTCHA was rejected by the service, check if it's a valid image.\n");
            err = 0x04;
        } else if (!strcmp(errstr, "service-overload")) {
            fprintf(stderr, "CAPTCHA was rejected due to service overload, try again later.\n");
            err = 0x05;
        } else {
            fprintf(stderr, "API server error occured: %s\n", errstr);
            err = 0xff;
        }
    }

    return err;
}

#if defined(DBC_HAS_CURL) && DBC_HAS_CURL
typedef struct {
    char *data;
    size_t len;
} dbc_http_buffer;

static int dbc_buf_append(char **buf, size_t *len, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int need = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);

    if (0 > need) {
        return -1;
    }

    char *tmp = (char *)realloc(*buf, *len + (size_t)need + 1);
    if (NULL == tmp) {
        return -1;
    }
    *buf = tmp;

    va_start(ap, fmt);
    vsnprintf(*buf + *len, (size_t)need + 1, fmt, ap);
    va_end(ap);

    *len += (size_t)need;
    return 0;
}

static size_t dbc_http_write_cb(void *contents, size_t size, size_t nmemb, void *userdata)
{
    size_t realsize = size * nmemb;
    dbc_http_buffer *buf = (dbc_http_buffer *)userdata;

    char *tmp = (char *)realloc(buf->data, buf->len + realsize + 1);
    if (NULL == tmp) {
        return 0;
    }
    buf->data = tmp;

    memcpy(&(buf->data[buf->len]), contents, realsize);
    buf->len += realsize;
    buf->data[buf->len] = '\0';

    return realsize;
}

static int dbc_http_append_param(CURL *curl,
                                 char **fields,
                                 size_t *len,
                                 const char *key,
                                 const char *value)
{
    char *ekey = curl_easy_escape(curl, key, 0);
    char *eval = curl_easy_escape(curl, value, 0);
    int rc = 0;

    if (NULL == ekey || NULL == eval) {
        rc = -1;
    } else {
        rc = dbc_buf_append(fields, len, "%s%s=%s", (*len > 0) ? "&" : "", ekey, eval);
    }

    if (NULL != ekey) {
        curl_free(ekey);
    }
    if (NULL != eval) {
        curl_free(eval);
    }

    return rc;
}

static int dbc_http_append_auth(CURL *curl, dbc_client *client, char **fields, size_t *len)
{
    if (NULL != client->authtoken) {
        return dbc_http_append_param(curl, fields, len, "authtoken", client->authtoken);
    }

    if (dbc_http_append_param(curl, fields, len, "username", client->username)) {
        return -1;
    }

    return dbc_http_append_param(curl, fields, len, "password", client->password);
}

static int dbc_http_append_args(CURL *curl,
                                char **fields,
                                size_t *len,
                                cJSON *args,
                                int skip_captcha_id)
{
    cJSON *it = args->child;
    for (; NULL != it; it = it->next) {
        if (NULL == it->string || !strcmp(it->string, "cmd") || !strcmp(it->string, "version")) {
            continue;
        }
        if (skip_captcha_id && !strcmp(it->string, "captcha")) {
            continue;
        }

        if (cJSON_String == (it->type & 0xff) && NULL != it->valuestring) {
            if (dbc_http_append_param(curl, fields, len, it->string, it->valuestring)) {
                return -1;
            }
        } else if (cJSON_Number == (it->type & 0xff)) {
            char num[64] = "";
            snprintf(num, sizeof(num), "%.15g", it->valuedouble);
            if (dbc_http_append_param(curl, fields, len, it->string, num)) {
                return -1;
            }
        } else if (cJSON_True == (it->type & 0xff) || cJSON_False == (it->type & 0xff)) {
            if (dbc_http_append_param(curl, fields, len, it->string, (cJSON_True == (it->type & 0xff)) ? "1" : "0")) {
                return -1;
            }
        } else {
            char *json = cJSON_PrintUnformatted(it);
            if (NULL == json) {
                return -1;
            }
            int rc = dbc_http_append_param(curl, fields, len, it->string, json);
            free(json);
            if (rc) {
                return -1;
            }
        }
    }
    return 0;
}

static int dbc_hex_digit(char c)
{
    if ('0' <= c && '9' >= c) {
        return c - '0';
    }
    if ('a' <= c && 'f' >= c) {
        return 10 + (c - 'a');
    }
    if ('A' <= c && 'F' >= c) {
        return 10 + (c - 'A');
    }
    return -1;
}

static char *dbc_url_decode(const char *src)
{
    size_t n = strlen(src);
    char *out = (char *)calloc(n + 1, sizeof(char));
    if (NULL == out) {
        return NULL;
    }

    size_t j = 0;
    for (size_t i = 0; i < n; i++) {
        if ('+' == src[i]) {
            out[j++] = ' ';
        } else if ('%' == src[i] && i + 2 < n) {
            int hi = dbc_hex_digit(src[i + 1]);
            int lo = dbc_hex_digit(src[i + 2]);
            if (0 <= hi && 0 <= lo) {
                out[j++] = (char)((hi << 4) | lo);
                i += 2;
            } else {
                out[j++] = src[i];
            }
        } else {
            out[j++] = src[i];
        }
    }
    out[j] = '\0';
    return out;
}

static cJSON *dbc_parse_urlencoded_response(const char *body)
{
    cJSON *obj = cJSON_CreateObject();
    if (NULL == obj) {
        return NULL;
    }

    char *tmp = (char *)calloc(strlen(body) + 1, sizeof(char));
    if (NULL == tmp) {
        cJSON_Delete(obj);
        return NULL;
    }
    strcpy(tmp, body);

    int added = 0;
    char *saveptr = NULL;
    char *pair = strtok_r(tmp, "&\r\n", &saveptr);
    for (; NULL != pair; pair = strtok_r(NULL, "&\r\n", &saveptr)) {
        char *eq = strchr(pair, '=');
        if (NULL == eq) {
            continue;
        }
        *eq = '\0';

        char *key = dbc_url_decode(pair);
        char *val = dbc_url_decode(eq + 1);
        if (NULL == key || NULL == val || 0 == strlen(key)) {
            if (NULL != key) {
                free(key);
            }
            if (NULL != val) {
                free(val);
            }
            continue;
        }

        char *end = NULL;
        double num = strtod(val, &end);
        if (!strcmp(val, "null")) {
            cJSON_AddNullToObject(obj, key);
        } else if (!strcmp(val, "true")) {
            cJSON_AddBoolToObject(obj, key, 1);
        } else if (!strcmp(val, "false")) {
            cJSON_AddBoolToObject(obj, key, 0);
        } else if (NULL != end && '\0' == *end && 0 < strlen(val)) {
            cJSON_AddNumberToObject(obj, key, num);
        } else {
            cJSON_AddStringToObject(obj, key, val);
        }

        added++;
        free(key);
        free(val);
    }

    free(tmp);
    if (0 == added) {
        cJSON_Delete(obj);
        return NULL;
    }
    return obj;
}

static cJSON *dbc_http_parse_response(const char *body)
{
    cJSON *parsed = cJSON_Parse(body);
    if (NULL != parsed) {
        return parsed;
    }
    return dbc_parse_urlencoded_response(body);
}

static cJSON *dbc_http_call(dbc_client *client, const char *cmd, cJSON *args)
{
    CURL *curl = curl_easy_init();
    if (NULL == curl) {
        fprintf(stderr, "curl_easy_init(): failed\n");
        return NULL;
    }

    char *url = NULL;
    size_t url_len = 0;
    char *fields = NULL;
    size_t fields_len = 0;
    struct curl_slist *headers = NULL;
    dbc_http_buffer http_buf = {0};

    int is_post = 1;
    unsigned int captcha_id = 0;

    if (!strcmp(cmd, "login") || !strcmp(cmd, "user")) {
        if (dbc_buf_append(&url, &url_len, "%s/", DBC_HTTP_BASE_URL)) {
            goto cleanup;
        }
        if (dbc_http_append_auth(curl, client, &fields, &fields_len)) {
            goto cleanup;
        }
    } else if (!strcmp(cmd, "upload")) {
        if (dbc_buf_append(&url, &url_len, "%s/captcha", DBC_HTTP_BASE_URL)) {
            goto cleanup;
        }
        if (dbc_http_append_auth(curl, client, &fields, &fields_len)) {
            goto cleanup;
        }
        if (dbc_http_append_args(curl, &fields, &fields_len, args, 0)) {
            goto cleanup;
        }
    } else if (!strcmp(cmd, "captcha")) {
        cJSON *tmp = cJSON_GetObjectItem(args, "captcha");
        if (NULL == tmp || cJSON_Number != (tmp->type & 0xff) || 0 >= tmp->valueint) {
            goto cleanup;
        }
        captcha_id = (unsigned int)tmp->valueint;
        if (dbc_buf_append(&url, &url_len, "%s/captcha/%u", DBC_HTTP_BASE_URL, captcha_id)) {
            goto cleanup;
        }
        if (dbc_http_append_auth(curl, client, &fields, &fields_len)) {
            goto cleanup;
        }
        if (dbc_buf_append(&url, &url_len, "?%s", fields)) {
            goto cleanup;
        }
        is_post = 0;
    } else if (!strcmp(cmd, "report")) {
        cJSON *tmp = cJSON_GetObjectItem(args, "captcha");
        if (NULL == tmp || cJSON_Number != (tmp->type & 0xff) || 0 >= tmp->valueint) {
            goto cleanup;
        }
        captcha_id = (unsigned int)tmp->valueint;
        if (dbc_buf_append(&url, &url_len, "%s/captcha/%u/report", DBC_HTTP_BASE_URL, captcha_id)) {
            goto cleanup;
        }
        if (dbc_http_append_auth(curl, client, &fields, &fields_len)) {
            goto cleanup;
        }
    } else {
        goto cleanup;
    }

    if (client->is_verbose) {
        fprintf(stderr, "%d HTTP %s: %s\n", (int)time(NULL), is_post ? "POST" : "GET", cmd);
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, DBC_API_VERSION);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, (long)DBC_TIMEOUT);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, (long)DBC_TIMEOUT);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, dbc_http_write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &http_buf);

    if (is_post) {
        headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, fields);
    }

    CURLcode cres = curl_easy_perform(curl);
    if (CURLE_OK != cres) {
        fprintf(stderr, "curl_easy_perform(): %s\n", curl_easy_strerror(cres));
        goto cleanup;
    }

    if (NULL == http_buf.data || 0 == http_buf.len) {
        goto cleanup;
    }

cleanup:
    if (NULL != headers) {
        curl_slist_free_all(headers);
    }
    if (NULL != curl) {
        curl_easy_cleanup(curl);
    }
    if (NULL != url) {
        free(url);
    }
    if (NULL != fields) {
        free(fields);
    }

    cJSON *result = NULL;
    if (NULL != http_buf.data) {
        result = dbc_http_parse_response(http_buf.data);
        if (NULL == result) {
            fprintf(stderr, "Failed parsing HTTPS API response\n");
        }
        free(http_buf.data);
    }

    return result;
}
#endif  /* DBC_HAS_CURL */

cJSON *dbc_send_and_recv(dbc_client *client, const char *sbuf)
{
    cJSON *response = NULL;

    struct timeval tv;
    int r = 0;
    size_t sent = 0, sbuflen = strlen(sbuf),
           received = 0, rchunk = 256;
    char *rbuf = NULL;

    if (client->is_verbose) {
        fprintf(stderr, "%d SEND: %d %s\n", (int)time(NULL), (int)sbuflen, sbuf);
    }

    if (dbc_connect(client)) {
        return NULL;
    } else {
        rbuf = (char *)calloc(rchunk, sizeof(char));
    }

    int intvl_idx = 0;

    while (1) {
        int intvl = dbc_get_poll_interval(intvl_idx++);
        fd_set rd, wr, ex;
        FD_ZERO(&rd);
        FD_ZERO(&wr);
        if (sbuflen > sent) {
            FD_SET(client->socket, &wr);
        } else {
            FD_SET(client->socket, &rd);
        }
        FD_ZERO(&ex);
        FD_SET(client->socket, &ex);

        tv.tv_sec = 4 * intvl;
        tv.tv_usec = 0;

        if (-1 == (r = select(client->socket + 1, &rd, &wr, &ex, &tv))) {
            fprintf(stderr, "select(): %d\n", errno);
            break;
        } else if (0 == r) {
            /* select() timed out */
            continue;
        } else if (FD_ISSET(client->socket, &ex)) {
            fprintf(stderr, "select(): exception\n");
            break;
        } else if (FD_ISSET(client->socket, &wr)) {
            dbc_io_result_t n = 0;
            while (sbuflen > sent && 0 < (n = send(client->socket, &(sbuf[sent]), sbuflen - sent, 0))) {
                sent += (size_t)n;
            }
            if (-1 == n) {
#ifdef _WIN32
                int wsaerr = WSAGetLastError();
                if (WSAEWOULDBLOCK != wsaerr) {
                    fprintf(stderr, "send(): %d\n", wsaerr);
#else
                if (EAGAIN != errno && EWOULDBLOCK != errno) {
                    fprintf(stderr, "send(): %d\n", errno);
#endif  /* _WIN32 */

                    break;
                }
            }
        } else if (FD_ISSET(client->socket, &rd)) {
            dbc_io_result_t n = 0;
            while (0 < (n = recv(client->socket, &(rbuf[received]), rchunk, 0))) {
                received += (size_t)n;
                if ('\r' == rbuf[received - 2] && '\n' == rbuf[received - 1]) {
                    rbuf = (char *)realloc(rbuf, (received + 1) * sizeof(char));
                    rbuf[received] = '\0';
                    break;
                } else {
                    rbuf = (char *)realloc(rbuf, (received + rchunk) * sizeof(char));
                }
            }
            if (-1 == n) {
#ifdef _WIN32
                int wsaerr = WSAGetLastError();
                if (WSAEWOULDBLOCK != wsaerr) {
                    fprintf(stderr, "recv(): %d\n", wsaerr);
#else
                if (EAGAIN != errno && EWOULDBLOCK != errno) {
                    fprintf(stderr, "recv(): %d\n", errno);
#endif  /* _WIN32 */
                    break;
                }
            } else if (2 <= received && '\r' == rbuf[received - 2] && '\n' == rbuf[received - 1]) {
                if (client->is_verbose) {
                    fprintf(stderr, "%d RECV: %d %s\n", (int)time(NULL), (int)received, rbuf);
                }
                break;
            } else if (0 == received) {
                break;
            }
        }
    }

    if (0 < received) {
        if (NULL == (response = cJSON_Parse(rbuf))) {
            printf("cJSON_Parse null\n");
            dbc_disconnect(client);
            fprintf(stderr, "Failed parsing API response\n");
            response = cJSON_CreateObject();
        }
    } else {
        dbc_disconnect(client);
        fprintf(stderr, "Connection lost\n");
    }

    free(rbuf);
    return response;
}

/**
 * Make a Death by Captcha API call.
 * Takes the active client, API command name, request arguments (can be NULL).
 * Returns API response on success, or NULL.
 */
cJSON *dbc_call(dbc_client *client, const char *cmd, cJSON *args)
{
    if (client->is_verbose) {
        printf("$$$$$$$$$$ dbc_call %s\n", cmd);
    }
    int err = 0x00;
    int attempts = 2;

    cJSON *response = NULL;
    char *sbuf = NULL;

    int is_args_local = 0;
    if (NULL == args) {
        is_args_local = 1;
        args = cJSON_CreateObject();
    }
    cJSON_AddStringToObject(args, "cmd", cmd);
    cJSON_AddStringToObject(args, "version", DBC_API_VERSION);

    sbuf = cJSON_PrintUnformatted(args);
    sbuf = (char *)realloc(sbuf, (strlen(sbuf) + 3) * sizeof(char));
    sbuf = strcat(sbuf, DBC_TERMINATOR);

    while (0 < attempts && NULL == response && 0x00 == err) {
        attempts--;

        if (!client->use_https && !dbc_connected(client) && strcmp(cmd, "login")) {
            cJSON *auth = cJSON_CreateObject();
            if (client->authtoken != NULL){
                cJSON_AddStringToObject(auth, "authtoken", client->authtoken);
            }else{
                cJSON_AddStringToObject(auth, "username", client->username);
                cJSON_AddStringToObject(auth, "password", client->password);
            }
            dbc_call(client, "login", auth);
        }

#ifdef _WIN32
        if (WAIT_OBJECT_0 == WaitForSingleObject(client->socket_lock, INFINITE)) {
#else
        if (!pthread_mutex_lock(&(client->socket_lock))) {
#endif  /* _WIN32 */
            if (client->use_https) {
#if defined(DBC_HAS_CURL) && DBC_HAS_CURL
                response = dbc_http_call(client, cmd, args);
#else
                fprintf(stderr, "HTTPS transport requested but this build has no libcurl support\n");
                response = NULL;
#endif
            } else {
                response = dbc_send_and_recv(client, sbuf);
            }

            if (NULL == response) {
                /* Worth retrying */
            } else {
                err = dbc_response_error_code(client, response);
            }
#ifdef _WIN32
            ReleaseMutex(client->socket_lock);
#else
            pthread_mutex_unlock(&(client->socket_lock));
#endif  /* _WIN32 */
        }
    }

    free(sbuf);

    if (is_args_local) {
        cJSON_Delete(args);
    }

    if (0x00 != err) {
        if (NULL != response) {
            cJSON_Delete(response);
            response = NULL;
        }
    }

    return response;
}

void dbc_close(dbc_client *client)
{
    if (NULL != client) {
        if (NULL != client->username) {
            free(client->username);
            client->username = NULL;
        }
        if (NULL != client->password) {
            free(client->password);
            client->password = NULL;
        }
        if (NULL != client->authtoken) {
            free(client->authtoken);
            client->authtoken = NULL;
        }
        if (NULL != client->server_addr) {
            freeaddrinfo(client->server_addr);
            client->server_addr = NULL;
        }
        dbc_disconnect(client);
#ifdef _WIN32
        WSACleanup();
        CloseHandle(client->socket_lock);
#else
        pthread_mutex_destroy(&(client->socket_lock));
#endif  /* _WIN32 */

#if defined(DBC_HAS_CURL) && DBC_HAS_CURL
        if (client->use_https) {
            curl_global_cleanup();
        }
#endif
    }
}

int dbc_init(dbc_client *client, const char *username, const char *password)
{
#ifdef _WIN32
    struct WSAData wsad;
    if (WSAStartup(MAKEWORD(2, 0), &wsad)) {
        fprintf(stderr, "WSAStartup(): %d\n", WSAGetLastError());
        return -1;
    }
#endif  /* _WIN32 */

    memset(client, 0, sizeof(dbc_client));
    if (NULL == username || !strlen(username)) {
        fprintf(stderr, "Username is required\n");
    } else if (NULL == password || !strlen(password)) {
        fprintf(stderr, "Password is required\n");
    } else {
        int err;
        char *port = (char *)calloc(6, sizeof(char));
        struct addrinfo hints;
        memset(&hints, 0, sizeof(struct addrinfo));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = 0;
        hints.ai_protocol = 0;
        sprintf(port, "%d", dbc_get_random_port());
        if (0 != (err = getaddrinfo(DBC_HOST, port, &hints, &(client->server_addr)))) {
            fprintf(stderr, "getaddrinfo(): %d %s\n", err, gai_strerror(err));
            client->server_addr = NULL;
        }
        free(port);
        if (NULL != client->server_addr) {
#ifdef _WIN32
            SECURITY_ATTRIBUTES lock_sec;
            lock_sec.nLength = sizeof(SECURITY_ATTRIBUTES);
            lock_sec.lpSecurityDescriptor = NULL;
            lock_sec.bInheritHandle = TRUE;
            if (NULL == (client->socket_lock = CreateMutex(&lock_sec, FALSE, NULL))) {
                fprintf(stderr, "CreateMutex(): %d\n", (int)GetLastError());
#else
            if (pthread_mutex_init(&(client->socket_lock), NULL)) {
                fprintf(stderr, "pthread_mutex_init(): %d\n", errno);
#endif  /* _WIN32 */
            } else {
                client->socket = DBC_INVALID_SOCKET;
                client->use_https = 0;
                client->username = (char *)calloc(strlen(username) + 1, sizeof(char));
                strcpy(client->username, username);
                client->password = (char *)calloc(strlen(password) + 1, sizeof(char));
                strcpy(client->password, password);
                return 0;
            }
        }
    }

#ifdef _WIN32
    WSACleanup();
#endif  /* _WIN32 */
    return -1;
}

int dbc_init_token(dbc_client *client, const char *authtoken)
{
#ifdef _WIN32
    struct WSAData wsad;
    if (WSAStartup(MAKEWORD(2, 0), &wsad)) {
        fprintf(stderr, "WSAStartup(): %d\n", WSAGetLastError());
        return -1;
    }
#endif  /* _WIN32 */

    memset(client, 0, sizeof(dbc_client));
    if (NULL == authtoken || !strlen(authtoken)) {
        fprintf(stderr, "Authentication token is required\n");
    } else {
        int err;
        char *port = (char *)calloc(6, sizeof(char));
        struct addrinfo hints;
        memset(&hints, 0, sizeof(struct addrinfo));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = 0;
        hints.ai_protocol = 0;
        sprintf(port, "%d", dbc_get_random_port());
        if (0 != (err = getaddrinfo(DBC_HOST, port, &hints, &(client->server_addr)))) {
            fprintf(stderr, "getaddrinfo(): %d %s\n", err, gai_strerror(err));
            client->server_addr = NULL;
        }
        free(port);
        if (NULL != client->server_addr) {
#ifdef _WIN32
            SECURITY_ATTRIBUTES lock_sec;
            lock_sec.nLength = sizeof(SECURITY_ATTRIBUTES);
            lock_sec.lpSecurityDescriptor = NULL;
            lock_sec.bInheritHandle = TRUE;
            if (NULL == (client->socket_lock = CreateMutex(&lock_sec, FALSE, NULL))) {
                fprintf(stderr, "CreateMutex(): %d\n", (int)GetLastError());
#else
            if (pthread_mutex_init(&(client->socket_lock), NULL)) {
                fprintf(stderr, "pthread_mutex_init(): %d\n", errno);
#endif  /* _WIN32 */
            } else {
                client->socket = DBC_INVALID_SOCKET;
                client->use_https = 0;
                client->authtoken = (char *)calloc(strlen(authtoken) + 1, sizeof(char));
                strcpy(client->authtoken, authtoken);
                return 0;
            }
        }
    }

#ifdef _WIN32
    WSACleanup();
#endif  /* _WIN32 */
    return -1;
}

int dbc_set_transport(dbc_client *client, unsigned int transport)
{
    if (NULL == client) {
        return -1;
    }

    if (DBC_TRANSPORT_SOCKET == transport) {
#if defined(DBC_HAS_CURL) && DBC_HAS_CURL
        if (client->use_https) {
            curl_global_cleanup();
        }
#endif
        client->use_https = 0;
        return 0;
    }

    if (DBC_TRANSPORT_HTTPS == transport) {
#if defined(DBC_HAS_CURL) && DBC_HAS_CURL
        if (CURLE_OK != curl_global_init(CURL_GLOBAL_DEFAULT)) {
            fprintf(stderr, "curl_global_init(): failed\n");
            return -1;
        }
        client->use_https = 1;
        return 0;
#else
        fprintf(stderr, "HTTPS transport is unavailable: built without libcurl support\n");
        return -1;
#endif
    }

    return -1;
}

double dbc_get_balance(dbc_client *client)
{
    if (NULL == client) {
        return 0.0;
    } else {
        // cJSON_Delete((cJSON *)dbc_call(client, "user", NULL));
        dbc_call(client, "user", NULL);
        return client->balance;
    }
}


void dbc_close_captcha(dbc_captcha *captcha)
{
    if (NULL != captcha) {
        captcha->id = 0;
        captcha->is_correct = 1;
        if (NULL != captcha->text) {
            free(captcha->text);
            captcha->text = NULL;
        }
    }
}

int dbc_init_captcha(dbc_captcha *captcha)
{
    if (NULL != captcha) {
        memset(captcha, 0, sizeof(dbc_captcha));
        dbc_close_captcha(captcha);
        return 0;
    } else {
        return -1;
    }
}

int dbc_get_captcha(dbc_client *client,
                    dbc_captcha *captcha,
                    unsigned int id)
{
    if (NULL != client && NULL != captcha && 0 < id) {
        cJSON *response = NULL;
        cJSON *args = cJSON_CreateObject();
        cJSON_AddNumberToObject(args, "captcha", id);
        response = dbc_call(client, "captcha", args);
        dbc_update_captcha(captcha, response);
        cJSON_Delete(args);
        cJSON_Delete(response);
        if (0 < captcha->id) {
            return 0;
        }
    }
    return -1;
}

int dbc_report(dbc_client *client, dbc_captcha *captcha)
{
    if (NULL != client && NULL != captcha && 0 < captcha->id) {
        cJSON *response = NULL;
        cJSON *args = cJSON_CreateObject();
        cJSON_AddNumberToObject(args, "captcha", captcha->id);
        if (NULL != (response = dbc_call(client, "report", args))) {
            dbc_update_captcha(captcha, response);
            cJSON_Delete(response);
        }
        cJSON_Delete(args);
        if (!captcha->is_correct) {
            return 0;
        }
    }
    return -1;
}


/**
 * Upload a CAPTCHA from buffer.
 * Returns 0 on success, cleans up CAPTCHA instance returns -1 if failed.
 */
int dbc_upload(dbc_client *client,
               dbc_captcha *captcha,
               const char *buf,
               size_t buflen)
{
    dbc_init_captcha(captcha);
    if (NULL != client && NULL != buf && 0 < buflen) {
        cJSON *args = cJSON_CreateObject();
        if (NULL != args) {
            cJSON *response = NULL;
            char *encoded_buf = NULL;
            b64encode(&encoded_buf, (const char *)buf, buflen);
            if (NULL != encoded_buf) {
                cJSON_AddStringToObject(args, "captcha", encoded_buf);
                cJSON_AddNumberToObject(args, "swid", DBC_SOFTWARE_VENDOR);
                response = dbc_call(client, "upload", args);
                dbc_update_captcha(captcha, response);
                cJSON_Delete(response);
                free(encoded_buf);
                encoded_buf = NULL;
            }
            cJSON_Delete(args);
        }
        if (0 < captcha->id) {
            return 0;
        }
    }
    dbc_close_captcha(captcha);
    return -1;
}

/**
 * Upload a CAPTCHA from file stream.  See dbc_upload() for details.
 */
int dbc_upload_file(dbc_client *client,
                    dbc_captcha *captcha,
                    FILE *f)
{
    int result = -1;
    if (NULL != client && NULL != f) {
        char *buf = NULL;
        size_t buflen = dbc_load_file(f, &buf);
        if (0 < buflen) {
            result = dbc_upload(client, captcha, buf, buflen);
            free(buf);
            buf = NULL;
        }
    }
    return result;
}

static int dbc_wait_captcha_result(dbc_client *client,
                                   dbc_captcha *captcha,
                                   unsigned int timeout)
{
    int deadline = time(NULL) + (0 < timeout ? timeout : DBC_TIMEOUT);
    int intvl_idx = 0;

    while (deadline > time(NULL) && NULL == captcha->text) {
        int intvl = dbc_get_poll_interval(intvl_idx++);

#ifdef _WIN32
        Sleep(intvl * 1000);
#else
        sleep(intvl);
#endif  /* _WIN32 */

        if (dbc_get_captcha(client, captcha, captcha->id)) {
            break;
        }
    }

    if (NULL == captcha->text || 0 == captcha->is_correct) {
        dbc_close_captcha(captcha);
    }

    return NULL != captcha->text ? 0 : -1;
}

static int dbc_upload_token_challenge(dbc_client *client,
                                      dbc_captcha *captcha,
                                      unsigned int captcha_type,
                                      const char *params_field,
                                      cJSON *params)
{
    dbc_init_captcha(captcha);

    if (NULL == client || NULL == params_field || NULL == params) {
        dbc_close_captcha(captcha);
        return -1;
    }

    int result = -1;
    cJSON *args = cJSON_CreateObject();
    if (NULL != args) {
        cJSON *response = NULL;
        cJSON *params_copy = cJSON_Duplicate(params, 1);
        if (NULL != params_copy) {
            cJSON_AddNumberToObject(args, "type", (double)captcha_type);
            cJSON_AddNumberToObject(args, "swid", DBC_SOFTWARE_VENDOR);
            cJSON_AddItemToObject(args, params_field, params_copy);

            response = dbc_call(client, "upload", args);
            dbc_update_captcha(captcha, response);
            cJSON_Delete(response);

            if (0 < captcha->id) {
                result = 0;
            }
        }
        cJSON_Delete(args);
    }

    if (0 != result) {
        dbc_close_captcha(captcha);
    }

    return result;
}


int dbc_decode(dbc_client *client,
               dbc_captcha *captcha,
               const char *buf,
               size_t buflen,
               unsigned int timeout)
{
    if (!dbc_upload(client, captcha, buf, buflen)) {
        return dbc_wait_captcha_result(client, captcha, timeout);
    }
    return -1;
}

int dbc_decode_file(dbc_client *client,
                    dbc_captcha *captcha,
                    FILE *f,
                    unsigned int timeout)
{
    int result = -1;
    char *buf = NULL;
    size_t buflen = dbc_load_file(f, &buf);
    result = dbc_decode(client, captcha, buf, buflen, timeout);
    if (NULL != buf) {
        free(buf);
    }
    return result;
}

int dbc_decode_recaptcha_v2(dbc_client *client,
                            dbc_captcha *captcha,
                            const char *sitekey,
                            const char *pageurl,
                            const char *proxy,
                            const char *proxytype,
                            unsigned int timeout)
{
    if (NULL == sitekey || NULL == pageurl || 0 == strlen(sitekey) || 0 == strlen(pageurl)) {
        return -1;
    }

    cJSON *params = cJSON_CreateObject();
    if (NULL == params) {
        return -1;
    }

    cJSON_AddStringToObject(params, "googlekey", sitekey);
    cJSON_AddStringToObject(params, "pageurl", pageurl);
    if (NULL != proxy && 0 < strlen(proxy)) {
        cJSON_AddStringToObject(params, "proxy", proxy);
    }
    if (NULL != proxytype && 0 < strlen(proxytype)) {
        cJSON_AddStringToObject(params, "proxytype", proxytype);
    }

    int result = -1;
    if (!dbc_upload_token_challenge(client,
                                    captcha,
                                    DBC_CAPTCHA_TYPE_TOKEN,
                                    "token_params",
                                    params)) {
        result = dbc_wait_captcha_result(client, captcha, timeout);
    }

    cJSON_Delete(params);
    return result;
}

int dbc_decode_recaptcha_v3(dbc_client *client,
                            dbc_captcha *captcha,
                            const char *sitekey,
                            const char *pageurl,
                            const char *action,
                            double min_score,
                            const char *proxy,
                            const char *proxytype,
                            unsigned int timeout)
{
    if (NULL == sitekey || NULL == pageurl || 0 == strlen(sitekey) || 0 == strlen(pageurl)) {
        return -1;
    }

    cJSON *params = cJSON_CreateObject();
    if (NULL == params) {
        return -1;
    }

    cJSON_AddStringToObject(params, "googlekey", sitekey);
    cJSON_AddStringToObject(params, "pageurl", pageurl);
    if (NULL != action && 0 < strlen(action)) {
        cJSON_AddStringToObject(params, "action", action);
    }
    if (0.0 < min_score) {
        cJSON_AddNumberToObject(params, "min_score", min_score);
    }
    if (NULL != proxy && 0 < strlen(proxy)) {
        cJSON_AddStringToObject(params, "proxy", proxy);
    }
    if (NULL != proxytype && 0 < strlen(proxytype)) {
        cJSON_AddStringToObject(params, "proxytype", proxytype);
    }

    int result = -1;
    if (!dbc_upload_token_challenge(client,
                                    captcha,
                                    DBC_CAPTCHA_TYPE_RECAPTCHA_V3,
                                    "token_params",
                                    params)) {
        result = dbc_wait_captcha_result(client, captcha, timeout);
    }

    cJSON_Delete(params);
    return result;
}

int dbc_decode_recaptcha_enterprise(dbc_client *client,
                                    dbc_captcha *captcha,
                                    const char *sitekey,
                                    const char *pageurl,
                                    const char *proxy,
                                    const char *proxytype,
                                    unsigned int timeout)
{
    if (NULL == sitekey || NULL == pageurl || 0 == strlen(sitekey) || 0 == strlen(pageurl)) {
        return -1;
    }

    cJSON *params = cJSON_CreateObject();
    if (NULL == params) {
        return -1;
    }

    cJSON_AddStringToObject(params, "googlekey", sitekey);
    cJSON_AddStringToObject(params, "pageurl", pageurl);
    if (NULL != proxy && 0 < strlen(proxy)) {
        cJSON_AddStringToObject(params, "proxy", proxy);
    }
    if (NULL != proxytype && 0 < strlen(proxytype)) {
        cJSON_AddStringToObject(params, "proxytype", proxytype);
    }

    int result = -1;
    if (!dbc_upload_token_challenge(client,
                                    captcha,
                                    DBC_CAPTCHA_TYPE_RECAPTCHA_V2_ENTERPRISE,
                                    "token_enterprise_params",
                                    params)) {
        result = dbc_wait_captcha_result(client, captcha, timeout);
    }

    cJSON_Delete(params);
    return result;
}

int dbc_get_poll_interval(int idx)
{
  int intvl = 0;

  if (DBC_INTERVALS_LEN > idx) {
    intvl = DBC_INTERVALS[idx];
  }
  else {
    intvl = DBC_DFLT_INTERVAL;
  }

  return intvl;
}
