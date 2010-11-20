/***************************************************************************
 pulex.cpp - description
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

#include "pulex.h"

#include <sys/time.h> // gettimeofday()


// stream operators to send the pulex out

std::ostream& operator << ( std::ostream& roStream, CPulex& roPulex )
  {
  return roPulex.Send( roStream );
  }

CSocket& operator << ( CSocket& roStream, CPulex& roPulex )
  {
  return roPulex.Send( roStream );
  }


// static const members of the pulex
const std::string CPulex::s_sClassName    = "FLOW.PULEX";
      long        CPulex::s_nClientSideId = 0;


CPulex::CPulex()
  {
  timeval time;
  gettimeofday( &time, 0 );
  m_tClientSideId = time.tv_sec;
  m_nClientSideId = ++s_nClientSideId;
  }

CPulex::~CPulex()
  {
  }


const std::string& CPulex::ClassNameGet() const
  {
  return s_sClassName;
  }


const std::string& CPulex::operator << ( const std::string& rsData )
  {
  push_back(rsData);
  return rsData;
  }


const std::string& CPulex::UsernameSet( const std::string& rsUserName )
  {
  return m_sUserName = rsUserName;
  }


const std::string& CPulex::UsernameGet() const
  {
  return m_sUserName;
  }


// indicators to be used to transport the pulex
const char* CPulex::scn_username       = "u";

// generic stream sending methode
template<typename T>
  T& CPulex::Send( T& roStream )
    {
    // content to manage the object if it's remote
    roStream << scn_username      << ":" << UsernameGet()    << "\n";
    roStream << scn_sender        << ":" << SenderGet()      << "\n";

    for ( CListString::iterator it=m_lsRecipients.begin(); it != m_lsRecipients.end(); ++it )
      if ( it->length() )
        roStream << scn_recipient << ":" << *it << "\n";

    roStream << scn_local_id      << ":" << ClientSideIdGet() << "\n";
    roStream << scn_local_id_time << ":" << ClientSideTmGet() << "\n";
    roStream << scn_remote_id     << ":" << ServerSideIdGet() << "\n";

    roStream << "===== to encrypt =====" << "\n";
    // content to encrypt - only for recipients
    roStream << scn_class_name    << ":" << ClassNameGet()   << "\n";
    for ( CPulex::iterator it=begin(); it != end(); ++it )
      {
      // with SSL the server breaks down if '*it' is empty but piped from the client
      if ( it->length() )
        roStream << scn_content_text << ":" << *it << "\n";
      else
        roStream << scn_content_text << ":" << "\n";
      }
    roStream << "===== to encrypt =====" << "\n";
    return roStream;
    } // T& CPulex::Send( T& roStream )
