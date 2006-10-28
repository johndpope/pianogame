// Piano Hero
// Copyright (c)2006 Nicholas Piegdon
// See license.txt for license information

#ifndef __STRING_UTIL_H
#define __STRING_UTIL_H

// Handy string macros

#ifndef STRING
#include <sstream>
#define STRING(v) ((static_cast<std::ostringstream&>(std::ostringstream().flush() << v)).str())
#endif

#ifndef WSTRING
#include <sstream>
#define WSTRING(v) ((static_cast<std::wostringstream&>(std::wostringstream().flush() << v)).str())
#endif

#include <string>
#include <vector>
#include <locale>
#include <algorithm>
#include <functional>
#include <iostream>


// Apparently "widen" is deprecated
#pragma warning(disable : 4996)

// string_type here can be things like std::string or std::wstring
template<class string_type>
const string_type StringLower(string_type s)
{
   std::locale loc;

   std::transform( s.begin(), s.end(), s.begin(),
      std::bind1st( std::mem_fun( &std::ctype<string_type::value_type>::tolower ),
      &std::use_facet< std::ctype<string_type::value_type> >( loc ) ) );

   return s;
}

// E here is usually wchar_t
template<class E, class T = std::char_traits<E>, class A = std::allocator<E> >
class Widen : public std::unary_function< const std::string&, std::basic_string<E, T, A> >
{
public:
   Widen(const std::locale& loc = std::locale()) : loc_(loc)
   {
      #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6.0...
         using namespace std;
         pCType_ = &_USE(loc, ctype<E> );
      #else
         pCType_ = &std::use_facet<std::ctype<E> >(loc);
      #endif
   }

   std::basic_string<E, T, A> operator() (const std::string& str) const
   {
      typename std::basic_string<E, T, A>::size_type srcLen =
         str.length();
      const char* pSrcBeg = str.c_str();
      std::vector<E> tmp(srcLen);

      pCType_->widen(pSrcBeg, pSrcBeg + srcLen, &tmp[0]);
      return std::basic_string<E, T, A>(&tmp[0], srcLen);
   }

private:
   std::locale loc_;
   const std::ctype<E>* pCType_;

   // No copy-constructor or no assignment operator
   Widen(const Widen&);
   Widen& operator= (const Widen&);
};



#endif
