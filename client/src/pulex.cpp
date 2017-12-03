/***************************************************************************
 pulex.cpp - description
 -----------------------
 begin                 : Fri Oct 29 2010
 copyright             : Copyright (C) 2010 by Manfred Morgner
 email                 : manfred@morgner.com
 ***************************************************************************/

#include "pulex.h"

#include "socketexception.h"


using namespace std;

// Stream operators to send the pulex to an output stream. The streams differ
// but the methode keeps the same by using a template for 'Send(...)'

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



CPulex::CPulex()
  {
  }

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

    roStream << "===== message runs =====\n";

    for ( CPulex::iterator it  = begin();
                           it != end();
                         ++it )
      {
      roStream /* << string(1, scn_content) << ":" */ << *it;
      } // for ( CPulex::iterator it  = begin();

    roStream << "\n===== message ends =====\n";
    return roStream;
    } // template<typename T> T& CPulex::Send( T& roStream )
