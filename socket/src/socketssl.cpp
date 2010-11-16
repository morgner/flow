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

/* openssl ciphers -tls1

DHE-RSA-AES256-SHA:DHE-DSS-AES256-SHA:AES256-SHA:EDH-RSA-DES-CBC3-SHA:
EDH-DSS-DES-CBC3-SHA:DES-CBC3-SHA:DHE-RSA-AES128-SHA:DHE-DSS-AES128-SHA:
AES128-SHA:RC4-SHA:RC4-MD5:EDH-RSA-DES-CBC-SHA:EDH-DSS-DES-CBC-SHA:DES-CBC-SHA:
EXP-EDH-RSA-DES-CBC-SHA:EXP-EDH-DSS-DES-CBC-SHA:EXP-DES-CBC-SHA:EXP-RC2-CBC-MD5:
EXP-RC4-MD5

Example: s_sCiphers = "DHE-RSA-AES256-SHA:DHE-RSA-AES128-SHA"
*/

std::string CSocketSSL::s_sCiphers        = "";
bool        CSocketSSL::s_bSslInitialized = false;


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
  } // int CSocketSSL::PasswordCallback( char *pszBuffer, int nBufferSize, int nRWFlag, void* pUserData )


int VerifyCallback(int n, X509_STORE_CTX* ptX509_store_ctx)
  {
  return 1;
  }


// called as result of a call to Accept(), gemnerating a new CSocketSSL object
// to connet with a client
CSocketSSL::CSocketSSL( const CSocketSSL& src )
  : inherited( src.m_nSock ),
    m_sHost( src.m_sHost),
    m_sPort( src.m_sPort ),
    // TLS credentials
    m_sFileCertificate( src.m_sFileCertificate ),
    m_sFileKey( src.m_sFileKey ),
    m_sPassword( src.m_sPassword ),
    m_sFileCaChainTrust( src.m_sFileCaChainTrust ),
    m_sPathCaTrust( src.m_sPathCaTrust ),
    // pointer to NULL
    m_ptSslCtx( src.m_ptSslCtx ),
    m_ptSsl( src.m_ptSsl )
  {

  ( (CSocket&)src ).Accept( *this );

  BIO* ptSBio = ::BIO_new_socket( m_nSock, BIO_NOCLOSE );
  m_ptSsl     = ::SSL_new( m_ptSslCtx );
  ::SSL_set_bio( m_ptSsl, ptSBio, ptSBio);

  // When the SSL_CTX object was created with SSL_CTX_new(3), it was either
  // assigned a dedicated client method, a dedicated server method, or a generic
  // method, that can be used for both client and server connections.
  // When using the SSL_connect(3) or SSL_accept(3) routines, the correct
  // handshake routines are automatically set. When performing a transparent
  // negotiation using SSL_write(3) or SSL_read(3), the handshake routines must
  // be explicitly set in advance using either SSL_set_connect_state() or
  // SSL_set_accept_state()
  ::SSL_set_accept_state( m_ptSsl );
  std::cout << "Configured SSL to server mode" << std::endl;

  ::SSL_set_verify( m_ptSsl, SSL_VERIFY_PEER, VerifyCallback );
  std::cout << "SSL mode set to verify to force the client to send his certificate" << std::endl;

  int nResult = ::SSL_accept( m_ptSsl );

  int nError = ::SSL_get_error(m_ptSsl, nResult);
  switch( nError )
    {      
    case SSL_ERROR_NONE:
      std::cout << "Connection Accepted" << std::endl;
      break;

    case SSL_ERROR_SSL:
      throw CSocketException( "SSL accept problem: SSL_ERROR_SSL" );
      break;
    case SSL_ERROR_WANT_READ:
      std::cout << "SSL accept problem: SSL_ERROR_WANT_READ" << std::endl;
      break;
    case SSL_ERROR_WANT_WRITE:
      std::cout << "SSL accept problem: SSL_ERROR_WANT_WRITE" << std::endl;
      break;
    case SSL_ERROR_SYSCALL:
      throw CSocketException( "SSL accept problem: SSL_ERROR_SYSCALL" );
    case SSL_ERROR_ZERO_RETURN:
      throw CSocketException( "SSL accept problem: SSL_ERROR_ZERO_RETURN" );

    default:
      throw CSocketException( "SSL accept problem: " + to_string(nError) );
    }

  if ( nResult <= 0 )
    {
    throw CSocketException( "SSL accept error" );
    }

  BIO* ptFBio   = ::BIO_new( BIO_f_buffer() );
  BIO* ptSslBio = ::BIO_new( BIO_f_ssl() );
  ::BIO_set_ssl( ptSslBio, m_ptSsl, BIO_CLOSE);
  ::BIO_push   ( ptFBio, ptSslBio );

  CertificateCheck();
  } // CSocketSSL::CSocketSSL( const CSocketSSL& src )

