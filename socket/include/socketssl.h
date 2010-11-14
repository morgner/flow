/***************************************************************************
 socketssl.h  description
 -----------------------
 begin                 : Thu Nov 09 2010
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


#ifndef _SOCKET_SSL_H
#define _SOCKET_SSL_H

#include "socket.h"

#include <string>

#include <openssl/ssl.h>

class CSocketSSL : public CSocket
  {
  private:
    typedef CSocket inherited;

    static std::string s_sCiphers;

  class Cx509
    {
    protected:
      X509* m_ptPeer;

       Cx509(){}
    public:
       Cx509( X509* ptPeer ) : m_ptPeer(ptPeer) {}
      ~Cx509()         { X509_free(m_ptPeer); }

      operator X509*() { return m_ptPeer; }
      bool isValid()   { return m_ptPeer != 0; }
    }; // class Cx509

  protected:
    std::string m_sHost;             // the name oder IP of the server
    std::string m_sPort;             // the port at the server
    std::string m_sFileCertificate;  // the certificate for the user
    std::string m_sFileKey;          // the key for the user
    std::string m_sPassword;         // the password for the key
    std::string m_sFileCaChainClient;// the CA chain for the user
    std::string m_sFileCaChainHost;  // the CA chain for the server
    std::string m_sTrustPathHost;    // not used yet

    SSL_CTX*    m_ptSslCtx;          // the SSL Context
    SSL*        m_ptSsl;             // the SSL socket
    BIO*        m_ptBio;             // 
    BIO*        m_ptSslBio;          // 
    BIO*        m_ptBioError;        // 

  public:
             CSocketSSL( const int          nSock           = INVALID_SOCKET );
             CSocketSSL( const std::string& rsHost,
                         const std::string& rsPort,
                         const std::string& rsFileCertificate,
                         const std::string& rsFileKey,
                         const std::string& rsPassword,
                         const std::string& rsFileCaChainClient,
                         const std::string& rsFileCaChainHost );
    virtual ~CSocketSSL();

    virtual       void         Accept  (       CSocketSSL& roSocket );
    virtual       CSocket*     Accept  ();
    virtual       void         Connect ();
    virtual       void         Close   ();
    virtual       void         Send    ( const std::string& s ) const;
    virtual const std::string& Receive (       std::string& s ) const;

  public:
    const CSocketSSL& operator << ( const std::string& s ) const;
    const CSocketSSL& operator << (       long         n ) const;
    const CSocketSSL& operator >> (       std::string& s ) const;

  protected:
    bool InitializeSslCtx();
    bool CertificateCheck();

  protected:
           const std::string& PasswordGet     () const;
    static       int          PasswordCallback( char* pszBuffer, int nBufferSize, int nRWFlag, void* pUserData );


  // server specific

    void LoadDHParameters ( const std::string& sParamsFile );
    void GenerateEphRsaKey();


  }; // class SocketSSL

#endif // _SOCKET_SSL_H
