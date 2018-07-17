#ifndef MLHELPER_HPP
#define MLHELPER_HPP

#include <memory>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "vlc_media_library.h"

template<typename T>
class MLDeleter
{
public:
    void operator() (T* obj) {
        vlc_ml_release_obj(obj);
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
