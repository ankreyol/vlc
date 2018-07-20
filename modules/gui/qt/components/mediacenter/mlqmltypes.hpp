#ifndef MLQMLTYPES_HPP
#define MLQMLTYPES_HPP

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <QObject>
#include <vlc_common.h>
#include <vlc_media_library.h>

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

#endif // MLQMLTYPES_HPP
