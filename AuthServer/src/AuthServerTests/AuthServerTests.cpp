// AuthServerTests.cpp: Unit tests for AuthServer.

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

// ----------------------------------------------------------------------------
// AuthServer-specific libraries.
// ----------------------------------------------------------------------------

// cryptLib implements SHA2-512 for some reason, rather than just using the
// one in Crypto++. AuthServer has a DES implementation inside, too, which is
// also questionable.
//
// Seriously, we should replace these with calls into an existing library, but
// does anything depend on potentially broken functionality?

#include "cryptLib/sha512.h"

using namespace cryptLib;

TEST_CASE("AuthServer SHA-512", "[crypto]")
{
    sha512 hash;
    digest512 digest;

    // Test vectors from https://www.di-mgt.com.au/sha_testvectors.html
    hash.GetMessageDigest(digest, "abc");
    REQUIRE(digest.ToString() == "ddaf35a193617abacc417349ae20413112e6fa4e89a97ea20a9eeee64b55d39a2192992a274fc1a836ba3c23a3feebbd454d4423643ce80e2a9ac94fa54ca49f");

    hash.GetMessageDigest(digest, "");
    REQUIRE(digest.ToString() == "cf83e1357eefb8bdf1542850d66d8007d620e4050b5715dc83f4a921d36ce9ce47d0d13c5d85f2b0ff8318d2877eec2f63b931bd47417a81a538327af927da3e");

    hash.GetMessageDigest(digest, "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq");
    REQUIRE(digest.ToString() == "204a8fc6dda82f0a0ced7beb8e08a41657c16ef468b228a8279be331a703c33596fd15c13b1b07f9aa1d3bea57789ca031ad85c7a71dd70354ec631238ca3445");

    hash.GetMessageDigest(digest, "abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu");
    REQUIRE(digest.ToString() == "8e959b75dae313da8cf4f72814fc143f8f7779c6eb9f7fa17299aeadb6889018501d289e4900f7e4331b99dec4b5433ac7d329eeb6dd26545e96e55b874be909");
}

#define MILLION 1000000

TEST_CASE("AuthServer SHA-512 (long string)", "[crypto][long]")
{
    sha512 hash;
    digest512 digest;

    // Test vector from https://www.di-mgt.com.au/sha_testvectors.html
    char *a_million = new char[MILLION];
	for (size_t i = 0; i < MILLION; i++) {
        a_million[i] = 'a';
    }
    string a_string = std::string(a_million, MILLION);

    hash.GetMessageDigest(digest, a_string);
    REQUIRE(digest.ToString() == "e718483d0ce769644e2e42c7bc15b4638e1f98b13b2044285632a803afa973ebde0ff244877ea60a4cb0432ce577c31beb009c5c2c49aa2e4eadb217ad8cc09b");

    delete[] a_million;
}
