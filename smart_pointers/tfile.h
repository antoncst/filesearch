#ifndef TFILE_H
#define TFILE_H

#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <memory>
#include <cassert>
#include "../to_lower_case.h"

class tfile ;

using ptfile = std::shared_ptr< tfile > ;

class tfile
{
    std::string m_filename ;
    std::vector< ptfile > mp_files ;
    std::weak_ptr< tfile > mp_parent  ; // mp_parent : p_ means pointer
    bool m_is_dir ;

public:

    explicit tfile( std::string filename , ptfile p_parent = nullptr , bool is_dir = false )
        : m_filename( filename )
        , mp_parent( p_parent )
        , m_is_dir( is_dir )
        { } ;

    bool is_dir() const { return m_is_dir ; }

    const std::vector< ptfile > & get_files() const
    { return mp_files ; }

    const std::string & get_name() const
    { return m_filename ; }

    void add_file ( ptfile pfile )
    {
        mp_files.push_back( std::move( pfile ) ) ;
    }

    const std::string get_full_name() const
    {
        const tfile * pfile = this ;
        std::string full_name = m_filename + ( is_dir() ?  "/" : "" ) ;
        while ( ptfile sptr_parent =  pfile->mp_parent.lock() )
        {
            assert( sptr_parent.get() ) ;
            pfile = sptr_parent.get() ;
            full_name = pfile->m_filename + "/" + full_name ;
        }
        return full_name ;
    }
} ;


class ttree  // maybe make it static or singleton , чтобы не тащить его в качестве параметра в fs_readdir_recursive() например
             // but I don't really like this idea
{
  public:

    ptfile add_tree( std::string str_root_dir )
    {
        ptfile root_dir = std::make_shared< tfile > ( str_root_dir , nullptr , true ) ;
        trees_.push_back( root_dir ) ;
        return root_dir ;
    }

    ptfile add_file( std::string filename , ptfile dir )
    {
                ptfile file = std::make_shared< tfile >( filename , dir ) ;
                dir->add_file( file ) ;
                return file ;
    }

    ptfile add_dir( std::string dirname , ptfile dir )
    {
                ptfile dir1 = std::make_shared< tfile >( dirname , dir , true ) ;
                dir->add_file( dir1 ) ;
                return dir1 ;
    }

    std::vector< ptfile > trees_ ; // holds root dirs of trees

  private:

} ;

//void traverse_tree( ptfile & p_parent_file , void ( * func ) ( ptfile & ))

template< typename F >
void traverse_tree( ptfile & p_parent_file , F func ) // excluding the root dir name  - it is not needed
{
    for ( auto  pfile : p_parent_file->get_files() )
    {
        func( pfile ) ;
        if ( pfile->is_dir() )
            traverse_tree( pfile , func ) ;
    }
} ;

std::vector<std::string> split_str( const std::string & str , const char delim )
{
    std::vector<std::string> vec_str;
    std::istringstream ss( str ) ;
    std::string line ;
    while( std::getline( ss ,  line  , delim ) )
    {
        if ( line == "" ) continue ;
        vec_str.push_back( line ) ;
    }
    return vec_str ;
}

std::vector< ptfile > find_str( ttree & tree , const std::string & str ) // ptfile & dir
{
    std::vector< ptfile > res ;
    for ( ptfile dir : tree.trees_ ) 
    {
        to_lower_case & to_lower = to_lower_case::init() ;

        std::vector< std::string > substrings_and2 = split_str( str , ' ' ) ;
        //    AND          OR
        std::vector< std::vector< std::string > > substrings_and_or ;
        for ( std::string & line : substrings_and2 )
            substrings_and_or.push_back( split_str( line , '|' ) ) ;
        traverse_tree( dir , [ & ] ( ptfile & pfile)
                    {
                        bool found = true ;
                        for ( auto & substrings_and : substrings_and_or )
                        {   bool found_or = false ;
                            for ( std::string & str_or : substrings_and )
                            {
                                if ( str_or[0] == '!') // negation
                                {
                                    std::string str_neg = str_or.substr( 1 ) ;
                                    size_t pos = to_lower( pfile->get_full_name() ).find( str_neg ) ;
                                    if ( pos != std::string::npos ) //found
                                    {   found_or = false ; break ; }
                                    else
                                        found_or = true ;
                                }
                                else
                                {
                                    size_t pos = to_lower( pfile->get_name() ).find( str_or ) ; //
                                    if ( pos != std::string::npos ) //found
                                    {   found_or = true ; break ; }
                                    else
                                        found_or = false ;
                                }
                            }
                            found &= found_or ;
                        }
                        if ( found )
                        {
                            res.push_back( pfile ) ;
                        }

                    } ) ;
    }
    return res ;
}




void fs_readdir_recursive( const std::filesystem::path path , ttree & tree , ptfile dir , std::vector< std::string > & skip_dirs )
{
    try
    {
        for ( auto const & dir_entry : std::filesystem::directory_iterator{ path } )
        {
            std::error_code err_code ;  // depends on platform (OS)
            // err_code detects "No such file or directory file" unlike try..catch - tested on linux g++
            if ( dir_entry.is_directory( err_code ) )
            {
                if (dir_entry.is_symlink() ) continue ;
                //skipping dirs
                bool bfound = false ;
                for ( auto & sdir : skip_dirs )
                    if ( dir_entry.path().string() == sdir )
                        { bfound = true ; break ; } 
                if ( bfound )
                    continue ;

                ptfile dir1 = tree.add_dir( dir_entry.path().filename().string() , dir ) ;

                //ptfile dir1 = std::make_shared< tfile >( dir_entry.path().filename().string() , dir , true ) ;
                //dir->add_file( dir1 ) ;
                fs_readdir_recursive( dir_entry , tree , dir1 , skip_dirs ) ;
            }
            else
            {
                // по хорошему, сперва надо было проверить err_code , но у нас в любом случае, определили мы ,
                // что это директория или нет, имя файла добавим 

                tree.add_file( dir_entry.path().filename().string() , dir ) ;

                //ptfile file = std::make_shared< tfile >( dir_entry.path().filename().string() , dir ) ;
                //dir->add_file( file ) ;
            }
            /*            catch ( std::filesystem::filesystem_error const& ex )
                {
                    std::cerr << "err : " << ex.code().value() << std::endl ;
                } */
            if ( err_code.value() != 0 )
                std::cerr << " err_code: " << err_code.value() << " file: " << dir_entry.path() << std::endl ;
        }
    }
    catch ( std::filesystem::filesystem_error const& ex )
    {
        std::cerr << "catch : " << ex.code().value()<< ", " << ex.code().message() << " file: "
                  << path << std::endl  ; // linux 2 - no such file, 13 - permission denied
    }
}



#endif // TFILE_H
