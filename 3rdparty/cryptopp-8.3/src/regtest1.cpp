// regtest1.cpp - originally written and placed in the public domain by Wei Dai
//                regtest.cpp split into 3 files due to OOM kills by JW
//                in April 2017. A second split occured in July 2018.

#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1

#include "../include/cryptlib/cryptlib.h"
#include "../include/cryptlib/factory.h"
#include "../include/cryptlib/bench.h"
#include "../include/cryptlib/cpu.h"

#include "../include/cryptlib/crc.h"
#include "../include/cryptlib/adler32.h"
#include "../include/cryptlib/md2.h"
#include "../include/cryptlib/md5.h"
#include "../include/cryptlib/keccak.h"
#include "../include/cryptlib/sha3.h"
#include "../include/cryptlib/shake.h"
#include "../include/cryptlib/blake2.h"
#include "../include/cryptlib/sha.h"
#include "../include/cryptlib/sha3.h"
#include "../include/cryptlib/sm3.h"
#include "../include/cryptlib/hkdf.h"
#include "../include/cryptlib/tiger.h"
#include "../include/cryptlib/ripemd.h"
#include "../include/cryptlib/panama.h"
#include "../include/cryptlib/whrlpool.h"

#include "../include/cryptlib/osrng.h"
#include "../include/cryptlib/drbg.h"
#include "../include/cryptlib/darn.h"
#include "../include/cryptlib/mersenne.h"
#include "../include/cryptlib/rdrand.h"
#include "../include/cryptlib/padlkrng.h"

#include "../include/cryptlib/modes.h"
#include "../include/cryptlib/aes.h"

// Aggressive stack checking with VS2005 SP1 and above.
#if (_MSC_FULL_VER >= 140050727)
# pragma strict_gs_check (on)
#endif

#if CRYPTOPP_MSC_VERSION
# pragma warning(disable: 4505 4355)
#endif

USING_NAMESPACE(CryptoPP)

// Unkeyed ciphers
void RegisterFactories1();
// MAC ciphers
void RegisterFactories2();
// Stream ciphers
void RegisterFactories3();
// Block ciphers
void RegisterFactories4();
// Public key ciphers
void RegisterFactories5();

void RegisterFactories(Test::TestClass suites)
{
	static bool s_registered = false;
	if (s_registered)
		return;

	if ((suites & Test::Unkeyed) == Test::Unkeyed)
		RegisterFactories1();

	if ((suites & Test::SharedKeyMAC) == Test::SharedKeyMAC)
		RegisterFactories2();

	if ((suites & Test::SharedKeyStream) == Test::SharedKeyStream)
		RegisterFactories3();

	if ((suites & Test::SharedKeyBlock) == Test::SharedKeyBlock)
		RegisterFactories4();

	if ((suites & Test::PublicKey) == Test::PublicKey)
		RegisterFactories5();

	s_registered = true;
}

