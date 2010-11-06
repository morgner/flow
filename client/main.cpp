/***************************************************************************
 main.cpp  - description
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

#include "environment.h"

#include "domain.h"
#include "socketclient.h"
#include "socketexception.h"

#include <map>
#include <iostream>
#include <sstream>  // for stringbuf


#define VERSION "0.1"

#include <getopt.h> // for static struct option
#include <stdlib.h> // for atoi(), exit()
#include <stdio.h>  // for EOF
#include <fcntl.h>  // for fnctl()

int main( int argc, char* argv[] )
  {
  static struct option const tOptions[] =
    {
      { "help",      no_argument,       0, 'H'},
      { "version",   no_argument,       0, 'V'},
      { "verbose",   no_argument,       0, 'v'},
      { "host",      required_argument, 0, 'h'},
      { "port",      required_argument, 0, 'p'},
      { "user",      required_argument, 0, 'u'},
      { "recipient", required_argument, 0, 'r'},
      { "message",   required_argument, 0, 'm'},
      { "cluid",     required_argument, 0, 'i'},
      {  0,          0,                 0,  0 }
    }; // struct tOptions

  g_oEnvironment.CommandlineRead( argc, argv, tOptions );

  bool bVerbose = g_oEnvironment.isVerbose();


  CDomain oDomain;
  CPulex* poPulex;

  oDomain += poPulex = new CPulex();

  int flags = fcntl(0, F_GETFL, 0);
  fcntl(0, F_SETFL, flags | O_NONBLOCK);
    std::string s;
    if ( bVerbose ) std::cout << "===pipe-begin===" << std::endl;
    for ( getline( std::cin, s ); std::cin.good(); getline( std::cin, s ) )
      {
      if ( bVerbose ) std::cout << s << std::endl;
      *poPulex << s;
      }
    if ( bVerbose ) std::cout << "===pipe-end===" << std::endl << std::endl;
    fcntl(0, F_SETFL, flags);

  if ( g_oEnvironment.find("user")     != g_oEnvironment.end() )  poPulex->UsernameSet( g_oEnvironment["user"] );
  if ( g_oEnvironment.find("cluid")    != g_oEnvironment.end() )  poPulex->LocalIdSet ( atoi(g_oEnvironment["cluid"].c_str()) );
  if ( g_oEnvironment.find("message")  != g_oEnvironment.end() ) 
    *poPulex << g_oEnvironment["message"];
  else
    *poPulex << "Hello Word, this is I, a lonley pulex";
  if ( g_oEnvironment.find("host")     == g_oEnvironment.end() )  g_oEnvironment["host"] = "localhost";
  if ( g_oEnvironment.find("port")     == g_oEnvironment.end() )  g_oEnvironment["port"] = "30000";

/*
  oDomain += poPulex = new CPulex();
  *poPulex << "Daten";

  oDomain += poPulex = new CPulex();
  *poPulex << "Senden";
  *poPulex << "und";
  *poPulex << "Empfangen";

  CPulex* p = *(--oDomain.end());
  *p << "additional data";
*/

  if ( bVerbose ) std::cout << *poPulex << std::endl;

  try
    {
    if ( bVerbose ) std::cout << " * HOST: " << g_oEnvironment["host"] << std::endl;
    if ( bVerbose ) std::cout << " * PORT: " << g_oEnvironment["port"] << std::endl;
    CSocketClient oConnection( g_oEnvironment["host"], g_oEnvironment["port"].c_str() );
    
    std::string sData;
    std::string sInput;
    
    try
      {
      for (CDomain::iterator it=oDomain.begin(); it != oDomain.end(); ++it)
        {
        oConnection << **it ;
        }
      oConnection << ".\r";
      do
        {
        oConnection >> sInput;
        sData += sInput;
        } while ( sData.find(".\r") == std::string::npos );
      }
    catch ( CSocketException& e )
      {
      std::cout << "Exception: " << e.Info() << "\n";
      }
    if ( bVerbose ) std::cout << "Response from server:\n" << sData << "\n";;
    }
  catch ( CSocketException& e )
    {
    std::cout << "Exception: " << e.Info() << "\n";
    }

  return 0;
  }


void CEnvironment::Usage( int nStatus )
  {
  std::cout << g_oEnvironment.ProgramNameGet() << " - carries your messages in protected mode" << std::endl;
  std::cout << "Usage: " <<  g_oEnvironment.ProgramNameGet() << " [OPTION]... [FILE]..." << std::endl;
  std::cout << "Options:\n"
            << "  -H, --help           display this help and exit" << std::endl
            << "  -V, --version        output version information and exit" << std::endl
            << std::endl
            << "  -h, --host           the host name or IP to send the message to" << std::endl
            << "                       default: localhost" << std::endl << std::endl
            << "  -p, --port           the port on the remote host to connect to" << std::endl
            << "                       default: 30000" << std::endl << std::endl
            << "  -u, --user           the user name to send the message from" << std::endl
            << "                       default: local user name" << std::endl << std::endl
            << "  -r, --recipient      the user name to send the message to" << std::endl
            << "                       default: all known recipients" << std::endl << std::endl
            << "  -m, --message        the message" << std::endl
            << "                       default: 'Hello Word, this is I, a lonley pulex'" << std::endl << std::endl
            << "  -i, --cluid          force the use a specific client unique identifier" << std::endl
            << "                       default: <automatic>" << std::endl << std::endl
            ;
  exit( nStatus );
  } // void CEnvironment::Usage( int nStatus )

