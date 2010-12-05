/***************************************************************************
 cryptoexception.cpp
 -----------------------
 begin                 : Thu Dec 02 2010
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


#include "cryptoexception.h"


CCryptoException::CCryptoException()
  {
  }

CCryptoException::CCryptoException( const std::string& sDescription,
                                   bool         bFatal )
  : m_sDescription ( sDescription ),
    m_bFatal( bFatal )
  {
  }

CCryptoException::~CCryptoException()
  {
  }

const std::string& CCryptoException::Info()
  {
  return m_sDescription;
  }

bool CCryptoException::isFatal()
  {
  return m_bFatal;
  }
