/*
 * curl.h - STUB de cURL (autenticacion online deshabilitada)
 * ==========================================================
 * Craft usa cURL unicamente para autenticar con el servidor publico de
 * Michael Fogleman (modo ONLINE). Para un uso personal/local (un jugador,
 * o tu propio servidor con server.py) no hace falta: por eso este stub
 * implementa la minima API que usa auth.c y devuelve siempre "fallo",
 * de modo que el cliente se conecta de forma anonima.
 *
 * Si algun dia quieres la autenticacion real, instala cURL y quita este
 * directorio (deps/curl) del include path en CMakeLists.txt.
 */
#ifndef _curl_stub_h_
#define _curl_stub_h_

#include <stddef.h>

typedef struct CURL CURL;
typedef int CURLcode;

#define CURLE_OK 0
#define CURL_GLOBAL_DEFAULT 0
#define CURLINFO_RESPONSE_CODE 0x200002

/* opciones que usa auth.c (valores irrelevantes: setopt es no-op) */
#define CURLOPT_SSL_VERIFYPEER 1
#define CURLOPT_WRITEFUNCTION 2
#define CURLOPT_WRITEDATA 3
#define CURLOPT_URL 4
#define CURLOPT_POSTFIELDS 5

static inline CURL *curl_easy_init(void) { return (CURL *)1; }
static inline CURLcode curl_easy_setopt(CURL *curl, int option, ...) {
    (void)curl; (void)option;
    return CURLE_OK;
}
static inline CURLcode curl_easy_perform(CURL *curl) {
    (void)curl;
    return 1; /* != CURLE_OK: autenticacion no disponible */
}
static inline CURLcode curl_easy_getinfo(CURL *curl, int info, ...) {
    (void)curl; (void)info;
    return 1;
}
static inline void curl_easy_cleanup(CURL *curl) { (void)curl; }
static inline CURLcode curl_global_init(long flags) {
    (void)flags;
    return CURLE_OK;
}
static inline void curl_global_cleanup(void) {}

#endif /* _curl_stub_h_ */
