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
  oEnvironment.OptionAppend( "list",        required_argument, 0, 'l', "List (all|part|mine|mine-part|senders|msg-id)",      "" );
  oEnvironment.OptionAppend( "call",        required_argument, 0, 'c', "Receive (msg-id)",                                   "" );
  oEnvironment.OptionAppend( "cbsr",        required_argument, 0, 'b', "Receive by sender (msg-id)",                         "" );

  // we need to read the command line parameters
  oEnvironment.CommandlineRead();

  // dump all recognized command line parameters including associated values
  if ( g_nVerbosity > 1 )
    {
    for ( CEnvironment::iterator it  = oEnvironment.begin();
                                 it != oEnvironment.end();
                               ++it )
      {
      cout << " >>>>" << it->first << ":"  << it->second << endl;
      }
    } // if ( g_nVerbosity > 1 )

  // the level of verbosity
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
      string sCommand;
      if ( oEnvironment.count("list") > 0 ) sCommand = "list";
      if ( oEnvironment.count("call") > 0 ) sCommand = "call";
      if ( oEnvironment.count("cbsr") > 0 ) sCommand = "cbsr";
      if ( sCommand.length() )
        {
        string sParameter = oEnvironment[sCommand];

        string sCall;
        sCall += sCommand[0];
        sCall += ":";
        if ( sParameter[0] == 'l' )
          sCall += sParameter[0];
        else // not list? we expect 'reCall operations'
          if ( sCommand == "call" )
            sCall += "m " + sParameter;
          else // not 'call', we assume 'cbsr'
            sCall += "s " + sParameter;
/*
        client command   | send to server   | receive from server
        =================+==================+====================
        --list all       | l:a              |  m:<message-id>
                         |                  | [...]
        --list part      | l:p <message-id> |  m:<message-id>
                         |                  | [...]
        --list mine      | l:m              |  m:<message-id>
                         |                  | [...]
        --list mine-part | l:y <message-id> |  m:<message-id>
                         |                  | [...]
        --list senders   | l:s              |  s:<sender-id>
                         |                  | [...]
        --call MsgID     | c:m <message-id> |  m:<message-id>
                         |                  |  z:<size in bytes>
                         |                  | [<body>]
                         |                  | [n:<next message-id>]
        --cbsr MsgID     | c:s <message-id> |  m:<message-id>
                         |                  |  z:<size in bytes>
                         |                  | [<body>]
                         |                  | [n:<next message-id>]
*/
        if ( g_nVerbosity > 1 ) cout << "Command: " << sCall << "\n";
        oConnection << sCall << "\n";
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
