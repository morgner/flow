/***************************************************************************
 home.cpp -  description
 -----------------------
 begin                 : Fri Oct 29 2010
 copyright             : Copyright (C) 2010 by Manfred Morgner
 email                 : manfred@morgner.com
 
$Id: home.cpp,v 1.6 2010/10/29 17:15:44 morgner Exp $
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

#include "home.h"

#include <sys/time.h>

const std::string CHome::s_sClassName   = "HOME";
      std::string CHome::s_sUsername    = "username";
      std::string CHome::s_sPassword    = "password";
      long        CHome::s_nLastLocalId = 0;

CHome::CHome()
  {
  timeval time;
  gettimeofday( &time, 0 );
  m_tLocalId = time.tv_sec;
  m_nLocalId = ++s_nLastLocalId;
  }

CHome::~CHome()
  {
  }

const std::string& CHome::ClassNameGet() const
  {
  return s_sClassName;
  }

long CHome::LocalIdGet()
  {
  return m_nLocalId;
  }

long CHome::LocalIdTimeGet()
  {
  return m_tLocalId;
  }

long CHome::RemoteIdSet( long lid )
  {
  return m_nRemoteId = lid;
  }

long CHome::RemoteIdGet()
  {
  return m_nRemoteId;
  }

const std::string& CHome::UsernameSet( const std::string& sUsername )
  {
  return s_sUsername = sUsername;
  }

const std::string& CHome::UsernameGet()
  {
  return s_sUsername;
  }

const std::string& CHome::PasswordSet( const std::string& sPassword )
  {
  return s_sPassword = sPassword;
  }

const std::string& CHome::PasswordGet()
  {
  return s_sPassword;
  }

