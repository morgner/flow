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

//- typedef std::list<std::string> CData;       // x:*, b:*
typedef std::list<std::string> CListString;

class CContainer;
typedef std::map <std::string, CContainer*> CContainerMapByCLUID;

class CContainer : public CListString
  {
  private:
    static const std::string s_sClassName;

  protected:
    std::string m_sSender;
    CListString m_lsRecipients;
    std::string m_sClass;

    static long s_nLastClientSideId;
    static long s_nLastServerSideId;

    long        m_tClientSideId;   // time based client side id
    long        m_nClientSideId;   // sequence base client side id
    long        m_nServerSideId;   // server side id

  public:
             CContainer();
    virtual ~CContainer();

    virtual const std::string& SenderGet() const;
    virtual const std::string& SenderSet( const std::string& rsSender );

    virtual size_t RecipientAdd( const std::string& rsRecipient );
    virtual size_t RecipientDel( const std::string& rsRecipient );

    virtual long ClientSideTmGet() const;
    virtual long ClientSideTmSet( long nClientSideTm = 0 );
    virtual long ClientSideTmSet( const std::string& rsClientSideTm );

    virtual long ClientSideIdGet();
    virtual long ClientSideIdSet( long nClientSideId = 0 );
    virtual long ClientSideIdSet( const std::string& rsClientSideId );
                  
    virtual long ServerSideIdGet();
    virtual long ServerSideIdSet( long nServerSideId = 0 );
    virtual long ServerSideIdSet( const std::string& rsServerSideId );
                  
    virtual std::string RGUIDGet();
    virtual std::string CLUIDGet();

    virtual bool isFor( const std::string& rsRecipient );

    // e.g. "c:PULEX", "x:please read", "x:the manual"
    const std::string& operator += ( const std::string& rsElement );

  public:
    static const char* scn_sender;
    static const char* scn_recipient;
    static const char* scn_destination;
    static const char* scn_local_id;
    static const char* scn_local_id_time;
    static const char* scn_remote_id;
    static const char* scn_content_text;

  }; // class CContainer

