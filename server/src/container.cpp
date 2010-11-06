/***************************************************************************
 container.cpp
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


#include "container.h"

#include <iostream>

CContainer::CContainer()
  {
  }

CContainer::~CContainer()
  {
  std::cout << "                      destruction of " << RGUIDGet() << std::endl;
  }

const std::string& CContainer::ServerIdGet() const
  {
  return m_sServerId;
  }

const std::string& CContainer::ServerIdSet( const std::string& rsServerId )
  {
  m_sServerId = rsServerId;
  return m_sServerId;
  }


std::string CContainer::RGUIDGet() const
  {
  return "RGUID:" + m_sClass + ":"
                  + m_sSender + ":"
                  + m_sClientLocalId + ":"
                  + m_sClientLocalIdTime + ":"
                  + m_sServerId ;
  }

std::string CContainer::CLUIDGet() const
  {
  return "CLUID:" + m_sClass + ":"
                  + m_sSender + ":"
                  + m_sClientLocalId;
  }

const std::string& CContainer::OwnerGet() const
  {
  return m_sSender;
  }


// e.g. "c:PULEX", "x:please read", "x:the manual"
const std::string& CContainer::operator += ( const std::string& rsElement )
  {
  if ( rsElement[1] == ':' )
    switch (rsElement[0])
      {
      case 'u': m_sSender              = rsElement.substr(2);   break;
      case 'p':                                                 break;
      case 'd': m_lsReceivers.push_back( rsElement.substr(2) ); break;
      case 'c': m_sClass               = rsElement.substr(2);   break;
      case 'l': m_sClientLocalId       = rsElement.substr(2);   break;
      case 't': m_sClientLocalIdTime   = rsElement.substr(2);   break;
      case 'r': m_sServerId            = rsElement.substr(2);   break;

      case 'x':
      case 'b': push_back( rsElement.substr(2) );
      }

  return rsElement;
  }

