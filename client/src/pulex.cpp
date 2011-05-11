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

#include "socketexception.h"


using namespace std;

// Stream operators to send the pulex to an output stream The streams differ,
// but the methode keeps the sam by using a template for 'Send(...)'

// Send the pulex to a generic ostream
ostream& operator << ( ostream& roStream, CPulex& roPulex )
  {
  return roPulex.Send( roStream );
  }

// Send the Pulex to a socket stream
CSocket& operator << ( CSocket& roStream, CPulex& roPulex )
  {
  return roPulex.Send( roStream );
  }


// Static const members of the pulex

// The name of the class 'CPulex' in the transport stream for reconstruction
const string CPulex::s_sClassName = "FLOW.PULEX";


// A Pulx object has its time tag and its ID
// This way it should be unique
CPulex::CPulex()
  {
  }

// No ressources allocated, nothings to free
CPulex::~CPulex()
  {
  }


const std::string& CPulex::MessageIdSet( const std::string& sMessageId )
  {
  return m_sMessageId = sMessageId;
  }

const std::string& CPulex::MessageIdGet()
  {
  return m_sMessageId;
  }



// The class name for reconstruction
const string& CPulex::ClassNameGet() const
  {
  return s_sClassName;
  }


// Accumulating data in the object
const string& CPulex::operator << ( const string& rsData )
  {
  push_back(rsData);
  return rsData;
  } // const string& CPulex::operator << ( const string& rsData )


// Creating the SHA1 checksum of the certificate directed to by rsName
// Generic stream sending methode, we may use different stream types but we
// need to be consistent. So we focus on the output operation, the stream
// typtes may vary
template<typename T>
  T& CPulex::Send( T& roStream )
    {
    // Information to manage the object if it's remote
    // sender has to be first because it is indicated as start of a new object
    roStream << string(1, scn_sender) << ":"
//        << CCrypto::FingerprintCrt( SenderGet() ) << "\n";
             << SenderGet()           << "\n";

    // messageid has to be second if present because means command "change" or "delete"
    if ( m_sMessageId.length() )
      {
      roStream << string(1, scn_messageid) << ":"
               << m_sMessageId             << "\n";
      } // if ( it->length() )

    if ( m_sRecipient.length() )
      {
      roStream << string(1, scn_recipient) << ":"
//          << CCrypto::FingerprintCrt( m_sRecipient ) << "\n";
               << m_sRecipient             << "\n";
      } // if ( it->length() )

    roStream << "===== message goes here =====" << "\n";

    // This is not the final solution, data have to be able to flow into the
    // Cropto object to miminzie memory footage
    ostringstream sosBuffer;
    for ( CPulex::iterator it  = begin();
                           it != end();
                         ++it )
      {
      // with SSL the server breaks down if '*it' is empty but piped from the client
      if ( it->length() )
        sosBuffer << scn_content << ":" << *it << "\n";
      else
        sosBuffer << scn_content << ":" << "\n";
      } // for ( CPulex::iterator it  = begin();

/*
    CCrypto oCrypto( sosBuffer.str() );
    oCrypto.RsaKeyLoadFromCertificate( "certificates/client/" + m_sRecipient + ".crt" );
    roStream << oCrypto.BuildTransportPackage();
*/
    roStream << "===== end of message =====" << "\n";
    return roStream;
    } // template<typename T> T& CPulex::Send( T& roStream )
