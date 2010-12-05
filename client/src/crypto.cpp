/***************************************************************************
 crypto.cpp  description
 -----------------------
 begin                 : Thu Dec 02 2010
 copyright             : Copyright (C) 2010 by Manfred Morgner
 email                 : manfred@morgner.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *                                                                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place Suite 330,                                            *
 *   Boston, MA  02111-1307, USA.                                          *
 *                                                                         *
 ***************************************************************************/


#include "crypto.h"
#include "cryptoexception.h"

#include <iostream>

#include <openssl/rand.h>   // RSA
#include <openssl/rsa.h>    // RSA_F4
#include <openssl/err.h>    // ERR_get_error, ERR_error_string

#include <openssl/buffer.h> // BUF_MEM

#include <fcntl.h> // open, read, close => RandomGet()

           bool            g_bVerbose   = true;
const std::string CCrypto::s_sDelimiter = "===\n";


// The content has to be initialized on contruction time
CCrypto::CCrypto( const std::string& rsInput )
  : m_pX509(0)
  {
  m_pfCipher = EVP_des_ede3_cfb();

  if ( rsInput.length() )
    {
    m_oData.resize(rsInput.length()+1);
    memcpy( &m_oData[0], rsInput.c_str(), rsInput.length() );
    m_oData[ rsInput.length() ] = 0;
    }
  }


// Anything frees itself on the close
CCrypto::~CCrypto()
  {
  }


// Call some random data into the buffer, if there are no
// random devices accessiable, we hope there were some
// random data where the buffer was allocated
void CCrypto::RandomGet( CUCBuffer& roBuffer )
  {
  RandomGet( &roBuffer[0], roBuffer.size() );
  } // void CCrypto::RandomGet( CUCBuffer& roBuffer )

void CCrypto::RandomGet( unsigned char* pucBuffer, size_t nBufferSize )
  {
  if ( !pucBuffer )
    {
    return;
    }
  int fh = open( "/dev/urandom", O_RDONLY );
  if ( fh == -1 )
    {
    fh = open( "/dev/random", O_RDONLY );
    }
  if ( fh != -1 )
    {
    read( fh, pucBuffer, nBufferSize );
    close( fh );
    }
  } // void CCrypto::RandomGet( CUCBuffer& roBuffer )


// Seed the Pseudo Random Number Generator
void CCrypto::RandomSeed()
  {
  // Generates a key pair and returns it in a newly allocated RSA structure.
  // The pseudo random number generator must be seeded prior to calling
  // RSA_generate_key().
  unsigned char aucRandom[RANDOM_BUFFER_SIZE];
  RandomGet( aucRandom, sizeof(aucRandom) );
  RAND_seed( aucRandom, sizeof(aucRandom) );
  }


// Generate a RSA key, usually a SSL session key
void CCrypto::RsaKeyGenerate()
  {
  RandomSeed();
  // RSA *RSA_generate_key(int num, unsigned long e,
  //                       void (*callback)(int, int, void *), void *cb_arg);
  // The modulus size will be num bits, and the public exponent will be e. Key
  // sizes with num < 1024 should be considered insecure. The exponent is an
  // odd number, typically 3, 17 or 65537.
  m_oRsa = ::RSA_generate_key( 1024, RSA_F4, NULL, NULL );
  } // CRsa& CCrypto::RsaKeyGenerate()


// Load a private RSA key from PEM file
void CCrypto::RsaKeyLoadPrivate( const std::string& rsFileRsaKey )
  {
  CBio oBio = ::BIO_new_file( rsFileRsaKey.c_str(), "r" );
    
  if ( !oBio.isValid() )
    {
    throw CCryptoException( "Couldn't open key file " + rsFileRsaKey + " - "
                            + ERR_error_string(ERR_get_error(), 0) );
    }
    
  if ( !::PEM_read_bio_RSAPrivateKey(oBio, m_oRsa, NULL, NULL) )
    {
    throw CCryptoException( "Couldn't read key file " + rsFileRsaKey + " - "
                            + ERR_error_string(ERR_get_error(), 0) );
    }
  } // void CCrypto::RsaKeyLoadPrivate( const std::string& rsFileRsaKey )


