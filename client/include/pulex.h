/***************************************************************************
 pulex.h
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

#include "container.h"
#include "socket.h"

#include <list>
#include <iostream>


/*****************************************************************************

 CPulex is a class knowing how to deal with messages to let them jump from one
 client to another one using appropriate servers. Meaning, a CPulex instande
 knows how to deal with flow-servers to push and pull messages.

 It may be important to understand that one pulex is intended to be sent to
 one receiver only. Thus not directly creating networks of receivers and also
 for later use if messages are encrypted.
 ----------
 Without TLS the pulex needs to send the plain sender identification with the
 message so the server.

 With TLS in place, this is no more neccessary because the server already
 knows all he needs to know from the TLS connection in use.
 ----------
 The flow messaging protocol is a plain ASCII protocol to prevent two effects:

  * Someone kills the server or a client by invalid input
  * Excessive us of computing ressources

 *****************************************************************************/

class CPulex : public CContainer
  {
  private:
    typedef CContainer inherited;

    static const std::string s_sClassName;

  protected:
    std::string m_sMessageId;

  public:
             CPulex();
    virtual ~CPulex();

    const std::string& MessageIdSet( const std::string& sMessageId );
    const std::string& MessageIdGet();

    // May be this is not a good idea at all, but enables compact code, which
    // also may not be good idea. This operator appends data to the object as
    // if it were a data sink which it is ;-)
            const std::string& operator << ( const std::string& rsData );

    // Friendly operators to pipe data to different types of stream
    friend std::ostream& operator << ( std::ostream&, CPulex& );
    friend      CSocket& operator << (      CSocket&, CPulex& );
  protected:
    // A template to be used by output pipe operators to send the pulex off
    template<typename T>
      T& Send( T& roStream );

  protected:
    virtual const std::string& ClassNameGet() const;

  }; // class CPulex


#endif // _PULEX_H
