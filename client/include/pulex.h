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

#include "container.h"
#include "socket.h"

#include <list>
#include <iostream>


extern bool g_bVerbose;

// The Pulex class represents a neutral object containing client data to become
// moved to other clients using a server able to move Container objects. To be
// able to do so, a Container is able to move data wiht minimum amount of
// knowledge of what these data are. This works similar to a packet moved by a
// post office. The sender ist known, the receiver ist known but the content is
// not.
// Regarding a Pulex, the sender ist mainly unknown, the receiver will become
// partially known if and when he calls is Container. The server will give the
// packet to every receiver who successfuly claims to be the receiver of it.
// To make things with a Pulex not as easy as with a Container, a Pulex sends
// the identity of its sender and its receiver in the form of a 40 character
// ascii representation of the fingerprint of their certificates.

class CPulex : public CContainer
  {
  private:
    // The parent class to be used as 'inherited::methode()' for abstract
    // usage of inherited methods. Mimimizes logical redundancy
    typedef CContainer inherited;

    static const std::string s_sClassName;

  public:
             CPulex();
    virtual ~CPulex();

    virtual const std::string& ClassNameGet() const;

    // May be this is not a good idea at all, but enables compact code, which
    // also may not be good idea. This operator appends data to the object as
    // if it were a data sink which it is ;-)
            const std::string& operator << ( const std::string& rsData );

    // Friendly operators to pipe data to different types of stream
    friend std::ostream& operator << ( std::ostream&, CPulex& );
    friend      CSocket& operator << (      CSocket&, CPulex& );
  protected:
    // A template to be used by output pipe operators to send the puley off
    template<typename T>
      T& Send( T& roStream );

  protected:
    long ClientSideIDGet();

  }; // class CPulex


#endif // _PULEX_H
