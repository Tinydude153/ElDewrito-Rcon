// A Library for generating a 128 bit nonce for websocket connections,
// and verifying the server response

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <openssl/sha.h>

#ifndef WSNONCE_H

// Functions are declared in wsnonce.c
// Link it at compile time and include this library to access them

// To use this function, you MUST assign it to a *variable, and call free() on
// it after use, as this function allocates memory with malloc().
// Do NOT reference the return value of this function by directly calling it for
// that very reason.
// This function returns a pointer to the location in memory that has been
// allocated.
char *genNonce();

// A function for encoding data formatted as a character array into a base64
// string.
// To use this function, you MUST assign it to a *variable, and call free() on
// it after use, as this function allocates memory with malloc().
// Do NOT reference the return value of this function by directly calling it for
// that very reason.
// This function returns a pointer to the location in memory that has been
// allocated.
char *toBase64(char *input);

// This function returns 0 if the supplied nonce corresponds to the hash
// supplied by the websocket response. It returns 1 otherwise.
int verifyResponseHash(char *nonce, char *response);

#endif

