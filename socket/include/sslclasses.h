/***************************************************************************
 sslclasses.h description
 -----------------------
 begin                 : Sun 22 11 2010
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

#include <openssl/ssl.h>
template<typename T>
  class TSslPointer
    {
    protected:
      T* m_ptElement;

    public:
      TSslPointer()         { m_ptElement = 0;  }
      TSslPointer( T* ptT ) { m_ptElement = ptT; }

      operator T*()  { return  m_ptElement; }
      operator T**() { return &m_ptElement; }

      bool isValid() { return m_ptElement != 0; }

    }; // template<typename T> class TSslClass

class CX509 : public TSslPointer< X509 >
  {
  public:
     CX509() { m_ptElement = X509_new(); }
     CX509( X509* ptX509 )  { m_ptElement = ptX509; }
    ~CX509() { X509_free( m_ptElement ); }
  }; // class CX509

class CBio : public TSslPointer< BIO >
  {
  public:
     CBio( BIO* ptBio )  { m_ptElement = ptBio; }
    ~CBio() { BIO_free( m_ptElement ); }
  }; // class CCBio

class CUChar : public TSslPointer< unsigned char >
  {
  public:
     CUChar() { m_ptElement = 0; }
     CUChar( unsigned char* ptUChar )  { m_ptElement = ptUChar; }
    ~CUChar() { free( m_ptElement ); }
  }; // class CUChar

#endif // _SSL_CLASSES_H
