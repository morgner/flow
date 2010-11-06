/***************************************************************************
 home.h  -  description
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


#ifndef _HOME_H
#define _HOME_H

#if _MSC_VER > 1000
#pragma once
#pragma warning (disable:4786)
#endif // _MSC_VER > 1000

#include <string>

class CHome
  {
  private:
    static const std::string s_sClassName;

  protected:
    static std::string s_sUsername;
    static std::string s_sPassword;
    static long        s_nLastLocalId;

  protected:
    long m_tLocalId;  // time base local id
    long m_nLocalId;  // sequence base local id
    long m_nRemoteId; // remote id (to receive from a server)

  public:
             CHome();
    virtual ~CHome();

    virtual const std::string& ClassNameGet() const;

            long LocalIdGet();
            long LocalIdSet( long lid );
            long LocalIdTimeGet();

            long RemoteIdSet( long lid );
            long RemoteIdGet();

            const std::string& UsernameSet( const std::string& sUsername );
            const std::string& UsernameGet();
            const std::string& PasswordSet( const std::string& sPassword );
            const std::string& PasswordGet();

  }; // class CHome

#endif // _HOME_H

