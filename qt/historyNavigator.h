#ifndef HISTORYNAVIGATOR_H
#define HISTORYNAVIGATOR_H

#include <deque>
#include <vector>

template< class T >
class historyNavigator
{
    std::deque<T> content_ ;
    mutable size_t cur_item_ = 0 ; // vector index = cur_item -1  ( content[ index ] == content[ cur_item -1 ] )
    size_t max_size_ = 0 ; // 0 - unlimited size

public:
    historyNavigator( size_t max_size = 0 ) : max_size_( max_size ) {}

    void add( const T & item ) {
        if ( cur_item_ >= 1 ) {
            if ( content_[ cur_item_ -1 ] == item ) {
                // no need to add the same item
                return ;
            }
        }
        if ( cur_item_ < content_.size() ) {
            content_.resize( cur_item_ );
        }
        assert( cur_item_ == content_.size() ) ;
        content_.push_back(item) ;
        cur_item_++ ;

        if ( max_size_ )
            if ( content_.size() > max_size_ ) {
                content_.pop_front() ;
                --cur_item_ ;
            }
    }

    T get_back() const {
        if ( cur_item_ > 1 ) --cur_item_ ;

        if ( cur_item_ > 0 )
            return content_[ cur_item_ - 1 ] ;
        else return T()  ;
    }

    T get_forward() const {
        if ( cur_item_ < content_.size() ) ++cur_item_ ;

        if ( cur_item_ > 0 )
            return content_[ cur_item_ - 1 ] ;
        else return T() ;
    }

    T get_cur_item_value() const
    { return content_[ cur_item_ - 1 ] ; }

    bool has_back_item() {
        return cur_item_ > 1 ;
    }

    bool has_forward_item() {
        return cur_item_ < content_.size() ;
    }

    const std::vector<T> get_content() const
    { return std::vector( content_.begin() , content_.end() ) ; }

    void set_content( const std::vector<T> & vcontent )
    { content_ = std::deque( vcontent.begin() , vcontent.end() )  ; }

    size_t get_cur_item_index() const
    { return cur_item_ ; }

    void set_cur_item( size_t cur_item )
    { cur_item_ = cur_item ; }

    void set_max_size( size_t max_size )
    { max_size_ = max_size ; }

} ;


#endif // HISTORYNAVIGATOR_H
