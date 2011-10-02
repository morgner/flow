/***************************************************************************
 partner.cpp description
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


#include "partner.h"
#include "socketexception.h"

#include <iostream>

#include <sys/time.h> // gettimeofday()
#include <errno.h>    // EAGAIN, ...

using namespace std;

// Thread function performing client communication the used object is left free
// into the open space, so at the end of the thread, the used object is to
// destroy
// We trust callers to provide us with an instance of a CPartner class, there
// is no check possible, given, that we use a reinterpret_cast on a void*
void* CPartner::thread(void* pObject)
  {
  // Threads receive void pointers, we know (assume) this pointer points to an
  // instance of CPartner.
  CPartner* poPartner = reinterpret_cast<CPartner*>(pObject);
  // We are context free, so there is only one action: "receive and react"
  poPartner->Action();
  // That's all for the object
  delete poPartner;
  // And off we flow
  pthread_exit( NULL );
  } // void* CPartner::thread(void* pObject)



CPartner::CPartner()
  : m_bStopRequested(false),
    m_bRunning(false),
    m_poSocket(0)
  {
  // Possibly, we need some coordination for database access. Multithreading
  // is a threadening kind of programming, so we have to be very carefull.
  // The best way to do multithreading is: Not to do it.
  pthread_mutex_init( &m_tMutex, 0 );
  }


// We erase everything we know of. Keep in mind, we are multithreaded!
CPartner::~CPartner()
  {
  if ( m_bRunning )
    {
    cout << "ERROR: ~CPartner() called while 'running' flag is 'on'" << endl;
    // This is not a point where Stop() can be a valid call. We expect to be
    // called by the terminating theard.
    // Calling Stop() here leads to recursion.
    // Stop();
    }

  // The mutex is no longer of use
  pthread_mutex_destroy( &m_tMutex );

  // We end the communication with client unconditioned
  delete m_poSocket;
  } // CPartner::~CPartner()


// This is the database of messages, defined elsewhere
extern CContainerMapByCLUID g_oContainerMapByCLUID;

// If we received a message, the message needs a unique key to refere to based
// on the content of the filled data container
string CPartner::MakeMessageKey( CContainer* poContainer )
  {
  string sMessageId = poContainer->MessageIdGet();

  if ( sMessageId.length() == 0 )
    {
    // generate a first simple version of Message-ID
    timeval time;
    gettimeofday( &time, 0 );
    string sTime = to_string( time.tv_sec );

    string sSender    = poContainer->SenderGet();
    string sRecipient = poContainer->RecipientGet();
    int    nSuffix    = 0;
#define CONSTUCT_ID sSender + ':' + sRecipient + ':' + sTime + ':' + to_string(nSuffix)
    for ( ; g_oContainerMapByCLUID.find( CONSTUCT_ID ) != g_oContainerMapByCLUID.end(); nSuffix++ )
      {
      if ( g_bVerbose ) cout << " -found- " << CONSTUCT_ID << endl;
      }
    sMessageId = CONSTUCT_ID;
#undef CONSTUCT_ID
    } // if ( sMessageId.length() == 0 )

  return sMessageId;
  } // string CPartner::MakeMessageKey( CContainer* poContainer )


// Start the thread
// We have to give the server, who called us into life, free. To do so, we
// start a thread to deal with us. This thread is given control about us. So we
// are able to return immediately to the call who is waitung to generate a new
// object of our kind to receiv the next call.
// So THIS is the action the server calls to start us up flying.
void CPartner::Communicate( CSocket* poSocket ) 
  {
  m_poSocket  = poSocket;
  int nResult = pthread_create(&m_tThread, 0, &CPartner::thread, this);
  // If we can't create the controlling thead we'r lost! No one ever will find
  // us we we must commit suicide. Otherwise, we will become a memory leak.
  if ( 0 != nResult )
    {
    cout << "ERROR: In Communicate(), thread not created, result: " << nResult;
    switch ( nResult )
      {
      case EAGAIN:
        cout << " (ressources not sufficient)";
        break;
      case EINVAL:
        cout << " (invalid thread parameters)";
        break;
      case EPERM:
        cout << " (unsufficient permissions)";
        break;
      }
    cout << endl;

    // No one knows us anymore, so we have to erase us ourself
    delete this;
    } // if ( 0 != nResult )
  } // void CPartner::Communicate( CSocket* poSocket ) 


// Stopp the thread
void CPartner::Stop()
  {
  m_bRunning = false;
  m_bStopRequested = true;
  pthread_join( m_tThread, 0 );
  } // void CPartner::Stop()


// What we do is:
//  * Receive client data
//  * do what's to be requested if the request is valid
//  * answer properly if possible (or not if not)
//  * end the call
void CPartner::Action()
  {
  m_bRunning  = true; // Now we're running

  string sClientData;
  string sInput;
  do
    {
    try
      {
      *m_poSocket >> sInput;
      }
    catch ( CSocketException& e)
      {
      cout << "ERROR: " << e.Info() << endl;
      if ( e.isFatal() ) break;
      }
    if ( g_bVerbose ) cout << sInput;
    sClientData += sInput;
    } while ( !m_bStopRequested && sClientData.rfind(".\r") == string::npos );

  if ( (sClientData[1] == ':') &&
      ((sClientData[0] == CContainer::scn_recall) ||
       (sClientData[0] == CContainer::scn_list)) )
    {
    if (g_bVerbose) cout << " * recall; container: "
                         << g_oContainerMapByCLUID.size() << endl;
    Recall( sClientData, m_poSocket );
    }
  if ( (sClientData[1] == ':') &&
       (sClientData[0] == CContainer::scn_sender) )
    {
    if (g_bVerbose) cout << " * received container " << endl;
    BuildContainers( sClientData );
    }
  *m_poSocket << ".\r";

  Stop(); // That's all for now and here
  } // void CPartner::Action()


// We recall messages stored in the database
size_t CPartner::Recall( const string&  rsClientData,
                               CSocket* poSocket )
  {
  typedef map<string, string> CMapString2String;
  CMapString2String moQuery;

  for (string::size_type p1=0, p2=0;
       string::npos != ( p2 = rsClientData.find('\n', p1) );
       p1 = p2+1)
    {
    string s = rsClientData.substr(p1, p2-p1);
    size_t l = s.length();
    if ( (l > 2) && (s[1] == ':') )
      {
      if ( l > 4)
        moQuery[ s.substr(0, 3) ] = s.substr(4);
        else
        moQuery[ s.substr(0, 3) ] = "";
      }
    } // for (string::size_type p1=0, p2=0; ...

/*
        client command      | send to server   | receive from server
        ====================+==================+====================
        list all            | l:a              |  m:<message-id>
                            |                  | [...]
        list partially      | l:p <message-id> |  m:<message-id>
                            |                  | [...]
        list mine           | l:m              |  m:<message-id>
                            |                  | [...]
        list mine partially | l:y <message-id> |  m:<message-id>
                            |                  | [...]
        list senders        | l:s              |  s:<sender-id>
                            |                  | [...]
        call all            | c:a              |  m:<message-id>
                            |                  | [...]
        call MsgID          | c:m <message-id> |  m:<message-id>
                            |                  | [<body>]
                            |                  | [n:<next message-id>]
        cbsr MsgID          | c:s <message-id> |  m:<message-id>
                            |                  | [<body>]
                            |                  | [n:<next message-id>]
*/
  for ( CMapString2String::iterator itQuery  = moQuery.begin();
                                    itQuery != moQuery.end();
                                  ++itQuery)
    {
    if ( g_bVerbose)
      {
      cout << " * " << itQuery->first;
      if ( itQuery->second.length() > 0 ) cout << " with " << itQuery->second;
      cout << "\n";
      }

    CContainerMapByCLUID::iterator it;

    switch ( itQuery->first[0] )
      {
      case 'l': // list operations
        switch ( itQuery->first[2] )
          {
          case 'a': // l:a list all
            if ( g_bVerbose) cout << " * list all \n";
/*
              1) get the recipient from the TLS-session
              2) resend all message ids for this sender
 */
            for ( it = g_oContainerMapByCLUID.begin(); it != g_oContainerMapByCLUID.end(); ++it )
              {
              *poSocket << it->first << "\n";
              }
            break;
          case 'p': // l:p list part, beginning with message-id
/*
              1) get the recipient from the TLS-session
              2) resend all message ids for this sender starting with the given message-id
 */
            if ( g_bVerbose) cout << " * list partial, beginning with '" << itQuery->second << "'\n";
            for ( it = g_oContainerMapByCLUID.find(itQuery->second); it != g_oContainerMapByCLUID.end(); ++it )
              {
              *poSocket << it->first << "\n";
              }
            break;
          case 'm': // l:m list mine (sent by me)
            if ( g_bVerbose) cout << " * list mine\n";
/*
              1) get the recipient from the TLS-session
              2) resend all message ids from this sender
 */
            break;
          case 'y': // l:y list mine part, beginning with message-id
            if ( g_bVerbose) cout << " * list mine partial, beginning with '" << itQuery->second << "'\n";
/*
              1) get the recipient from the TLS-session
              2) resend all message ids from this sender starting at the given message-id
 */
            break;
          case 's': // l:s list senders
            if ( g_bVerbose) cout << " * list senders\n";
/*
              1) get the recipient from the TLS-session
              2) resend all senders with messages for this recipients
 */
            typedef map<string, bool> CMapString2Bool;
            CMapString2Bool omSenders;
            for ( it = g_oContainerMapByCLUID.begin(); it != g_oContainerMapByCLUID.end(); ++it )
              {
              omSenders[ it->second->SenderGet() ] = true;
              }
            for ( CMapString2Bool::iterator itSenders = omSenders.begin(); itSenders != omSenders.end(); ++itSenders )
              {
              *poSocket << itSenders->first << "\n";
              }
            break;
          } // switch ( itQuery->first[2] )
        break; // switch ( itQuery->first[0] ) => case 'l'

      case 'c': // call operations
        CContainer*          poc;
        CContainer::iterator itc;
        switch ( itQuery->first[2] )
          {
          case 'a': // c:a = call all
            if ( g_bVerbose) cout << " * recall all for me\n";
/*
              1) get the recipient from the TLS-session
              2) resend all messages for the recipient
 */
            for ( it = g_oContainerMapByCLUID.begin(); it != g_oContainerMapByCLUID.end(); ++it )
              {
              poc = it->second;

              *poSocket << string(1, CContainer::scn_sender)  << ":" << poc->SenderGet() << "\n";
              *poSocket << it->first                          << "\n";

              for ( itc  = poc->begin(); itc != poc->end(); ++itc )
                {
                *poSocket << *itc << "\n";
                } // for ( CContainer::iterator itc  = poc->begin();
              } // for ( it = g_oContainerMapByCLUID.begin(); ...
            break; // case 'a': // c:a = call all

          case 'm': // c:m = call message
            if ( g_bVerbose) cout << " * recall message '" << itQuery->second << "'\n";
/*
              1) get the recipient from the TLS-session
              2) resend the queried message if its for the recipient
              3) resend the next message-id from the same sender (if any)
 */
            it = g_oContainerMapByCLUID.find( itQuery->second );
            if ( it != g_oContainerMapByCLUID.end() )
              {
              poc = it->second;

              *poSocket << string(1, CContainer::scn_sender)  << ":" << poc->SenderGet() << "\n";
              *poSocket << it->first                          << "\n";

              for ( itc  = poc->begin(); itc != poc->end(); ++itc )
                {
                *poSocket << *itc << "\n";
                } // for ( CContainer::iterator itc  = poc->begin();

              // the next message id (has to be filtered for receiver later)
              if ( ++it != g_oContainerMapByCLUID.end() )
                {
                *poSocket << string(1, CContainer::scn_messageid_next) << ":" << it->first << "\n";
                } // if ( ++it != g_oContainerMapByCLUID.end() )
              } // if ( it != g_oContainerMapByCLUID.end() )
            break; // case 'm': // c:m = call message

          case 's': // c:s = call message by sender
            if ( g_bVerbose) cout << " * recall message '" << itQuery->second << "' by sender\n";
/*
              1) get the sender from the message-id
              2) resend the queried message
              3) resend the next message-id from the same sender (if any)
 */
            string sMsgId = itQuery->second;
            size_t nCut   = sMsgId.find( ":" );
            if ( nCut != string::npos )
              {
              string sSender = sMsgId.substr(0, nCut);
              if ( g_bVerbose) cout << " * from sender '" << sSender << "'\n";

              for ( it = g_oContainerMapByCLUID.find( itQuery->second ); it != g_oContainerMapByCLUID.end(); ++it )
                {
                poc = it->second;
                if ( poc->SenderGet() != sSender ) continue;

                *poSocket << string(1, CContainer::scn_sender)  << ":" << poc->SenderGet() << "\n";
                *poSocket << it->first                          << "\n";

                for ( itc  = poc->begin(); itc != poc->end(); ++itc )
                  {
                  *poSocket << *itc << "\n";
                  } // for ( CContainer::iterator itc  = poc->begin();
                } // for ( it = g_oContainerMapByCLUID.begin(); ...
              } // if ( nCut != string:npos )
            break; // case 's': // c:s = call message by sender

          } // switch ( itQuery->first[2] )
        break; // switch ( itQuery->first[0] ) => case 'c'
      } // switch ( itQuery->first[0] )
    } //   for ( CMapString2String::iterator itQuery  = moQuery.begin();

  return 0;
  } // size_t CPartner::Recall( ... )


