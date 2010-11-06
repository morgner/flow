/***************************************************************************
 pulex.cpp - description
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

#include "pulex.h"

const char* scn_username       = "u";
const char* scn_password       = "p";
const char* scn_destination    = "d";
const char* scn_class_name     = "c";
const char* scn_local_id       = "l";
const char* scn_local_id_time  = "t";
const char* scn_remote_id      = "r";
const char* scn_content_text   = "x";
const char* scn_content_binary = "b";

std::ostream& operator << (std::ostream& oStream, CPulex& oPulex )
  {
  oStream << scn_username      << ":" << oPulex.UsernameGet()    << "\n";
  oStream << scn_password      << ":" << oPulex.PasswordGet()    << "\n";
  oStream << scn_class_name    << ":" << oPulex.ClassNameGet()   << "\n";
  oStream << scn_local_id      << ":" << oPulex.LocalIdGet()     << "\n";
  oStream << scn_local_id_time << ":" << oPulex.LocalIdTimeGet() << "\n";
  oStream << scn_remote_id     << ":" << oPulex.RemoteIdGet()    << "\n";
  oStream << "===data-begin===\n";
  for ( CPulex::iterator it=oPulex.begin(); it != oPulex.end(); ++it )
    {
    oStream << scn_content_text << ":" << *it << "\n";
    }
  oStream << "===data-end===\n";
  return oStream;
  }

CSocket& operator << (CSocket& oStream, CPulex& oPulex )
  {
  oStream << scn_username      << ":" << oPulex.UsernameGet()    << "\n";
  oStream << scn_password      << ":" << oPulex.PasswordGet()    << "\n";
  oStream << scn_class_name    << ":" << oPulex.ClassNameGet()   << "\n";
  oStream << scn_local_id      << ":" << oPulex.LocalIdGet()     << "\n";
  oStream << scn_local_id_time << ":" << oPulex.LocalIdTimeGet() << "\n";
  oStream << scn_remote_id     << ":" << oPulex.RemoteIdGet()    << "\n";
  for ( CPulex::iterator it=oPulex.begin(); it != oPulex.end(); ++it )
    {
    oStream << scn_content_text << ":" << *it << "\n";
    }
  return oStream;
  }



const std::string CPulex::s_sClassName = "PULEX";


CPulex::CPulex()
  {
  }

CPulex::~CPulex()
  {
  }

const std::string& CPulex::ClassNameGet() const
  {
  return s_sClassName;
  }

const std::string& CPulex::operator << ( const std::string& rsData )
  {
  push_back(rsData);
  return rsData;
  }
