/***************************************************************************
 partner.cpp description
 -----------------------
 begin                 : Fri Nov 05 2010
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


#include "partner.h"
#include "socketexception.h"

#include <iostream>

#include <sys/time.h> // gettimeofday()

using namespace std;

// Thread function performing client communication the used object is left free
// into the open space, so at the end of the thread, the used object is to
// destroy
// We trust callers to provide us with an instance of a CPartner class, there
// is not check possible, given, that we use a reinterpret_cast on a void*
void* CPartner::thread(void* pObject)
  {
  CPartner* poPartner = reinterpret_cast<CPartner*>(pObject);
  poPartner->Action();
  delete poPartner;
  pthread_exit( NULL );
  } // void* CPartner::thread(void* pObject)



CPartner::CPartner()
  : m_bStopRequested(false),
    m_bRunning(false),
    m_poSocket(0)
  {
  pthread_mutex_init( &m_tMutex, 0 );
  }


CPartner::~CPartner()
  {
  if ( m_bRunning ) Stop();
  pthread_mutex_destroy( &m_tMutex );
  delete m_poSocket;
  } // CPartner::~CPartner()


extern CContainerMapByCLUID g_oContainerMapByCLUID;

string CPartner::MakeMessageKey( CContainer* poContainer )
  {
  string sSender    = poContainer->SenderGet();
  string sMessageId = poContainer->MessageIdGet();

  if ( sMessageId.length() == 0 )
    {
    // generate a first simple version of Message-ID
    if ( ! sMessageId.length() )
      {
      timeval time;
      gettimeofday( &time, 0 );
      sMessageId = sSender + to_string( time.tv_sec );

      int nSuffix = 0;
      for ( ; g_oContainerMapByCLUID.find( sMessageId + ':' + to_string(nSuffix) ) != g_oContainerMapByCLUID.end(); nSuffix++ )
        {
        if ( g_bVerbose ) cout << sMessageId;
        }
        sMessageId += ':' + to_string(nSuffix);
      }
    } // if ( sMessageId.length() == 0 )

  return sSender + ':' + sMessageId;
  } // string CPartner::MakeMessageKey( CContainer* poContainer )


// start the thread
void CPartner::Communicate( CSocket* poSocket ) 
  {
  m_poSocket  = poSocket;
  int nResult = pthread_create(&m_tThread, 0, &CPartner::thread, this);
  // If qwe can't create the controlling thead we'r lost! No one ever will find
  // us we we must commit suicide. Otherwise, we will become a memory leak.
  if ( 0 != nResult )
    {
    cout << "ERROR: thread not created, result: " << nResult << endl;
    delete this;
    } // if ( 0 != nResult )
  } // void CPartner::Communicate( CSocket* poSocket ) 


// stopp the thread
void CPartner::Stop()
  {
  m_bRunning = false;
  m_bStopRequested = true;
  pthread_join( m_tThread, 0 );
  } // void CPartner::Stop()


void CPartner::Action()
  {
  m_bRunning  = true; // Now we're running

  string sClientData;
  string sInput;
  do
    {
    try
      {
      *m_poSocket >> sInput;
      }
    catch ( CSocketException& e)
      {
      cout << "ERROR: " << e.Info() << endl;
      if ( e.isFatal() ) break;
      }
    if ( g_bVerbose ) cout << sInput;
    sClientData += sInput;
    } while ( !m_bStopRequested && sClientData.rfind(".\r") == string::npos );

  if ( (sClientData[1] == ':') &&
       (sClientData[0] == CContainer::scn_command) )
    {
    if (g_bVerbose) cout << " * recall; container: "
                         << g_oContainerMapByCLUID.size() << endl;
    Recall( sClientData, m_poSocket );
    }
  if ( (sClientData[1] == ':') &&
       (sClientData[0] == CContainer::scn_sender) )
    {
    if (g_bVerbose) cout << " * received container " << endl;
    BuildContainers( sClientData );
    }
  *m_poSocket << ".\r";

  Stop(); // That's all for now and here
  } // void CPartner::Action()


size_t CPartner::Recall( const string&  rsClientData,
                               CSocket* poSocket )
  {
  map<char, string> moQuery;

  for (string::size_type p1=0, p2=0;
       string::npos != ( p2 = rsClientData.find('\n', p1) );
       p1 = p2+1)
    {
    string s = rsClientData.substr(p1, p2-p1);
    if ( (s.length() > 2) && (s[1] == ':') )
      {
      moQuery[ s[0] ] = s.substr(2);
      }
    } // for (string::size_type p1=0, p2=0; ...

  for ( CContainerMapByCLUID::iterator it  = g_oContainerMapByCLUID.begin(); 
                                       it != g_oContainerMapByCLUID.end(); 
                                     ++it )
    {
    CContainer* poc = it->second;
//    if ( poc->isFor(m_poSocket->PeerFingerprintGet()) )
      {
      *poSocket << string(1, CContainer::scn_sender)  << ":" << poc->SenderGet() << "\n";
      *poSocket << it->first                          << "\n";

      for ( CContainer::iterator itc  = poc->begin();
                                 itc != poc->end();
                               ++itc )
        {
        *poSocket << *itc << "\n";
        } // for ( CContainer::iterator itc  = poc->begin();
      } // if ( poc->isFor(m_poSocket->PeerFingerprintGet()) )
    } // for ( CContainerMapByCLUID::iterator it  = g_oContainerMapByCLUID.begin(); 
  return 0;
  } // size_t CPartner::Recall( const string&  rsClientData, CSocket* poSocket )


size_t CPartner::BuildContainers( const string& rsClientData )
  {
//  g_bVerbose = true;

  /// a list to collect containers
  typedef list<CContainer*> CContainerList;

  CContainerList oContainerList;  // temporary list to collect what we got
  CContainer*    poContainer = 0; // a potential container for if we get someting
  // splits the lines on their unixconform line ending seperator '\n'
  for (string::size_type p1=0, p2=0;
       string::npos != ( p2 = rsClientData.find('\n', p1) );
       p1 = p2+1)
    {
    string s = rsClientData.substr(p1, p2-p1);
    if ( g_bVerbose ) cout << s << endl;
    // is it a sender-id?

    if ( (s[1] == ':') &&
         (s[0] == CContainer::scn_sender) )
      {
      // if so this means a new message is starting
      poContainer = new CContainer;
      oContainerList.push_back( poContainer );
      }
    if ( poContainer )
      {
      *poContainer += s;
      } // if ( poContainer )
    } // for ( string::size_type p1=0, p2=0; ...

  pthread_mutex_lock( &m_tMutex );
  for ( CContainerList::iterator it  = oContainerList.begin();
                                 it != oContainerList.end();
                               ++it)
    {
    string sKey = MakeMessageKey(*it);
    CContainerMapByCLUID::iterator itf = g_oContainerMapByCLUID.find( sKey );
    if ( itf == g_oContainerMapByCLUID.end() )
      {
      if ( g_bVerbose ) cout << "appending: " << sKey << endl;
      *m_poSocket << sKey << "\n";
      }
    else
      {
      if ( g_bVerbose ) cout << "replacing: " << sKey << endl;
      delete itf->second;
      itf->second = 0;
      g_oContainerMapByCLUID.erase( itf );
      }
    g_oContainerMapByCLUID[ sKey ] = *it;
/*
    if ( g_bVerbose )
      {
      for ( CListString::iterator its=(*it)->begin(); its != (*it)->end(); ++its )
        {
        cout << " *** " << *its << endl;
        }
      } // if ( g_bVerbose )
 */
    } // for ( CContainerList::iterator it  = oContainerList.begin(); ...
  pthread_mutex_unlock( &m_tMutex );

  if ( g_bVerbose ) cout << oContainerList.size() << " elements received ";
  if ( g_bVerbose ) cout << g_oContainerMapByCLUID.size() << " elements total here." << endl;
  /* 
   for ( CContainerMapByCLUID::iterator it = g_oContainerMapByCLUID.begin(); 
   it != g_oContainerMapByCLUID.end(); 
   ++it )
   {
   *m_poSocket << it->second->MessageId() << "\n";
   }
   */
  return g_oContainerMapByCLUID.size();
  } // size_t CPartner::BuildContainers( const string& rsClientData )
