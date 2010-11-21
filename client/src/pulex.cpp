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

#include <sys/time.h> // gettimeofday()

#include <openssl/ssl.h>
#include <openssl/x509.h>

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


std::string Fingerprint( const std::string& rsName )
  {
  if ( !rsName.length() ) return "no-name";
  std::cout << "name was: " << rsName << std::endl; 

  std::string sFileName = "certificates/client/" + rsName + ".crt";
  BIO* ptBio;
  if ( (ptBio = ::BIO_new_file( sFileName.c_str(),"r")) == NULL)
    {
    ::BIO_free( ptBio );
    return "naname-" + rsName;
    throw CSocketException( "Couldn't open X509 file for: " + rsName );
    }

  X509* ptX509 = X509_new();
  X509* ptX509result = PEM_read_bio_X509_AUX(ptBio, &ptX509, 0, 0);
  ::BIO_free( ptBio );

  unsigned char* aucDer = 0;
  int i = i2d_X509( ptX509result, &aucDer );

  unsigned char* pucSha1;
  pucSha1 = (unsigned char*)malloc( SHA_DIGEST_LENGTH +1 );
  if ( !SHA1( aucDer, i, pucSha1) )
    {
    free( aucDer );
    X509_free( ptX509 );
    throw CSocketException( "Couldn't create fingerprint for: " + rsName );
    }

  free( aucDer );
  X509_free( ptX509 );

  char pszHex[ 64 ];
  sprintf( pszHex, "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X", 
      ((unsigned char*)pucSha1)[00],
      ((unsigned char*)pucSha1)[01],
      ((unsigned char*)pucSha1)[02],
      ((unsigned char*)pucSha1)[03],
      ((unsigned char*)pucSha1)[04],
      ((unsigned char*)pucSha1)[05],
      ((unsigned char*)pucSha1)[06],
      ((unsigned char*)pucSha1)[07],
      ((unsigned char*)pucSha1)[08],
      ((unsigned char*)pucSha1)[09],
      ((unsigned char*)pucSha1)[10],
      ((unsigned char*)pucSha1)[11],
      ((unsigned char*)pucSha1)[12],
      ((unsigned char*)pucSha1)[13],
      ((unsigned char*)pucSha1)[14],
      ((unsigned char*)pucSha1)[15],
      ((unsigned char*)pucSha1)[16],
      ((unsigned char*)pucSha1)[17],
      ((unsigned char*)pucSha1)[18],
      ((unsigned char*)pucSha1)[19]
       );

  std::string sSha1 = pszHex;
//  std::cout << "SHA1 of " << rsName << ".crt: " << sSha1 << " len: " << i << std::endl; 
  return sSha1;
  }


// indicators to be used to transport the pulex
const char* CPulex::scn_username       = "u";

// generic stream sending methode
template<typename T>
  T& CPulex::Send( T& roStream )
    {
    // content to manage the object if it's remote
//    roStream << scn_username      << ":" << NameToDigest( UsernameGet() ) << "\n";
    roStream << scn_sender        << ":" << Fingerprint( UsernameGet() ) << "\n";

    for ( CListString::iterator it=m_lsRecipients.begin(); it != m_lsRecipients.end(); ++it )
      if ( it->length() )
        roStream << scn_recipient << ":" << Fingerprint( *it ) << "\n";

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
