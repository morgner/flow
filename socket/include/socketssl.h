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


class CSocketSSL : public CSocket
  {
  private:
    typedef CSocket inherited;

  protected:
    std::string m_sCertificate;
    std::string m_sCA;

  public:
             CSocketSSL( const int          nSock           = INVALID_SOCKET );
             CSocketSSL( const std::string& rsCertificate,
                         const std::string& rsCA,
                         const int          nSock           = INVALID_SOCKET );
    virtual ~CSocketSSL();

    void     ConnectSSL( const std::string& rsCertificate,
                         const std::string& rsCA,
                         const std::string& rsHost,
                         const std::string& rsPort );

  }; // class SocketSSL

#endif // _SOCKET_SSL_H
