/***************************************************************************
 socket.cpp  description
 -----------------------
 begin                 : Fri Oct 29 2010
 copyright             : Copyright (C) 2010 by Manfred Morgner
 email                 : manfred@morgner.com
 
$Id: socket.cpp,v 1.6 2010/10/29 17:15:44 morgner Exp $
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


#include "socket.h"
#include "socketexception.h"

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <iostream>


CSocket::CSocket( const int nSock )
  : m_nSock( nSock )
  {
  }

CSocket::~CSocket()
  {
  if ( isValid() )
    {
    ::close(m_nSock);
    m_nSock = INVALID_SOCKET;
    }
  }

void CSocket::Create()
  {
  m_nSock = ::socket(AF_INET,
                     SOCK_STREAM,
                     0);

  if ( ! isValid() )
    {
    std::cout << "errno == " << errno << "  in CSocket::Receive\n";
    throw CSocketException( "Could not create socket in CSocket::Create()." );
    }

  int on = 1;
  int result = setsockopt(m_nSock,
                          SOL_SOCKET,
                          SO_REUSEADDR,
                          (const char*)&on,
                          sizeof(on));
  if ( result == -1 )
    {
    throw CSocketException ( "Could not 'setsockopt' SO_REUSEADDR in CSocket::Create()." );
    }

  }


void CSocket::Bind ( const int nPort )
  {
  if ( ! isValid() )
    {
    throw CSocketException( "Socket is invalid in CSocket::Bind()." );
    }
  
  m_tAddr.sin_family      = AF_INET;
  m_tAddr.sin_addr.s_addr = INADDR_ANY; // inet_addr("127.0.0.1"); // INADDR_ANY;
  m_tAddr.sin_port        = htons ( nPort );
  
  int result = ::bind(m_nSock,
                      m_tAddr,
                      m_tAddr.SizeGet());

  if ( result == -1 )
    {
    throw CSocketException( "Could not bind to port in CSocket::Bind." );
    }
  }


void CSocket::Listen() const
  {
  if ( ! isValid() )
    {
    throw CSocketException( "Socket is invalid in CSocket::Listen()." );
    }

  int result = ::listen( m_nSock, CLIENT_BACKLOG );

  if ( result == -1 )
    {
    throw CSocketException( "Could not listen to socket ind CSocket::Listen()." );
    }
  }

CSocket* CSocket::Accept () const
  {
  CSocket* poSocket = new CSocket;
  Accept( *poSocket );
  return poSocket;
  }

void CSocket::Accept( CSocket& roSocket ) const
  {
  if ( ! isValid() )
    {
    throw CSocketException( "Socket is invalid in CSocket::Accept()." );
    }

  socklen_t addr_length = m_tAddr.SizeGet();
  roSocket.m_nSock = ::accept( m_nSock,
                               m_tAddr,
                               &addr_length );
  
  if ( roSocket.m_nSock <= 0 )
    {
    throw CSocketException( "Could not 'accept' in CSocket::Accept()." );
    }

  timeval timeout;
  timeout.tv_sec  = 0;
  timeout.tv_usec = 100;
  
  int result = setsockopt( roSocket.m_nSock,
                           SOL_SOCKET,
                           SO_RCVTIMEO,
                           (const char*)&timeout,
                           sizeof(timeout) );
  
  if ( result == -1 )
    {
    throw CSocketException ( "Could not 'setsockopt' SO_RCVTIMEO in CSocket::Create()." );
    }

  }


void CSocket::Close( )
  {
  if ( ! isValid() )
    {
    return;
    }
  
  int result = ::close( m_nSock );
  
  if ( result == -1 )
    {
    throw CSocketException( "Could not 'close' in CSocket::Close()." );
    }

  m_nSock = INVALID_SOCKET;
  }


const CSocket& CSocket::operator << ( const std::string& s ) const
  {
  Send(s);
  return *this;
  }


const CSocket& CSocket::operator << ( long n ) const
  {
  Send( to_string(n) );
  return *this;
  }


const CSocket& CSocket::operator >> ( std::string& s ) const
  {
  Receive(s);
  return *this;
  }


void CSocket::Send(const std::string& s) const
  {
  if ( ! isValid() )
    {
    throw CSocketException( "Socket is invalid in CSocket::Send()." );
    }

  int status = ::send(m_nSock, 
                      s.c_str(),
                      s.size(),
                      0);

  if ( status == -1 )
    {
    throw CSocketException( "Could not write to socket." );
    }
  }


const std::string& CSocket::Receive( std::string& s ) const
  {
  if ( ! isValid() )
    {
    throw CSocketException( "Socket is invalid in CSocket::Reeive()." );
    }
  s = "";

  char cBuffer[RECEIVE_BUFFER_SIZE];

  int result = ::recv( m_nSock,
                       cBuffer,
                       RECEIVE_BUFFER_SIZE -1,
                       0 );
  if ( (result == -1) and ( errno != EAGAIN ) )
    {
    std::cout << "errno == " << errno << "  in CSocket::Receive\n";
    throw CSocketException( "Could not read from socket in CSocket::Receive()." );
    }
  else
    if ( result > 0 )
      {
      cBuffer[result] = 0;
      s = cBuffer;
      }
  return s;
  }


void CSocket::Connect(const std::string sHost,
                      const int         nPort )
  {
  if ( ! isValid() )
    {
    throw CSocketException( "Socket is invalid in CSocket::Connect()." );
    }

  m_tAddr.sin_family = AF_INET;
  m_tAddr.sin_port   = htons(nPort);
  
  int result = inet_pton(AF_INET,
                         sHost.c_str(), 
                         &m_tAddr.sin_addr);

  if ( errno == EAFNOSUPPORT )
    {
    throw CSocketException( "Error EAFNOSUPPORT in CSocket::Connect()." );
    }

  result = ::connect(m_nSock,
                     m_tAddr,
                     m_tAddr.SizeGet());

  if ( result != 0 )
    {
    throw CSocketException( "Could not connect to host in CSocket::Connect()." );
    }
  }


bool CSocket::isValid() const
  {
  return m_nSock != INVALID_SOCKET;
  }

