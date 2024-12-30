#ifndef TO_LOWER_CASE_H
#define TO_LOWER_CASE_H


#include <iostream>
#include <codecvt>
#include <locale>


//#include "text.h"

class to_lower_case
{
public:

    static to_lower_case & init()
    {
        static to_lower_case instance ;
        return instance ;
    }

    std::string operator()( const std::string & str)
    {
        std::wstring wstr = utf8ToUtf16( str ) ;

        for ( auto & ch : wstr )
        {
            if ( ch >= L'A' && ch <= L'Z' )  // latin
                ch += 0x20 ;
            else if ( ch >= L'А' && ch <= L'Я' )  // cyrillic
                ch += 0x20 ;
            else if ( ch == L'Ё' )
                ch = L'ё' ;

            // non-strict matching :
            if ( ch == L'ё' )
                ch = L'е' ;
            else if ( ch == L'y' || ch == L'j' )
                ch = L'i' ;
            else if ( ch == L'c' )
                ch = L'k' ;

        }


        std::string result = utf16ToUtf8( wstr ) ;
        return result ;
    }

private:
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> m_conv;

    std::wstring utf8ToUtf16(const std::string & utf8Str)
    {
        return m_conv.from_bytes(utf8Str);
    }

    std::string utf16ToUtf8(const std::wstring & utf16Str)
    {
        return m_conv.to_bytes(utf16Str);
    }

    // for singletone:
    ~to_lower_case() {} ;
    to_lower_case() {} ;

    //to_lower_case() ;
    to_lower_case( to_lower_case const & ) = delete ;

} ;


#endif // TO_LOWER_CASE_H
