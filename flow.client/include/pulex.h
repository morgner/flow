/***************************************************************************
 pulex.h  -  description
 -----------------------
 begin                 : Fri Oct 29 2010
 copyright             : Copyright (C) 2010 by Manfred Morgner
 email                 : manfred@morgner.com

$Id: pulex.h,v 1.6 2010/10/29 17:15:44 morgner Exp $
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

#include "home.h"

#include <list>
#include <iostream>
#include "socket.h"

class CPulex;

std::ostream& operator<< ( std::ostream&, CPulex& );
     CSocket& operator<< (      CSocket&, CPulex& );

class CPulex : public CHome, public std::list<std::string>
  {
  private:
    typedef CHome inherited;

    static const std::string s_sClassName;

  public:
             CPulex();
    virtual ~CPulex();

    virtual const std::string& ClassNameGet() const;
  
            const std::string& operator << ( const std::string&  oString );

  }; // class CPulex


#endif // _PULEX_H
