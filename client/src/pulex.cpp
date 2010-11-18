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


const std::string CPulex::s_sClassName = "PULEX";

CPulex::CPulex()
  {
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


const std::string& CPulex::SenderSet( const std::string& rsSender )
  {
  m_sSender = rsSender;
  return rsSender;
  }


const std::string& CPulex::SenderGet()
  {
  return m_sSender;
  }


// add the recipient to the recipient list
size_t CPulex::RecipientAdd( const std::string& rsRecipient )
  {
  m_lsRecipients.push_back( rsRecipient );
  return m_lsRecipients.size();
  }


// remove all recipient entries for the given alias
size_t CPulex::RecipientDel( const std::string& rsRecipient )
  {
  m_lsRecipients.remove( rsRecipient );
  return m_lsRecipients.size();
  }


// indicators to be used to transport the pulex
const char* CPulex::scn_username       = "u";
const char* CPulex::scn_sender         = "s";
const char* CPulex::scn_recipient      = "e";
const char* CPulex::scn_destination    = "d";
const char* CPulex::scn_class_name     = "c";
const char* CPulex::scn_local_id       = "l";
const char* CPulex::scn_local_id_time  = "t";
const char* CPulex::scn_remote_id      = "r";
const char* CPulex::scn_content_text   = "x";
const char* CPulex::scn_content_binary = "b";

// generic stream sending methode
template<typename T>
  T& CPulex::Send( T& roStream )
    {
    // content to manage the object if it's remote
    roStream << scn_username      << ":" << UsernameGet()    << "\n";
    roStream << scn_sender        << ":" << SenderGet()      << "\n";

    for ( CListString::iterator it=m_lsRecipients.begin(); it != m_lsRecipients.end(); ++it )
      if ( it->length() )
        roStream << scn_recipient   << ":" << *it << "\n";

    roStream << scn_local_id      << ":" << LocalIdGet()     << "\n";
    roStream << scn_local_id_time << ":" << LocalIdTimeGet() << "\n";
    roStream << scn_remote_id     << ":" << RemoteIdGet()    << "\n";

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
