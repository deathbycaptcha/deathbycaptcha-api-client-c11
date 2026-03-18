#ifndef TEST_FAKE_CURL_H
#define TEST_FAKE_CURL_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CURL CURL;
typedef int CURLcode;

struct curl_slist {
    char *data;
    struct curl_slist *next;
};

#define CURLE_OK 0

#define CURLOPT_URL 10002
#define CURLOPT_USERAGENT 10018
#define CURLOPT_TIMEOUT 13
#define CURLOPT_CONNECTTIMEOUT 78
#define CURLOPT_FOLLOWLOCATION 52
#define CURLOPT_SSL_VERIFYPEER 64
#define CURLOPT_SSL_VERIFYHOST 81
#define CURLOPT_WRITEFUNCTION 20011
#define CURLOPT_WRITEDATA 10001
#define CURLOPT_HTTPHEADER 10023
#define CURLOPT_POST 47
#define CURLOPT_POSTFIELDS 10015

CURL *curl_easy_init(void);
void curl_easy_cleanup(CURL *curl);
CURLcode curl_easy_setopt(CURL *curl, int option, ...);
CURLcode curl_easy_perform(CURL *curl);
char *curl_easy_escape(CURL *curl, const char *string, int length);
void curl_free(void *ptr);
const char *curl_easy_strerror(CURLcode code);

struct curl_slist *curl_slist_append(struct curl_slist *list, const char *string);
void curl_slist_free_all(struct curl_slist *list);

#define CURL_GLOBAL_DEFAULT 0
CURLcode curl_global_init(long flags);
void curl_global_cleanup(void);

#ifdef __cplusplus
}
#endif

#endif
