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

    static std::string s_sPassword;
    static std::string s_sCiphers;


  protected:
    std::string m_sCertificate;
    std::string m_sCA;
    SSL_CTX*    m_ptSslCtx;
    SSL*        m_ptSsl;
    BIO*        m_ptBio;
    BIO*        m_ptSslBio;
    BIO*        m_ptBioError;

  public:
             CSocketSSL( const int          nSock           = INVALID_SOCKET );
             CSocketSSL( const std::string& rsCertificate,
                         const std::string& rsCA,
                         const int          nSock           = INVALID_SOCKET );
    virtual ~CSocketSSL();

    virtual       void         Accept (       CSocketSSL& roSocket );
    virtual       CSocket*     Accept ();
    virtual       void         Connect( const std::string& rsHost,
                                        const std::string& rsPort );

    virtual       void         Connect( const std::string& rsCertificate,
                                        const std::string& rsCA,
                                        const std::string& rsHost,
                                        const std::string& rsPort );


    virtual       void         Close   ();

    virtual       void         Send    ( const std::string& s ) const;
    virtual const std::string& Receive (       std::string& s ) const;

  public:
    const CSocketSSL& operator << ( const std::string& s ) const;
    const CSocketSSL& operator << (       long         n ) const;
    const CSocketSSL& operator >> (       std::string& s ) const;




    SSL_CTX* InitializeSslCtx( const std::string rsKeyfile,
                               const std::string rsPassword  = "" );

    bool CertificateCheck(       SSL*         ptSsl,
                           const std::string& rsHost);

  protected:
    static int PasswordCallback( char* pszBuffer, int nBufferSize, int nRWFlag, void* pUserData );

  // server specific

    void LoadDHParameters ( const std::string& sParamsFile );
    void GenerateEphRsaKey();


  }; // class SocketSSL

#endif // _SOCKET_SSL_H
