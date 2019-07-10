// regtest3.cpp - originally written and placed in the public domain by Wei Dai
//                regtest.cpp split into 3 files due to OOM kills by JW
//                in April 2017. A second split occured in July 2018.

#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1

#include "../include/cryptlib/cryptlib.h"
#include "../include/cryptlib/factory.h"
#include "../include/cryptlib/bench.h"
#include "../include/cryptlib/cpu.h"

#include "../include/cryptlib/modes.h"
#include "../include/cryptlib/aria.h"
#include "../include/cryptlib/seed.h"
#include "../include/cryptlib/hight.h"
#include "../include/cryptlib/camellia.h"
#include "../include/cryptlib/shacal2.h"
#include "../include/cryptlib/tea.h"
#include "../include/cryptlib/aes.h"
#include "../include/cryptlib/tiger.h"
#include "../include/cryptlib/ccm.h"
#include "../include/cryptlib/gcm.h"
#include "../include/cryptlib/eax.h"
#include "../include/cryptlib/twofish.h"
#include "../include/cryptlib/serpent.h"
#include "../include/cryptlib/cast.h"
#include "../include/cryptlib/rc6.h"
#include "../include/cryptlib/mars.h"
#include "../include/cryptlib/kalyna.h"
#include "../include/cryptlib/threefish.h"
#include "../include/cryptlib/cham.h"
#include "../include/cryptlib/lea.h"
#include "../include/cryptlib/simeck.h"
#include "../include/cryptlib/simon.h"
#include "../include/cryptlib/speck.h"
#include "../include/cryptlib/sm4.h"
#include "../include/cryptlib/des.h"
#include "../include/cryptlib/idea.h"
#include "../include/cryptlib/rc5.h"
#include "../include/cryptlib/skipjack.h"
#include "../include/cryptlib/blowfish.h"
#include "../include/cryptlib/chachapoly.h"

// Aggressive stack checking with VS2005 SP1 and above.
#if (_MSC_FULL_VER >= 140050727)
# pragma strict_gs_check (on)
#endif

#if CRYPTOPP_MSC_VERSION
# pragma warning(disable: 4505 4355)
#endif

USING_NAMESPACE(CryptoPP)

