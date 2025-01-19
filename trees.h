#ifndef TREES_H
#define TREES_H

#include <string>
#include <vector>

class tfile ;

// ttrees interface :

struct ttreesI
{
    //make implementation of the interface
    static ttreesI * create() ;

    // set dirictories 
    // vs_dirs : root dirictories of the trees
    // vs_skip_dirs : do not search in these directories
    // vs_ - vector of strings
    virtual void set_dirs( const std::vector< std::string> & vs_dirs , 
                           const std::vector< std::string> & vs_skip_dirs  ) = 0 ; 

    // making of the trees
    virtual void make_tree() = 0 ;

    // find string in the trees
    // str : string to find
    // return value : vector of pointers to file structs that contain the given string
    virtual std::vector< const tfile * > find_str( const std::string & str ) = 0 ;

    virtual const std::string get_full_name( const tfile * file ) const = 0 ;

    virtual ~ttreesI() = default ;

} ;


#endif // TREES_H

