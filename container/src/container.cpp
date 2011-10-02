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
  {
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


const string& CContainer::RecipientSet( const string& rsRecipient )
  {
  return m_sRecipient = rsRecipient;
  } // const string& CContainer::RecipientSet( const string& rsRecipient )


const string& CContainer::RecipientGet()
  {
  return m_sRecipient;
  } // const string& CContainer::RecipientGet()


const string& CContainer::MessageIdSet( const string& rsMessageId )
  {
  return m_sMessageId = rsMessageId;
  } // const string& CContainer::MessageIdSet( const string& rsMessageId )


const string& CContainer::MessageIdGet()
  {
  return m_sMessageId;
  } // const string& CContainer::MessageIdGet()


#include <sstream>
template <typename T>
  string to_string(const T& val)
    {
    ostringstream sout;
    sout << val;
    return sout.str();
    } // template <typename T> string to_string(const T& val)


bool CContainer::isFor( const string& rsRecipient )
  {
  return m_sRecipient == rsRecipient;
  } // bool CContainer::isFor( const string& rsRecipient )


bool CContainer::isFrom( const string& rsSender )
  {
  return m_sSender == rsSender;
  } // bool CContainer::isFrom( const string& rsSender )


// indicators to be used to transport the pulex
// message send parameters
const char CContainer::scn_sender           = 'a';
const char CContainer::scn_recipient        = 'r';
const char CContainer::scn_content          = 'x';
const char CContainer::scn_messageid        = 'm';
const char CContainer::scn_messageid_next   = 'n';
// message receive parameters
const char CContainer::scn_recall           = 'c';
const char CContainer::scn_list             = 'l';

// e.g. "s:<FP>", "r:<FP>", "x:please read", "x:the manual"
const string& CContainer::operator += ( const string& rsElement )
  {
  if ( rsElement[1] == ':' )
    switch ( rsElement[0] )
      {
      case scn_sender   : SenderSet   ( rsElement.substr(2) ); break;
      case scn_recipient: RecipientSet( rsElement.substr(2) ); break;
      case scn_messageid: MessageIdSet( rsElement.substr(2) ); break;

      case scn_content  : push_back( rsElement.substr(2) ); break;

      default : push_back( rsElement );
      }
  else
    {
    push_back( rsElement );
    }

  return rsElement;
  } // const string& CContainer::operator += ( const string& rsElement )
