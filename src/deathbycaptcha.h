/**
 * Death By Captcha socket API client.
 * Feel free to use however you see fit.
 */

#ifndef _C_DEATHBYCAPTCHA_H_
#define _C_DEATHBYCAPTCHA_H_

#if !defined(_WIN32) && !defined(_POSIX_C_SOURCE)
    #define _POSIX_C_SOURCE 200112L
#endif

#include <stdio.h>
#include <sys/types.h>
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
#else
    #include <pthread.h>
    #include <sys/socket.h>
    #include <netdb.h>
#endif  /* _WIN32 */

#ifdef _WIN32
    #define DBC_DLL_PUBLIC __declspec(dllexport)
#else
    #define DBC_DLL_PUBLIC __attribute__ ((visibility ("default")))
#endif  /* _WIN32 */

#ifdef __cplusplus
extern "C"
{
#endif  /* __cplusplus */


#define DBC_API_VERSION "DBC/C v4.7.1"
#define DBC_SOFTWARE_VENDOR 0

#define DBC_HOST "api.dbcapi.me"
#define DBC_HTTP_BASE_URL "https://api.dbcapi.me/api"
#define DBC_FIRST_PORT 8123
#define DBC_LAST_PORT 8123
// #define DBC_LAST_PORT 8130

#define DBC_TRANSPORT_SOCKET 0
#define DBC_TRANSPORT_HTTPS 1

#define DBC_CAPTCHA_TYPE_TOKEN 4
#define DBC_CAPTCHA_TYPE_RECAPTCHA_V3 5
#define DBC_CAPTCHA_TYPE_RECAPTCHA_V2_ENTERPRISE 25


#define DBC_TIMEOUT 60
#define DBC_TOKEN_TIMEOUT 120
#define DBC_INTERVALS_LEN 9
extern const int DBC_INTERVALS[DBC_INTERVALS_LEN];
#define DBC_DFLT_INTERVAL 3

#define DBC_TERMINATOR "\r\n"

#ifdef _WIN32
typedef SOCKET dbc_socket_t;
#define DBC_INVALID_SOCKET INVALID_SOCKET
#else
typedef int dbc_socket_t;
#define DBC_INVALID_SOCKET (-1)
#endif  /* _WIN32 */


typedef struct {
    char *username, *password, *authtoken;
    unsigned long user_id;
    unsigned int is_banned;
    unsigned int is_verbose;
    unsigned int use_https;
    double balance;
    dbc_socket_t socket;
#ifdef _WIN32
    HANDLE socket_lock;
#else
    pthread_mutex_t socket_lock;
#endif  /* _WIN32 */
    struct addrinfo *server_addr;
} dbc_client;

typedef struct {
    unsigned int id;
    unsigned int is_correct;
    char *text;
} dbc_captcha;


/**
 * Clean/free the API client up.
 */
extern DBC_DLL_PUBLIC void dbc_close(dbc_client *client);

/**
 * Initialize a new Death by Captcha socket API client using supplied
 * credentials.  Returns 0 on success, -1 of failures.
 */
extern DBC_DLL_PUBLIC int dbc_init(dbc_client *client,
                                   const char *username,
                                   const char *password);

/**
 * Initialize a new Death by Captcha socket API client using authtoken
 * credentials.  Returns 0 on success, -1 of failures.
 */
extern DBC_DLL_PUBLIC int dbc_init_token(dbc_client *client,
                                   const char *authtoken);

/**
 * Explicitly select transport: DBC_TRANSPORT_SOCKET or DBC_TRANSPORT_HTTPS.
 * Returns 0 on success, -1 on invalid input or unavailable HTTPS support.
 */
extern DBC_DLL_PUBLIC int dbc_set_transport(dbc_client *client,
                                            unsigned int transport);

/**
 * Fetch user's balance (in US cents).
 */
extern DBC_DLL_PUBLIC double dbc_get_balance(dbc_client *client);


/**
 * Clean/free the CAPTCHA instance up.
 */
extern DBC_DLL_PUBLIC void dbc_close_captcha(dbc_captcha *captcha);

/**
 * Initialize a new CAPTCHA instance.  Returns 0 on success, -1 on failures.
 */
extern DBC_DLL_PUBLIC int dbc_init_captcha(dbc_captcha *captcha);

/**
 * Fetch an uploaded CAPTCHA details.  Returns 0 on success, -1 otherwise.
 */
int DBC_DLL_PUBLIC dbc_get_captcha(dbc_client *client,
                                   dbc_captcha *captcha,
                                   unsigned int id);

/**
 * Report an incorrectly solved CAPTCHA.
 * Returns 0 on success, -1 on failures.
 */
extern DBC_DLL_PUBLIC int dbc_report(dbc_client *client, dbc_captcha *captcha);

/**
 * Solve a token-based reCAPTCHA v2 challenge.
 * `proxy` and `proxytype` are optional and may be NULL.
 */
extern DBC_DLL_PUBLIC int dbc_decode_recaptcha_v2(dbc_client *client,
                                                  dbc_captcha *captcha,
                                                  const char *sitekey,
                                                  const char *pageurl,
                                                  const char *proxy,
                                                  const char *proxytype,
                                                  unsigned int timeout);

/**
 * Solve a token-based reCAPTCHA v3 challenge.
 * `action` can be NULL, `min_score` <= 0 uses API defaults.
 * `proxy` and `proxytype` are optional and may be NULL.
 */
extern DBC_DLL_PUBLIC int dbc_decode_recaptcha_v3(dbc_client *client,
                                                  dbc_captcha *captcha,
                                                  const char *sitekey,
                                                  const char *pageurl,
                                                  const char *action,
                                                  double min_score,
                                                  const char *proxy,
                                                  const char *proxytype,
                                                  unsigned int timeout);

/**
 * Solve a token-based reCAPTCHA v2 Enterprise challenge.
 * `proxy` and `proxytype` are optional and may be NULL.
 */
extern DBC_DLL_PUBLIC int dbc_decode_recaptcha_enterprise(dbc_client *client,
                                                           dbc_captcha *captcha,
                                                           const char *sitekey,
                                                           const char *pageurl,
                                                           const char *proxy,
                                                           const char *proxytype,
                                                           unsigned int timeout);

/**
 * Generic token-based CAPTCHA decode for any type not covered by a specific
 * wrapper (e.g. Turnstile type 12, GeeTest v3 type 8, Amazon WAF type 16).
 *
 * captcha_type  - type ID (see DBC_CAPTCHA_TYPE_* or the API docs).
 * params_field  - JSON field name expected by the API (e.g. "turnstile_params").
 * params_json   - JSON object string with the challenge parameters,
 *                 e.g. "{\"sitekey\":\"...\",\"pageurl\":\"...\"}"
 * timeout       - polling timeout in seconds (0 uses DBC_TOKEN_TIMEOUT).
 *
 * Returns 0 on success, -1 on failure or timeout.
 */
extern DBC_DLL_PUBLIC int dbc_decode_token(dbc_client *client,
                                           dbc_captcha *captcha,
                                           unsigned int captcha_type,
                                           const char *params_field,
                                           const char *params_json,
                                           unsigned int timeout);

/**
 * Upload a CAPTCHA from buffer and poll for its status with desired timeout
 * (in seconds).  Returns 0 if solved; cleans the supplied CAPTCHA instance
 * up, and returns -1 on failures.
 */
extern DBC_DLL_PUBLIC int dbc_decode(dbc_client *client,
                                     dbc_captcha *captcha,
                                     const char *buf,
                                     size_t buflen,
                                     unsigned int timeout);

/**
 * Upload a CAPTCHA from a stream and poll for its status with desired timeout
 * (in seconds).  See dbc_decode() for details.
 */
extern DBC_DLL_PUBLIC int dbc_decode_file(dbc_client *client,
                                          dbc_captcha *captcha,
                                          FILE *f,
                                          unsigned int timeout);

/**
 * Returns the DBC_INTERVALS[idx] value. If idx is out of range
 * returns DFLT_POLL_INTERVAL.
 */
extern DBC_DLL_PUBLIC int dbc_get_poll_interval(int idx);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif  /* !_C_DEATHBYCAPTCHA_H_ */
