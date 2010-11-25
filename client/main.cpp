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


#include <stdlib.h> // for atoi(), exit()
#include <stdio.h>  // for EOF
#include <fcntl.h>  // for fnctl()

#include "environment.h"


int main( int argc, const char* argv[] )
  {
  CEnvironment oEnvironment( argc, argv );

  /// these are the command line options                           
  oEnvironment.OptionAppend( "help",      no_argument,       0, 'H', "Show this help text",                           "" );
  oEnvironment.OptionAppend( "version",   no_argument,       0, 'V', "Show version information",                      "" );
  oEnvironment.OptionAppend( "verbose",   no_argument,       0, 'v', "Act verbose",                                   "" );
  oEnvironment.OptionAppend( "host",      required_argument, 0, 'h', "The host to conncet to",                        DEFAULT_HOST );
  oEnvironment.OptionAppend( "port",      required_argument, 0, 'p', "The port to connect to",                        DEFAULT_PORT );
  oEnvironment.OptionAppend( "sender",    required_argument, 0, 's', "Sender name or alias",                          "$LOGNAME" );
  oEnvironment.OptionAppend( "password",  required_argument, 0, 'w', "Password for senders key",                      PASSWORD );
  oEnvironment.OptionAppend( "recipient", required_argument, 0, 'r', "Recipients name or alias",                      "recipient" );
  oEnvironment.OptionAppend( "message",   required_argument, 0, 'm', "The message",                                   "Hi there, I'm from far away" );
  oEnvironment.OptionAppend( "cluid",     required_argument, 0, 'i', "Client side local ID (numeric) of the message", "1" );
  oEnvironment.OptionAppend( "call",      optional_argument, 0, 'c', "Call message(s) from the server",               "" );

  /// we need to read the command line parameters
  oEnvironment.CommandlineRead();
  /// for convienice we create a local shortcut to the environment value of 'isVerbose()'
  bool bVerbose = oEnvironment.find("verbose") != oEnvironment.end();

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
    poPulex->SenderSet       ( oEnvironment["sender"] );
    poPulex->RecipientAdd    ( oEnvironment["recipient"] );
    poPulex->ClientSideIdSet ( atoi(oEnvironment["cluid"].c_str()) );

    *poPulex << oEnvironment["message"];

  std::string sSenderCrt = CRT_CLT_PATH + poPulex->SenderGet() + ".crt";
  std::string sSenderKey = CRT_CLT_PATH + poPulex->SenderGet() + ".key";
  std::string sHostChain = CRT_HST_PATH  "cachain.pem";
  std::string sHostTPath = CRT_TRS_PATH;

  if ( bVerbose ) std::cout << sSenderCrt << std::endl;
  if ( bVerbose ) std::cout << sSenderKey << std::endl;
  if ( bVerbose ) std::cout << "*" << oEnvironment["password"] << "*" << std::endl;
  if ( bVerbose ) std::cout << sHostChain << std::endl;
  if ( bVerbose ) std::cout << sHostTPath << std::endl;

  if ( bVerbose ) std::cout << *poPulex << std::endl;

  /// try to let all Pulexes jump to the server
  try
    {
    if ( bVerbose ) std::cout << "HOST: " << oEnvironment["host"] << std::endl;
    if ( bVerbose ) std::cout << "PORT: " << oEnvironment["port"] << std::endl;
    CSocketClient oConnection( oEnvironment["host"],
                               oEnvironment["port"],
                               sSenderCrt,
                               sSenderKey,
                               oEnvironment["password"],
                               sHostChain,
                               sHostTPath );

    std::string sServerReply;
    /// iterate over all Pulexes in our Domain and let them jump
    try
      {
      for (CDomain::iterator it=oDomain.begin(); it != oDomain.end(); ++it)
        {
        oConnection << **it ;
//        oConnection << "c:all" ;
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
