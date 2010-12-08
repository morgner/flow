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
#include <stdlib.h>

#include <sys/time.h> // gettimeofday()


using namespace std;

const string CContainer::s_sClassName = "FLOW.CONTAINER";


CContainer::CContainer()
  : m_bHasId( false )
  {
  timeval time;
  gettimeofday( &time, 0 );
  m_tClientSideId = time.tv_sec;
  } // CContainer::CContainer()

CContainer::~CContainer()
  {
//  cout << " ** destruction of " << RGUIDGet() << endl;
  } // CContainer::~CContainer()



const string& CContainer::SenderSet( const string& rsSender )
  {
  return m_sSender = rsSender;
  } // const string& CContainer::SenderSet( const string& rsSender )


const string& CContainer::SenderGet() const
  {
  return m_sSender;
  } // const string& CContainer::SenderGet() const


// add the recipient to the recipient list
size_t CContainer::RecipientAdd( const string& rsRecipient )
  {
  for ( CListString::iterator it  = m_lsRecipients.begin();
                              it != m_lsRecipients.end();
                            ++it )
    {
    if ( *it == rsRecipient ) return m_lsRecipients.size();
    }
  m_lsRecipients.push_back( rsRecipient );
  return m_lsRecipients.size();
  } // size_t CContainer::RecipientAdd( const string& rsRecipient )


// remove all recipient entries for the given alias
size_t CContainer::RecipientDel( const string& rsRecipient )
  {
  m_lsRecipients.remove( rsRecipient );
  return m_lsRecipients.size();
  } // size_t CContainer::RecipientDel( const string& rsRecipient )


long CContainer::ClientSideTmGet() const
  {
  return m_tClientSideId;
  } // long CContainer::ClientSideTmGet() const


long CContainer::ClientSideTmSet( long nClientSideTm )
  {
  return m_tClientSideId = nClientSideTm;
  } // long CContainer::ClientSideTmSet( long nClientSideTm )


long CContainer::ClientSideTmSet( const string& rsClientSideTm )
  {
  return m_tClientSideId = atol( rsClientSideTm.c_str() );
  } // long CContainer::ClientSideTmSet( const string& rsClientSideTm )


// The unique client ID for a Container object
long CContainer::ClientSideIdGet()
  {
  static int nClientSideId = 0;
  if ( m_bHasId )
    {
    return m_nClientSideId;
    }
  else
    {
    m_bHasId = true;
    return m_nClientSideId = ++nClientSideId;
    }
  } // long CContainer::ClientSideIdGet()


long CContainer::ClientSideIdSet( long nClientSideId )
  {
  m_bHasId = true;
  return m_nClientSideId = nClientSideId;
  } // long CContainer::ClientSideIdSet( long nClientSideId )


long CContainer::ClientSideIdSet( const string& rsClientSideId )
  {
  return m_nClientSideId = atol( rsClientSideId.c_str() );
  } // long CContainer::ClientSideIdSet( const string& rsClientSideId )


long CContainer::ServerSideIdGet() const
  {
  return m_nServerSideId;
  } // long CContainer::ServerSideIdGet() const


long CContainer::ServerSideIdSet( long rnServerSideId )
  {
  return m_nServerSideId = rnServerSideId;
  } // long CContainer::ServerSideIdSet( long rnServerSideId )


long CContainer::ServerSideIdSet( const string& rsServerSideId )
  {
  return m_nServerSideId = atol( rsServerSideId.c_str() );
  } // long CContainer::ServerSideIdSet( const string& rsServerSideId )



#include <sstream>
template <typename T>
  string to_string(const T& val)
    {
    ostringstream sout;
    sout << val;
    return sout.str();
    } // template <typename T> string to_string(const T& val)

string CContainer::RGUIDGet()
  {
  ostringstream sout;
  sout << "RGUID"           << ":"
       << ClientSideIdGet() << ":"
       << ClientSideTmGet() << ":"
       << ServerSideIdGet();
  return sout.str();
  } // string CContainer::RGUIDGet()


string CContainer::CLUIDGet()
  {
  ostringstream sout;
  sout << "CLUID"         << ":"
       << m_sSender       << ":"
       << ClientSideIdGet();
  return sout.str();
  } // string CContainer::CLUIDGet()


const string& CContainer::OwnerGet() const
  {
  return m_sSender;
  } // const string& CContainer::OwnerGet() const


bool CContainer::isFor( const string& rsRecipient )
  {
  for ( CListString::iterator it = m_lsRecipients.begin(); it != m_lsRecipients.end(); ++it)
    {
    if ( *it == rsRecipient ) return true;
    }
  return false;
  } // bool CContainer::isFor( const string& rsRecipient )


// indicators to be used to transport the pulex
const char* CContainer::scn_sender         = "s";
const char* CContainer::scn_recipient      = "e";
const char* CContainer::scn_local_id       = "l";
const char* CContainer::scn_local_id_time  = "t";
const char* CContainer::scn_remote_id      = "r";
const char* CContainer::scn_content_text   = "x";

// e.g. "c:PULEX", "x:please read", "x:the manual"
const string& CContainer::operator += ( const string& rsElement )
  {
  if ( rsElement[1] == ':' )
    switch ( rsElement[0] )
      {
      case 's': m_sSender               = rsElement.substr(2);   break;
      case 'e': m_lsRecipients.push_back( rsElement.substr(2) ); break;
      case 'l': m_nClientSideId         = atol( rsElement.substr(2).c_str() ); break;
      case 't': m_tClientSideId         = atol( rsElement.substr(2).c_str() ); break;
      case 'r': m_nServerSideId         = atol( rsElement.substr(2).c_str() ); break;

      case 'x': push_back( rsElement.substr(2) );
      }

  return rsElement;
  } // const string& CContainer::operator += ( const string& rsElement )
