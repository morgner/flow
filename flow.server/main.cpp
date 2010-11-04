/***************************************************************************
 main.cpp
 -----------------------
 begin                 : Fri Oct 29 2010
 copyright             : Copyright (C) 2010 by Manfred Morgner
 email                 : manfred@morgner.com
 
$Id: main.cpp,v 1.6 2010/10/29 17:15:44 morgner Exp $
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


#include <string>
#include <list>
#include <map>

#include <iostream>

#include "socketserver.h"
#include "socketexception.h"

#include "session.h"


#include <pthread.h>


/// a map for collecting containers by there Client UIDs
typedef std::map <std::string, CContainer*> CContainerMapByCLUID;


long                 s_lLastRemoteId = 100;
CContainerMapByCLUID g_oContainerMapByCLUID;

void* threadReadSocket(void* pParam);

/*
 
 get next|all
 
 from     sender/key
 since    client-timeval/cluid
 for      user/key
 elements new|updates|all
 
 */

int main ( int argc, char* argv[] )
  {
  try // server bind ...
    {
    CSocketServer server( 30000 );
    std::cout << "Waiting for clients..." << std::endl;
    while ( true )
      {
      try // accept 
        {
        CSocket* poSocket = server.Accept();

        pthread_t tThread;
        int result = pthread_create( &tThread,
                                     NULL,
                                     threadReadSocket,
                                     new CSession(poSocket) );
        if ( 0 != result )
          {
          std::cout << "ERROR: thread not created, result: " << result << std::endl;
          }
        } // try  -  accept
      catch ( CSocketException& e )
        {
        std::cout << "Exception: " << e.Info() << "\nExiting.\n";
        } // catch  -  accept
      } // while ( true )
    } // try  -  server bind ...
  catch ( CSocketException& e )
    {
    std::cout << "Exception: " << e.Info() << "\nExiting.\n";
    } // catch  -  server bind ...
  
  return 0;
  } // main

/* -------------------------------------------------------------------------
 
                    thread receiving data from a socket
 
   ------------------------------------------------------------------------- */
void* threadReadSocket(void* pParam)
{
  CSession* poSession = (CSession*) pParam;
  CSocket*  poSocket  = (CSocket*)*poSession;
  
  std::string sData;
  std::string sInput;
  do
    {
    try
      {
      *poSocket >> sInput;
      }
    catch ( CSocketException& e)
      {
      std::cout << "Exception: " << e.Info() << std::endl;
      }
    sData += sInput;
    } while ( sData.find("\r") == std::string::npos );

  /// a list to collect containers
  typedef std::list<CContainer*> CContainerList;

  CContainerList oContainerList;  // temporary list to collect what we got
  CContainer*    poContainer = 0; // a potential container for if we get someting
  for (std::string::size_type p1=0, p2=0;
       std::string::npos != ( p2 = sData.find('\n', p1) );
       p1 = p2+1)
    {
    std::string s = sData.substr(p1, p2-p1);
    if ( s.find("u:") != std::string::npos )
      {
      poContainer = new CContainer;
      oContainerList.push_back( poContainer );
      }
    if ( poContainer )
      {
      *poContainer += s;
      } // if ( poContainer )
    } // for ( std::string::size_type p1=0, p2=0; ...
  
  for ( CContainerList::iterator it=oContainerList.begin(); it != oContainerList.end(); ++it)
    {
    std::string sKey = (*it)->CLUIDGet();
    CContainerMapByCLUID::iterator itf = g_oContainerMapByCLUID.find( sKey );
    if ( itf != g_oContainerMapByCLUID.end() )
      {
      (*it)->ServerIdSet( itf->second->ServerIdGet() );
      std::cout << "replacing: " << sKey << " by " << (*it)->RGUIDGet() << "\n";
      delete itf->second;
      g_oContainerMapByCLUID.erase(itf);
      }
    else
      {
      std::cout << "appending: " << sKey << " as ";
      (*it)->ServerIdSet( to_string(++s_lLastRemoteId) );
      std::cout << (*it)->RGUIDGet() << "\n";
      }
    g_oContainerMapByCLUID[ sKey ] = *it;
    }
  
  for ( CContainerMapByCLUID::iterator it = g_oContainerMapByCLUID.begin(); 
       it != g_oContainerMapByCLUID.end(); 
       ++it )
    {
    *poSocket << (*it).second->RGUIDGet() << "\n";
    *poSocket << (*it).second->CLUIDGet() << "\n";
    }
  
  *poSocket << ".\r";
  //  delete poSession;

  pthread_exit( NULL );
} // threadReadSocket
