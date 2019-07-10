// regtest2.cpp - originally written and placed in the public domain by Wei Dai
//                regtest.cpp split into 3 files due to OOM kills by JW
//                in April 2017. A second split occured in July 2018.

#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1

#include "../include/cryptlib/cryptlib.h"
#include "../include/cryptlib/factory.h"
#include "../include/cryptlib/bench.h"
#include "../include/cryptlib/cpu.h"

// For MAC's
#include "../include/cryptlib/hmac.h"
#include "../include/cryptlib/cmac.h"
#include "../include/cryptlib/dmac.h"
#include "../include/cryptlib/vmac.h"
#include "../include/cryptlib/ttmac.h"

// Ciphers
#include "../include/cryptlib/md5.h"
#include "../include/cryptlib/keccak.h"
#include "../include/cryptlib/sha.h"
#include "../include/cryptlib/sha3.h"
#include "../include/cryptlib/blake2.h"
#include "../include/cryptlib/ripemd.h"
#include "../include/cryptlib/chacha.h"
#include "../include/cryptlib/poly1305.h"
#include "../include/cryptlib/siphash.h"
#include "../include/cryptlib/panama.h"

// Stream ciphers
#include "../include/cryptlib/arc4.h"
#include "../include/cryptlib/seal.h"
#include "../include/cryptlib/wake.h"
#include "../include/cryptlib/chacha.h"
#include "../include/cryptlib/salsa.h"
#include "../include/cryptlib/rabbit.h"
#include "../include/cryptlib/hc128.h"
#include "../include/cryptlib/hc256.h"
#include "../include/cryptlib/panama.h"
#include "../include/cryptlib/sosemanuk.h"

// Block for CMAC
#include "aes.h"
#include "des.h"

// Aggressive stack checking with VS2005 SP1 and above.
#if (_MSC_FULL_VER >= 140050727)
# pragma strict_gs_check (on)
#endif

#if CRYPTOPP_MSC_VERSION
# pragma warning(disable: 4505 4355)
#endif

USING_NAMESPACE(CryptoPP)

// MAC ciphers
void RegisterFactories2()
{
	RegisterDefaultFactoryFor<MessageAuthenticationCode, HMAC<Weak::MD5> >();
	RegisterDefaultFactoryFor<MessageAuthenticationCode, HMAC<RIPEMD160> >();
	RegisterDefaultFactoryFor<MessageAuthenticationCode, HMAC<SHA1> >();
	RegisterDefaultFactoryFor<MessageAuthenticationCode, HMAC<SHA224> >();
	RegisterDefaultFactoryFor<MessageAuthenticationCode, HMAC<SHA256> >();
	RegisterDefaultFactoryFor<MessageAuthenticationCode, HMAC<SHA384> >();
	RegisterDefaultFactoryFor<MessageAuthenticationCode, HMAC<SHA512> >();
	RegisterDefaultFactoryFor<MessageAuthenticationCode, TTMAC>();
	RegisterDefaultFactoryFor<MessageAuthenticationCode, VMAC<AES> >();
	RegisterDefaultFactoryFor<MessageAuthenticationCode, VMAC<AES, 64> >();
	RegisterDefaultFactoryFor<MessageAuthenticationCode, Weak::PanamaMAC<LittleEndian> >();
	RegisterDefaultFactoryFor<MessageAuthenticationCode, Weak::PanamaMAC<BigEndian> >();
	RegisterDefaultFactoryFor<MessageAuthenticationCode, CMAC<AES> >();
	RegisterDefaultFactoryFor<MessageAuthenticationCode, DMAC<AES> >();
	RegisterDefaultFactoryFor<MessageAuthenticationCode, Poly1305<AES> >();
	RegisterDefaultFactoryFor<MessageAuthenticationCode, Poly1305TLS>();
	RegisterDefaultFactoryFor<MessageAuthenticationCode, CMAC<DES_EDE3> >();
	RegisterDefaultFactoryFor<MessageAuthenticationCode, BLAKE2s>();
	RegisterDefaultFactoryFor<MessageAuthenticationCode, BLAKE2b>();
	RegisterDefaultFactoryFor<MessageAuthenticationCode, SipHash<2,4> >();
	RegisterDefaultFactoryFor<MessageAuthenticationCode, SipHash<4,8> >();
}

// Stream ciphers
void RegisterFactories3()
{
	RegisterSymmetricCipherDefaultFactories<Weak::MARC4>();
	RegisterSymmetricCipherDefaultFactories<SEAL<> >();
	RegisterSymmetricCipherDefaultFactories<SEAL<LittleEndian> >();
	RegisterSymmetricCipherDefaultFactories<WAKE_OFB<LittleEndian> >();
	RegisterSymmetricCipherDefaultFactories<WAKE_OFB<BigEndian> >();
	RegisterSymmetricCipherDefaultFactories<PanamaCipher<LittleEndian> >();
	RegisterSymmetricCipherDefaultFactories<PanamaCipher<BigEndian> >();

	RegisterSymmetricCipherDefaultFactories<Salsa20>();
	RegisterSymmetricCipherDefaultFactories<XSalsa20>();
	RegisterSymmetricCipherDefaultFactories<ChaCha>();
	RegisterSymmetricCipherDefaultFactories<ChaChaTLS>();
	RegisterSymmetricCipherDefaultFactories<XChaCha20>();
	RegisterSymmetricCipherDefaultFactories<Sosemanuk>();
	RegisterSymmetricCipherDefaultFactories<Rabbit>();
	RegisterSymmetricCipherDefaultFactories<RabbitWithIV>();
	RegisterSymmetricCipherDefaultFactories<HC128>();
	RegisterSymmetricCipherDefaultFactories<HC256>();
}
