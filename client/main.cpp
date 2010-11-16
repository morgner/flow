/***************************************************************************
 main.cpp  - description
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

#include "environment.h"

#include "domain.h"
#include "socketclient.h"
#include "socketexception.h"

#include <map>
#include <iostream>

#define VERSION "0.1"

#define DEFAULT_HOST "localhost"
#define DEFAULT_PORT "30000"

#define CRT_CLT_PATH  "certificates/client/"
#define CRT_HST_PATH  "certificates/server/"
#define CRT_TRS_PATH  "certificates/trust/"
#define PASSWORD      ""


#include <getopt.h> // for static struct option
#include <stdlib.h> // for atoi(), exit()
#include <stdio.h>  // for EOF
#include <fcntl.h>  // for fnctl()

int main( int argc, char* argv[] )
  {
  /// these are the command line options
  static struct option const tOptions[] =
    {
      { "help",      no_argument,       0, 'H'},
      { "version",   no_argument,       0, 'V'},
      { "verbose",   no_argument,       0, 'v'},
      { "host",      required_argument, 0, 'h'},
      { "port",      required_argument, 0, 'p'},
      { "user",      required_argument, 0, 'u'},
      { "password",  required_argument, 0, 'w'},
      { "recipient", required_argument, 0, 'r'},
      { "message",   required_argument, 0, 'm'},
      { "cluid",     required_argument, 0, 'i'},
      {  0,          0,                 0,  0 }
    }; // struct tOptions
  /// we need to read the command line parameters
  g_oEnvironment.CommandlineRead( argc, argv, tOptions );
  /// for convienice we create a local shortcut to the environment value of 'isVerbose()'
  bool bVerbose = g_oEnvironment.isVerbose();

  /// the real operation starts here
  CDomain oDomain;
  CPulex* poPulex;
  /// our domain starts with only one Pulex
  oDomain += poPulex = new CPulex();


  /// reading the (potentially given) piped input'directly' into the pulex object
  /// the operation is a bit tricky because of a little problem with std::cin
  /// we need to read unblocked for several reanson. one of the is the fact that
  /// we can't be certain, tha piped input is presented anyway
  /// possibly importand is the operation to set the file control flags back to
  /// the patter from before out manipulation to ensure normal behavior of  std::cin
  /// after the unblocked operation
  /// -- this methode (?) of deblocking std::cin not reliable for pipe input -- have to work on it !
  if ( bVerbose ) std::cout << "===pipe-begin===" << std::endl;
  int flags = fcntl(fileno(stdin), F_GETFL, 0);
  fcntl(fileno(stdin), F_SETFL, flags | O_NONBLOCK);
    std::string s;
    std::cin.clear();
//    for ( getline( std::cin, s ); std::cin.good(); getline( std::cin, s ) )
//    while ( getline( std::cin, s ) )
    for ( getline( std::cin, s ); !std::cin.eof(); getline( std::cin, s ) )
      {
      if ( bVerbose ) std::cout << s << std::endl;
      *poPulex << s;
//      if ( !std::cin.good() ) break;
      }
    fcntl(fileno(stdin), F_SETFL, flags);
  std::cin.clear();
  if ( bVerbose ) std::cout << "===pipe-end===" << std::endl << std::endl;

  /// putting together defaults and command line input for the Pulex and the server parameters
  if ( g_oEnvironment.find("user")     != g_oEnvironment.end() )  poPulex->UsernameSet( g_oEnvironment["user"] );
  if ( g_oEnvironment.find("cluid")    != g_oEnvironment.end() )  poPulex->LocalIdSet ( atoi(g_oEnvironment["cluid"].c_str()) );
  if ( g_oEnvironment.find("message")  != g_oEnvironment.end() ) 
    *poPulex << g_oEnvironment["message"];
  else
    *poPulex << "Hello World, this is I, a lonley pulex";
  if ( g_oEnvironment.find("host")     == g_oEnvironment.end() )  g_oEnvironment["host"]     = DEFAULT_HOST;
  if ( g_oEnvironment.find("port")     == g_oEnvironment.end() )  g_oEnvironment["port"]     = DEFAULT_PORT;
  if ( g_oEnvironment.find("password") == g_oEnvironment.end() )  g_oEnvironment["password"] = PASSWORD;

  std::string sUserCert  = CRT_CLT_PATH + poPulex->UsernameGet() + ".crt";
  std::string sUserKey   = CRT_CLT_PATH + poPulex->UsernameGet() + ".key";
  std::string sHostChain = CRT_HST_PATH  "cachain.pem";
  std::string sHostTPath = CRT_TRS_PATH;

  if ( bVerbose ) std::cout << sUserCert  << std::endl;
  if ( bVerbose ) std::cout << sUserKey   << std::endl;
  if ( bVerbose ) std::cout << "*" << g_oEnvironment["password"] << "*" << std::endl;
  if ( bVerbose ) std::cout << sHostChain << std::endl;
  if ( bVerbose ) std::cout << sHostTPath << std::endl;

  if ( bVerbose ) std::cout << *poPulex << std::endl;

  /// try to let all Pulexes jump to the server
  try
    {
    if ( bVerbose ) std::cout << " * HOST: " << g_oEnvironment["host"] << std::endl;
    if ( bVerbose ) std::cout << " * PORT: " << g_oEnvironment["port"] << std::endl;
	CSocketClient oConnection( g_oEnvironment["host"],
                               g_oEnvironment["port"],
                               sUserCert,
                               sUserKey,
                               g_oEnvironment["password"],
                               sHostChain,
                               sHostTPath );

    std::string sServerReply;
    /// iterate over all Pulexes in our Domain and let them jump
    try
      {
      for (CDomain::iterator it=oDomain.begin(); it != oDomain.end(); ++it)
        {
        oConnection << **it ;
        }
      oConnection << ".\r";

      /// collect the answer from the server (regarding ALL jumped Pulexes)
      std::string sInput;
      do
        {
        oConnection >> sInput;
        sServerReply += sInput;
        } while ( sInput.rfind(".\r") == std::string::npos );
      }
    catch ( CSocketException& e )
      {
      std::cout << "Exception: " << e.Info() << "\n";
      }
    if ( bVerbose ) std::cout << "Response from server:\n" << sServerReply << "\n";;
    }
  catch ( CSocketException& e )
    {
    std::cout << "Exception: " << e.Info() << "\n";
    }

  return 0;
  } // int main( int argc, char* argv[] )

/// print the normal console usage text to the console
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
            << "                       default: 'Hello World, this is I, a lonley pulex'" << std::endl << std::endl
            << "  -i, --cluid          force the use a specific client unique identifier" << std::endl
            << "                       default: <automatic>" << std::endl << std::endl
            ;
  exit( nStatus );
  } // void CEnvironment::Usage( int nStatus )
