/***************************************************************************
 container.h description
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


#include <map>
#include <list>
#include <string>

typedef std::list<std::string> CData;         // x:*
typedef std::list<std::string> CReceiverList; // d:*

class CContainer;
typedef std::map <std::string, CContainer*> CContainerMapByCLUID;

class CContainer : public std::list<std::string>
  {
  protected:
    std::string   m_sSender;
    CReceiverList m_lsReceivers;
    std::string   m_sClass;
    std::string   m_sClientLocalId;
    std::string   m_sClientLocalIdTime;
    std::string   m_sServerId;
  

  public:
     CContainer();
    ~CContainer();

    const std::string& ServerIdGet() const;
    const std::string& ServerIdSet( const std::string& sServerId );

          std::string  RGUIDGet() const;
          std::string  CLUIDGet() const;
    const std::string& OwnerGet() const;

    // e.g. "c:PULEX", "x:please read", "x:the manual"
    const std::string& operator += ( const std::string& rsElement );

  }; // class CContainer

