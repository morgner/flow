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
#include "sslclasses.h"


#include <sys/time.h> // gettimeofday()

#include <openssl/ssl.h>
#include <openssl/x509.h>

// Stream operators to send the pulex to an output stream The streams differ,
// but the methode keeps the sam by using a template for 'Send(...)'

// Send the pulex to a generic ostream
std::ostream& operator << ( std::ostream& roStream, CPulex& roPulex )
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
const std::string CPulex::s_sClassName = "FLOW.PULEX";


// A Pulx object has its time tag and its ID
// This way it should be unique
CPulex::CPulex()
  {
  timeval time;
  gettimeofday( &time, 0 );
  m_tClientSideId = time.tv_sec;
  m_nClientSideId = ClientSideIDGet();
  }

// No ressources allocated, nothings to free
CPulex::~CPulex()
  {
  }


// The unique client ID for a Pulex object instance
long CPulex::ClientSideIDGet()
  {
  static int nClientSideId = 0;
  return ++nClientSideId;
  }


// The class name for reconstruction
const std::string& CPulex::ClassNameGet() const
  {
  return s_sClassName;
  }


// Accumulating data in the objectc
const std::string& CPulex::operator << ( const std::string& rsData )
  {
  push_back(rsData);
  return rsData;
  }


// Creating the SHA1 checksum of the certificate directed to by rsName
std::string Fingerprint( const std::string& rsName )
  {
  if ( !rsName.length() ) return "no-name";

  // This is where we expect the certificate to live
  std::string sFileName = "certificates/client/" + rsName + ".crt";

  // Opens a file based readonly BIO to read from the certificate file
  CBio oBio = ::BIO_new_file( sFileName.c_str(),"r");
  if ( !oBio.isValid() )
    {
    return "naname-" + rsName;
    throw CSocketException( "Couldn't open X509 file for: " + rsName );
    }

  // Structured storage for a certificate
  CX509 oX509 = X509_new();
  // Reads a PEM certificate file, AUX is for PEM, without AUX DER is expected
  // ptX509result is the same as (X509*)oX509 = not to be deleted!
  X509* ptX509result = PEM_read_bio_X509_AUX( oBio, oX509, 0, 0 );

  CUChar oDer;
  // i2d_X509() encodes the structure pointed to by x into DER format. If out
  // is not NULL is writes the DER encoded data to the buffer at *out, and
  // increments it to point after the data just written. If the return value is
  // negative an error occurred, otherwise it returns the length of the encoded
  // data.
  //
  // For OpenSSL 0.9.7 and later if *out is NULL memory will be allocated for a
  // buffer and the encoded data written to it. In this case *out is not
  //  incremented and it points to the start of the data just written.
  int nSize = i2d_X509( ptX509result, oDer );

  CUChar oSha1 = (unsigned char*)malloc(SHA_DIGEST_LENGTH);
  // Generates the SHA1 checksum from given DER certificate
  if ( !SHA1( oDer, nSize, oSha1) )
    {
    throw CSocketException( "Couldn't create fingerprint for: " + rsName );
    }

  // Building the ASCII-HEX form of the fingerprint
  char pszHexSha1[ 41 ];
  for ( int n = 0; n < 20; n++ )
    {
    sprintf( &pszHexSha1[ n << 1 ], "%02X", ((unsigned char*)oSha1)[ n ] );
    }
  std::cout << pszHexSha1 << " for " << rsName << std::endl; 
  return pszHexSha1;
  }


// Generic stream sending methode, we may use different stream types but we
// need to be consistent. So we focus on the output operation, the stream
// typtes may vary
template<typename T>
  T& CPulex::Send( T& roStream )
    {
    // information to manage the object if it's remote
    roStream << scn_sender << ":" << Fingerprint( SenderGet() ) << "\n";
    for ( CListString::iterator it  = m_lsRecipients.begin();
                                it != m_lsRecipients.end();
                              ++it )
      {
      if ( it->length() )
        {
        roStream << scn_recipient << ":" << Fingerprint( *it ) << "\n";
        } // if ( it->length() )
      } // for ( CListString::iterator it  = m_lsRecipients.begin();
    roStream << scn_local_id      << ":" << ClientSideIdGet() << "\n";
    roStream << scn_local_id_time << ":" << ClientSideTmGet() << "\n";
    roStream << scn_remote_id     << ":" << ServerSideIdGet() << "\n";

    roStream << "===== to encrypt =====" << "\n";
    // content to encrypt for recipients
    roStream << scn_content_text << ":" << ClassNameGet()   << "\n";
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
