//#ifndef TREES_IMPL_H
//#define TREES_IMPL_H

#include <string>
#include <vector>
#include <filesystem>
#include <iostream>
#include <cassert>

#include "trees.h"
#include "to_lower_case.h"

// This implementation stores all files and dirs in a vector.
// Parent index is index of vector.

class tfile
{
    // file or dirictory name
    std::string m_name ;

    int m_parent_dir ;
  
  public:
    tfile( const std::string & name , int parent_index ) :
      m_name( name ) , m_parent_dir( parent_index ) 
      {} ;
    
    // setters :
    void set_name( const std::string & name ) 
    { m_name = name ; }

    void set_parent_dir( int parent_dir )
    { m_parent_dir = parent_dir ; }
    
    // getters :
    const std::string & get_name() const
    { return m_name ; }

    int get_parent_dir() const
    { return m_parent_dir ; } 
    //

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
private:

  std::vector< tfile> m_files ;
  std::vector< std::string > m_vs_dirs ;
  std::vector< std::string > m_vs_skip_dirs ;

public:

  void add_file( const std::string & file , int parent_dir )
  { 
    m_files.emplace_back( file , parent_dir ) ; 
  }

  // getters :
  const std::vector< tfile > & get_files() const
  { return m_files ; }

  std::vector< tfile > & get_files() 
  { return m_files ; }
  //


  virtual void set_dirs( const std::vector< std::string> & vs_dirs ,
                         const std::vector< std::string> & vs_skip_dirs  ) override // vs - vector of strings
  { 
    m_vs_dirs = vs_dirs ; 
    m_vs_skip_dirs = vs_skip_dirs ;
  }

  // get the full name beginning from the root dir
  virtual const std::string get_full_name( const tfile * file ) const override
  {
      std::string full_name = file->get_name() ;
      int index = file->get_parent_dir() ;

      //const tfile * cur_dir = m_files[ m_parent_dir ] ;
      while ( index != -1 )
      {
        //std::string str =   ;
        full_name = m_files[ index ].get_name() + "/" + full_name ;
        index = m_files[ index ].get_parent_dir() ;
      } ;

      return full_name ;
  } ;


  virtual std::vector< const tfile *  > find_str( const std::string & str ) override
  { 
    to_lower_case & to_lower = to_lower_case::init() ;

    std::vector< const tfile *  > res_vec ;

    //prepare search query:
    std::vector< std::string > substrings_and2 = split_str( str , ' ' ) ;
    //    AND          OR
    std::vector< std::vector< std::string > > substrings_and_or ;
    for ( std::string & line : substrings_and2 )
        substrings_and_or.push_back( split_str( line , '|' ) ) ;
    //

        for ( const tfile & file : m_files )
        {
            bool found = true ;
            for ( auto & substrings_and : substrings_and_or )
            {   bool found_or = false ;
                for ( std::string & str_or : substrings_and )
                {
                    if ( str_or[0] == '!' ) // negation
                    {
                        std::string str_neg = str_or.substr( 1 ) ;
                        size_t pos = to_lower( get_full_name( &file ) ).find( str_neg ) ;
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
                if ( !found ) break ;
            }
            if ( found )
            {
                res_vec.push_back( & file ) ;
            }

        }  ;
    return res_vec ;
  } 

  void fs_readdir_recursive( int parent_index ,  const std::filesystem::path path , std::vector< std::string > & skip_dirs )
  {
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
                  add_file( name , parent_index ) ; 
                  fs_readdir_recursive( m_files.size() - 1 , dir_entry ,  skip_dirs ) ;
              }
              else
              {
                  // по хорошему, сперва надо было проверить err_code , но у нас в любом случае, определили мы ,
                  // что это директория или нет, имя файла добавим 

                  add_file( name , parent_index ) ; 
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
      return  ;
  }


  virtual void make_tree() 
  { 
    for ( std::string & str_root_dir : m_vs_dirs )
    {
      const std::filesystem::path fs_rootdir{ str_root_dir } ;
      add_file( str_root_dir , -1 ) ;
      fs_readdir_recursive( m_files.size() - 1 , fs_rootdir , m_vs_skip_dirs ) ;
    }
  }

} ;

ttreesI * ttreesI::create() 
{ return new ttrees ; }


//#endif // TREES_IMPL_H

