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

const string CContainer::s_sClassName        = "FLOW.CONTAINER";
      long   CContainer::s_nLastClientSideId = 0;
      long   CContainer::s_nLastServerSideId = 0;


CContainer::CContainer()
  : m_tClientSideId( 0 ),
    m_nClientSideId( 0 ),
    m_nServerSideId( 0 )
  {
  ClientSideTmSet();
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
  if ( nClientSideTm )
    {
    m_tClientSideId = nClientSideTm;
    }
  else
    {
    timeval time;
    gettimeofday( &time, 0 );
    m_tClientSideId = time.tv_sec;
    }
  return m_tClientSideId;
  } // long CContainer::ClientSideTmSet( long nClientSideTm )


long CContainer::ClientSideTmSet( const string& rsClientSideTm )
  {
  return ClientSideTmSet( atol( rsClientSideTm.c_str()) );
  } // long CContainer::ClientSideTmSet( const string& rsClientSideTm )


// The unique client ID for a Container object
long CContainer::ClientSideIdGet()
  {
  return ( m_nClientSideId ) ? m_nClientSideId : ClientSideIdSet();
  } // long CContainer::ClientSideIdGet()


long CContainer::ClientSideIdSet( long nClientSideId )
  {
  if ( nClientSideId )
    {
    m_nClientSideId = nClientSideId;
    if ( nClientSideId > s_nLastClientSideId )
      {
      s_nLastClientSideId = nClientSideId;
      } 
    }
  else
    {
    m_nClientSideId = ++s_nLastClientSideId;
    }
  return m_nClientSideId;
  } // long CContainer::ClientSideIdSet( long nClientSideId )


long CContainer::ClientSideIdSet( const string& rsClientSideId )
  {
  return ClientSideIdSet( atol(rsClientSideId.c_str()) );
  } // long CContainer::ClientSideIdSet( const string& rsClientSideId )


long CContainer::ServerSideIdGet()
  {
  return ( m_nServerSideId ) ? m_nServerSideId : ServerSideIdSet();
  } // long CContainer::ServerSideIdGet() const


long CContainer::ServerSideIdSet( long nServerSideId )
  {
  if ( !m_nServerSideId )
    {
    if ( nServerSideId )
      {
      m_nServerSideId = nServerSideId;
      if ( nServerSideId > s_nLastServerSideId )
        {
        s_nLastServerSideId = nServerSideId;
        } 
      }
    else // if ( nServerSideId )
      {
      m_nServerSideId = ++s_nLastServerSideId;
      }
    } // if ( !m_nServerSideId )
  return m_nServerSideId;
  } // long CContainer::ServerSideIdSet( long nServerSideId )


long CContainer::ServerSideIdSet( const string& rsServerSideId )
  {
  return ServerSideIdSet( atol( rsServerSideId.c_str()) );
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
  sout << "CLUID"           << ":"
       << m_sSender         << ":"
       << ClientSideIdGet();
  return sout.str();
  } // string CContainer::CLUIDGet()


bool CContainer::isFor( const string& rsRecipient )
  {
  for ( CListString::iterator it = m_lsRecipients.begin(); it != m_lsRecipients.end(); ++it)
    {
    if ( *it == rsRecipient ) return true;
    }
  return false;
  } // bool CContainer::isFor( const string& rsRecipient )


// indicators to be used to transport the pulex
const char* CContainer::scn_sender        = "s";
const char* CContainer::scn_recipient     = "e";
const char* CContainer::scn_local_id      = "l";
const char* CContainer::scn_local_id_time = "t";
const char* CContainer::scn_remote_id     = "r";
const char* CContainer::scn_content_text  = "x";
// client/src/crypto.cpp:const string CCrypto::s_sDelimiter = "===\n";
const char* CContainer::scn_block_delimiter = "===\n";

// e.g. "s:<FP>", "r:<FP>", "x:please read", "x:the manual"
const string& CContainer::operator += ( const string& rsElement )
  {
  if ( rsElement[1] == ':' )
    switch ( rsElement[0] )
      {
      case 's': SenderSet      ( rsElement.substr(2) ); break;
      case 'e': RecipientAdd   ( rsElement.substr(2) ); break;
      case 'l': ClientSideIdSet( rsElement.substr(2) ); break;
      case 't': ClientSideTmSet( rsElement.substr(2) ); break;
      case 'r': ServerSideIdSet( rsElement.substr(2) ); break;

      case 'x': push_back( rsElement.substr(2) ); break;

      default : push_back( rsElement );
      }
  else
    {
    push_back( rsElement );
    }

  return rsElement;
  } // const string& CContainer::operator += ( const string& rsElement )
