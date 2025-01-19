#ifndef CONSOLE_IN_OUT_H
#define CONSOLE_IN_OUT_H

#include <iostream>

#ifdef WINNT
#include <io.h>    // for _setmode
#include <fcntl.h> // for _O_U16TEXT
#include <codecvt>
#endif

class console_in_out
{
#ifdef WINNT

    public :
        console_in_out()
        { // console initialization:
            _setmode(_fileno(stdout), _O_U16TEXT);
            _setmode(_fileno(stdin), _O_U16TEXT);
            setlocale(LC_ALL, "");

            // utf8 code page - it turned out to be impractical in Windows
            //if ( !SetConsoleCP(65001) )
            //    std::cout<< " SetConsoleCP error \n ";
            //if ( !SetConsoleOutputCP(65001) )
            //    std::cout<< " SetConsoleOutputCP error \n ";

        }

        void out_string( const std::string & line )
        {
            std::wcout << utf8ToUtf16( line ) << L"\n"  ; //std::endl
        }

        std::string input_string()
        {
            std::wcin.sync() ; // otherwise it is buggy (глючит) - in windows only
            std::string result = "" ;
            std::wstring wstr ;
            std::getline(std::wcin, wstr) ;
            result = utf16ToUtf8( wstr ) ;
            return result ;
        }
    private :
        std::wstring utf8ToUtf16(const std::string& utf8Str)
        {
            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
            return conv.from_bytes(utf8Str);
        }

        std::string utf16ToUtf8(const std::wstring& utf16Str)
        {
            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
            return conv.to_bytes(utf16Str);
        }
#else //linux

public :
    void out_string( const std::string & line )
    {
        std::cout << line << "\n"  ;
    }

    std::string input_string()
    {
        std::string result = "" ;
        std::getline(std::cin, result) ;
        return result ;
    }
#endif
} ;

#endif // CONSOLE_IN_OUT_H