/// for server connections (calling Accept()) ??
CSocketSSL::CSocketSSL( const int nSock )
  : inherited( nSock )
  {
  } // CSocketSSL::CSocketSSL( const int nSock )

/// ?? for client connection (calling 'Connect()')
CSocketSSL::CSocketSSL( const std::string& rsHost,
                        const std::string& rsPort,
                        const std::string& rsFileCertificate,
                        const std::string& rsFileKey,
                        const std::string& rsPassword,
                        const std::string& rsFileCaChainTrust,
                        const std::string& rsPathCaTrust )
  : m_sHost( rsHost),
    m_sPort( rsPort ),
    // TLS credentials
    m_sFileCertificate( rsFileCertificate ),
    m_sFileKey( rsFileKey ),
    m_sPassword( rsPassword ),
    m_sFileCaChainTrust( rsFileCaChainTrust ),
    m_sPathCaTrust( rsPathCaTrust ),
    // pointer to NULL
    m_ptSslCtx( 0 ),
    m_ptSsl( 0 )
  {
  if( !s_bSslInitialized )
    {
    std::cout << "Load OpenSSL tools" << std::endl;

    // Global system initialization
    ::SSL_library_init();
    ::SSL_load_error_strings();

    s_bSslInitialized = true;
    std::cout << "Initialize SSL" << std::endl;
    }

  InitializeSslCtx();
  std::cout << "Initialized SSL Context" << std::endl;

  } // CSocketSSL::CSocketSSL( const std::string& rsHost, ...

CSocketSSL::~CSocketSSL()
  {
  Close();
  } // CSocketSSL::~CSocketSSL()


CSocket* CSocketSSL::Accept ()
  {
  return new CSocketSSL( *this );
  } // CSocket* CSocketSSL::Accept ()


