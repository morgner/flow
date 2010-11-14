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


#include "socketssl.h"
#include "socketexception.h"

#include <iostream>

std::string CSocketSSL::s_sCiphers  = ""; // "DHE-RSA-AES256-SHA:DHE-RSA-AES128-SHA";

/* openssl ciphers
DHE-RSA-AES256-SHA:DHE-DSS-AES256-SHA:AES256-SHA:EDH-RSA-DES-CBC3-SHA:EDH-DSS-DES-CBC3-SHA:DES-CBC3-SHA:DES-CBC3-MD5:
DHE-RSA-AES128-SHA:DHE-DSS-AES128-SHA:AES128-SHA:DHE-RSA-SEED-SHA:DHE-DSS-SEED-SHA:SEED-SHA:RC2-CBC-MD5:RC4-SHA:RC4-MD5:
RC4-MD5:EDH-RSA-DES-CBC-SHA:EDH-DSS-DES-CBC-SHA:DES-CBC-SHA:DES-CBC-MD5:EXP-EDH-RSA-DES-CBC-SHA:EXP-EDH-DSS-DES-CBC-SHA:
EXP-DES-CBC-SHA:EXP-RC2-CBC-MD5:EXP-RC2-CBC-MD5:EXP-RC4-MD5:EXP-RC4-MD5
*/

// The password code is not thread safe
int CSocketSSL::PasswordCallback( char *pszBuffer, int nBufferSize, int nRWFlag, void* pUserData )
  {
  std::cout << "PasswordCallback called" << std::endl;

  std::string sPassword = reinterpret_cast<CSocketSSL*>(pUserData)->PasswordGet();
  if ( nBufferSize < (int)sPassword.length()+1 )
    {
    return(0);
    }
  strcpy( pszBuffer, sPassword.c_str() );
  return( sPassword.length() );
  }


/// for server connections (calling Accept()) ??
CSocketSSL::CSocketSSL( const int nSock )
  : inherited( nSock ),
    // pointer to NULL
    m_ptSslCtx( 0 ),
    m_ptSsl( 0 ),
    m_ptBio( 0 ),
    m_ptSslBio( 0 ),
    m_ptBioError( 0 )
  {
  }

/// for client connection (calling 'Connect()')
CSocketSSL::CSocketSSL( const std::string& rsHost,
                        const std::string& rsPort,
                        const std::string& rsFileCertificate,
                        const std::string& rsFileKey,
                        const std::string& rsPassword,
                        const std::string& rsFileCaChainClient,
                        const std::string& rsFileCaChainHost )
  : m_sHost( rsHost),
    m_sPort( rsPort ),
    // TLS credentials
    m_sFileCertificate( rsFileCertificate ),
    m_sFileKey( rsFileKey ),
    m_sPassword( rsPassword ),
    m_sFileCaChainClient( rsFileCaChainClient ),
    m_sFileCaChainHost( rsFileCaChainHost ),
    // pointer to NULL
    m_ptSslCtx( 0 ),
    m_ptSsl( 0 ),
    m_ptBio( 0 ),
    m_ptSslBio( 0 ),
    m_ptBioError( 0 )
  {
  }

CSocketSSL::~CSocketSSL()
  {
  Close();
  }


CSocket* CSocketSSL::Accept ()
  {
  CSocketSSL* poSocket = new CSocketSSL;
  Accept( *poSocket );
  return poSocket;
  }


void CSocketSSL::Accept( CSocketSSL& roSocket )
  {
  std::cout << "CSocketSSL::Accept( CSocketSSL& )" << std::endl;
  inherited::Accept( roSocket );

  m_ptBio = ::BIO_new_socket( roSocket.m_nSock, BIO_NOCLOSE );
  m_ptSsl = ::SSL_new( m_ptSslCtx );
  ::SSL_set_bio( m_ptSsl, m_ptBio, m_ptBio);

  int nResult;
  if ( (nResult = ::SSL_accept(roSocket.m_ptSsl)) <= 0 )
    {
    throw CSocketException( "SSL accept error" );
    }

  m_ptBio    = ::BIO_new( BIO_f_buffer() );
  m_ptSslBio = ::BIO_new( BIO_f_ssl() );
  ::BIO_set_ssl( m_ptSslBio, m_ptSsl, BIO_CLOSE);
  ::BIO_push( m_ptBio, m_ptSslBio );

  } // void CSocketSSL::Accept( CSocket& roSocket ) const


