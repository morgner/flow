#ifndef _ENVIRONMENT_H
#define _ENVIRONMENT_H

/***************************************************************************
 environment.h
 -----------------------
 begin                 : Sat Nov 06 2010
 copyright             : Copyright (C) 2010 by Manfred Morgner
 email                 : manfred@morgner.com
 ***************************************************************************/

/***************************************************************************

                    ==============================
                    Interpretation Callback Sample
                    ==============================

bool OptionCallback(       char         c,        // short option (letter)
                    std::string const & rsName,   // long option (word)
                    std::string const & rsData,   // parameter data
                    CEnvironment&       roThis)   // the calling environment
  {
  switch ( c )
    {
    case 'c':
      roThis[rsName] = "No5lives";
      return true;
    }
  return false;
  }

CEnvironment oEnvironment( argc, argv );
oEnvironment.OptionCallbackSet( OptionCallback );

***************************************************************************/

#if _MSC_VER > 1000
#pragma once
#pragma warning (disable:4786)
#endif // _MSC_VER > 1000

#include <iostream>

#include <string>
#include <vector>
#include <map>
#include <tuple>

#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>   // EOF

using namespace std::string_literals;

typedef std::tuple<int, std::string, std::string> TOption;
TOption t(3, "abc"s, "ccc"s);

