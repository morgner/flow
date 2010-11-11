/***************************************************************************
 socketssl.cpp
 -----------------------
 begin                 : Thu Nocv 09 2010
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


#define CA_LIST  "certificates/server/server-CA-chain.pem"
#define CA_PATH  "certificates/trust/"
#define KEYFILE  "certificates/client/username.key"
#define CRTFILE  "certificates/client/username.crt"
#define PASSWORD ""

/*
 for i in ca.crt ims.crt
   do
   openssl x509 -in $i -text >> server-CA-chain.pem
   done

 for i in ca.crt imc.crt
   do
   openssl x509 -in $i -text >> client-CA-chain.pem
   done

openssl s_server -accept 30000 \
                 -cert certificates/server/localhost.crt -key certificates/server/localhost.key \
                 -CAfile certificates/server/server-CA-chain.pem \
                 -tls1

openssl s_client -connect localhost:30000 \
                 -cert certificates/client/username.crt -key certificates/client/username.key \
                 -CAfile certificates/client/client-CA-chain.pem \
                 -tls1 -verify 3

*/

#include "socketssl.h"
#include "socketexception.h"

#include <iostream>

std::string CSocketSSL::s_sPassword;
std::string CSocketSSL::s_sCiphers  = "DHE-RSA-AES256-SHA:DHE-RSA-AES128-SHA";

/* openssl ciphers
DHE-RSA-AES256-SHA:DHE-DSS-AES256-SHA:AES256-SHA:EDH-RSA-DES-CBC3-SHA:EDH-DSS-DES-CBC3-SHA:DES-CBC3-SHA:DES-CBC3-MD5:
DHE-RSA-AES128-SHA:DHE-DSS-AES128-SHA:AES128-SHA:DHE-RSA-SEED-SHA:DHE-DSS-SEED-SHA:SEED-SHA:RC2-CBC-MD5:RC4-SHA:RC4-MD5:
RC4-MD5:EDH-RSA-DES-CBC-SHA:EDH-DSS-DES-CBC-SHA:DES-CBC-SHA:DES-CBC-MD5:EXP-EDH-RSA-DES-CBC-SHA:EXP-EDH-DSS-DES-CBC-SHA:
EXP-DES-CBC-SHA:EXP-RC2-CBC-MD5:EXP-RC2-CBC-MD5:EXP-RC4-MD5:EXP-RC4-MD5
*/

// The password code is not thread safe
int CSocketSSL::PasswordCallback( char *pszBuffer, int nBufferSize, int nRWFlag, void* pUserData )
  {
//  std::cout << "PasswordCallback called" << std::endl;

  int nPwdLength = CSocketSSL::s_sPassword.length();
  if ( nBufferSize < nPwdLength+1 )
    {
    return(0);
    }
  strcpy( pszBuffer, CSocketSSL::s_sPassword.c_str() );
  return( nPwdLength );
  }



CSocketSSL::CSocketSSL( const int nSock )
  : inherited( nSock ),
    m_ptSslCtx( 0 ),
    m_ptSsl( 0 ),
    m_ptBioError( 0 )
  {
  }

CSocketSSL::CSocketSSL( const std::string& rsCertificate,
                        const std::string& rsCA,
                        const int          nSock )
  : inherited( nSock ),
    m_sCertificate( rsCertificate ),
    m_sCA( rsCA ),
    m_ptSslCtx( 0 ),
    m_ptSsl( 0 ),
    m_ptBioError( 0 )
  {
  }

CSocketSSL::~CSocketSSL()
  {
  SSL_CTX_free( m_ptSslCtx );
  }


void CSocketSSL::ConnectSSL( const std::string& rsCertificate,
                             const std::string& rsCA,
                             const std::string& rsHost,
                             const std::string& rsPort )
  {
  inherited::Connect( rsHost, rsPort );
//  std::cout << "Socket connected" << std::endl;

  if ( !isValid() ) return;

  /* ++++++++++++++++++++++++++++++++++++++++++++++++++++++
   *
   *   start opening the Secure Layer over the Socket
   *
   * ++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

  BIO* ptSbio;   // sbio

  /// Build our SSL context
  m_ptSslCtx = InitializeSslCtx( rsCertificate, PASSWORD );
  std::cout << "Initialized SSL Context" << std::endl;

  // Set our cipher list
  if ( s_sCiphers.length() )
    {
    SSL_CTX_set_cipher_list( m_ptSslCtx, s_sCiphers.c_str() );
    }
  std::cout << "Selected cipher list" << std::endl;

/* only server
  int s_server_session_id_context = 1;
  SSL_CTX_set_session_id_context( m_ptSslCtx,
                                  (const unsigned char*)&s_server_session_id_context,
                                  sizeof(s_server_session_id_context) ); 
*/

  // Connect the SSL socket
  m_ptSsl = SSL_new( m_ptSslCtx );
  ptSbio  = BIO_new_socket( m_nSock, BIO_NOCLOSE );
  SSL_set_bio( m_ptSsl, ptSbio, ptSbio );
  if( SSL_connect(m_ptSsl) <= 0 )
    {
    //- berr_exit("SSL connect error");
    throw CSocketException("SSL connect error connecting to " + rsHost);
    }
  std::cout << "Connected with SSL" << std::endl;

  CertificateCheck( m_ptSsl, rsHost );
  std::cout << "Host is ok" << std::endl;
  }