void CSocketSSL::Close()
  {
  if ( !m_ptSsl ) return;

  // this is a complicated matter, see: http://www.openssl.org/docs/ssl/SSL_shutdown.html
  int nResult = ::SSL_shutdown( m_ptSsl );
  if( nResult != 1 )
    {
    std::cout << "SSL_shutdown first  result: " << nResult << std::endl;
    ::shutdown( m_nSock, 1 );
    nResult = ::SSL_shutdown( m_ptSsl );
    } // if ( nResult != 1 )

  switch( nResult )
    {  
    case  1:
      std::cout << "SSL_shutdown completed successful" << std::endl;
      break; // ok both sides negotiated the shutdown status, which is not neccessary
    case  0:
    case -1:
    default:
      std::cout << "SSL_shutdown second result: " << nResult << std::endl;
//      throw CSocketException("Shutdown failed");
    }

  // SSL_free() decrements the reference count of ssl, and removes the SSL structure pointed to by ssl and frees up the
  // allocated memory if the reference count has reached 0.
  if ( m_ptSsl )
    {
    ::SSL_free ( m_ptSsl );
    }

  // SSL_CTX_free() decrements the reference count of ctx, and removes the SSL_CTX object pointed to by ctx and frees up the
  // allocated memory if the the reference count has reached 0.
  if ( m_ptSslCtx )
    {
    ::SSL_CTX_free( m_ptSslCtx );
    }

  m_ptSsl = 0;
  m_ptSslCtx = 0;
  inherited::Close();
  }

void CSocketSSL::Connect()
  {
  inherited::Connect( m_sHost, m_sPort );
//  std::cout << "Socket connected" << std::endl;

  if ( !isValid() ) return;

  /* ++++++++++++++++++++++++++++++++++++++++++++++++++++++
   *
   *   start opening the Secure Layer over the Socket
   *
   * ++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

  BIO* ptSbio;

  /// Build our SSL context
  InitializeSslCtx(); // m_ptSslCtx
  std::cout << "Initialized SSL Context" << std::endl;

  // Set our cipher list
  if ( s_sCiphers.length() )
    {
    ::SSL_CTX_set_cipher_list( m_ptSslCtx, s_sCiphers.c_str() );
    }
  std::cout << "Selected cipher list" << std::endl;

/* only server
  int s_server_session_id_context = 1;
  SSL_CTX_set_session_id_context( m_ptSslCtx,
                                  (const unsigned char*)&s_server_session_id_context,
                                  sizeof(s_server_session_id_context) ); 
*/

  // Connect the SSL socket
  m_ptSsl = ::SSL_new( m_ptSslCtx );
  ptSbio  = ::BIO_new_socket( m_nSock, BIO_NOCLOSE );
  ::SSL_set_bio( m_ptSsl, ptSbio, ptSbio );
  if( ::SSL_connect(m_ptSsl) <= 0 )
    {
    throw CSocketException("SSL_connect() error connecting to " + m_sHost);
    }
  std::cout << "Connected with SSL" << std::endl;
/*
  // sets ssl to work in client mode
  ::SSL_set_connect_state( m_ptSsl );
*/
  if ( !CertificateCheck() )
    {
    std::cout << " ** Certificate did not pass check" << std::endl;
    }
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

  switch( ::SSL_get_error(m_ptSsl, nResult) )
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

  // Tries to read <num bytes> from the specified ssl into the buffer.
  int nResult = ::SSL_read( m_ptSsl, m_pcBuffer, RECEIVE_BUFFER_SIZE -1 );
  if ( (nResult > 0) && (nResult < RECEIVE_BUFFER_SIZE) )
     {
     m_pcBuffer[nResult+1] = 0;
     }
  switch( ::SSL_get_error(m_ptSsl, nResult) )
    {
    case SSL_ERROR_WANT_READ:
    case SSL_ERROR_WANT_WRITE:
    case SSL_ERROR_NONE:
//      std::cout << "SSL Read Data: " << m_pcBuffer << std::endl;
      break;
    case SSL_ERROR_ZERO_RETURN:
      throw CSocketException( "SSL_ERROR_ZERO_RETURN" );
    case SSL_ERROR_SYSCALL:
      throw CSocketException( "SSL_ERROR_SYSCALL" );
    default:
      throw CSocketException( "Unknown SSL Error - Read problem" );
    }

  s = m_pcBuffer;
  return s;
  } // const std::string& CSocketSSL::Receive( std::string& s ) const