// Load a public RSA key from PEM file
//!? does not work - currently 
void CCrypto::RsaKeyLoadPublic( const std::string& rsFileRsaKey )
  {
  CBio oBio = ::BIO_new_file( rsFileRsaKey.c_str(), "r" );
    
  if ( !oBio.isValid() )
    {
    throw CCryptoException( "Couldn't open key file " + rsFileRsaKey + " - "
                            + ERR_error_string(ERR_get_error(), 0) );
    }
    
  if ( !::PEM_read_bio_RSAPublicKey(oBio, m_oRsa, NULL, NULL) )
    {
    throw CCryptoException( "Couldn't read key file " + rsFileRsaKey + " - "
                            + ERR_error_string(ERR_get_error(), 0) );
    }
  } // void CCrypto::RsaKeyLoadPublic( const std::string& rsFileRsaKey )


// Load the public RSA key from the recipients certificate
void CCrypto::RsaKeyLoadFromCertificate( const std::string& rsFileCertificate )
  {
  CBio oBio = ::BIO_new_file( rsFileCertificate.c_str(), "r" );

  if ( !oBio.isValid() )
    {
    throw CCryptoException( "Couldn't open certificate file "
                            + rsFileCertificate + " - "
                            + ERR_error_string(ERR_get_error(), 0) );
    }

  if ( !::PEM_read_bio_X509_AUX(oBio, &m_pX509, NULL, NULL) )
    {
    throw CCryptoException( "Couldn't read certificate file "
                            + rsFileCertificate + " - "
                            + ERR_error_string(ERR_get_error(), 0) );
    }
    
  m_oEvpPkey.Delete();
  m_oEvpPkey = ::X509_get_pubkey( m_pX509 );

  m_oRsa.Delete();
  m_oRsa = EVP_PKEY_get1_RSA( m_oEvpPkey );
  }


// RSA encrypt a buffer, return values will replace input values
bool CCrypto::EncryptRsaPublic( CUCBuffer& roBuffer )
  {
  if ( (!m_oRsa.isValid()) || (!m_oRsa->n) )
    {
    return false;
    }

  // The pseudo random number generator (PRNG) must be seeded prior to calling
  // RSA_public_encrypt().
  RandomSeed();

  int            nRsaSize     = RSA_size(m_oRsa);
  unsigned char* pucInput     = &roBuffer[0];
  unsigned char* pucEncrypted = new unsigned char[nRsaSize];

  CUCBuffer oOutput;

  // flen must be
  // less than RSA_size(rsa) - 11 for RSA_PKCS1_PADDING or RSA_SSLV23_PADDING,
  // less than RSA_size(rsa) - 41 for RSA_PKCS1_OAEP_PADDING and
  //   exactly RSA_size(rsa)      for RSA_NO_PADDING.
  int nPadding = RSA_PKCS1_OAEP_PADDING;
  int nStep    = nRsaSize-12;
  int nSize    = roBuffer.size();

  oOutput.reserve( (nSize/nStep +1) * nRsaSize );

  int nPos  = 0;
  int nLen  = nStep;
  while ( nPos < nSize )
    {
    if ( nPos + nStep > nSize )
      {
      nLen = nSize - nPos;
      }
    int nEncrypted = RSA_public_encrypt( nLen,
                                         pucInput,
                                         pucEncrypted,
                                         m_oRsa,
                                         nPadding );
    if ( 0 > nEncrypted )
      {
      throw CCryptoException( std::string("Couldn't public RSA encrypt - ")
                              + ERR_error_string(ERR_get_error(), 0) );
      }
    nPos     += nLen;
    pucInput += nLen;

    for ( int n = 0; n < nEncrypted; n++ )
      {
      oOutput.push_back( pucEncrypted[n] );
      }
    }

  if (g_bVerbose) std::cout << "Encrypted size: " << oOutput.size() << std::endl;

  roBuffer = oOutput;

  return true;
  } // bool CCrypto::EncryptRsaPublic( CUCBuffer& roBuffer, RSA* pRsa )


