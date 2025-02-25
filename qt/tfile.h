#ifndef TFILE_H
#define TFILE_H

#include <vector>
#include <unordered_map>
//#include <filesystem>
#include <memory>
//#include <QDebug>
#include <QString>
#include <QStringList>
#include <QDir>

constexpr size_t max_find_count = 3000 ;

class tfile
{
    QString m_filename ;
    //std::vector< ptfile > mp_files ;
    int m_parent_dir ;
    bool m_is_dir ;

public:

    explicit tfile( const QString & filename , int parent_index = -1 , bool is_dir = false )
        : m_filename( filename )
        , m_parent_dir( parent_index )
        , m_is_dir( is_dir )
        { }

    bool is_dir() const { return m_is_dir ; }

    //const std::vector< ptfile > & get_files() const
    //{ return mp_files ; }

    const QString & get_name() const
    { return m_filename ; }

    int get_parent_dir() const
    { return m_parent_dir ; }

    //void add_file ( ptfile pfile )
    //{
    //    mp_files.push_back( std::move( pfile ) ) ;
    //}

/*    const QString get_full_name() const
    {
        const tfile * pfile = this ;
        QString full_name = m_filename ; // + ( is_dir() ?  QString(QDir::separator()).toStdString().c_str() : "" ) ;
        while ( ptfile sptr_parent =  pfile->mp_parent.lock() )
        {
            assert( sptr_parent.get() ) ;
            pfile = sptr_parent.get() ;
            full_name = pfile->m_filename + full_name ; // + QDir::separator()
        }
        return full_name ;
    }
*/
} ;

class ttrees
{
    QStringList dirs_ = {} ;
    QStringList skip_dirs_ = {} ;
    //std::vector< ptfile > trees_ ; // holds root dirs of trees
    std::vector< tfile > m_files ;

    size_t count_files_ ;

    void fs_readdir_recursive(const QString & path_norm , int parent_index ) ;

    QString normalize_str( QString const & str ) ;

    //template< typename F >
    //void traverse_tree( ptfile & p_parent_file , F func ) ; // excluding the root dir name  - it is not needed

    // dir_semistr - string containing directories separated by semicolon
    bool organizeDirString( const QString & dir_semistr ) ;
    bool show_hidden_ ;

  public:

    QAtomicInt filestree_build_running_ = 0 ;
    bool dirs_set_ = false ;

    // the following flag reset to false when application has been started and the tree is built
    // now, this is necessary to handle on_hidden_files_checked, so as not to trigger start_build_tree()
    bool app_starting = true ;

private:
    //for searching skip dirs, that do not exist
    // we will set the flag in the map (via fs_readdir_recursive):
    std::unordered_map< QString, bool > umap_undiscovered_skip_dirs_ ;
public:
    //and the return array we will pass in:
    std::vector< QString > undiscovered_skip_dirs_ ;
    //


    std::vector< const tfile * > find_str( const QString & str ) ; // ptfile & dir

    // Set directories and clear whole tree.
    //     if directories was not changed , do nothing ( if forse flag is false )
    // QString sdir_semistr : directories seprated by ';'
    // bool force : reset dirs even if dirs has not been changed .
    //
    // return value:
    //  true - directories have been changed, thus it should be performed next actions in underlying function
    //  false - directories remained the same, there is no need to do anything.
    bool set_dirs( const QString & sdir_semistr ) ; //, bool force = false

    // does not clear tree.
    void make_tree() ;

    void add_file( const QString & filename , int parent_dir , bool is_dir )
    {
        m_files.emplace_back( filename , parent_dir , is_dir ) ;
    }

    const QString get_full_name( const tfile * file ) const ;

    void clear()
    {
        m_files.clear() ;
    }

    size_t get_count_files()
    {
        return count_files_ ;
    }

    void show_hidden( bool show_hid )
    { show_hidden_ = show_hid ; }

} ; // ttrees


#endif // TFILE_H