// Seems we were given a new message (or more) so we try to pack them into
// containers, seal them and keep them warm and healthy until some one calls to
// erase them or another reason makes them die
size_t CPartner::BuildContainers( const string& rsClientData )
  {
  /// a list to collect containers
  typedef list<CContainer*> CContainerList;

  CContainerList oContainerList;  // temporary list to collect what we got
  CContainer*    poContainer = 0; // a potential container for if we get someting
  // splits the lines on their unixconform line ending seperator '\n'
  for (string::size_type p1=0, p2=0;
       string::npos != ( p2 = rsClientData.find('\n', p1) );
       p1 = p2+1)
    {
    string s = rsClientData.substr(p1, p2-p1);
    if ( g_bVerbose ) cout << s << endl;
    // is it a sender-id?

    if ( (s[1] == ':') &&
         (s[0] == CContainer::scn_sender) )
      {
      // if so this means a new message is starting
      poContainer = new CContainer;
      oContainerList.push_back( poContainer );
      }
    if ( poContainer )
      {
      *poContainer += s;
      } // if ( poContainer )
    } // for ( string::size_type p1=0, p2=0; ...

  pthread_mutex_lock( &m_tMutex );
  for ( CContainerList::iterator it  = oContainerList.begin();
                                 it != oContainerList.end();
                               ++it)
    {
    string sKey = MakeMessageKey(*it);
    CContainerMapByCLUID::iterator itf = g_oContainerMapByCLUID.find( sKey );
    if ( itf == g_oContainerMapByCLUID.end() )
      {
      if ( g_bVerbose ) cout << "appending: " << sKey << endl;
      *m_poSocket << sKey << "\n";
      }
    else
      {
      if ( g_bVerbose ) cout << "replacing: " << sKey << endl;
      delete itf->second;
      itf->second = 0;
      g_oContainerMapByCLUID.erase( itf );
      }
    g_oContainerMapByCLUID[ sKey ] = *it;
/*
    if ( g_bVerbose )
      {
      for ( CListString::iterator its=(*it)->begin(); its != (*it)->end(); ++its )
        {
        cout << " *** " << *its << endl;
        }
      } // if ( g_bVerbose )
 */
    } // for ( CContainerList::iterator it  = oContainerList.begin(); ...
  pthread_mutex_unlock( &m_tMutex );

  if ( g_bVerbose ) cout << oContainerList.size() << " elements received ";
  if ( g_bVerbose ) cout << g_oContainerMapByCLUID.size() << " elements total here." << endl;
  /* 
   for ( CContainerMapByCLUID::iterator it = g_oContainerMapByCLUID.begin(); 
   it != g_oContainerMapByCLUID.end(); 
   ++it )
   {
   *m_poSocket << it->second->MessageId() << "\n";
   }
   */
  return g_oContainerMapByCLUID.size();
  } // size_t CPartner::BuildContainers( ... )
