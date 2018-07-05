#ifndef MLHELPER_HPP
#define MLHELPER_HPP

#include <memory>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "vlc_media_library.h"

inline void MLRelease( vlc_ml_show_t* show ) { vlc_ml_show_release( show ); }
inline void MLRelease( vlc_ml_artist_t* artist ) { vlc_ml_artist_release( artist ); }
inline void MLRelease( vlc_ml_album_t* album ) { vlc_ml_album_release( album ); }
inline void MLRelease( vlc_ml_genre_t* genre ) { vlc_ml_genre_release( genre ); }
inline void MLRelease( vlc_ml_media_t* media ) { vlc_ml_media_release( media ); }
inline void MLRelease( vlc_ml_label_list_t* list ) { vlc_ml_label_list_release( list ); }
inline void MLRelease( vlc_ml_file_list_t* list ) { vlc_ml_file_list_release( list ); }
inline void MLRelease( vlc_ml_artist_list_t* list ) { vlc_ml_artist_list_release( list ); }
inline void MLRelease( vlc_ml_media_list_t* list ) { vlc_ml_media_list_release( list ); }
inline void MLRelease( vlc_ml_album_list_t* list ) { vlc_ml_album_list_release( list ); }
inline void MLRelease( vlc_ml_show_list_t* list ) { vlc_ml_show_list_release( list ); }
inline void MLRelease( vlc_ml_genre_list_t* list ) { vlc_ml_genre_list_release( list ); }

template<typename T>
class MLDeleter
{
public:
    void operator() (T* obj) {
        MLRelease(obj);
    }
};

template<typename T>
using ml_unique_ptr = std::unique_ptr<T, MLDeleter<T> >;

template<typename T>
class MLListRange
{
public:
    MLListRange( T* begin, T* end )
        : m_begin(begin)
        , m_end(end)
    {
    }

    T* begin() const
    {
        return m_begin;
    }

    T* end() const
    {
        return m_end;
    }

private:
    T* m_begin;
    T* m_end;
};

template<typename T, typename L>
MLListRange<T> ml_range_iterate(L& list)
{
    return MLListRange<T>{ list->p_items, list->p_items + list->i_nb_items };
}


#endif // MLHELPER_HPP
