//#ifndef TREES_IMPL_H
//#define TREES_IMPL_H

#include <string>
#include <vector>
#include <filesystem>
#include <iostream>
#include <cassert>

#include "trees.h"
#include "to_lower_case.h"

// This implementation stores the tree in a vector
// The main disadvantage is the validity of the pointer to parent.
// Thus, we cannot assign or move the tree.  
// Also we cannot delete copy and move constructor from class definition because it is stored in vector.
// it works well, but this is a bad implementation. 
// We can write constructors and assignments that rewrites pointers to parents each times.

class tdir : public tfile
{
  std::vector< tdir > m_dirs ; // the tree
  std::vector< tfile> m_files ;
public:

  void add_file( const tfile && file )
  { 
    m_files.emplace_back( file ) ; 
  }

  void add_dir( tdir & dir )
  { m_dirs.push_back( dir ) ; } 

  // getters :
  const std::vector< tfile > & get_files() const
  { return m_files ; }

  std::vector< tfile > & get_files() 
  { return m_files ; }

  const std::vector< tdir > & get_dirs() const
  { return m_dirs ; }

  std::vector< tdir > & get_dirs() 
  { return m_dirs ; }

  //
  void set_childs()
  {
    for ( tfile & cur_file : get_files() )
      cur_file.set_parent_dir( this ) ;
    for ( tdir & cur_dir : get_dirs() )
      cur_dir.set_parent_dir( this ) ;
  }

} ;

tdir fs_readdir_recursive( const std::filesystem::path path , std::vector< std::string > & skip_dirs )
{
    tdir res_dir ;
    try
    {
        for ( auto const & dir_entry : std::filesystem::directory_iterator{ path } )
        {
            std::error_code err_code ;  // depends on platform (OS)
            // err_code detects "No such file or directory file" unlike try..catch - tested on linux g++
            std::string name = dir_entry.path().filename().string() ;

            if ( dir_entry.is_directory( err_code ) )
            {
              if (dir_entry.is_symlink() ) continue ;
                // skipping dirs
                bool bfound = false ;
                for ( auto & sdir : skip_dirs )
                    if ( dir_entry.path().string() == sdir )
                        { bfound = true ; break ; } 
                if ( bfound )
                    continue ;
                // end of skipping dirs
                tdir dir = fs_readdir_recursive( dir_entry ,  skip_dirs ) ;
                dir.set_name( name ) ;
                res_dir.add_dir( dir ) ;
            }
            else
            {
                // по хорошему, сперва надо было проверить err_code , но у нас в любом случае, определили мы ,
                // что это директория или нет, имя файла добавим 

                tfile file ;
                file.set_name( name ) ;
                res_dir.add_file( std::move(file) ) ;
            }
            if ( err_code.value() != 0 )
                std::cerr << " err_code: " << err_code.value() << " file: " << dir_entry.path() << std::endl ;
        }
    }
    catch ( std::filesystem::filesystem_error const& ex )
    {
        std::cerr << "catch : " << ex.code().value()<< ", " << ex.code().message() << " file: "
                  << path << std::endl  ; // linux 2 - no such file, 13 - permission denied
    }
    return res_dir ;
}

template< typename F >
void traverse_tree( tdir & parent_dir , F func ) 
{
    for ( tdir & dir : parent_dir.get_dirs() )
    {
        traverse_tree( dir , func ) ;
        func( dir ) ;
    }
    for ( tfile & file : parent_dir.get_files() )
        func( file ) ;
} ;

template< typename F >
void traverse_tree_dirs( tdir & parent_dir , F func )
{
    for ( tdir & dir : parent_dir.get_dirs() )
    {
        func( dir ) ;
        traverse_tree_dirs( dir , func ) ;
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


class ttrees : public ttreesI
{
  std::vector< tdir > m_dirs ;
  std::vector< std::string > m_vs_dirs ;
  std::vector< std::string > m_vs_skip_dirs ;
public:
  virtual void set_dirs( std::vector< std::string> & vs_dirs ,
                         std::vector< std::string> & vs_skip_dirs  ) override // vs - vector of strings
  { 
    m_vs_dirs = vs_dirs ; 
    m_vs_skip_dirs = vs_skip_dirs ;
  }


  virtual std::vector< tfile * > find_str( std::string & str ) override
  { 
    to_lower_case & to_lower = to_lower_case::init() ;

    std::vector< tfile * > res_vec ;

    //prepare search query:
    std::vector< std::string > substrings_and2 = split_str( str , ' ' ) ;
    //    AND          OR
    std::vector< std::vector< std::string > > substrings_and_or ;
    for ( std::string & line : substrings_and2 )
        substrings_and_or.push_back( split_str( line , '|' ) ) ;
    //

    for ( tdir & dir : m_dirs ) 
    {
        traverse_tree( dir , [ & ] ( tfile & file)
        {
          assert( file.get_parent_dir() )  ;
            bool found = true ;
            for ( auto & substrings_and : substrings_and_or )
            {   bool found_or = false ;
                for ( std::string & str_or : substrings_and )
                {
                    if ( str_or[0] == '!') // negation
                    {
                        std::string str_neg = str_or.substr( 1 ) ;
                        size_t pos = to_lower( file.get_full_name() ).find( str_neg ) ;
                        if ( pos != std::string::npos ) //found
                        {   found_or = false ; break ; }
                        else
                            found_or = true ;
                    }
                    else
                    {
                        size_t pos = to_lower( file.get_name() ).find( str_or ) ; //
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
                res_vec.push_back( & file ) ;
            }

        } ) ;
    }
    return res_vec ;

    ; 
  } 

  virtual void make_tree() 
  { 
    for ( std::string & str_root_dir : m_vs_dirs )
    {
      const std::filesystem::path fs_rootdir{ str_root_dir } ;
      tdir dir = fs_readdir_recursive( fs_rootdir , m_vs_skip_dirs ) ;
      dir.set_parent_dir( nullptr ) ;
      dir.set_name( str_root_dir ) ;
      m_dirs.push_back( dir ) ;

      //set parents :
      
      assert( m_dirs.size() > 0 ) ;
      m_dirs.back().set_childs() ;
      traverse_tree_dirs( m_dirs.back() , [] ( tdir & dir ) { 
        dir.set_childs() ;
      } ) ;

    }
  }

} ;



const std::string tfile::get_full_name() const
{
    std::string full_name = m_name ;
    const tfile * cur_dir = this->get_parent_dir() ;
    while ( cur_dir != nullptr )
    {
      std::string str =  cur_dir->get_name() ;
      full_name = str + "/" + full_name ;
      cur_dir = cur_dir->get_parent_dir() ;
    } ;

    return full_name ;
} ;

ttreesI * ttreesI::create() 
{ return new ttrees ; }

//#endif // TREES_IMPL_H

