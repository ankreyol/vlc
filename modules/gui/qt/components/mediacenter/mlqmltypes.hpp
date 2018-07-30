#ifndef MLQMLTYPES_HPP
#define MLQMLTYPES_HPP

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <QObject>
#include <vlc_common.h>
#include <vlc_media_library.h>

//FIXME: most likely useless now.
class ParentType : public QObject
{
    Q_OBJECT

public:
    enum ParentTypes
    {
        Album = VLC_ML_PARENT_ALBUM,
        Artist = VLC_ML_PARENT_ARTIST,
        Show = VLC_ML_PARENT_SHOW,
        Genre = VLC_ML_PARENT_GENRE,
        Playlist = VLC_ML_PARENT_PLAYLIST,
    };
    Q_ENUMS( ParentTypes )
};

class MLParentId
{
    Q_GADGET
public:
    MLParentId() : id(0), type( static_cast<vlc_ml_parent_type>( -1 ) ) {}
    MLParentId( int64_t i, vlc_ml_parent_type t ) : id( i ), type( t ) {}
    bool operator!=( const MLParentId& lhs )
    {
        return id != lhs.id || type != lhs.type;
    }
    int64_t id;
    vlc_ml_parent_type type;
};

Q_DECLARE_METATYPE(MLParentId)

#endif // MLQMLTYPES_HPP
