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

#include <iostream>

int main( int argc, char* argv[] )
  {
  CDomain oDomain;
  CPulex* poPulex;
  
  oDomain += poPulex = new CPulex();
  *poPulex << "Daten";

  for ( int i=0; i < 2000; i++)
    {
    oDomain += poPulex = new CPulex();
    *poPulex << "Senden";
    }
/*
  oDomain += poPulex = new CPulex();
  *poPulex << "und";
  *poPulex << "Empfangen";
*/
/*
  CPulex* p = *(--oDomain.end());
  *p << "additional data";
*/

  std::string sHost = "localhost";
  if ( argc > 1 )
    {
    sHost = argv[1];
    }

  try
    {
    CSocketClient oConnection( sHost, 30000 );
    
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
      std::cout << "Exception:" << e.Info() << "\n";
      }
    std::cout << "Response from server:\n" << sData << "\n";;
    }
  catch ( CSocketException& e )
    {
    std::cout << "Exception:" << e.Info() << "\n";
    }

	return 0;
  }
