/*
 * c_openssl.c - Static OpenSSL class
 *
 * Copyright (C) 2019 Tobias Boege <tobias@gambas-buch.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL. If you modify
 * file(s) with this exception, you may extend this exception to
 * your version of the file(s), but you are not obligated to do so.
 * If you do not wish to do so, delete this exception statement
 * from your version. If you delete this exception statement from
 * all source files in the program, then also delete it here.
 */

#define __C_OPENSSL_C

#include <openssl/crypto.h>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <openssl/err.h>

#include "main.h"
#include "c_openssl.h"

/*
 * OpenSSL
 */

/**G
 * Return a cryptographically strongly pseudo-random string of the
 * given *Length*.
 **/
BEGIN_METHOD(OpenSSL_RandomBytes, GB_INTEGER length)

	char *ret = GB.TempString(NULL, VARG(length));
	int res;

	#if OPENSSL_VERSION_NUMBER < 0x10100000L
	res = RAND_pseudo_bytes((unsigned char *) ret, VARG(length));
	#else
	res = RAND_bytes((unsigned char *) ret, VARG(length));
	#endif
	if (res == -1) {
		GB.Error("Random number generator not supported");
		return;
	} else if (res == 0) {
		GB.Error(ERR_error_string(ERR_get_error(), NULL));
		return;
	}

	GB.ReturnString(ret);

END_METHOD


BEGIN_METHOD(OpenSSL_Pbkdf2, GB_STRING password; GB_STRING salt; GB_LONG iterations; GB_INTEGER keylength; GB_STRING method)

#if OPENSSL_VERSION_NUMBER < 0x10002000L
    GB.Error("Pbkdf2 not supported");
    return;
#else
    int ret;
    char hash[VARG(keylength)];
    const EVP_MD *emethod;
    emethod = EVP_get_digestbyname(STRING(method));
    if (!emethod) {
        GB.Error("Digest method not supported");
        return;
    }
    memset(hash, 0, sizeof(hash));
    ret = PKCS5_PBKDF2_HMAC((const char *) STRING(password), LENGTH(password), (const unsigned char *) STRING(salt),
          LENGTH(salt), (int) VARG(iterations), emethod, VARG(keylength), (unsigned char *) hash);
    if (ret == 0)  {
         GB.Error("Pbkdf2 call failed");
         return;
    }
    GB.ReturnNewString(hash, VARG(keylength));
#endif

END_METHOD

BEGIN_METHOD(OpenSSL_Scrypt, GB_STRING password; GB_STRING salt; GB_LONG N; GB_LONG r; GB_LONG p; GB_LONG keylength)

#if OPENSSL_VERSION_NUMBER < 0x10100000L
    GB.Error("Scrypt not supported");
    return;
#else
    int ret;
    uint64_t lN, lR, lP;
    size_t lKey;
    char hash[VARG(keylength)];

    lN = VARG(N);
    if (lN < 2) {
        GB.Error("N must be above 1");
        return;
    }
    lR = VARG(r);
    if (lR == 0) {
        GB.Error("R must be above 0");
        return;
    }
    lP = VARG(p);
    if (lP == 0) {
        GB.Error("P must be above 0");
        return;
    }
    lKey = VARG(keylength);
    if (lKey == 0) {
        GB.Error("Key must not be null");
        return;
    }
    if (EVP_PBE_scrypt(NULL, 0, NULL, 0, lN, lR, lP, 0, NULL, 0) == 0) {
        GB.Error("Scrypt prep call failed");
        return;
    }
    memset(hash, 0, sizeof(hash));
    ret = EVP_PBE_scrypt((const char *) STRING(password), LENGTH(password), (const unsigned char *) STRING(salt),
          LENGTH(salt), lN, lR, lP, 0, (unsigned char *) hash, lKey);
    if (ret == 0) {
        GB.Error("Scrypt call failed");
        return;
    }
    GB.ReturnNewString(hash, lKey);
#endif

END_METHOD

GB_DESC COpenSSL[] = {
	GB_DECLARE_STATIC("OpenSSL"),

	GB_STATIC_METHOD("RandomBytes", "s", OpenSSL_RandomBytes, "(Length)i"),

	GB_STATIC_METHOD("Pbkdf2", "s", OpenSSL_Pbkdf2, "(Password)s(Salt)s(Iterations)i(KeyLength)i(Method)s"),

	GB_STATIC_METHOD("Scrypt", "s", OpenSSL_Scrypt, "(Password)s(Salt)s(N)i(R)i(P)i(KeyLength)i"),

	GB_END_DECLARE
};
