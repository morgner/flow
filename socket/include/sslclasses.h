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


#ifndef _SSL_CLASSES_H
#define _SSL_CLASSES_H

#include <iostream>
#include <openssl/ssl.h>


template<typename T>
  class TSslPointer
    {
    protected:
      T* m_ptT;
        
    public:
       TSslPointer( T* ptT = 0 ) { m_ptT = ptT; }
      ~TSslPointer()             { Delete( m_ptT ); }

      T* operator =  ( T* ptT ) { Delete( m_ptT ); return m_ptT = ptT; }
      T& operator *  ()         { return *m_ptT; }
      T* operator -> ()         { return m_ptT; }
         operator T* ()         { return m_ptT; }
         operator T**()         { return &m_ptT; }
        
             bool isValid() { return m_ptT != 0; }
      static void Delete( T*& rpt);
    }; // template<typename T> class TSslPointer

template<typename T>
  void TSslPointer<T>::Delete( T*& rpt )
    {
    delete rpt;
    rpt = 0;
    }


typedef TSslPointer< X509 >          CX509;
typedef TSslPointer< RSA >           CRsa;
typedef TSslPointer< BIO >           CBio;
typedef TSslPointer< unsigned char > CUChar;
typedef TSslPointer< EVP_PKEY >      CEvpPkey;

template<> void CX509::Delete( X509*& rpt );
template<> void CRsa::Delete( RSA*& rpt );
template<> void CBio::Delete( BIO*& rpt );
template<> void CEvpPkey::Delete ( EVP_PKEY*& rpt );

#endif // _SSL_CLASSES_H
