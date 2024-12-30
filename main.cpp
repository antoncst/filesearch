
#include <memory>

#include "trees.h"
#include "console_in_out.h"
#include "to_lower_case.h"
#include "configfile.h"

//#include "trees.cpp"

int main()
{
    std::vector< std::string > dirs ;
    std::vector< std::string > skip_dirs ;

    //open config file and read dirictories:
    try
    {
        config_file conf ;
        dirs = conf.get_dirs() ;
        skip_dirs = conf.get_skip_dirs() ;
    }
    catch ( std::string & s )
    {
        std::cerr << s ;
        return 1 ;
    }
    //

    ttreesI * trees = ttreesI::create() ;

    trees->set_dirs( dirs , skip_dirs ) ;
    trees->make_tree() ;

    console_in_out cnsl ;
    to_lower_case & to_lower = to_lower_case::init() ;
    std::string str_to_find ;
    
    // enter strings for search until an empty string is entered (or enter the end of the file Ctrl-Z Ctrl-D )
    while ( 1 )  // вводим строки, пока не будет пустой строки (или ввести конец файла Ctrl-Z Ctrl-D : тоже введётся пустая строка )
    {
        std::cerr << "Enter the search string: " ; // cerr - to make this message visible in console, when redirecting output to a file
        
        // input string from the console
        str_to_find = cnsl.input_string() ;
        if ( str_to_find == "" )
            break ;
        // convert to lowercase
        str_to_find = to_lower( str_to_find ) ;
        // ootpit string to the console
        cnsl.out_string( "str: " + str_to_find) ;

        // find string

        std::vector<tfile * > res_vec =  trees->find_str( str_to_find ) ; 

        // --------------------

        // output results
        for ( tfile * tfile : res_vec )
            cnsl.out_string( tfile->get_full_name() ) ;
    }



    delete trees ;

    return 0 ;
}