// Unkeyed ciphers
void RegisterFactories1()
{
	RegisterDefaultFactoryFor<HashTransformation, CRC32>();
	RegisterDefaultFactoryFor<HashTransformation, CRC32C>();
	RegisterDefaultFactoryFor<HashTransformation, Adler32>();
	RegisterDefaultFactoryFor<HashTransformation, Weak::MD5>();
	RegisterDefaultFactoryFor<HashTransformation, SHA1>();
	RegisterDefaultFactoryFor<HashTransformation, SHA224>();
	RegisterDefaultFactoryFor<HashTransformation, SHA256>();
	RegisterDefaultFactoryFor<HashTransformation, SHA384>();
	RegisterDefaultFactoryFor<HashTransformation, SHA512>();
	RegisterDefaultFactoryFor<HashTransformation, Whirlpool>();
	RegisterDefaultFactoryFor<HashTransformation, Tiger>();
	RegisterDefaultFactoryFor<HashTransformation, RIPEMD160>();
	RegisterDefaultFactoryFor<HashTransformation, RIPEMD320>();
	RegisterDefaultFactoryFor<HashTransformation, RIPEMD128>();
	RegisterDefaultFactoryFor<HashTransformation, RIPEMD256>();
	RegisterDefaultFactoryFor<HashTransformation, Weak::PanamaHash<LittleEndian> >();
	RegisterDefaultFactoryFor<HashTransformation, Weak::PanamaHash<BigEndian> >();
	RegisterDefaultFactoryFor<HashTransformation, Keccak_224>();
	RegisterDefaultFactoryFor<HashTransformation, Keccak_256>();
	RegisterDefaultFactoryFor<HashTransformation, Keccak_384>();
	RegisterDefaultFactoryFor<HashTransformation, Keccak_512>();
	RegisterDefaultFactoryFor<HashTransformation, SHA3_224>();
	RegisterDefaultFactoryFor<HashTransformation, SHA3_256>();
	RegisterDefaultFactoryFor<HashTransformation, SHA3_384>();
	RegisterDefaultFactoryFor<HashTransformation, SHA3_512>();
	RegisterDefaultFactoryFor<HashTransformation, SHAKE128>();
	RegisterDefaultFactoryFor<HashTransformation, SHAKE256>();
	RegisterDefaultFactoryFor<HashTransformation, SM3>();
	RegisterDefaultFactoryFor<HashTransformation, BLAKE2s>();
	RegisterDefaultFactoryFor<HashTransformation, BLAKE2b>();

#ifdef BLOCKING_RNG_AVAILABLE
	RegisterDefaultFactoryFor<RandomNumberGenerator, BlockingRng>();
#endif
#ifdef NONBLOCKING_RNG_AVAILABLE
	RegisterDefaultFactoryFor<RandomNumberGenerator, NonblockingRng>();
#endif
#ifdef OS_RNG_AVAILABLE
	RegisterDefaultFactoryFor<RandomNumberGenerator, AutoSeededRandomPool>();
	RegisterDefaultFactoryFor<RandomNumberGenerator, AutoSeededX917RNG<AES> >();
#endif
	RegisterDefaultFactoryFor<RandomNumberGenerator, MT19937>();
#if (CRYPTOPP_BOOL_X86)
	if (HasPadlockRNG())
		RegisterDefaultFactoryFor<RandomNumberGenerator, PadlockRNG>();
#endif
#if (CRYPTOPP_BOOL_X86 || CRYPTOPP_BOOL_X32 || CRYPTOPP_BOOL_X64)
	if (HasRDRAND())
		RegisterDefaultFactoryFor<RandomNumberGenerator, RDRAND>();
	if (HasRDSEED())
		RegisterDefaultFactoryFor<RandomNumberGenerator, RDSEED>();
#endif
#if (CRYPTOPP_BOOL_PPC32 || CRYPTOPP_BOOL_PPC64)
	if (HasDARN())
		RegisterDefaultFactoryFor<RandomNumberGenerator, DARN>();
#endif
	RegisterDefaultFactoryFor<RandomNumberGenerator, OFB_Mode<AES>::Encryption >("AES/OFB RNG");
	RegisterDefaultFactoryFor<NIST_DRBG, Hash_DRBG<SHA1> >("Hash_DRBG(SHA1)");
	RegisterDefaultFactoryFor<NIST_DRBG, Hash_DRBG<SHA256> >("Hash_DRBG(SHA256)");
	RegisterDefaultFactoryFor<NIST_DRBG, HMAC_DRBG<SHA1> >("HMAC_DRBG(SHA1)");
	RegisterDefaultFactoryFor<NIST_DRBG, HMAC_DRBG<SHA256> >("HMAC_DRBG(SHA256)");

	RegisterDefaultFactoryFor<KeyDerivationFunction, HKDF<SHA1> >();
	RegisterDefaultFactoryFor<KeyDerivationFunction, HKDF<SHA256> >();
	RegisterDefaultFactoryFor<KeyDerivationFunction, HKDF<SHA512> >();
	RegisterDefaultFactoryFor<KeyDerivationFunction, HKDF<Whirlpool> >();
}
