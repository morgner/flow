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

const std::string CContainer::s_sClassName = "FLOW.CONTAINER";


CContainer::CContainer()
  {
  }

CContainer::~CContainer()
  {
  std::cout << " ** destruction of " << RGUIDGet() << std::endl;
  }



const std::string& CContainer::SenderSet( const std::string& rsSender )
  {
  return m_sSender = rsSender;
  }


const std::string& CContainer::SenderGet() const
  {
  return m_sSender;
  }


// add the recipient to the recipient list
size_t CContainer::RecipientAdd( const std::string& rsRecipient )
  {
  for ( CListString::iterator it  = m_lsRecipients.begin();
                             it != m_lsRecipients.end();
                           ++it )
    {
    if ( *it == rsRecipient ) return m_lsRecipients.size();
    }
  m_lsRecipients.push_back( rsRecipient );
  return m_lsRecipients.size();
  }


// remove all recipient entries for the given alias
size_t CContainer::RecipientDel( const std::string& rsRecipient )
  {
  m_lsRecipients.remove( rsRecipient );
  return m_lsRecipients.size();
  }


long CContainer::ClientSideTmGet() const
  {
  return m_tClientSideId;
  }


long CContainer::ClientSideTmSet( long nClientSideTm )
  {
  return m_tClientSideId = nClientSideTm;
  }


long CContainer::ClientSideTmSet( const std::string& rsClientSideTm )
  {
  return m_tClientSideId = atol( rsClientSideTm.c_str() );
  }


long CContainer::ClientSideIdGet() const
  {
  return m_nClientSideId;
  }


long CContainer::ClientSideIdSet( long nClientSideId )
  {
  return m_nClientSideId = nClientSideId;
  }


long CContainer::ClientSideIdSet( const std::string& rsClientSideId )
  {
  return m_nClientSideId = atol( rsClientSideId.c_str() );
  }


long CContainer::ServerSideIdGet() const
  {
  return m_nServerSideId;
  }


long CContainer::ServerSideIdSet( long rnServerSideId )
  {
  return m_nServerSideId = rnServerSideId;
  }


long CContainer::ServerSideIdSet( const std::string& rsServerSideId )
  {
  return m_nServerSideId = atol( rsServerSideId.c_str() );
  }



#include <sstream>
template <typename T>
  std::string to_string(const T& val)
    {
    std::ostringstream sout;
    sout << val;
    return sout.str();
    }

std::string CContainer::RGUIDGet() const
  {
  std::ostringstream sout;
  sout << "RGUID"         << ":"
       << m_sClass        << ":"
       << m_nClientSideId << ":"
       << m_tClientSideId << ":"
       << m_nServerSideId;
  return sout.str();
  }

std::string CContainer::CLUIDGet() const
  {
  std::ostringstream sout;
  sout << "CLUID"         << ":"
       << m_sClass        << ":"
       << m_sSender       << ":"
       << m_nClientSideId;
  return sout.str();
  }

const std::string& CContainer::OwnerGet() const
  {
  return m_sSender;
  }

bool CContainer::isFor( const std::string& rsRecipient )
  {
  for ( CListString::iterator it = m_lsRecipients.begin(); it != m_lsRecipients.end(); ++it)
    {
    if ( *it == rsRecipient ) return true;
    }
  return false;
  }

// indicators to be used to transport the pulex
const char* CContainer::scn_sender         = "s";
const char* CContainer::scn_recipient      = "e";
const char* CContainer::scn_class_name     = "o";
const char* CContainer::scn_local_id       = "l";
const char* CContainer::scn_local_id_time  = "t";
const char* CContainer::scn_remote_id      = "r";
const char* CContainer::scn_content_text   = "x";
const char* CContainer::scn_content_binary = "b";

// e.g. "c:PULEX", "x:please read", "x:the manual"
const std::string& CContainer::operator += ( const std::string& rsElement )
  {
  if ( rsElement[1] == ':' )
    switch ( rsElement[0] )
      {
      case 's': m_sSender               = rsElement.substr(2);   break;
      case 'e': m_lsRecipients.push_back( rsElement.substr(2) ); break;
      case 'o': m_sClass                = rsElement.substr(2);   break;
      case 'l': m_nClientSideId         = atol( rsElement.substr(2).c_str() ); break;
      case 't': m_tClientSideId         = atol( rsElement.substr(2).c_str() ); break;
      case 'r': m_nServerSideId         = atol( rsElement.substr(2).c_str() ); break;

      case 'x':
      case 'b': push_back( rsElement.substr(2) );
      }

  return rsElement;
  }

