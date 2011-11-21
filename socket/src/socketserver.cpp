/***************************************************************************
 socketserver.cpp
 -----------------------
 begin                 : Fri Oct 29 2010
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


#include "socketserver.h"
#include <stdlib.h>          // atoi

CSocketServer::CSocketServer( const std::string& rsHost,
                              const std::string& rsPort,
                                         bool     bVerbose )
  : m_sHost(rsHost),
    m_sPort(rsPort)
  {
  m_bVerbose = bVerbose;
  Create();
  Bind( atoi( rsPort.c_str() ) );
  Listen();
  } // CSocketServer::CSocketServer( const std::string& rsHost,

CSocketServer::~CSocketServer()
  {
  } // 	CSocketServer::~CSocketServer()
