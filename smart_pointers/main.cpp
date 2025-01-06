
#include "tfile.h"
#include "../console_in_out.h"
#include "../to_lower_case.h"
#include "../configfile.h"

int main( )
{
    //std::ios_base::sync_with_stdio(false);

    std::vector< std::string > dirs ;
    std::vector< std::string > skip_dirs ;

    console_in_out cnsl ;


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

 /*
    std::string str_root_dir = "" ;
    std::string str_root_dir2 = "" ;
    #ifdef WINNT
        str_root_dir = "c:/Users/Admin/YandexDisk" ;
    #else //linux
        str_root_dir =  "/home/anton/1" ; // 2019/07MPI
        str_root_dir2 = "/home/anton/d" ; 
    #endif
*/

    // -------- MAIN1 -----
    ttree tree ;

    for ( std::string & str_root_dir : dirs )
    {
        ptfile root_dir = tree.add_tree(  str_root_dir ) ;

        const std::filesystem::path fs_rootdir{ str_root_dir } ;
        fs_readdir_recursive( fs_rootdir , tree , root_dir , skip_dirs ) ;
    }

    /*
    ptfile root_dir = tree.add_tree(  str_root_dir ) ;
    ptfile root_dir2 = tree.add_tree(  str_root_dir2 ) ;

    // //ptfile root_dir = std::make_shared< tfile > ( str_root_dir , nullptr , true ) ;
    
    const std::filesystem::path fs_rootdir{ str_root_dir } ;
    fs_readdir_recursive( fs_rootdir , tree , root_dir ) ;

    const std::filesystem::path fs_rootdir2{ str_root_dir2 } ;
    fs_readdir_recursive( fs_rootdir2 , tree , root_dir2 ) ;
*/
    // --------------------

    //cout:
    //traverse_tree( root_dir , [] (ptfile & pfile ) { std::cout << pfile->get_full_name() << "\n" ; }) ;
    //std::cout << sizeof( root_dir.mp_files ) << "\n" ;


    std::string str_to_find ;

    while ( 1 )  // вводим строки, пока не будет пустой строки (или ввести конец файла Ctrl-Z Ctrl-D : тоже введётся пустая строка )
    {
        std::cerr << "Enter the search string: " ; // cerr - to make this message visible in console, when redirecting output to a file
        //cnsl.out_string( "Enter the search string: " ) ;
        //str_to_find = "Daewo" ; //".cpp ex 05-0|03"
        //str_to_find = "qt !dll" ;
        str_to_find = cnsl.input_string() ;
        if ( str_to_find == "" )
            break ;

        to_lower_case & to_lower = to_lower_case::init() ;
        str_to_find = to_lower( str_to_find ) ;
        cnsl.out_string( "str: " + str_to_find) ;

        // -------- MAIN2 -----

        std::vector<ptfile> res_vec = find_str( tree , str_to_find ) ; 

        // --------------------

        for ( ptfile & tfile : res_vec )
            cnsl.out_string( tfile->get_full_name() ) ;
            //std::cout << tfile->get_full_name() << "\n" ;
    }

    return 0 ;
}