void CSocketSSL::Connect()
  {
  inherited::Connect( m_sHost, m_sPort );
  std::cout << "Simple TCP/IP connection to " + m_sHost + ":" + m_sPort
            << ", Socket: " << m_nSock << std::endl;

  if ( !isValid() )
    {
    throw CSocketException( "Could not get a conneciton to" + 
                            m_sHost + ":" + m_sPort );
    }

  /* ++++++++++++++++++++++++++++++++++++++++++++++++++++++
   *
   *   start opening the Secure Layer over the Socket
   *
   * ++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

  // Set our cipher list
  if ( s_sCiphers.length() )
    {
    // Sets the list of available ciphers for ctx using the control string str.
    // The list of ciphers is inherited by all ssl objects created from ctx.
    //   See: openssl ciphers -tls1
    ::SSL_CTX_set_cipher_list( m_ptSslCtx, s_sCiphers.c_str() );
    }
  std::cout << "Selected cipher list" << std::endl;

/* only server == here invalid
  int s_server_session_id_context = 1;
  SSL_CTX_set_session_id_context( m_ptSslCtx,
                                  (const unsigned char*)&s_server_session_id_context,
                                  sizeof(s_server_session_id_context) ); 
*/

  // SSL_new() creates a new SSL structure which is needed to hold the data for
  // a TLS/SSL connection. The new structure inherits the settings of the
  // underlying context ctx: connection method (SSLv2/v3/TLSv1), options,
  // verification settings, timeout settings.
  m_ptSsl = ::SSL_new( m_ptSslCtx );
  std::cout << "Created new SSL Context" << std::endl;

  // When the SSL_CTX object was created with SSL_CTX_new(3), it was either
  // assigned a dedicated client method, a dedicated server method, or a generic
  // method, that can be used for both client and server connections.
  // When using the SSL_connect(3) or SSL_accept(3) routines, the correct
  // handshake routines are automatically set. When performing a transparent
  // negotiation using SSL_write(3) or SSL_read(3), the handshake routines must
  // be explicitly set in advance using either SSL_set_connect_state() or
  // SSL_set_accept_state()
  ::SSL_set_connect_state( m_ptSsl );
  std::cout << "Configured SSL to client mode" << std::endl;

  // BIO_new_socket() returns a socket BIO using sock and close_flag. 
  // It returns the newly allocated BIO or NULL is an error occurred.
  BIO* ptSbio = ::BIO_new_socket( m_nSock, BIO_NOCLOSE );
  std::cout << "Created SocketBIO" << std::endl;

  // SSL_set_bio() connects the BIOs rbio and wbio for the read and write
  // operations of the TLS/SSL (encrypted) side of ssl.
  // The SSL engine inherits the behaviour of rbio and wbio, respectively. If a
  // BIO is non-blocking, the ssl will also have non-blocking behaviour.
  // If there was already a BIO connected to ssl, BIO_free() will be called (for
  // both the reading and writing side, if different).
  ::SSL_set_bio( m_ptSsl, ptSbio, ptSbio );
  std::cout << "Associated BIO with SSL" << std::endl;

  // Initiates the TLS/SSL handshake with a server. The communication channel
  // must already have been set and assigned to the ssl by setting an underlying
  // BIO.
  if( ::SSL_connect(m_ptSsl) <= 0 )
    {
    throw CSocketException("SSL_connect() error connecting to " + m_sHost);
    }
  std::cout << "Connected with SSL" << std::endl;

  ::SSL_do_handshake( m_ptSsl );

  if ( !CertificateCheck() )
    {
    std::cout << " ** Certificate did not pass check" << std::endl;
    }
  std::cout << "Host is ok" << std::endl;
  } // void CSocketSSL::Connect()


