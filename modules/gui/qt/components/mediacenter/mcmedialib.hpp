/*****************************************************************************
 * mcmedialib.hpp : Medialibrary object
 ****************************************************************************
 * Copyright (C) 2006-2011 VideoLAN and AUTHORS
 * $Id$
 *
 * Authors: Maël Kervella <dev@maelkervella.eu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

#pragma once

#include <Qt>
#include <QAbstractListModel>
#include <QVariant>
#include <QHash>
#include <QByteArray>
#include <QList>
#include <QQuickWidget>
#include <QQuickItem>
#include <QMetaObject>
#include <QMetaMethod>
#include <QQmlEngine>

#include <memory>

#include "qt.hpp"
#include "mlalbum.hpp"
#include "mlmovie.hpp"
#include "mlserie.hpp"
#include "mlgenre.hpp"
#include "mlartist.hpp"

#include "components/playlist/plitem.hpp"
#include "components/playlist/plmodel.hpp"

#include "mlgenremodel.hpp"
#include "mlalbummodel.hpp"
#include "mlartistmodel.hpp"
#include "mlalbumtrackmodel.hpp"

enum MCMediaLibCategory {
    CAT_MUSIC_ALBUM,
    CAT_MUSIC_ARTIST,
    CAT_MUSIC_GENRE,
    CAT_MUSIC_TRACKS,
    CAT_VIDEO,
    CAT_NETWORK
};

class MCMediaLib : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool gridView READ isGridView WRITE setGridView NOTIFY gridViewChanged)
    Q_PROPERTY(int category READ getCategory WRITE setCategory NOTIFY categoryChanged)

    Q_PROPERTY(MLAlbumModel* albums READ getAlbums NOTIFY albumsChanged)
    Q_PROPERTY(MLArtistModel* artists READ getArtists NOTIFY artistsChanged)
    Q_PROPERTY(MLGenreModel* genres READ getGenres NOTIFY genreChanged)
    Q_PROPERTY(MLAlbumTrackModel* tracks READ getTracks NOTIFY tracksChanged)

public:
    MCMediaLib(
        intf_thread_t* _intf,
        QQuickWidget* _qml_item,
        std::shared_ptr<PLModel> _pl_model,
        QObject* _parent = nullptr
    );

    Q_INVOKABLE QVariant hasPresentation();
    Q_INVOKABLE void backPresentation();

    int getCategory() const ;
    void setCategory(int category);

    bool isGridView() const;
    void setGridView(bool);
    Q_INVOKABLE void toogleGridView();

    MLAlbumModel* getAlbums();
    MLArtistModel* getArtists();
    MLGenreModel* getGenres();
    MLAlbumTrackModel* getTracks();
    Q_INVOKABLE void getMovies();
    Q_INVOKABLE void getSeries();

    Q_INVOKABLE void select(const int &item_id);
    Q_INVOKABLE void addToPlaylist(const int &item_id);
    Q_INVOKABLE void addToPlaylist(const int &item_id, const int &subitem_id);
    Q_INVOKABLE void addAndPlay(const int &item_id);
    Q_INVOKABLE void addAndPlay(const int &item_id, const int &subitem_id);
    Q_INVOKABLE void selectSource(const QString &name );

signals:
    void gridViewChanged();
    void categoryChanged();

    void albumsChanged();
    void artistsChanged();
    void genreChanged();
    void tracksChanged();

private:
    intf_thread_t* m_intf;
    QQuickWidget *m_qmlItem;
    std::shared_ptr<PLModel> m_PLModel;

    bool m_gridView;

    MCMediaLibCategory m_oldCat;
    MCMediaLibCategory m_currentCat;

    MLGenreModel*  m_genreModel;
    MLArtistModel* m_artistModel;
    MLAlbumModel*  m_albumModel;
    MLAlbumTrackModel*  m_trackModel;

    /* Medialibrary */
    std::shared_ptr<vlc_medialibrary_t> m_ml;

    void invokeQML(const char *func );
};