// RSA decrypt a buffer, return values will replace input values
bool CCrypto::DecryptRsaPrivate( CUCBuffer& roBuffer )
  {
  if ( (!m_oRsa.isValid()) || (!m_oRsa->n) )
    {
    return false;
    }

  int            nRsaSize     = RSA_size(m_oRsa);
  unsigned char* pucInput     = &roBuffer[0];
  unsigned char* pucDecrypted = new unsigned char[nRsaSize];

  CUCBuffer oOutput;

  // nPadding must be the same as in "EncryptRsaPublic"
  int nPadding = RSA_PKCS1_OAEP_PADDING;
  int nStep    = nRsaSize;
  int nSize    = roBuffer.size();

  oOutput.reserve( nSize );

  int nPos  = 0;
  int nLen  = nStep;
  while ( nPos < nSize )
    {
    if ( nPos + nStep > nSize )
      {
      nLen = nSize - nPos;
      }
    int nDecrypted = RSA_private_decrypt( nLen,
                                          pucInput,
                                          pucDecrypted,
                                          m_oRsa,
                                          nPadding );
    if ( 0 > nDecrypted )
      {
      throw CCryptoException( std::string("Couldn't private RSA decrypt - ")
                              + ERR_error_string(ERR_get_error(), 0) );
      }
    nPos     += nLen;
    pucInput += nLen;
        
    for ( int n = 0; n < nDecrypted; n++ )
      {
      oOutput.push_back( pucDecrypted[n] );
      }
    }

  if (g_bVerbose) std::cout << "Encrypted size: " << oOutput.size() << std::endl;

  roBuffer = oOutput;

  return true;
  } // bool CCrypto::DecryptRsaPrivate( CUCBuffer& roBuffer )


// Create a new symetric key to encrypt large amount of data
int CCrypto::SymetricKeyMake( const EVP_CIPHER* pfCipher,
                              const EVP_MD*     pfDigest )
  {
  m_pfCipher = pfCipher;
  m_pfDigest = pfDigest;

  unsigned char aucKeyBytes[256];
  RandomGet( aucKeyBytes, sizeof(aucKeyBytes) );

  strncpy((char*)m_aucSalt, "flowencrypto", PKCS5_SALT_LEN);
    
  RandomGet( m_aucKey, EVP_MAX_KEY_LENGTH );
  RandomGet( m_aucIv,  EVP_MAX_IV_LENGTH  );

  return EVP_BytesToKey( m_pfCipher,
                         m_pfDigest,
                         m_aucSalt,
                         aucKeyBytes,
                         sizeof(aucKeyBytes),
                         16,
                         m_aucKey,
                         m_aucIv );
  } // int CCrypto::SymetricKeyMake( const EVP_CIPHER* pfCipher, ...


// Encrypt the symetric key using RSA encryption
std::string CCrypto::SymetricKeyRsaPublicEncrypt()
  {
  CUCBuffer oBuffer;
  oBuffer.resize( sizeof(m_aucKey) + sizeof(m_aucIv), 0 );

  memcpy( &oBuffer[0],                m_aucKey, sizeof(m_aucKey) );
  memcpy( &oBuffer[sizeof(m_aucKey)], m_aucIv,  sizeof(m_aucIv)  );

  if ( !EncryptRsaPublic( oBuffer ) )
    {
    return "";
    }

  std::string sResult = ConvertToBase64( &oBuffer[0], oBuffer.size() );

  if ( g_bVerbose ) std::cout << "TransportKey64: " << std::endl << sResult << std::endl;

  return sResult;
  } // std::string CCrypto::SymetricKeyRsaPublicEncrypt( CRsa& roRsa )


// Convert Buffer to BASE64 format
std::string CCrypto::ConvertToBase64()
  {
  return ConvertToBase64( &m_oData[0], m_oData.size() );
  } // std::string CCrypto::ConvertToBase64( const CUCBuffer& roBuffer )

std::string CCrypto::ConvertToBase64( const unsigned char* pucData, size_t nSize )
  {
  BIO* pBio64 = BIO_new(BIO_f_base64());
  BIO* pBio   = BIO_new(BIO_s_mem());
       pBio   = BIO_push(pBio64, pBio);
  BIO_write(pBio, pucData, nSize);
  int nResult = BIO_flush(pBio);
  if ( 1 != nResult )
    {
    throw CCryptoException( "BIO_flush in ConvertToBase64" ); 
    }
  BUF_MEM* ptBuffer;
  BIO_get_mem_ptr(pBio, &ptBuffer);

  char* buff = new char[ptBuffer->length+1];
  memcpy(buff, ptBuffer->data, ptBuffer->length);
  buff[ptBuffer->length] = 0;
  std::string sResult = buff;
  delete buff;

//- NOT: std::string sResult = (char*)ptBuffer->data;

  BIO_free_all(pBio); // frees ptBuffer too

  return sResult;
  } // std::string CCrypto::ConvertToBase64( unsigned char* pucData, int nSize )


// See next methode
std::string CCrypto::EncryptToBase64()
  {
  return EncryptToBase64( &m_oData[0], m_oData.size() );
  } // std::string CCrypto::EncryptToBase64( const CUCBuffer& roBuffer )

