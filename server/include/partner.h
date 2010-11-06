/***************************************************************************
 partner.h - description
 -----------------------
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


#include "socket.h"

#include <pthread.h>

class CPartner
  {
  protected:
    volatile bool   m_bStopRequested;
    volatile bool   m_bRunning;
    pthread_mutex_t m_tMutex;
    pthread_t       m_tThread;

    CSocket*        m_poSocket;  
  
  public:
    CPartner();
   ~CPartner();
  
    void Communicate( CSocket* poSocket );
    void Stop();

  protected:
    static void* thread( void* );

    void Action();

  }; // class CPartner