bool CSocketSSL::InitializeSslCtx()
  {
  if( !m_ptBioError )
    {
    std::cout << "Locad OpenSSL tools" << std::endl;

    // Global system initialization
    ::SSL_library_init();
    ::SSL_load_error_strings();

    // An error write context
    m_ptBioError = ::BIO_new_fp(stderr,BIO_NOCLOSE);
    }
  std::cout << "Initialize SSL" << std::endl;

  // Create our context
  SSL_METHOD* ptMethode;
  ptMethode   = ::TLSv1_method(); // TLSv1_method() SSLv2_method() SSLv23_method() SSLv3_method()
  m_ptSslCtx  = ::SSL_CTX_new( ptMethode );
  std::cout << "SSL Context created" << std::endl;

  // loads a certificate chain from file into ctx. The certificates must be in PEM format and must be sorted starting with the
  // subject's certificate (actual client or server certificate), followed by intermediate CA certificates if applicable, and ending
  // at the highest level (root) CA. There is no corresponding function working on a single SSL object.
  if ( ! ::SSL_CTX_use_certificate_chain_file(m_ptSslCtx, m_sFileCertificate.c_str()) )
    {
    throw CSocketException("Can't read certificate file");
    }
  std::cout << "Read certificate file" << std::endl;

  // sets the default password callback called when loading/storing a PEM certificate with encryption.
  ::SSL_CTX_set_default_passwd_cb         ( m_ptSslCtx, PasswordCallback );
  // sets a pointer to userdata which will be provided to the password callback on invocation.
  ::SSL_CTX_set_default_passwd_cb_userdata( m_ptSslCtx, this );
  std::cout << "PasswordCallback set" << std::endl;

  // adds the first private key found in file to ctx.
  // the formatting type of the certificate must be specified from the known types
  //    * SSL_FILETYPE_PEM
  //    * SSL_FILETYPE_ASN1
  if ( ! ::SSL_CTX_use_PrivateKey_file(m_ptSslCtx, m_sFileKey.c_str(), SSL_FILETYPE_PEM ) )
    {
    throw CSocketException("Can't read key file");
    }
  std::cout << "Read key file" << std::endl;

  // specifies the locations for ctx, at which CA certificates for verification purposes are located.
  // the certificates available via CAfile and CApath are trusted.
  if( ! ::SSL_CTX_load_verify_locations( m_ptSslCtx, 
                                        (m_sFileCaChainHost.length()) ? m_sFileCaChainHost.c_str() : 0, 
                                        (m_sTrustPathHost.length()  ) ? m_sTrustPathHost.c_str()   : 0) )
    {
    throw CSocketException("Can't read CA list");
    }
  std::cout << "Checked CA list" << std::endl;

  return true;
  }


// Check that 
//  (1) the host presented a certificate
//  (2) the common name from the certificate matches the server we called
bool CSocketSSL::CertificateCheck()
  {
  // Returns a pointer to the X509 certificate the peer presented. If the peer did not present a certificate, NULL is returned.
  // -----
  // !! The reference count of the X509 object is incremented by one, so that it will not be destroyed when the session containing
  //    the peer certificate is freed. The X509 object must be explicitly freed using X509_free(X509*). ==> class Cx509
  Cx509 oPeer = ::SSL_get_peer_certificate( m_ptSsl );

  if ( ! oPeer.isValid() )
    {
    throw CSocketException( "Peer did not present a certificate" );
    }

  // Returns the result of the verification of the X509 certificate presented by the peer, if any.
  int nResult = ::SSL_get_verify_result( m_ptSsl );
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
  std::cout << "Certificate issued for " + m_sHost << std::endl;


  char acPeerCommonName[256];
  // Retrieve the ``text'' from the first entry in name which matches nid, if no such entry exists -1 is returned. At most len bytes
  // will be written and the text written to buf will be null terminated. The length of the output string written is returned
  // excluding the terminating null. If buf is <NULL> then the amount of space needed in buf (excluding the final null) is returned
  ::X509_NAME_get_text_by_NID( X509_get_subject_name(oPeer),
                               NID_commonName,
                               acPeerCommonName,
                               sizeof(acPeerCommonName) );
  if( strcasecmp( acPeerCommonName, m_sHost.c_str() ))
    {
    throw CSocketException("Common name doesn't match host name");
    }
  std::cout << "Common name matches host name: " << acPeerCommonName << std::endl;

  return true;
  } // bool CSocketSSL::CertificateCheck()

// provides the password for the PasswordCallback(...)
const std::string& CSocketSSL::PasswordGet() const
  {
  return m_sPassword;
  } // const std::string& CSocketSSL::PasswordGet() const


/* -------------------------

       SERVER SPECIFIC

   ------------------------- */

void CSocketSSL::LoadDHParameters( const std::string& sParamsFile )
  {
  DH*  ptDH=0; // ret
  BIO* ptBio;  // bio

  if ( (ptBio = ::BIO_new_file( sParamsFile.c_str(),"r")) == NULL)
    {
    throw CSocketException( "Couldn't open DH file" );
    }

  ptDH = ::PEM_read_bio_DHparams( ptBio, NULL, NULL,  NULL);
  ::BIO_free( ptBio );

  if( ::SSL_CTX_set_tmp_dh(m_ptSslCtx, ptDH) < 0 )
    {
    throw CSocketException( "Couldn't set DH parameters" );
    }
  } // void CSocketSSL::LoadDHParameters( SSL_CTX* ptSslCtx, const std::string& sParamsFile)

void CSocketSSL::GenerateEphRsaKey()
  {
  RSA* ptRsa;

  ptRsa = ::RSA_generate_key( 512, RSA_F4, NULL, NULL );

  if ( !SSL_CTX_set_tmp_rsa(m_ptSslCtx, ptRsa) )
    {
    throw CSocketException( "Couldn't set RSA key" );
    }

  ::RSA_free( ptRsa );
  } // void CSocketSSL::GenerateEphRsaKey()