const CSocketSSL& CSocketSSL::operator << ( const std::string& s ) const
  {
//  std::cout << "CSocketSSL::operator << (s)" << std::endl;
  Send( s );
  return *this;
  }

const CSocketSSL& CSocketSSL::operator << ( long n ) const
  {
//  std::cout << "CSocketSSL::operator << (n)" << std::endl;
  Send( to_string(n) );
  return *this;
  }


const CSocketSSL& CSocketSSL::operator >> ( std::string& s ) const
  {
  Receive( s );
  return *this;
  }


void CSocketSSL::Send( const std::string& s ) const
  {
//  std::cout << "CSocketSSL::Send(s)" << std::endl;
  unsigned int nResult = ::SSL_write( m_ptSsl,
                                      s.c_str(),
                                      s.length() );

  switch( SSL_get_error(m_ptSsl, nResult) )
    {      
    case SSL_ERROR_NONE:
      if ( s.length() != nResult )
        {
        throw CSocketException("Incomplete write!");
        }
      break;

    default:
    throw CSocketException("SSL write problem");
    }
  }


const std::string& CSocketSSL::Receive( std::string& s ) const
  {
  std::cout << "CSocketSSL::Receive(s)" << std::endl;

  static char aszBuffer[RECEIVE_BUFFER_SIZE];

  unsigned int nResult = SSL_read( m_ptSsl, aszBuffer, RECEIVE_BUFFER_SIZE-1 );
  switch( SSL_get_error(m_ptSsl, nResult) )
    {
    case SSL_ERROR_NONE:
      std::cout << "SSL Read Data: " << aszBuffer << std::endl;
      break;
    case SSL_ERROR_ZERO_RETURN:
      throw CSocketException( "SSL_ERROR_ZERO_RETURN" );
    case SSL_ERROR_SYSCALL:
      throw CSocketException( "SSL_ERROR_SYSCALL" );
    default:
      throw CSocketException( "Unknown SSL Error - Read problem" );
    }

  aszBuffer[nResult+1] = 0;
  s += aszBuffer;
  return s;
  }


SSL_CTX* CSocketSSL::InitializeSslCtx( const std::string rsKeyfile,
                                       const std::string rsPassword )
  {
  SSL_METHOD* ptMethode;
  SSL_CTX*    ptSslCtx;

  if( !m_ptBioError )
    {
    // Global system initialization
    SSL_library_init();
    SSL_load_error_strings();

    // An error write context
    m_ptBioError = BIO_new_fp(stderr,BIO_NOCLOSE);
    }
  std::cout << "Initialized SSL" << std::endl;

  // Create our context
  ptMethode = TLSv1_method(); // TLSv1_method() SSLv2_method() SSLv23_method() SSLv3_method()
  ptSslCtx  = SSL_CTX_new( ptMethode );
  std::cout << "SSL Context created" << std::endl;

  // Load our certificates
  if ( ! SSL_CTX_use_certificate_chain_file(ptSslCtx, (rsKeyfile + ".crt").c_str()) )
    {
    throw CSocketException("Can't read certificate file");
    }
  std::cout << "Read certificate file" << std::endl;

  s_sPassword = rsPassword;
  SSL_CTX_set_default_passwd_cb( ptSslCtx, PasswordCallback );

  // Load our keys
  if ( ! SSL_CTX_use_PrivateKey_file(ptSslCtx, (rsKeyfile + ".key").c_str(), SSL_FILETYPE_PEM ) )
    {
    throw CSocketException("Can't read key file");
    }
  std::cout << "Read key file" << std::endl;

  // Load the CAs we trust
  if( ! SSL_CTX_load_verify_locations(ptSslCtx, CA_LIST, 0) )
//  if( ! SSL_CTX_load_verify_locations(ptSslCtx, 0, CA_PATH) )
    {
    throw CSocketException("Can't read CA list");
    }
  std::cout << "Checked CA list" << std::endl;

  return ptSslCtx;
  }


