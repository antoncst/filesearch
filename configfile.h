#ifndef CONFIGFILE_H
#define CONFIGFILE_H

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

class config_file
{
    const std::string config_filename = "config.txt" ; // /usr/bin/config.txt

    const std::string err1 = "Cannot open config.txt\n" ;
    const std::string err2 = "No dirs in  config.txt\n" ;
    const std::string info = "config.txt (utf-8 encoding) example: \n"
                             "dir: c:/Users/Admin \n"
                             "dir:d:\\Example \n"
                             "skip:c:\\windows\n"
                             "#comment\n";
    std::ifstream ifs_ ;
    std::vector< std::string > dirs_ ;
    std::vector< std::string > skip_dirs_ ;
public:
    config_file()
    {
        ifs_.open( config_filename ) ;
        if ( ! ifs_ )
            throw ( err1 + info ) ;
        parse() ;
    }

    std::vector< std::string > get_dirs()
    { return dirs_ ; } 

    std::vector< std::string > get_skip_dirs()
    { return skip_dirs_ ; } 

private:
    void parse()
    {
        std::string line ;

        auto skip_spaces {
            [] ( std::string & line ) {
                auto p = std::find_if( line.begin(), line.end() , []( char c ) { return c!='\t' && c!=' ' ; } ) ;
                line = line.substr( p - line.begin() ) ;
            }
        } ;

        while ( std::getline( ifs_ , line  ) )
        {
            //skipping spaces:
            skip_spaces( line ) ;
            //check comment
            if ( line[0] == '#' || line.substr(0,2) == "//" )
                continue ;
            if ( line.substr(0,4) == "dir:" )
            {
                line = line.substr(4) ;
                //skipping spaces:
                skip_spaces( line ) ;

                dirs_.push_back( line ) ;
            }
            else if ( line.substr(0,5) == "skip:" )
            {
                line = line.substr(5) ;
                //skipping spaces:
                skip_spaces( line ) ;

                skip_dirs_.push_back( line ) ;
            }
        }
        if ( dirs_.size() == 0 )
            throw ( err2 + info ) ;
    }

public:
    ~config_file()
    {
        ifs_.close() ;
    }

};



#endif // CONFIGFILE_H
