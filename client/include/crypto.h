/***************************************************************************
 crypto.h  - description
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


#ifndef _CRYPTO_H
#define _CRYPTO_H

#include "ssltemplates.h"

#include <string>
#include <vector>

#include <openssl/rand.h> // RSA
#include <openssl/evp.h>  // EVP_MAX_KEY_LENGTH, ...


extern bool g_bVerbose;

#define RANDOM_BUFFER_SIZE 1024

typedef std::vector<unsigned char> CUCBuffer;

class CCrypto
  {
  protected:
    CUCBuffer m_oData;

    X509*     m_pX509;      // m_oEvpPkey frees this pointer
    CEvpPkey  m_oEvpPkey;
    CRsa      m_oRsa;

    const EVP_CIPHER* m_pfCipher;
    const EVP_MD*     m_pfDigest;

    unsigned char m_aucKey [EVP_MAX_KEY_LENGTH]; // 32
    unsigned char m_aucIv  [EVP_MAX_IV_LENGTH];  // 16
    unsigned char m_aucSalt[PKCS5_SALT_LEN];     //  8

    static const std::string s_sDelimiter;

             CCrypto(){}
  public:
             CCrypto( const std::string& rsInput );
    virtual ~CCrypto();

    void operator =    ( RSA* pRsa ) { m_oRsa = pRsa; }
         operator RSA* ()            { return (RSA*)m_oRsa; }

    void RsaKeyLoadPublic ( const std::string& rsFileRsaKey );
    void RsaKeyLoadPrivate( const std::string& rsFileRsaKey );
    void RsaKeyLoadFromCertificate( const std::string& rsFileCertificate );

    std::string      BuildTransportPackage();
    const CUCBuffer& SolveTransportPackage( const std::string& rsBase64 );

    static std::string FingerprintCrt( const std::string& rsPartnerAlias );

    std::string      EncryptToBase64  ();
    const CUCBuffer& DecryptFromBase64( const std::string& rsBase64 );

  protected:
    void RandomSeed();
    void RsaKeyGenerate();

    void RandomGet( CUCBuffer& roBuffer );
    void RandomGet( unsigned char* pucBuffer, size_t nBufferSize );

    std::string      ConvertToBase64  ();
    const CUCBuffer& ConvertFromBase64( const std::string& rsBase64 );

    std::string ConvertToBase64( const unsigned char* pucData, size_t nSize );
    std::string EncryptToBase64( const unsigned char* pucData, size_t nSize );

    bool EncryptRsaPublic ( CUCBuffer& roBuffer );
    bool DecryptRsaPrivate( CUCBuffer& roBuffer );

    std::string SymetricKeyRsaPublicEncrypt();
    bool SymetricKeyRsaPrivateDecrypt( const std::string& rsEncrypted );

    int  SymetricKeyMake( const EVP_CIPHER* pfCipher = EVP_des_ede3_cfb(),
                          const EVP_MD*     pfDigest = EVP_sha1() );

  }; // class CCrypto

#endif // _CRYPTO_H