// Generate a symetric key
// Encrypt the data
// Encrypt the symtric key asymetrically
// Convert both to BASE64
// Combine the output with a seperator
std::string CCrypto::EncryptToBase64( const unsigned char* pucData, size_t nSize )
  {
  SymetricKeyMake( m_pfCipher );

  BIO* pBioEnc = BIO_new ( BIO_f_cipher() );
  BIO* pBio64  = BIO_new ( BIO_f_base64() );
  BIO* pBio    = BIO_new ( BIO_s_mem() );
       pBio    = BIO_push( pBio64,  pBio );
       pBio    = BIO_push( pBioEnc, pBio );

  BIO_set_cipher(pBio, m_pfCipher, m_aucKey, m_aucIv, 1);
  BIO_write( pBio, pucData, nSize );
  int nResult = BIO_flush( pBio );
  if ( 1 != nResult )
    {
    throw CCryptoException( "BIO_flush in EncryptToBase64" ); 
    }
  BUF_MEM* ptBuffer;
  BIO_get_mem_ptr(pBio, &ptBuffer);

  char* buff = new char[ptBuffer->length+1];
  memcpy(buff, ptBuffer->data, ptBuffer->length);
  buff[ptBuffer->length] = 0;
  std::string sResult = buff;
  delete buff;

//- NOT: std::string sResult = (char*)ptBuffer->data;

  BIO_free_all(pBio);

  return SymetricKeyRsaPublicEncrypt() + s_sDelimiter + sResult;
  } // std::string CCrypto::EncryptToBase64( unsigned char* pucData, int nSize );


// Convert BASE64 input to binary output, should spill out simple strings ;-)
CUCBuffer CCrypto::ConvertFromBase64( const std::string& rsBase64 )
  {
  CUCBuffer     oOutput;
  unsigned char uc;

  BIO* pBio64 = BIO_new(BIO_f_base64()); // may yield 0!
  BIO* pBio   = BIO_new_mem_buf( (void*)rsBase64.c_str(), -1 );
       pBio   = BIO_push(pBio64, pBio);
  while ( BIO_read(pBio, &uc, 1) > 0 )
    {
    oOutput.push_back(uc);
    }

  BIO_free_all(pBio);

  return oOutput;
  } // CUCBuffer CCrypto::ConvertFromBase64( const std::string& sBase64 )


// Decrypt a symetric key using RSA asymetric decryption
bool CCrypto::SymetricKeyRsaPrivateDecrypt( const std::string& rsEncrypted )
  {
  CUCBuffer oBuffer = ConvertFromBase64( rsEncrypted );

  DecryptRsaPrivate( oBuffer );

  memcpy(m_aucKey, &oBuffer[0],                sizeof(m_aucKey) );
  memcpy(m_aucIv,  &oBuffer[sizeof(m_aucKey)], sizeof(m_aucIv)  );

  return true;
  } // bool CCrypto::SymetricKeyRsaPrivateDecrypt( const std::string& rsEncrypted )


// Split an encrypted 'BASE64' package to key and content blocks
// Decrypt the symetric key by asymetric RSA decryption
// Decrypt the symetric encrypted content using the decrypted key encrypted key
CUCBuffer CCrypto::DecryptFromBase64( const std::string& rsBase64 )
  {
  std::string sKey = rsBase64.substr(0, rsBase64.find(s_sDelimiter));
//  std::cout << "KEY-BASE64" << std::endl << sKey << std::endl;
  SymetricKeyRsaPrivateDecrypt( sKey );
    
  CUCBuffer     oOutput;
  unsigned char uc;

  std::string sData = rsBase64.substr( rsBase64.find(s_sDelimiter)
                                                     + s_sDelimiter.length() );
//  std::cout << "DATA-BASE64" << std::endl << sData << std::endl;
  oOutput.reserve( sData.length() );

  BIO* pBioEnc = BIO_new( BIO_f_cipher() );
  BIO* pBio64  = BIO_new( BIO_f_base64() ); // may yield 0!
  BIO* pBio    = BIO_new_mem_buf( (void*)sData.c_str(), -1 );
       pBio    = BIO_push( pBio64,  pBio);
       pBio    = BIO_push( pBioEnc, pBio );
  BIO_set_cipher(pBio, m_pfCipher, m_aucKey, m_aucIv, 0);
  while ( BIO_read(pBio, &uc, 1) > 0 )
    {
    oOutput.push_back( uc );
    }

  BIO_free_all(pBio);

  return oOutput;
  } // CUCBuffer CCrypto::ConvertFromBase64( const std::string& sBase64 )