// Shared key ciphers
void RegisterFactories4()
{
	RegisterSymmetricCipherDefaultFactories<ECB_Mode<SHACAL2> >();
	RegisterSymmetricCipherDefaultFactories<ECB_Mode<ARIA> >();
	RegisterSymmetricCipherDefaultFactories<ECB_Mode<HIGHT> >();
	RegisterSymmetricCipherDefaultFactories<CTR_Mode<HIGHT> >();
	RegisterSymmetricCipherDefaultFactories<ECB_Mode<Camellia> >();
	RegisterSymmetricCipherDefaultFactories<ECB_Mode<TEA> >();
	RegisterSymmetricCipherDefaultFactories<ECB_Mode<XTEA> >();
	RegisterSymmetricCipherDefaultFactories<ECB_Mode<AES> >();
	RegisterSymmetricCipherDefaultFactories<CBC_Mode<AES> >();
	RegisterSymmetricCipherDefaultFactories<CFB_Mode<AES> >();
	RegisterSymmetricCipherDefaultFactories<OFB_Mode<AES> >();
	RegisterSymmetricCipherDefaultFactories<CTR_Mode<AES> >();

	RegisterAuthenticatedSymmetricCipherDefaultFactories<CCM<AES> >();
	RegisterAuthenticatedSymmetricCipherDefaultFactories<GCM<AES> >();
	RegisterAuthenticatedSymmetricCipherDefaultFactories<EAX<AES> >();
	RegisterAuthenticatedSymmetricCipherDefaultFactories<ChaCha20Poly1305>();
	RegisterAuthenticatedSymmetricCipherDefaultFactories<XChaCha20Poly1305>();

	RegisterSymmetricCipherDefaultFactories<CBC_Mode<ARIA> >();  // For test vectors
	RegisterSymmetricCipherDefaultFactories<CTR_Mode<ARIA> >();
	RegisterSymmetricCipherDefaultFactories<CTR_Mode<Camellia> >();
	RegisterSymmetricCipherDefaultFactories<CTR_Mode<Twofish> >();
	RegisterSymmetricCipherDefaultFactories<CTR_Mode<Serpent> >();
	RegisterSymmetricCipherDefaultFactories<CTR_Mode<CAST256> >();
	RegisterSymmetricCipherDefaultFactories<CTR_Mode<RC6> >();
	RegisterSymmetricCipherDefaultFactories<ECB_Mode<MARS> >();
	RegisterSymmetricCipherDefaultFactories<CTR_Mode<MARS> >();
	RegisterSymmetricCipherDefaultFactories<CTR_Mode<SHACAL2> >();
	RegisterSymmetricCipherDefaultFactories<CTR_Mode<DES> >();
	RegisterSymmetricCipherDefaultFactories<CTR_Mode<DES_XEX3> >();
	RegisterSymmetricCipherDefaultFactories<CTR_Mode<DES_EDE3> >();
	RegisterSymmetricCipherDefaultFactories<CTR_Mode<IDEA> >();
	RegisterSymmetricCipherDefaultFactories<CTR_Mode<RC5> >();
	RegisterSymmetricCipherDefaultFactories<CTR_Mode<TEA> >();
	RegisterSymmetricCipherDefaultFactories<CTR_Mode<XTEA> >();
	RegisterSymmetricCipherDefaultFactories<CTR_Mode<CAST128> >();
	RegisterSymmetricCipherDefaultFactories<ECB_Mode<SKIPJACK> >();
	RegisterSymmetricCipherDefaultFactories<CTR_Mode<SKIPJACK> >();
	RegisterSymmetricCipherDefaultFactories<CBC_Mode<SKIPJACK> >();
	RegisterSymmetricCipherDefaultFactories<CTR_Mode<Blowfish> >();
	RegisterSymmetricCipherDefaultFactories<ECB_Mode<SEED> >();
	RegisterSymmetricCipherDefaultFactories<CTR_Mode<SEED> >();

	RegisterSymmetricCipherDefaultFactories<ECB_Mode<Kalyna128> >();  // Test Vectors
	RegisterSymmetricCipherDefaultFactories<CBC_Mode<Kalyna128> >();  // Test Vectors
	RegisterSymmetricCipherDefaultFactories<ECB_Mode<Kalyna256> >();  // Test Vectors
	RegisterSymmetricCipherDefaultFactories<CBC_Mode<Kalyna256> >();  // Test Vectors
	RegisterSymmetricCipherDefaultFactories<ECB_Mode<Kalyna512> >();  // Test Vectors
	RegisterSymmetricCipherDefaultFactories<CBC_Mode<Kalyna512> >();  // Test Vectors
	RegisterSymmetricCipherDefaultFactories<CTR_Mode<Kalyna128> >();  // Benchmarks
	RegisterSymmetricCipherDefaultFactories<CTR_Mode<Kalyna256> >();  // Benchmarks
	RegisterSymmetricCipherDefaultFactories<CTR_Mode<Kalyna512> >();  // Benchmarks

	RegisterSymmetricCipherDefaultFactories<ECB_Mode<Threefish256> >();  // Test Vectors
	RegisterSymmetricCipherDefaultFactories<CBC_Mode<Threefish256> >();  // Test Vectors
	RegisterSymmetricCipherDefaultFactories<ECB_Mode<Threefish512> >();  // Test Vectors
	RegisterSymmetricCipherDefaultFactories<CBC_Mode<Threefish512> >();  // Test Vectors
	RegisterSymmetricCipherDefaultFactories<ECB_Mode<Threefish1024> >(); // Test Vectors
	RegisterSymmetricCipherDefaultFactories<CBC_Mode<Threefish1024> >(); // Test Vectors
	RegisterSymmetricCipherDefaultFactories<CTR_Mode<Threefish256> >();  // Benchmarks
	RegisterSymmetricCipherDefaultFactories<CTR_Mode<Threefish512> >();  // Benchmarks
	RegisterSymmetricCipherDefaultFactories<CTR_Mode<Threefish1024> >(); // Benchmarks

	RegisterSymmetricCipherDefaultFactories<ECB_Mode<CHAM64> >();  // Test Vectors
	RegisterSymmetricCipherDefaultFactories<ECB_Mode<CHAM128> >(); // Test Vectors
	RegisterSymmetricCipherDefaultFactories<CTR_Mode<CHAM64> >();  // Benchmarks
	RegisterSymmetricCipherDefaultFactories<CTR_Mode<CHAM128> >(); // Benchmarks

	RegisterSymmetricCipherDefaultFactories<ECB_Mode<LEA> >(); // Test Vectors
	RegisterSymmetricCipherDefaultFactories<CBC_Mode<LEA> >(); // Test Vectors
	RegisterSymmetricCipherDefaultFactories<CTR_Mode<LEA> >(); // Benchmarks

	RegisterSymmetricCipherDefaultFactories<ECB_Mode<SIMECK32> >(); // Test Vectors
	RegisterSymmetricCipherDefaultFactories<CTR_Mode<SIMECK32> >(); // Benchmarks
	RegisterSymmetricCipherDefaultFactories<ECB_Mode<SIMECK64> >(); // Test Vectors
	RegisterSymmetricCipherDefaultFactories<CTR_Mode<SIMECK64> >(); // Benchmarks

	RegisterSymmetricCipherDefaultFactories<ECB_Mode<SIMON64> >();  // Test Vectors
	RegisterSymmetricCipherDefaultFactories<CBC_Mode<SIMON64> >();  // Test Vectors
	RegisterSymmetricCipherDefaultFactories<ECB_Mode<SIMON128> >(); // Test Vectors
	RegisterSymmetricCipherDefaultFactories<CBC_Mode<SIMON128> >(); // Test Vectors
	RegisterSymmetricCipherDefaultFactories<CTR_Mode<SIMON64> >();  // Benchmarks
	RegisterSymmetricCipherDefaultFactories<CTR_Mode<SIMON128> >(); // Benchmarks

	RegisterSymmetricCipherDefaultFactories<ECB_Mode<SPECK64> >();  // Test Vectors
	RegisterSymmetricCipherDefaultFactories<CBC_Mode<SPECK64> >();  // Test Vectors
	RegisterSymmetricCipherDefaultFactories<ECB_Mode<SPECK128> >(); // Test Vectors
	RegisterSymmetricCipherDefaultFactories<CBC_Mode<SPECK128> >(); // Test Vectors
	RegisterSymmetricCipherDefaultFactories<CTR_Mode<SPECK64> >();  // Benchmarks
	RegisterSymmetricCipherDefaultFactories<CTR_Mode<SPECK128> >(); // Benchmarks

	RegisterSymmetricCipherDefaultFactories<ECB_Mode<SM4> >();  // Test Vectors
	RegisterSymmetricCipherDefaultFactories<CBC_Mode<SM4> >();  // Test Vectors
	RegisterSymmetricCipherDefaultFactories<CTR_Mode<SM4> >();  // Benchmarks
}
