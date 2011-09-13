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

// STL
#include <map>
#include <iostream>

// SOCKET IO OBJECTS
#include "socketclient.h"
#include "socketexception.h"

// DATA OBJECTS
#include "domain.h"

// DEFINES
// These have to be bevor including environment.h because it uses them
#define VERSION  0.3
#define APPLNAME "flow Client"

#include "environment.h"

// Locale defines
#define CLT_HOST "localhost"
#define CLT_PORT "30000"

#define CRT_PATH  "certificates/client/"
#define CA_CHAIN  "certificates/server/server-CA-chain.pem"
#define CA_PATH   "certificates/trust/"
#define PASSWORD  ""


// We assume we are expected to be quiet
short g_nVerbosity = 0;


using namespace std;

int main( int argc, const char* argv[] )
  {
  CEnvironment oEnvironment( argc, argv );

  /// These are the command line options
  oEnvironment.OptionAppend( "help",        no_argument,       0, 'h', "Show this help text",                                "" );
  oEnvironment.OptionAppend( "version",     no_argument,       0, 'V', "Show version information",                           "" );
  oEnvironment.OptionAppend( "verbose",     optional_argument, 0, 'v', "Act verbose (0..2)",                                 "0" );
  // Contact
  oEnvironment.OptionAppend( "host",        required_argument, 0, 'H', "The host to conncet to",                             CLT_HOST );
  oEnvironment.OptionAppend( "port",        required_argument, 0, 'P', "The port to connect to",                             CLT_PORT );
  // Client Ident
  oEnvironment.OptionAppend( "sender",      required_argument, 0, 's', "Sender name or alias",                               "$LOGNAME" );
  oEnvironment.OptionAppend( "password",    required_argument, 0, 'w', "Password for senders key",                           PASSWORD );
  oEnvironment.OptionAppend( "recipient",   required_argument, 0, 'r', "Recipients name or alias",                           "recipient" );
  // Message
  oEnvironment.OptionAppend( "message",     required_argument, 0, 'm', "The message",                                        "" );
  // Container info
  oEnvironment.OptionAppend( "cluid",       required_argument, 0, 'i', "Client side local ID (numeric) of the message",      "" );
  oEnvironment.OptionAppend( "cert-dir",    required_argument, 0, 'd', "Directory for trusted and client certificates",      CRT_PATH );
  oEnvironment.OptionAppend( "trust-chain", required_argument, 0, 'a', "Trusted CA chain",                                   CA_CHAIN );
  oEnvironment.OptionAppend( "trust-path",  required_argument, 0, 't', "Path to trusted certificates",                       CA_PATH );
  // Calling control
  oEnvironment.OptionAppend( "call",        optional_argument, 0, 'c', "Call message(s) from the server",                    "" );
  oEnvironment.OptionAppend( "list",        no_argument,       0, 'l', "- list all (your) messages on the server",           "" );
  oEnvironment.OptionAppend( "start-time",  required_argument, 0, 'b', "- beginning with time as `date +%Y-%m-%d-%H:%M:%S`", "" );
  oEnvironment.OptionAppend( "start-glid",  required_argument, 0, 'g', "- beginning with ServerID 'n'",                      "" );
  oEnvironment.OptionAppend( "next",        no_argument,       0, 'n', "- the next message",                                 "" );
  oEnvironment.OptionAppend( "rest",        no_argument,       0, 'e', "- all new messages (start-time or start-glid)",      "" );

  // we need to read the command line parameters
  oEnvironment.CommandlineRead();
  // being verbose?
  g_nVerbosity =  atoi( oEnvironment["verbose"].c_str() );
  if ( g_nVerbosity ) cout << "Verbosity: " << g_nVerbosity << endl;

  // the real operation starts here
  CDomain oDomain;
  CPulex* poPulex;
  // our domain starts with only one Pulex
  oDomain += poPulex = new CPulex();

  // We need to know if we had piped input later on because if we also have
  // command line input, we need to insert a delimitter between both messages
  // Encryption will encrypt the complete set including the delimiter to higher
  // the complexity of decrypting the message by make the encrypted junk as
  // large as possible. I believe, decrypting short sentences may be easier,
  // so if one of the parts is short and the other long and both are encrypted
  // with the same key, it may become too easy to decrypt the message
  bool bHadPipeInput = false;
  // here we do a bit C to read stdin if any, oherwise ignore it silently
  // try to seek to the end of stdin,
  // nothings will move, we don't need to rewind(stdin) but ask ftell(stdin):
  //      0 = stdin is empty
  //     -1 = there is anything in there
  fseek (stdin, 0 , SEEK_END);
  // tell me if there are data in stdin
  if ( ftell(stdin) )
    {
    bHadPipeInput = true;

    #define CBS 2047
    char* pcBuffer = new char[CBS+1];
    if ( g_nVerbosity > 1 ) cout << "===pipe-begin===" << endl;
    while ( !feof(stdin) )
      {
      size_t nRead;
      // read CBS or the rest of from the in put device
      nRead = fread( pcBuffer, sizeof(char), CBS, stdin );

      if ( ferror(stdin) )
        {
        break;
        } // if ( ferror(stdin) )

      pcBuffer[nRead] = 0;
      if ( g_nVerbosity > 1 ) cout << pcBuffer;
      *poPulex << pcBuffer;
      } // while ( !feof(stdin) )
    if ( g_nVerbosity > 1 ) cout << "===pipe-end===" << endl << endl;
    delete pcBuffer;
    #undef CBS
    } // if ( !feof(stdin) )

  // putting together defaults and command line input for the Pulex
  poPulex->SenderSet   ( oEnvironment["sender"] );
  poPulex->RecipientSet( oEnvironment["recipient"] );
  poPulex->MessageIdSet( oEnvironment["cluid"] );

  if ( bHadPipeInput && ( oEnvironment["message"].length() ) )
    {
    *poPulex << "\n===== message next =====\n";
    }

  *poPulex << oEnvironment["message"];

  string sSenderCrt = oEnvironment["cert-dir"] + poPulex->SenderGet() + ".crt";
  string sSenderKey = oEnvironment["cert-dir"] + poPulex->SenderGet() + ".key";

  if ( g_nVerbosity > 1 )
    {
    cout << "Host.......: " << oEnvironment["host"] << endl;
    cout << "Port.......: " << oEnvironment["port"] << endl;
    cout << "Sender CRT.: " << sSenderCrt << endl;
    cout << "Sender KEY.: " << sSenderKey << endl;
    cout << "Password...: " << oEnvironment["password"]    << endl;
    cout << "T-Chain....: " << oEnvironment["trust-chain"] << endl;
    cout << "T-Path.....: " << oEnvironment["trust-path"]  << endl;
    cout << endl;
    }

  // try to let all Pulexes jump to the server
  try
    {
    CSocketClient oConnection( oEnvironment["host"],
                               oEnvironment["port"] );
    // This ist where the servers reply is going to
    string sServerReply;
    try
      {
      if ( oEnvironment.find("call") != oEnvironment.end() )
        {
        // call them all (the server knows which ones)
        oConnection << "c:all";
        }
      else
        {
        // iterate over all Pulexes in our Domain and let them jump
        for ( CDomain::iterator it  = oDomain.begin();
                                it != oDomain.end();
                              ++it )
          {
          oConnection << **it ;
          if ( g_nVerbosity > 1 ) cout << **it ;
          }
        }
      oConnection << ".\r";

      // for the parts the reply is parted into
      string sInput;
      // collect the answer from the server (regarding ALL jumped Pulexes)
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
    if ( g_nVerbosity ) cout << "Response from server:\n" << sServerReply << "\n";
    }
  catch ( CSocketException& e )
    {
    cout << "Exception: " << e.Info() << "\n";
    }

  return 0;
  } // int main( int argc, char* argv[] )
