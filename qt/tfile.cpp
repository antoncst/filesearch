#include "tfile.h"

#include <filesystem>
#include <QDebug>
#include <QStringList>
#include <QDir>

//void traverse_tree( ptfile & p_parent_file , void ( * func ) ( ptfile & ))

/*template< typename F >
void ttrees::traverse_tree( ptfile & p_parent_file , F func ) // excluding the root dir name  - it is not needed
{
    for ( auto  pfile : p_parent_file->get_files() )
    {
        func( pfile ) ;
        if ( pfile->is_dir() )
            traverse_tree( pfile , func ) ;
    }
}
*/

QString ttrees::normalize_str( QString const & str )
{
    QString res = str.toLower() ;

    // non-strict matching :
    for ( auto & ch : res )
    {
        if ( ch == QChar(L'ё') )
            ch = QChar(L'е') ;
        else if ( ch == QChar(L'y') || ch == QChar(L'j') )
            ch = QChar(L'i') ;
        else if ( ch == QChar(L'c') )
            ch = QChar(L'k') ;
    }

    return res ;
}

std::vector< const tfile * > ttrees::find_str( const QString & str ) // ptfile & dir
{
    std::vector< const tfile * > res ;
    QString str_norm = normalize_str( str ) ;

    QStringList qsl_str = str_norm.split( ' ' , Qt::SkipEmptyParts ) ; // qt6  ;
    //QStringList qsl_str = str_norm.split( ' ' , QString::SplitBehavior::SkipEmptyParts ) ; // qt5  ;
    std::vector< QString > substrings_and2 ( qsl_str.constBegin() , qsl_str.constEnd() )  ;
    //    AND          OR
    std::vector< std::vector< QString > > substrings_and_or ;
    for ( QString & line : substrings_and2 )
    {
        // qt5 :
        //substrings_and_or.push_back(  line.split( '|' ).toVector().toStdVector() ) ;

        // c++14 or 17 :
        //QStringList qsl_substr2 = line.split( '|' , QString::SplitBehavior::SkipEmptyParts ) ; // qt5
        QStringList qsl_substr2 = line.split( '|' , Qt::SkipEmptyParts ) ; // qt6 :

        QStringList qsl_substr ;
        //Пройдемся по всем и исключим пустые отрицания , то есть '!'
        for ( QString & qs : qsl_substr2 )
            if ( qs.trimmed() != "!" )
                qsl_substr.push_back( qs.trimmed() );

        substrings_and_or.push_back(  std::vector( qsl_substr.constBegin() , qsl_substr.constEnd() )  ) ;
    }



    size_t counter = 0 ;
    bool bstop = false ;

    //for ( ptfile dir : trees_ )
    for ( const tfile & file : m_files )
    {
//        traverse_tree( dir , [ & ] ( ptfile & pfile)
//                    {
                        if ( bstop )
                            return res ;
                        bool found = true ;

                        for ( auto & substrings_and : substrings_and_or )
                        {   bool found_or = false ;
                            for ( QString & str_or : substrings_and )
                            {
                                if ( str_or[0] == '!') // negation
                                {
                                    QString str_neg = str_or.mid( 1 ) ;
                                    qsizetype pos = normalize_str( get_full_name( &file ) ).indexOf( str_neg ) ; //to do get_full_name
                                    if ( pos != -1 ) //found
                                    {   found_or = false ; break ; }
                                    else
                                        found_or = true ;
                                }
                                else
                                {
                                    qsizetype pos = normalize_str( file.get_name() ).indexOf( str_or ) ; //
                                    if ( pos != -1 ) //found
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
                            res.push_back( &file ) ;
                            if ( ++counter == max_find_count  )
                                bstop = true ;
                        }

                    //} ) ;
                        ;
    }
    return res ;
}


// return value and parameter the same as in ttrees::set_dirs
bool ttrees::organizeDirString( const QString & dir_semistr )
{
    QStringList dirs_root ;
    QStringList dirs_skip ;
    QStringList dirs_all = dir_semistr.split(";" , Qt::SkipEmptyParts ) ; //qt6
    //QStringList dirs_all = dir_semistr.split(";" , QString::SplitBehavior::SkipEmptyParts ) ; //qt5
    for ( QString dir : dirs_all )
    {
        dir = dir.trimmed() ;
        if ( dir.size() == 0 )
            continue ;
        if ( dir[0] == '!' )
            dirs_skip.push_back( QDir::toNativeSeparators ( dir.mid( 1 ).trimmed() ) ) ;
        else
            dirs_root.push_back( QDir::toNativeSeparators( dir ) ) ;
    }
    if ( dirs_ == dirs_root  &&  skip_dirs_ == dirs_skip )
        return false ;
    dirs_ = dirs_root ;
    skip_dirs_ = dirs_skip ;
    return true ;
}


bool ttrees::set_dirs( const QString & sdir_semistr ) //, bool force
{
    if ( filestree_build_running_ )
    {
        qDebug() << " Warning! set_dirs when filestree_build_running " ;
        return false ;
    }

    dirs_set_ = true ;


    //if ( force )
        //organizeDirString( sdir_semistr ) ;
    //else
        if ( !organizeDirString( sdir_semistr ) )
        return false ;

    //m_files.clear() ;
    //count_files_ = 0 ;

    for ( QString & sdir_par : dirs_ )
    {

        // i.e. if root_dir is "c:" or "c:\"
        if ( ! sdir_par.endsWith( QDir::separator() ) )
            sdir_par = sdir_par + QDir::separator() ;
        //

        if ( ! QDir( sdir_par ).exists() )
            throw ( sdir_par ) ;


        //ptfile root_dir = add_tree( sdir ) ;
        qDebug() << "tree: " << sdir_par ;

    }

    return true ;
}

// get the full file name beginning from the root dir
const QString ttrees::get_full_name( const tfile * file ) const
{
    QString full_name = file->get_name() ;
    int index = file->get_parent_dir() ;

    //const tfile * cur_dir = m_files[ m_parent_dir ] ;
    while ( index != -1 )
    {
        full_name = m_files[ index ].get_name() + full_name ; //QDir::separator()
        index = m_files[ index ].get_parent_dir() ;
    } ;

    return full_name ;
}



void ttrees::make_tree(  )
{
    qDebug() << "MAKING TREE!!!" ;
    assert( filestree_build_running_ == 1 ) ;

    //reset the container of skip dirs
    umap_undiscovered_skip_dirs_.clear() ;
    for ( QString & skip_dir_qstr : skip_dirs_ )
        umap_undiscovered_skip_dirs_[ skip_dir_qstr ] = false ;
    //

    count_files_ = 0 ;
    clear() ;
    for ( QString & str_root_dir :  dirs_ )
    {
        //const std::filesystem::path fs_rootdir{ str_root_dir } ;
        add_file( str_root_dir , -1 , true ) ;
        fs_readdir_recursive( str_root_dir , m_files.size() - 1 ) ;
    }

    // covering skip dirs
    undiscovered_skip_dirs_.clear() ;
    for ( auto & item : umap_undiscovered_skip_dirs_ ) {
        //qDebug() << "skip dir:  " << item.first << " found " << item.second ;
        if ( ! item.second )
            undiscovered_skip_dirs_.push_back( item.first ) ;
    }
    //
    qDebug() << " make_tree: found " << count_files_ ;
}


void ttrees::fs_readdir_recursive( const QString & path , int parent_index )
{

    QString path_norm = path.endsWith( QDir::separator() ) ? path : path + QDir::separator() ;
    QDir fsdir( path_norm  ) ;

    auto dir_mask = QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks ;
    dir_mask =  show_hidden_ ? dir_mask | QDir::Hidden : dir_mask ;
    for ( QString const & subdir : fsdir.entryList( dir_mask ) )
    {
        if ( ! filestree_build_running_ )
            break ;
        //skipping dirs
        bool bSkipDirFound = false;
        for ( QString & skip_dir : skip_dirs_ )
            if ( (path_norm + subdir).contains( skip_dir ) )
            {
                bSkipDirFound = true ;
                umap_undiscovered_skip_dirs_[ skip_dir ] = true ;
                break ;
            }
        if ( bSkipDirFound )
            continue ;
        // end of skipping dirs
        add_file( subdir + QDir::separator()  , parent_index , true ) ;
        count_files_ ++ ;
        fs_readdir_recursive( path_norm + subdir , m_files.size() - 1 ) ;
    }

    auto file_mask = QDir::Files | QDir::Files ;  //QFlags<QDir::Filter>
    file_mask =  show_hidden_ ? file_mask | QDir::Hidden : file_mask ;
    for ( QString const & filename : fsdir.entryList( file_mask ) )
    {
        add_file( filename , parent_index , false) ;
        count_files_ ++ ;
    }
}