void CSocketSSL::Close()
  {
  if ( !m_ptSsl ) return;

  // SSL_shutdown() his is a complicated matter,
  // See: http://www.openssl.org/docs/ssl/SSL_shutdown.html
  int nResult = ::SSL_shutdown( m_ptSsl );
  if( nResult != 1 )
    {
    std::cout << "SSL_shutdown first  result: " << nResult << std::endl;
    // destroys the socket (?) after sending FIN/ACK regardless of the usage counter 
    ::shutdown( m_nSock, SHUT_RDWR );
    nResult = ::SSL_shutdown( m_ptSsl );
    } // if ( nResult != 1 )

  switch( nResult )
    {  
    case  1:
      std::cout << "SSL_shutdown completed successful" << std::endl;
      break; // ok both sides negotiated the shutdown status (not neccessary)
    case  0:
    case -1:
    default:
      std::cout << "SSL_shutdown second result: " << nResult << std::endl;
//      throw CSocketException("Shutdown failed");
      break;
    }

  // SSL_free() decrements the reference count of ssl, and removes the SSL 
  // structure pointed to by ssl and frees up the allocated memory if the
  // reference count has reached 0.
  if ( m_ptSsl )
    {
    ::SSL_free ( m_ptSsl );
    }

  // SSL_CTX_free() decrements the reference count of ctx, and removes the
  // SSL_CTX object pointed to by ctx and frees up the allocated memory if the
  // the reference count has reached 0.
  if ( m_ptSslCtx )
    {
    ::SSL_CTX_free( m_ptSslCtx );
    }

  m_ptSsl = 0;
  m_ptSslCtx = 0;
  inherited::Close();
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


size_t CSocketSSL::Send( const std::string& s ) const
  {
  // SSL_write() writes num bytes from the buffer buf into the specified
  // ssl connection.
  size_t nResult = ::SSL_write( m_ptSsl, s.c_str(), s.length() );

  switch( ::SSL_get_error(m_ptSsl, nResult) )
    {      
    case SSL_ERROR_NONE:
      if ( s.length() != nResult )
        {
        std::cout << "Incomplete write!" << std::endl;
        }
      return nResult;
      break;

    case SSL_ERROR_WANT_READ:
      throw CSocketException("SSL_ERROR_WANT_READ");
    case SSL_ERROR_WANT_WRITE:
      throw CSocketException("SSL_ERROR_WANT_WRITE");
    default:
      throw CSocketException("SSL write problem");
    }
  } // void CSocketSSL::Send( const std::string& s ) const


const std::string& CSocketSSL::Receive( std::string& s ) const
  {
  // Tries to read <num bytes> from the specified ssl into the buffer.
  int nResult = ::SSL_read( m_ptSsl, m_pcBuffer, RECEIVE_BUFFER_SIZE -1 );
  if ( (nResult > 0) && (nResult < RECEIVE_BUFFER_SIZE) )
     {
     m_pcBuffer[nResult+1] = 0;
     }
  switch( ::SSL_get_error(m_ptSsl, nResult) )
    {
    case SSL_ERROR_NONE:
//      std::cout << "SSL Read Data: " << m_pcBuffer << std::endl;
      break;

    case SSL_ERROR_WANT_READ:
      throw CSocketException("read timeout (SSL_ERROR_WANT_READ)");
    case SSL_ERROR_WANT_WRITE:
      throw CSocketException("SSL_ERROR_WANT_WRITE");
    case SSL_ERROR_ZERO_RETURN:
      throw CSocketException( "SSL_ERROR_ZERO_RETURN" );
    case SSL_ERROR_SYSCALL:
      throw CSocketException( "remote disconnect (SSL_ERROR_SYSCALL)", true );
    default:
      throw CSocketException( "Unknown SSL Error - Read problem" );
    }

  s = m_pcBuffer;
  return s;
  } // const std::string& CSocketSSL::Receive( std::string& s ) const


bool CSocketSSL::InitializeSslCtx()
  {
  // A TLS/SSL connection established with these methods will only understand 
  // the TLSv1 protocol. A client will send out TLSv1 client hello messages and
  // will indicate that it only understands TLSv1. A server will only under-
  // stand TLSv1 client hello messages.
  //  * TLSv1_method(), SSLv2_method(), SSLv23_method(), SSLv3_method()
  SSL_METHOD* ptMethode = ::TLSv1_method(); 

  // SSL_CTX_new() creates a new SSL_CTX object as framework to establish
  // TLS/SSL enabled connections.
  m_ptSslCtx  = ::SSL_CTX_new( ptMethode );
  std::cout << "SSL Context created" << std::endl;

  // loads a certificate chain from file into ctx. The certificates must be in
  // PEM format and must be sorted starting with the subject's certificate
  // (actual client or server certificate), followed by intermediate CA certi-
  // ficates if applicable, and ending at the highest level (root) CA. There
  // is no corresponding function working on a single SSL object.
  if ( ! ::SSL_CTX_use_certificate_chain_file(m_ptSslCtx,
                                              m_sFileCertificate.c_str()) )
    {
    throw CSocketException("Can't read certificate file: " + m_sFileCertificate);
    }
  std::cout << "Read certificate file: " + m_sFileCertificate << std::endl;

  // sets the default password callback called when loading/storing a
  // PEM certificate with encryption.
  ::SSL_CTX_set_default_passwd_cb( m_ptSslCtx, PasswordCallback );

  // sets a pointer to userdata which will be provided to the password
  // callback on invocation.
  ::SSL_CTX_set_default_passwd_cb_userdata( m_ptSslCtx, this );

  // Adds the first private key found in file to ctx.
  // The formatting type of the certificate must be specified from:
  //    * SSL_FILETYPE_PEM
  //    * SSL_FILETYPE_ASN1
  if ( ! ::SSL_CTX_use_PrivateKey_file(m_ptSslCtx,
                                       m_sFileKey.c_str(),
                                       SSL_FILETYPE_PEM ) )
    {
    throw CSocketException("Can't read key file");
    }
  std::cout << "Read key file" << std::endl;

  // Specifies the locations for ctx, at which CA certificates for verification
  // purposes are located. The certificates available via CAfile and CApath
  // are trusted.
  bool bHasCaChain  = m_sFileCaChainTrust.length() > 0;
  bool bHasTrustDir = m_sPathCaTrust.length()      > 0;
  std::cout << "CA chain :" << m_sFileCaChainTrust << std::endl;
  std::cout << "CA path  :" << m_sPathCaTrust      << std::endl;
  if( ! ::SSL_CTX_load_verify_locations( m_ptSslCtx, 
                               (bHasCaChain)  ? m_sFileCaChainTrust.c_str() : 0,
                               (bHasTrustDir) ? m_sPathCaTrust.c_str()      : 0) )
    {
    throw CSocketException("Can't read CA list");
    }
  std::cout << "Read verify CA chain :" << m_sFileCaChainTrust << std::endl;
  std::cout << "Read verify CA path  :" << m_sPathCaTrust      << std::endl;

  return true;
  } // bool CSocketSSL::InitializeSslCtx()


// Check that 
//  (1) the host presented a certificate
//  (2) the common name from the certificate matches the server we called
bool CSocketSSL::CertificateCheck()
  {
  // Returns a pointer to the X509 certificate the peer presented. If the peer
  // did not present a certificate, NULL is returned.
  // -----
  // !! The reference count of the X509 object is incremented by one, so that
  // it will not be destroyed when the session containing the peer certificate
  // is freed. The X509 object must be explicitly freed using X509_free(X509*).
  //     Theswhy ==> class Cx509
  Cx509 oPeer = ::SSL_get_peer_certificate( m_ptSsl );

  if ( ! oPeer.isValid() )
    {
    throw CSocketException( "Peer did not present a certificate" );
    }

  // Returns the result of the verification of the X509 certificate presented
  // by the peer, if any.
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
        sError = "Self signed certificate and the certificate cannot be found "
                 "in the list of trusted certificates";
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
        sError = "Certificate rejected (the root CA is marked to reject the "
                 "specified purpose)";
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


  char acPeerCommonName[256];
  // Retrieve the ``text'' from the first entry in name which matches nid, if no
  // such entry exists -1 is returned. At most len bytes will be written and the
  // text written to buf will be null terminated. The length of the output
  // string written is returned excluding the terminating null. If buf is NULL
  // then the amount of space needed in buf (excluding the final null) is
  // returned
  ::X509_NAME_get_text_by_NID( X509_get_subject_name(oPeer),
                               NID_commonName,
                               acPeerCommonName,
                               sizeof(acPeerCommonName) );
  if( strcasecmp(acPeerCommonName, m_sHost.c_str()) )
    {
//-    throw CSocketException("Certificate CN doesn't match host name " +
//-                           std::string(acPeerCommonName) );
    }

  m_sPeerCN = acPeerCommonName;
  if ( m_sPeerCN.length() )
    {
    std::cout << "Peer certificate issued to: " + m_sPeerCN << std::endl;
    }

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
  std::cout << "void CSocketSSL::LoadDHParameters( " << sParamsFile << " )" << std::endl;

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
  } // void CSocketSSL::LoadDHParameters( SSL_CTX* ptSslCtx, const std::...)

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