/*
int         o;
std::string a;
std::string c;
std::tie (o, a, c) = t;
std::tie (std::ignore, a, c) = t;
std::get<0>(t) = 1;
a = std::get<1>(t);
*/
class CEnvironment : public std::map<std::string, std::string>
  {
  public:
    typedef bool (*TOptionCallback)(        char         c, 
                                     std::string const & rsName,
                                     std::string const & rsData,
                                     CEnvironment&      roThis);

    class COptions : public std::vector<option>
      {
      protected:
        CEnvironment* m_poEnvironment{nullptr};
        option const  m_tNullOption ={nullptr,0,nullptr,0};

        std::vector<std::string> m_vsHelp;
        std::vector<std::string> m_vsDefault;
        std::string              m_sShortOpt;

        size_t m_nLenMaxLongOpt{0};
        size_t m_nLenMaxHelp   {0};
        size_t m_nLenMaxDefault{0};

      public:
        COptions()
          {
          push_back( m_tNullOption );
          } // COptions()

        ~COptions()
          {
          for ( iterator it=begin(); it != --end(); ++it )
            {
            delete it->name;
            } // for ( iterator it=begin(); ...
          } // ~COptions()

        CEnvironment* EnvironmentSet( CEnvironment* poEnvironment )
          {
          return m_poEnvironment = poEnvironment;
          } // CEnvironment* EnvironmentSet( CEnvironment* poEnvironment )

        void Append (     option const & rtOption,
                     std::string const & rsHelp,
                     std::string const & rsDefault )
          {
          back() = rtOption;
          push_back( m_tNullOption );

          m_vsHelp.push_back(rsHelp);
          if ( (rsDefault.length() > 0) && (rsDefault[0] == '$') && (rsDefault.substr(1).c_str()) )
            {
            m_vsDefault.push_back( getenv(rsDefault.substr(1).c_str()) );
            }
          else // if ( (rsDefault.length() > 0) && (rsD ...
            {
            m_vsDefault.push_back(rsDefault);
            } // if ( (rsDefault.length() > 0) && (rsD ...
          size_t n;
          m_nLenMaxLongOpt = ( (n = strlen(rtOption.name)       ) > m_nLenMaxLongOpt ) ? n : m_nLenMaxLongOpt;
          m_nLenMaxHelp    = ( (n = rsHelp.length()             ) > m_nLenMaxHelp    ) ? n : m_nLenMaxHelp;
          m_nLenMaxDefault = ( (n = m_vsDefault.back().length() ) > m_nLenMaxDefault ) ? n : m_nLenMaxDefault;

          m_sShortOpt += (char) rtOption.val;
          if ( rtOption.has_arg )
            {
            m_sShortOpt += ":";
            } // if ( rtOption.has_arg )

          if ( m_poEnvironment && m_vsDefault.back().length() )
            {
            m_poEnvironment->OptionSet( rtOption.val, rtOption.name, m_vsDefault.back() );
            } // if ( m_poEnvironment && m_vsD ...
          } // void Append (option const & rtOption, ...

        operator option* () { return &this->front(); }
        operator option& () { return  this->front(); }

        std::string const & ShortOptionsGet() { return m_sShortOpt; }

        bool hasShortOption( int const c )
          {
          return ShortOptionsGet().find( (char)c ) != std::string::npos;
          }
        
        std::string LongOptionByShortOption( int const c )
          {
          static std::string const sUnknown = "unknown";
          for ( iterator it=begin(); it != end(); ++it )
            {
            if ( it->val == c )
              {
              return it->name;
              }
            }
          return sUnknown;
          }

        void UsagePrint()
          {
          std::string space;
          for ( size_t n = 0; n < m_nLenMaxLongOpt + 8; n++ ) space += " ";
          std::vector<std::string>::iterator itH = m_vsHelp.begin();
          std::vector<std::string>::iterator itD = m_vsDefault.begin();
          for (iterator it = begin();
                        it != --end();
                      ++it, ++itH, ++itD)
            {
            std::cout << "-" << char(it->val) << ", --" << it->name << space.substr( strlen(it->name) + 6 ) << *itH << std::endl;
            if ( itD->length() )
              {
              std::cout << space << "default: " << *itD << std::endl;
              } // if ( itD->length() )
            } // for (iterator it = begin(); ...
          } // void UsagePrint()
      }; // class COptions

    protected:
      COptions        m_oOptions;
      std::string     m_sProgramName;
      int             m_nArgumentCount;
      char**          m_ppcArguments;
      TOptionCallback m_fOptionsCallback;

    public:
      CEnvironment( int argc, char const ** argv )
        : m_nArgumentCount ( argc ),
          m_ppcArguments   ( const_cast<char**>(argv) ),
          m_fOptionsCallback( 0 )
        {
        m_sProgramName = m_ppcArguments[0];
        m_sProgramName = m_sProgramName.substr( m_sProgramName.rfind('/')+1 );

        m_oOptions.EnvironmentSet( this );
        } // CEnvironment( int argc, char const ** argv )

    void OptionAppend ( std::string const &  rsName,
                             int             nHasFlag,
                             int*            pnFlag,
                             int             nVal,
                        std::string const &  rsHelp,
                        std::string const &  rsDefault )
      {
      char* pcName = new char[rsName.length()+1];
      strncpy( pcName, rsName.c_str(), rsName.length() );
      pcName[ rsName.length() ] = 0;
      option tOption = { pcName, nHasFlag, pnFlag, nVal } ;
      m_oOptions.Append( tOption, rsHelp, rsDefault);
      } // void OptionAppend ( std::string const &  rsName,

    void OptionCallbackSet( TOptionCallback fOptionsCallback )
      {
      m_fOptionsCallback = fOptionsCallback;
      } // void OptionCallbackSet( TOptionCallback fOptionsCallback )

    std::string const & ProgramNameGet()
      {
      return m_sProgramName;
      } // std::string const & ProgramNameGet()

    void CommandlineRead ()
      {
      extern char* optarg;
      int          c;
      int          option_index = 0;
      while ( (c = getopt_long ( m_nArgumentCount,
                                 m_ppcArguments,
                                 m_oOptions.ShortOptionsGet().c_str(),
                                 m_oOptions,
                                 &option_index)) != EOF )
        {
        if ( m_oOptions.hasShortOption(c) )
          {
          OptionSet( c, m_oOptions.LongOptionByShortOption(c).c_str(), (optarg) ? optarg : "" );
          } // if ( m_oOptions.hasShortOption(c) )
        } // while ( (c = getopt_long ( m_nArgumentCount, ...
      } // void CommandlineRead ()

    virtual void Usage()
      {
      std::cout << ProgramNameGet() << std::endl;
      std::cout << "Usage: " <<  ProgramNameGet() << " [OPTION]..." << std::endl;
      std::cout << "Options:\n";
      m_oOptions.UsagePrint();
      } // virtual void Usage()

    virtual void UsageExit( int nStatus )
      {
      Usage();
      exit( nStatus );
      } // virtual void UsageExit( int nStatus )

  protected:
    void OptionSet( char c, std::string const & rsName, std::string const & rsData )
      {
      if ( ! ( m_fOptionsCallback != 0 && m_fOptionsCallback(c, rsName, rsData, *this)) )
        {
        switch ( c )
          {
          case 'h':
            UsageExit(0);
          case 'V':
#ifdef VERSION
#ifdef APPLNAME
            std::cout << m_sProgramName << ": " << APPLNAME << " Version " << VERSION << std::endl;
#else // APPLNAME
            std::cout << m_sProgramName << " Version " << VERSION << std::endl;
#endif // APPLNAME
#else // VERSION
            std::cout << m_sProgramName << std::endl;
#endif // VERSION
            std::cout << "Compile Time: " << __DATE__ << " at " << __TIME__ << std::endl;
            exit(0);
/*
          case 'v':
            if ( !rsData.length() )
              {
              (*this)["verbose"] = "1";
              break;
              }
            // if not - advance to "default"
*/
          default:
            if ( rsData.length() )
              {
              (*this)[rsName] = rsData;
              }
            else
              {
              (*this)[rsName] = "1";
              }
            break;
          } // switch ( c )
        } // if ( ! ( m_fOptionsCallback != 0 && m_fOptionsCallback( …
      } // void OptionSet( char c, std::string const & rsName, con...

  }; // class CEnvironment

#endif // _ENVIRONMENT_H
