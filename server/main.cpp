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

#include "environment.h"

#include "partner.h"
#include "container.h"


#define SRV_HOST  "localhost"
#define SRV_PORT  "30000"
#define KEY_FILE  "certificates/server/localhost.key"
#define PASSWORD  ""
#define CRT_FILE  "certificates/server/localhost.crt"
#define CA_CHAIN  "certificates/server/server-CA-chain.pem"
#define CA_PATH   "certificates/trust/"
#define DH_FILE   "dh1024.pem"

bool g_bVerbose = false;

long                 g_lLastRemoteId = 100;
CContainerMapByCLUID g_oContainerMapByCLUID;

/*
 
 get next|all
 
 from     sender/key
 since    client-timeval/cluid
 for      user/key
 elements new|updates|all
 
 */

int main ( int argc, const char* argv[] )
  {
  CEnvironment oEnvironment( argc, argv );

  /// these are the command line options
  oEnvironment.OptionAppend( "help",        no_argument,       0, 'h', "Show this help text",          "" );
  oEnvironment.OptionAppend( "version",     no_argument,       0, 'V', "Show version information",     "" );
  oEnvironment.OptionAppend( "verbose",     no_argument,       0, 'v', "Act verbose",                  "" );
  oEnvironment.OptionAppend( "host",        required_argument, 0, 'H', "The host to listen on",        SRV_HOST );
  oEnvironment.OptionAppend( "port",        required_argument, 0, 'P', "The port to listen on",        SRV_PORT );
  oEnvironment.OptionAppend( "cert",        required_argument, 0, 'c', "Server certificate",           CRT_FILE );
  oEnvironment.OptionAppend( "key",         required_argument, 0, 'k', "Server key",                   KEY_FILE );
  oEnvironment.OptionAppend( "password",    required_argument, 0, 'w', "Password for servers key",     PASSWORD );
  oEnvironment.OptionAppend( "trust-chain", required_argument, 0, 'a', "Trusted CA chain",             CA_CHAIN );
  oEnvironment.OptionAppend( "trust-path",  required_argument, 0, 't', "Path to trusted certificates", CA_PATH );
  oEnvironment.OptionAppend( "dh-file",     required_argument, 0, 'd', "DH parameter file",            DH_FILE );

  /// we need to read the command line parameters
  oEnvironment.CommandlineRead();
  /// for convienice we create a local shortcut to the environment value of 'isVerbose()'
  g_bVerbose = oEnvironment.find("verbose") != oEnvironment.end();

  try // server bind ...
    {
    CSocketServer oServer( oEnvironment["host"],
                           oEnvironment["port"],
                           g_bVerbose );

    if ( g_bVerbose ) std::cout << "Waiting for clients..." << std::endl;
    while ( true )
      {
      /// wait for a client
      try
        {
        /// don't panic! this threaded object terminates itself after doing its mission
        (new CPartner())->Communicate( oServer.Accept() );
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
    } // catch  -  oServer bind ...

  return 0;
  } // main
