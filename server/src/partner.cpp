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
#include "container.h"
#include "socketexception.h"

#include <iostream>


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


// start the thread
void CPartner::Communicate( CSocketSSL* poSocket ) 
  {
  m_poSocket  = poSocket;
  int nResult = pthread_create(&m_tThread, 0, &CPartner::thread, this);
  // If qwe can't create the controlling thead we'r lost! No one ever will find
  // us we we must commit suicide. Otherwise, we will become a memory leak.
  if ( 0 != nResult )
    {
    cout << "ERROR: thread not created, result: " << nResult << endl;
//    this->~CPartner();
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


extern CContainerMapByCLUID g_oContainerMapByCLUID;
extern long                 g_lLastRemoteId;

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
    sClientData += sInput;
    } while ( sInput.rfind("\r") == string::npos ); // m_bStopRequested

  if ( sClientData.find( "c:" ) == 0 )
    {
    cout << " * recall; container: " << g_oContainerMapByCLUID.size() << endl;
    Recall( sClientData, m_poSocket );
    }
  else
    {
    cout << " * receive; container: ";
    BuildContainers( sClientData );
	cout << g_oContainerMapByCLUID.size() << endl;
    }
  *m_poSocket << ".\r";

  Stop(); // That's all for now and here
  } // void CPartner::Action()


// c:recipient
// o:all
// t:from_time
// q:from_id
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
//    if ( it->second->isFor(moQuery['c']) )
    if ( it->second->isFor(m_poSocket->PeerFingerprintGet()) )
      {
      *poSocket << it->second->RGUIDGet() << "\n";
      *poSocket << it->second->CLUIDGet() << "\n";
      }
    }
  return 0;
  }


size_t CPartner::BuildContainers( const string& rsClientData )
  {
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
    // the first two chars of a line is the line content descriptor
    if ( s.find("s:") == 0 )
      {
      poContainer = new CContainer;
      oContainerList.push_back( poContainer );
      }
    if ( poContainer )
      {
      *poContainer += s;
      } // if ( poContainer )
    } // for ( string::size_type p1=0, p2=0; ...

  pthread_mutex_lock( &m_tMutex );
  for ( CContainerList::iterator it=oContainerList.begin(); it != oContainerList.end(); ++it)
    {
    string sKey = (*it)->CLUIDGet();
    CContainerMapByCLUID::iterator itf = g_oContainerMapByCLUID.find( sKey );
    if ( itf != g_oContainerMapByCLUID.end() )
      {
      (*it)->ServerSideIdSet( itf->second->ServerSideIdGet() );
      if ( g_bVerbose ) cout << "replacing: " << sKey << " by " << (*it)->RGUIDGet() << "\n";
      delete itf->second;
      g_oContainerMapByCLUID.erase(itf);
      }
    else
      {
      if ( g_bVerbose ) cout << "appending: " << sKey << " as ";
      (*it)->ServerSideIdSet( to_string(++g_lLastRemoteId) );
      if ( g_bVerbose ) cout << (*it)->RGUIDGet() << endl;
      }
    g_oContainerMapByCLUID[ sKey ] = *it;

    for ( CListString::iterator its=(*it)->begin(); its != (*it)->end(); ++its )
      {
      if ( g_bVerbose ) cout << " *** " << *its << endl;
      }

    pthread_mutex_unlock( &m_tMutex );
    }

  if ( g_bVerbose ) cout << oContainerList.size() << " elements received ";
  if ( g_bVerbose ) cout << g_oContainerMapByCLUID.size() << " elements total here." << endl;
  /* 
   for ( CContainerMapByCLUID::iterator it = g_oContainerMapByCLUID.begin(); 
   it != g_oContainerMapByCLUID.end(); 
   ++it )
   {
   *m_poSocket << it->second->RGUIDGet() << "\n";
   *mPpoSocket << it->second->CLUIDGet() << "\n";
   }
   */
  return g_oContainerMapByCLUID.size();
  } // size_t CPartner::BuildContainers( const string& rsClientData )
