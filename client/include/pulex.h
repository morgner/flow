/***************************************************************************
 pulex.h  -  description
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


#ifndef _PULEX_H
#define _PULEX_H

#if _MSC_VER > 1000
#pragma once
#pragma warning (disable:4786)
#endif // _MSC_VER > 1000

#include "container.h"

#include "socket.h"

#include <list>
#include <iostream>

class CPulex;

std::ostream& operator << ( std::ostream&, CPulex& );
     CSocket& operator << (      CSocket&, CPulex& );

class CPulex : public CContainer // , public CListString
  {
  private:
    typedef CContainer inherited;

    static const std::string s_sClassName;
    static       long        s_nClientSideId;

  protected:
    std::string m_sUserName;    // name or alias of the user who uses the object

  public:
             CPulex();
    virtual ~CPulex();

    virtual const std::string& ClassNameGet() const;

            const std::string& operator << ( const std::string& rsData );

    const std::string& UsernameSet( const std::string& rsUserName );
    const std::string& UsernameGet() const;

  public:
    template<typename T>
      T& Send( T& roStream );

  public:
    static const char* scn_username;

  }; // class CPulex


#endif // _PULEX_H
