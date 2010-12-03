/***************************************************************************
 sslclasses.h description
 -----------------------
 begin                 : Sun 21 11 2010
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


#ifndef _SSL_TEMPLATES_H
#define _SSL_TEMPLATES_H

// A template to ensure automatic freeing of memory resources if the
// object runs out of context. In case of SSL it's tricky to use
// because no one knows which allocations will be deleted cascading
// down the object linkage tree

// This template does not has the option to prevent autodeletion
// because the code should be meaningfull for readers. So if you find
// a declaration like this:
//
//   CRsa m_oRsa;
//
// this object will autodelete if allocated. On the other hand:
//
//   RSA* m_pRsa;
//
// will not autodelete but is expected to be deleted by other means.
// I call this 'side along deletion'.
//
// But BOTH object types are able to receive reallocations like this:
//
//   ::PEM_read_bio_RSAPublicKey(oBio,  m_oRsa, NULL, NULL)
//
// or this:
//
//   ::PEM_read_bio_RSAPublicKey(oBio, &m_pRsa, NULL, NULL)
//
// Where the second parameter has to be 'RSA**'. The template converts
// automatically to pointer to pointer
//
// Using CRsa oder RSA* visibly demonstrates your expectation if the
// element will be 'automatically deleted' or 'side along deleted'

// Before Delete() calls we don't need to check if the pointer is vaild,
// meaning 'not null', because the delete/free operations care about it
// inherently

#include <iostream>
#include <openssl/ssl.h>

template<typename T>
  class TSslPointer
    {
    protected:
      T*   m_ptT;
        
    public:
       TSslPointer( T* ptT = 0 ) {         m_ptT = ptT; }
      ~TSslPointer()             { Delete( m_ptT ); }

      T* operator =  ( T* ptT )  { Delete( m_ptT ); return m_ptT = ptT; }
      T& operator *  ()          { return *m_ptT; }
      T* operator -> ()          { return  m_ptT; }
         operator T* ()          { return  m_ptT; }
         operator T**()          { return &m_ptT; }

             bool isValid() { return m_ptT != 0; }
             void Delete()  { Delete( m_ptT ); m_ptT = 0; }
      static void Delete( T*& rpt );
    }; // template<typename T> class TSslPointer

template<typename T>
  void TSslPointer<T>::Delete( T*& rpt )
    {
    delete rpt;
    rpt = 0;
    }

// The reason for the excess is the fact that in OpenSSL each
// structured element has his own 'free methode'. Which means
// freeing of memory resources has to go a special way for
// every OpenSSL element covered by the upper template. The
// way to do this is to add
//
//   1. a type for the class to cover the element
//   2. a forward declaration of the specific Delete() methode
//   3. in .cpp the implementation of this methode

// Phase 1:
// ========
typedef TSslPointer< X509 >          CX509;
typedef TSslPointer< RSA >           CRsa;
typedef TSslPointer< BIO >           CBio;
typedef TSslPointer< unsigned char > CUChar;
typedef TSslPointer< EVP_PKEY >      CEvpPkey;

// Phase 2:
// ========
template<> void CX509::Delete( X509*& rpt );
template<> void CRsa::Delete( RSA*& rpt );
template<> void CBio::Delete( BIO*& rpt );
template<> void CUChar::Delete( unsigned char*& rpt );
template<> void CEvpPkey::Delete( EVP_PKEY*& rpt );

// Phase 3: (in .cpp)
// ==================
// template <>
//   void CX509::Delete( X509*& rpt )
//     {
//     X509_free( rpt );
//     }

#endif // _SSL_TEMPLATES_H