// Check that the common name matches the host name
bool CSocketSSL::CertificateCheck( SSL* ptSsl, const std::string& rsHost )
  {
  X509  *ptPeer;
  char   acPeer_CN[256];

  ptPeer = SSL_get_peer_certificate( ptSsl );

// ?? verifies against the system CAs? ??
  int nResult = 0; // SSL_get_verify_result( ptSsl );
  if ( nResult != X509_V_OK )
    {
    std::string sError;
    switch ( nResult )
      {
      case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT:
        sError = "Unable to get or find issuer certificate";
        break;
      case X509_V_ERR_UNABLE_TO_GET_CRL:
         sError = "Unable to get certificate CRL";
         break;
      case X509_V_ERR_UNABLE_TO_DECRYPT_CERT_SIGNATURE:
         sError = "Unable to decrypt certificate's signature";
         break;
      case X509_V_ERR_UNABLE_TO_DECRYPT_CRL_SIGNATURE:
         sError = "Unable to decrypt CRL's signature";
         break;
      case X509_V_ERR_UNABLE_TO_DECODE_ISSUER_PUBLIC_KEY:
         sError = "Unable to decode issuer public key";
         break;
      case X509_V_ERR_CERT_SIGNATURE_FAILURE:
         sError = "Certificate signature failure";
         break;
      case X509_V_ERR_CRL_SIGNATURE_FAILURE:
        sError = "CRL signature failure";
        break;
      case X509_V_ERR_CERT_NOT_YET_VALID:
        sError = "Certificate is not yet valid";
        break;
      case X509_V_ERR_CERT_HAS_EXPIRED:
        sError = "Certificate has expired";
        break;
      case X509_V_ERR_CRL_NOT_YET_VALID:
        sError = "CRL is not yet valid";
        break;
      case X509_V_ERR_CRL_HAS_EXPIRED:
        sError = "CRL has expired";
        break;
      case X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD:
        sError = "Format error in certificate's notBefore field";
        break;
      case X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD:
        sError = "Format error in certificate's notAfter field";
        break;
      case X509_V_ERR_ERROR_IN_CRL_LAST_UPDATE_FIELD:
         sError = "Format error in CRL's lastUpdate field";
         break;
      case X509_V_ERR_ERROR_IN_CRL_NEXT_UPDATE_FIELD:
        sError = "Format error in CRL's nextUpdate field";
        break;
      case X509_V_ERR_OUT_OF_MEM:
        sError = "Out of memory";
        break;
      case X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
        sError = "Self signed certificate and the certificate cannot be found in the list of trusted certificates";
        break;
      case X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN:
        sError = "Self signed certificate in certificate chain";
        break;
      case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY:
        sError = "Unable to get local issuer certificate";
        break;
      case X509_V_ERR_UNABLE_TO_VERIFY_LEAF_SIGNATURE:
        sError = "Unable to verify the first certificate";
        break;
      case X509_V_ERR_CERT_CHAIN_TOO_LONG:
        sError = "Certificate chain too long";
        break;
      case X509_V_ERR_CERT_REVOKED:
        sError = "Certificate revoked";
        break;
      case X509_V_ERR_INVALID_CA:
        sError = "Invalid CA certificate";
        break;
      case X509_V_ERR_PATH_LENGTH_EXCEEDED:
        sError = "Path length constraint exceeded";
        break;
      case X509_V_ERR_INVALID_PURPOSE:
        sError = "Unsupported certificate purpose";
        break;
      case X509_V_ERR_CERT_UNTRUSTED:
        sError = "Certificate not trusted";
        break;
      case X509_V_ERR_CERT_REJECTED:
        sError = "Certificate rejected (the root CA is marked to reject the specified purpose)";
        break;
      case X509_V_ERR_SUBJECT_ISSUER_MISMATCH:
        sError = "Subject issuer mismatch";
        break;
      case X509_V_ERR_AKID_SKID_MISMATCH:
        sError = "Authority and subject key identifier mismatch";
        break;
      case X509_V_ERR_AKID_ISSUER_SERIAL_MISMATCH:
        sError = "Authority and issuer serial number mismatch";
        break;
      case X509_V_ERR_KEYUSAGE_NO_CERTSIGN:
        sError = "Key usage does not include certificate signing";
        break;
      case X509_V_ERR_APPLICATION_VERIFICATION:
        sError = "Application verification failure";
        break;
      default: sError = "Unknown Error #" + to_string(nResult);
      } // switch ( nResult )
    throw CSocketException( "Certificate verification: " + sError );
    } // if ( nResult != X509_V_OK )
//  std::cout << "Certificate issued for host " + rsHost << std::endl;

  // Check the common name
  X509_NAME_get_text_by_NID( X509_get_subject_name(ptPeer),
                             NID_commonName,
                             acPeer_CN,
                             sizeof(acPeer_CN) );
  if( strcasecmp( acPeer_CN, rsHost.c_str() ))
    {
    throw CSocketException("Common name doesn't match host name");
    }
  std::cout << "Common name matches host name: " << acPeer_CN << std::endl;
  return true;
  }
