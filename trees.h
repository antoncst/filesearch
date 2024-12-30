#ifndef TREES_H
#define TREES_H

#include <string>
#include <vector>


// ttrees interface :

class tfile ;

struct ttreesI
{
    //make implementation of the interface
    static ttreesI * create() ;

    // set dirictories 
    // vs_dirs : root dirictories of the trees
    // vs_skip_dirs : 
    // vs_ - vector of strings
    virtual void set_dirs( std::vector< std::string> & vs_dirs , 
                           std::vector< std::string> & vs_skip_dirs  ) = 0 ; // vs - vector of strings

    // recursive making of the trees
    virtual void make_tree() = 0 ;

    // find string in the trees
    // str : string to find
    // return value : vector of pointers to file structs that contain the given string
    virtual std::vector< tfile * > find_str( std::string & str ) = 0 ;

    virtual ~ttreesI() = default ;

} ;


class tfile
{
    // file or dirictory name
    std::string m_name ;

    tfile * m_parent_dir = nullptr;
  
  public:
    //explicit tfile( std::string & name ) : m_name( name ) {} ;
    //tfile() {} ;
    
    // setters :
    void set_name( const std::string & name ) 
    { m_name = name ; }

    void set_parent_dir( tfile * pdir )
    { m_parent_dir = pdir ; }
    
    // getters :
    const std::string & get_name() const
    { return m_name ; }

    const tfile * get_parent_dir() const
    { return m_parent_dir ; } 
    //

    // get the full name beginning from the root dir
    virtual const std::string get_full_name() const ;
} ;


#endif // TREES_H

