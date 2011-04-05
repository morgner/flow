/***************************************************************************
 partner.h 

 begin                 : Fri Nov 05 2010
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

/***************************************************************************
 partner.h - description
 -----------------------
 A CPartner object represents an COMMUNICATION PARTNER. Such communication
 partner is an abstract entity. Its identity is given by the client
 certificate it used to connect to the server. Don't be fooled by this
 certificate! A real person is not limited by one certificate. But CPartner
 only ever identifies the partner, not the real person.

 The mission of a CPartner instance is to generate CContainer objects
 containing all information to serve as a recognizable, enumerable, sortable
 message from a sender to a receiver.
 ***************************************************************************/

#include "socketssl.h"

#include <pthread.h>

extern bool g_bVerbose;

class CPartner
  {
  protected:
    volatile bool   m_bStopRequested;
    volatile bool   m_bRunning;
    pthread_mutex_t m_tMutex;
    pthread_t       m_tThread;

    CSocketSSL*     m_poSocket;
    std::string     m_sIdentity;

  public:
    CPartner();
   ~CPartner();

    void Communicate( CSocketSSL* poSocket );
    void Stop();

  protected:
    static void* thread( void* );

    void Action();

    size_t Recall         ( const std::string& rsClientData,
                                  CSocket*     roSocket );
    size_t BuildContainers( const std::string& rsClientData );

  }; // class CPartner
