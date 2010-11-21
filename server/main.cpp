/***************************************************************************
 main.cpp
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


#include <string>
#include <list>
#include <map>

#include <iostream>

#include "socketserver.h"
#include "socketexception.h"

#include "partner.h"
#include "container.h"

#define SRV_HOST  "localhost"
#define SRV_PORT  "30000"
#define KEY_FILE  "certificates/client/localhost.key"
#define PASSWORD  ""
#define CRT_FILE  "certificates/client/localhost.crt"
#define CA_CHAIN  "certificates/server/server-CA-chain.pem"
#define CA_PATH   "certificates/trust/"
#define DHFILE    "dh1024.pem"


long                 g_lLastRemoteId = 100;
CContainerMapByCLUID g_oContainerMapByCLUID;

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
    CSocketServer server( "localhost",
                          "30000",
                          "certificates/server/localhost.crt",
                          "certificates/server/localhost.key",
                          "",
                          "certificates/client/cachain.pem",
                          "certificates/trust/" );

    std::cout << "Waiting for clients..." << std::endl;
    while ( true )
      {
      /// wait for a client
      try
        {
        /// don't panic! this threaded object terminates itself after doing its mission
        (new CPartner())->Communicate( server.Accept() );
        } // try  -  wait for a client
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
