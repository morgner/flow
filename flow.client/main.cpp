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

#include "domain.h"
#include "socketclient.h"
#include "socketexception.h"

#include <getopt.h>

#include <map>
#include <iostream>


#define VERSION "0.1"

static int decode_switches (int argc, char **argv);

typedef std::map<std::string, std::string> CEnvironment;

bool         g_bVerbose = false;
CEnvironment g_oEnvironment;

int main( int argc, char* argv[] )
  {
  decode_switches (argc, argv);

  CDomain oDomain;
  CPulex* poPulex;

  oDomain += poPulex = new CPulex();

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

  try
    {
    if ( g_bVerbose ) std::cout << " * HOST: " << g_oEnvironment["host"] << std::endl;
    if ( g_bVerbose ) std::cout << " * PORT: " << g_oEnvironment["port"] << std::endl;
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
    if ( g_bVerbose ) std::cout << "Response from server:\n" << sData << "\n";;
    }
  catch ( CSocketException& e )
    {
    std::cout << "Exception: " << e.Info() << "\n";
    }

  return 0;
  }



std::string g_sProgramName;

static struct option const g_tLongOptions[] =
  {
    { "help",     no_argument, 0, 'H'},
    { "version",  no_argument, 0, 'V'},
    { "verbose",  no_argument, 0, 'v'},

    { "host",      required_argument, 0, 'h'},
    { "port",      required_argument, 0, 'p'},
    { "user",      required_argument, 0, 'u'},
    { "recipient", required_argument, 0, 'r'},
    { "message",   required_argument, 0, 'm'},
    { "cluid",     required_argument, 0, 'i'},

    { NULL, 0, NULL, 0}
  };

static void usage (int status)
  {
  std::cout << g_sProgramName << " - carries your messages in protected mode" << std::endl;
  std::cout << "Usage: " <<  g_sProgramName << " [OPTION]... [FILE]..." << std::endl;
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
  exit (status);
  } // void usage (int status)

static int decode_switches (int argc, char* argv[])
  {
  g_sProgramName = argv[0];
  g_sProgramName = g_sProgramName.substr( g_sProgramName.rfind('/')+1 );

  int c;
  int option_index = 0;
  while ( (c = getopt_long (argc, argv, "HVvh:p:u:r:m:i:", g_tLongOptions, &option_index)) != EOF )
    {
    switch (c)
      {
      case 0:
        // If this option set a flag, do nothing else now
        if (g_tLongOptions[option_index].flag != 0)
          break;
        printf ("option %s", g_tLongOptions[option_index].name);
        if (optarg)
          printf (" with arg %s", optarg);
        printf ("\n");
        break;

      case 'V':
        exit (0);

      case 'H':
        usage (0);

      case 'v':
        g_bVerbose = true;
        break;

      case 'h':
        g_oEnvironment["host"] = optarg;
        break;

      case 'p':
        g_oEnvironment["port"] = optarg;
        break;

      case 'u':
        g_oEnvironment["user"] = optarg;
        break;

      case 'r':
        g_oEnvironment["recipient"] = optarg;
        break;

      case 'm':
        g_oEnvironment["message"] = optarg;
        break;

      case 'i':
        if ( atoi( optarg ) )
          {
          g_oEnvironment["cluid"] = optarg;
          }
        else
          {
          std::cout << "ERROR: --cluid (-i) has to be a numeric value greater than 0" << std::endl;
          exit(EXIT_FAILURE);
          }
        break;

//      default:
//        usage (EXIT_FAILURE);
      }
    }

  if ( g_bVerbose )
    for ( CEnvironment::iterator it=g_oEnvironment.begin(); it != g_oEnvironment.end(); ++it )
      {
      std::cout << it->first << ": " << it->second << std::endl;
      }

  return optind;
  } // int decode_switches (int argc, char* argv[])
