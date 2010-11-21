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

// thread funciton performing client communication
// the used object is left free into the open space,
// so at the end of the thread, the used object is
// to destroy
void* CPartner::thread(void *pSelf)
  {
  CPartner* poPartner = reinterpret_cast<CPartner*>(pSelf);
  poPartner->Action();
  delete poPartner;
  pthread_exit( NULL );
  }



CPartner::CPartner()
  : m_bStopRequested(false),
    m_bRunning(false),
    m_poSocket(0)
  {
  pthread_mutex_init( &m_tMutex, 0 );
  }

CPartner::~CPartner()
  {
  pthread_mutex_destroy(&m_tMutex);
  delete m_poSocket;
  } // CPartner::~CPartner()

// start the thread
void CPartner::Communicate( CSocket* poSocket ) 
  {
  m_poSocket = poSocket;
  m_bRunning = true;
  int nResult = pthread_create(&m_tThread, 0, &CPartner::thread, this);
  if ( 0 != nResult )
    {
    std::cout << "ERROR: thread not created, result: " << nResult << std::endl;
    this->~CPartner();
    }  
  }

// stopp the thread
void CPartner::Stop()
  {
  m_bRunning = false;
  m_bStopRequested = true;
  pthread_join( m_tThread, 0 );
  }


extern CContainerMapByCLUID g_oContainerMapByCLUID;
extern long                 g_lLastRemoteId;

void CPartner::Action()
  {
  std::string sClientData;
  std::string sInput;
  do
    {
    try
      {
      *m_poSocket >> sInput;
      }
    catch ( CSocketException& e)
      {
      std::cout << "Exception: " << e.Info() << std::endl;
      if ( e.isFatal() ) break;
      }
    sClientData += sInput;
    } while ( sInput.rfind("\r") == std::string::npos ); // m_bStopRequested

  if ( sClientData.find( "c:" ) == 0 )
    Recall( sClientData, m_poSocket );
  else
    BuildContainers( sClientData );

  *m_poSocket << ".\r";
  }


// c:recipient
// o:all
// t:from_time
// q:from_id
size_t CPartner::Recall( const std::string& rsClientData,
                               CSocket*     poSocket )
  {
  std::map<char, std::string> moQuery;

  for (std::string::size_type p1=0, p2=0;
       std::string::npos != ( p2 = rsClientData.find('\n', p1) );
       p1 = p2+1)
    {
    std::string s = rsClientData.substr(p1, p2-p1);
    if ( (s.length() > 2) && (s[1] == ':') )
      {
      moQuery[ s[0] ] = s.substr(2);
      }
    }

  for ( CContainerMapByCLUID::iterator it = g_oContainerMapByCLUID.begin(); 
                                       it != g_oContainerMapByCLUID.end(); 
                                     ++it )
    {
    if ( it->second->isFor(moQuery['c']) )
      {
      *poSocket << it->second->RGUIDGet() << "\n";
      *poSocket << it->second->CLUIDGet() << "\n";
      }
    }
  return 0;
  }


size_t CPartner::BuildContainers( const std::string& rsClientData )
  {
  /// a list to collect containers
  typedef std::list<CContainer*> CContainerList;

  CContainerList oContainerList;  // temporary list to collect what we got
  CContainer*    poContainer = 0; // a potential container for if we get someting
  // splits the lines on their unixconform line ending seperator '\n'
  for (std::string::size_type p1=0, p2=0;
       std::string::npos != ( p2 = rsClientData.find('\n', p1) );
       p1 = p2+1)
    {
    std::string s = rsClientData.substr(p1, p2-p1);
    std::cout << s << std::endl;
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
    } // for ( std::string::size_type p1=0, p2=0; ...

  pthread_mutex_lock( &m_tMutex );
  for ( CContainerList::iterator it=oContainerList.begin(); it != oContainerList.end(); ++it)
    {
    std::string sKey = (*it)->CLUIDGet();
    CContainerMapByCLUID::iterator itf = g_oContainerMapByCLUID.find( sKey );
    if ( itf != g_oContainerMapByCLUID.end() )
      {
      (*it)->ServerSideIdSet( itf->second->ServerSideIdGet() );
      std::cout << "replacing: " << sKey << " by " << (*it)->RGUIDGet() << "\n";
      delete itf->second;
      g_oContainerMapByCLUID.erase(itf);
      }
    else
      {
      std::cout << "appending: " << sKey << " as ";
      (*it)->ServerSideIdSet( to_string(++g_lLastRemoteId) );
      std::cout << (*it)->RGUIDGet() << "\n";
      }
    g_oContainerMapByCLUID[ sKey ] = *it;

    for ( CListString::iterator its=(*it)->begin(); its != (*it)->end(); ++its )
      {
      std::cout << " *** " << *its << std::endl;
      }

    pthread_mutex_unlock( &m_tMutex );
    }

  std::cout << oContainerList.size() << " elements received ";
  std::cout << g_oContainerMapByCLUID.size() << " elements total here." << std::endl;
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
  } // size_t CPartner::BuildContainers( const std::string& rsClientData )
