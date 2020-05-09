
#define __C_KDF_C

#include <openssl/evp.h>
#include <openssl/kdf.h>

#include "main.h"
#include "c_kdf.h"

BEGIN_METHOD(Pbkdf2_call, GB_STRING password; GB_STRING salt; GB_LONG iterations; GB_INTEGER keylength; GB_STRING method)

#if OPENSSL_VERSION_NUMBER < 0x10002000L
    return;
#else
    int ret;
    char hash[VARG(keylength)];
    const EVP_MD *emethod;
    emethod = EVP_get_digestbyname(STRING(method));
    if (!emethod)
        return;
    memset(hash, 0, sizeof(hash));
    ret = PKCS5_PBKDF2_HMAC((const char *) STRING(password), LENGTH(password), (const unsigned char *) STRING(salt),
          LENGTH(salt), (int) VARG(iterations), emethod, VARG(keylength), (unsigned char *) hash);
    if (ret == 0)
         return;
    GB.ReturnNewString(hash, VARG(keylength));
#endif

END_METHOD

GB_DESC CPbkdf2[] = {
	GB_DECLARE_STATIC("Pbkdf2"),

	GB_STATIC_METHOD("_call", "s", Pbkdf2_call, "(Password)s(Salt)s(Iterations)i(KeyLength)i(Method)s"),

	GB_END_DECLARE
};


BEGIN_METHOD(Scrypt_call, GB_STRING password; GB_STRING salt; GB_LONG N; GB_LONG r; GB_LONG p; GB_LONG keylength)

#if OPENSSL_VERSION_NUMBER < 0x10100000L
    return;
#else
    int ret;
    uint64_t lN, lR, lP;
    size_t lKey;
    char hash[VARG(keylength)];

    lN = VARG(N);
    if (lN < 2)
        return;
    lR = VARG(r);
    if (lR == 0)
        return;
    lP = VARG(p);
    if (lP == 0)
        return;
    lKey = VARG(keylength);
    if (lKey == 0)
        return;
    if (EVP_PBE_scrypt(NULL, 0, NULL, 0, lN, lR, lP, 0, NULL, 0) == 0)
        return;
    memset(hash, 0, sizeof(hash));
    ret = EVP_PBE_scrypt((const char *) STRING(password), LENGTH(password), (const unsigned char *) STRING(salt),
          LENGTH(salt), lN, lR, lP, 0, (unsigned char *) hash, lKey);
    if (ret == 0)
         return;
    GB.ReturnNewString(hash, lKey);
#endif

END_METHOD

GB_DESC CScrypt[] = {
	GB_DECLARE_STATIC("Scrypt"),

	GB_STATIC_METHOD("_call", "s", Pbkdf2_call, "(Password)s(Salt)s(N)i(R)i(P)i(KeyLength)i"),

	GB_END_DECLARE
};

