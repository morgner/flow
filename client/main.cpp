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

#define CRT_PATH  "certificates/client/"
#define CA_CHAIN  "certificates/server/server-CA-chain.pem"
#define CA_PATH   "certificates/trust/"
#define PASSWORD  ""


#include <stdlib.h> // for atoi(), exit()
#include <stdio.h>  // for EOF
#include <fcntl.h>  // for fnctl()

#include "environment.h"

#include <openssl/err.h>


bool g_bVerbose = false;

using namespace std;

int main( int argc, const char* argv[] )
  {
  CEnvironment oEnvironment( argc, argv );

  /// these are the command line options                           
  oEnvironment.OptionAppend( "help",        no_argument,       0, 'H', "Show this help text",                           "" );
  oEnvironment.OptionAppend( "version",     no_argument,       0, 'V', "Show version information",                      "" );
  oEnvironment.OptionAppend( "verbose",     no_argument,       0, 'v', "Act verbose",                                   "" );
  oEnvironment.OptionAppend( "host",        required_argument, 0, 'h', "The host to conncet to",                        DEFAULT_HOST );
  oEnvironment.OptionAppend( "port",        required_argument, 0, 'p', "The port to connect to",                        DEFAULT_PORT );
  oEnvironment.OptionAppend( "sender",      required_argument, 0, 's', "Sender name or alias",                          "$LOGNAME" );
  oEnvironment.OptionAppend( "password",    required_argument, 0, 'w', "Password for senders key",                      PASSWORD );
  oEnvironment.OptionAppend( "recipient",   required_argument, 0, 'r', "Recipients name or alias",                      "recipient" );
  oEnvironment.OptionAppend( "message",     required_argument, 0, 'm', "The message",                                   "Hi there, I'm from far away" );
  oEnvironment.OptionAppend( "cluid",       required_argument, 0, 'i', "Client side local ID (numeric) of the message", "1" );
  oEnvironment.OptionAppend( "cert-dir",    required_argument, 0, 'd', "Directory for trusted and client certificates", CRT_PATH );
  oEnvironment.OptionAppend( "trust-chain", required_argument, 0, 'a', "Trusted CA chain",                              CA_CHAIN );
  oEnvironment.OptionAppend( "trust-path",  required_argument, 0, 't', "Path to trusted certificates",                  CA_PATH );
  oEnvironment.OptionAppend( "call",        optional_argument, 0, 'c', "Call message(s) from the server",               "" );

  // we need to read the command line parameters
  oEnvironment.CommandlineRead();
  // being verbose?
  g_bVerbose = oEnvironment.find("verbose") != oEnvironment.end();

  // the real operation starts here
  CDomain oDomain;
  CPulex* poPulex;
  // our domain starts with only one Pulex
  oDomain += poPulex = new CPulex();


  // reading the (potentially given) piped input'directly' into the pulex object
  // the operation is a bit tricky because of a little problem with cin we need
  // to read unblocked for several reanson. one of the is the fact that we can't
  // be certain, tha piped input is presented anyway possibly importand is the
  // operation to set the file control flags back to the patter from before out
  // manipulation to ensure normal behavior of  cin after the unblocked operation
  // --
  // -- this methode (?) of deblocking cin not reliable for pipe input
  // -- have to work on it !
  if ( g_bVerbose ) cout << "===pipe-begin===" << endl;
  int flags = ::fcntl(fileno(stdin), F_GETFL, 0);
  ::fcntl( ::fileno(stdin), F_SETFL, flags | O_NONBLOCK );
    string s;
    cin.clear();
//    for ( getline(cin, s); cin.good(); getline(cin, s) )
//    while ( getline(cin, s) )
    for ( getline( cin, s ); !cin.eof(); getline(cin, s) )
      {
      if ( g_bVerbose ) cout << s << endl;
      *poPulex << s;
//      if ( !cin.good() ) break;
      }
    ::fcntl( ::fileno(stdin), F_SETFL, flags );
  cin.clear();
  if ( g_bVerbose ) cout << "===pipe-end===" << endl << endl;

  // putting together defaults and command line input for the Pulex
  poPulex->SenderSet      ( oEnvironment["sender"] );
  poPulex->RecipientAdd   ( oEnvironment["recipient"] );
  poPulex->ClientSideIdSet( atoi(oEnvironment["cluid"].c_str()) );

  *poPulex << oEnvironment["message"];

  string sSenderCrt = oEnvironment["cert-dir"] + poPulex->SenderGet() + ".crt";
  string sSenderKey = oEnvironment["cert-dir"] + poPulex->SenderGet() + ".key";

  if ( g_bVerbose )
    {
    cout << sSenderCrt << endl;
    cout << sSenderKey << endl;
    cout << "*" << oEnvironment["password"] << "*" << endl;
    cout << oEnvironment["trust-chain"] << endl;
    cout << oEnvironment["trust-path"]  << endl;
    cout << *poPulex << endl;
    cout << "HOST: " << oEnvironment["host"] << endl;
    cout << "PORT: " << oEnvironment["port"] << endl;
    }

  // try to let all Pulexes jump to the server

  ERR_load_crypto_strings(); // or: SSL_load_error_strings();
  try
    {
    CSocketClient oConnection( oEnvironment["host"],
                               oEnvironment["port"],
                               sSenderCrt,
                               sSenderKey,
                               oEnvironment["password"],
                               oEnvironment["trust-chain"],
                               oEnvironment["trust-path"] );

    string sServerReply;
    // iterate over all Pulexes in our Domain and let them jump
    try
      {
      for ( CDomain::iterator it  = oDomain.begin();
                              it != oDomain.end();
                            ++it )
        {
        oConnection << **it ;
//        cout << **it ;
//        oConnection << "c:all" ;
        }
      oConnection << ".\r";

      // collect the answer from the server (regarding ALL jumped Pulexes)
      string sInput;
      do
        {
        oConnection >> sInput;
        sServerReply += sInput;
        } while ( sInput.rfind(".\r") == string::npos );
      }
    catch ( CSocketException& e )
      {
      cout << "Exception: " << e.Info() << "\n";
      }
    if ( g_bVerbose ) cout << "Response from server:\n" << sServerReply << "\n";;
    }
  catch ( CSocketException& e )
    {
    cout << "Exception: " << e.Info() << "\n";
    }
  ERR_free_strings();

  return 0;
  } // int main( int argc, char* argv[] )
