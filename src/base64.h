/**
 * Base64 encoder. Feel free to use however you see fit.
 * Based on implementation by Jim Meyering <metering@redhat.com>.
 */

#ifndef _C_BASE64_H_
#define _C_BASE64_H_

#ifdef __cplusplus
extern "C"
{
#endif  /* __cplusplus */

extern size_t b64encode(char **, const char *, size_t);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif  /* !_C_BASE64_H_ */
