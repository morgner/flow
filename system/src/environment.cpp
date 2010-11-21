/***************************************************************************
 environment.cpp
 -----------------------
 begin                 : Sat Nov 06 2010
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

#include <getopt.h>
#include <stdio.h>  // for EOF
#include <stdlib.h> // for atoi(), exit()


#include <iostream>

CEnvironment::CEnvironment()
  : m_bVerbose( false )
  {
  }

CEnvironment::~CEnvironment()
  {
  }

bool CEnvironment::isVerbose()
  {
  return m_bVerbose;
  }

const std::string& CEnvironment::ProgramNameGet()
  {
  return m_sProgramName;
  }

void CEnvironment::CommandlineRead( int argc, char* argv[], const struct option* ptOption )
  {
  m_sProgramName = argv[0];
  m_sProgramName = m_sProgramName.substr( m_sProgramName.rfind('/')+1 );

  extern char* optarg;
  int          c;
  int          option_index = 0;
  while ( (c = getopt_long (argc, argv, "HVvh:p:s:w:r:m:i:c:", ptOption, &option_index)) != EOF )
    {
    switch (c)
      {
      case 0:
        // If this option set a flag, do nothing else now
        if (ptOption[option_index].flag != 0)
          break;
        std::cout << "option " << ptOption[option_index].name;
        if (optarg)
          std::cout << " with arg " << optarg;
        std::cout << std::endl;
        break;

      case 'V':
        exit(0);

      case 'H':
        Usage(0);

      case 'v':
        m_bVerbose = true;
        break;

      case 'h':
        (*this)["host"] = optarg;
        break;

      case 'p':
        (*this)["port"] = optarg;
        break;

      case 's':
        (*this)["sender"] = optarg;
        break;

      case 'w':
        (*this)["password"] = optarg;
        break;

      case 'r':
        (*this)["recipient"] = optarg;
        break;

      case 'm':
        (*this)["message"] = optarg;
        break;

      case 'i':
        if ( atoi( optarg ) )
          {
          (*this)["cluid"] = optarg;
          }
        else
          {
          std::cout << "ERROR: --cluid (-i) has to be a numeric value greater than 0" << std::endl;
          exit(EXIT_FAILURE);
          }
        break;

      case 'c':
        (*this)["call"] = optarg;
        break;

//      default:
//        Usage (EXIT_FAILURE);
      }
    }

  if ( isVerbose() )
    for ( CEnvironment::iterator it=g_oEnvironment.begin(); it != g_oEnvironment.end(); ++it )
      {
      std::cout << it->first << ": " << it->second << std::endl;
      }

  } // void CEnvironment::CommandlineRead( ... )

