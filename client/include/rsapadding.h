/***************************************************************************
 rsapadding.h
 -----------------------
 begin                 : Sat Dec 11 2010
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


#ifndef _RSA_PADDING_H
#define _RSA_PADDING_H


#include <openssl/rsa.h>


// flen must be
// less than RSA_size(rsa) - 11 for RSA_PKCS1_PADDING or RSA_SSLV23_PADDING,
// less than RSA_size(rsa) - 41 for RSA_PKCS1_OAEP_PADDING and
//   exactly RSA_size(rsa)      for RSA_NO_PADDING.
class CRsaPadding
  {
  protected:
    int m_nPadding;

  public:
    CRsaPadding( int nPadding = RSA_PKCS1_OAEP_PADDING )
      : m_nPadding(nPadding)
      {}

    int PaddingGet()
      {
      return m_nPadding;
      }

    // full size of the RSA thing
    int RsaEncSizeGet( const RSA* pRsa )
      {
      return ::RSA_size( pRsa );
      }

    // by padding reduced size of the RSA thing
    int RsaPadSizeGet( const RSA* pRsa )
      {
      int nRsaSize = RsaEncSizeGet( pRsa );
      switch ( m_nPadding )
        {
        case RSA_PKCS1_PADDING:
        case RSA_SSLV23_PADDING:
          return nRsaSize -12;
        case RSA_PKCS1_OAEP_PADDING:
          return nRsaSize -42;
        default:
          return nRsaSize;
        } // switch ( m_nPadding )
      } // int MaxSizeGet( const RSA* pRsa )

    long ResultSizeGet( int nSize, int nRsaSize )
      {
      int nRest = ( nSize % nRsaSize > 0) ? 1 : 0;
      return ( nSize/nRsaSize + nRest ) * nRsaSize;
      }
  }; // class CRsaPadding

#endif // _RSA_PADDING_H
