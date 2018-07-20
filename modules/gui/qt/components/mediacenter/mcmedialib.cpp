/*****************************************************************************
 * mcmedialib.cpp : Medialibrary object
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

#include "mcmedialib.hpp"
#include "mlhelper.hpp"

#include <vlc_playlist.h>
#include <vlc_input_item.h>

MCMediaLib::MCMediaLib(
        intf_thread_t *_intf,
        QQuickWidget *_qml_item,
        std::shared_ptr<PLModel> _pl_model,
        QObject *_parent)
    : QObject( _parent )
    , m_intf( _intf )
    , m_qmlItem( _qml_item )
    , m_PLModel( _pl_model )
    , m_gridView( true )
    , m_oldCat ( CAT_MUSIC_ALBUM )
    , m_currentCat ( CAT_MUSIC_ALBUM )
{
    m_ml = vlc_ml_get(_intf);
    m_genreModel = new MLGenreModel(m_ml, this);
    m_trackModel = new MLAlbumTrackModel(m_ml, this);
}

// Are we exploring a specific item or just browsing generic category
QVariant MCMediaLib::hasPresentation() {
    return false;
}

// Remove presentation to get back to the list of items that were in place before
void MCMediaLib::backPresentation() {
    m_currentCat = m_oldCat;
    emit categoryChanged();
    invokeQML("reloadPresentation()");
}

// Which category should be displayed
int MCMediaLib::getCategory() const
{
    return m_currentCat;
}

void MCMediaLib::setCategory(int category)
{
    //FIXME is there somthing to do
    m_currentCat = (MCMediaLibCategory)category;
    emit categoryChanged();
}

// Should the items be displayed as a grid or as list ?
bool MCMediaLib::isGridView() const
{
    return m_gridView;
}

void MCMediaLib::setGridView(bool state)
{
    m_gridView = state;
    emit gridViewChanged();
}

// Toogle between grid and list view for the displayed items
void MCMediaLib::toogleGridView()
{
    setGridView(!m_gridView);
}

// A specific item has been selected -> update the list of obejcts and the presentation
void MCMediaLib::select( const int & )
{
    //if (item_id >= 0 && item_id <= m_currentObj.count())
    //{
    //    if (!m_currentMainObj)
    //        m_oldCat = m_currentCat;
    //
    //    m_currentMainObj = m_currentObj.at(item_id);
    //    m_currentObj = m_currentMainObj->getDetailsObjects(m_currentSort, m_isDesc);
    //
    //    switch (m_currentCat)
    //    {
    //    case CAT_MUSIC_ALBUM:
    //        m_currentCat = CAT_MUSIC_TRACKS;
    //        break;
    //
    //    case CAT_MUSIC_ARTIST:
    //        m_currentCat = CAT_MUSIC_ALBUM;
    //        break;
    //
    //    case CAT_MUSIC_GENRE:
    //        m_currentCat = CAT_MUSIC_ALBUM;
    //        break;
    //
    //    default:
    //        break;
    //    }
    //}
    //
    //invokeQML("reloadPresentation()");
    //emit categoryChanged();
}

// A specific item has been asked to be added to the playlist
void MCMediaLib::addToPlaylist( const int & )
{
    //if (item_id >= 0 && item_id <= m_currentObj.count())
    //{
    //    std::shared_ptr<MLItem> selected_item = m_currentObj.at(item_id);
    //    QList<MLAlbumTrack*> tracks = selected_item->getPLTracks();
    //
    //    for (int i=0 ; i<tracks.size() ; i++)
    //    {
    //        std::shared_ptr<PLItem> pl_item = std::make_shared<PLItem>(tracks.at(i));
    //        m_PLModel->appendItem(pl_item, false);
    //    }
    //}
}

// A specific sub-item has been asked to be added to the playlist
void MCMediaLib::addToPlaylist( const int &, const int & )
{
    //if (item_id >= 0 && item_id <= m_currentObj.count())
    //{
    //    std::shared_ptr<MLItem> selected_item = m_currentObj.at(item_id);
    //    QList<std::shared_ptr<MLItem>> subitems = selected_item->getDetailsObjects(m_currentSort, m_isDesc);
    //    if (subitem_id >= 0 && subitem_id <= subitems.count())
    //    {
    //        std::shared_ptr<MLItem> selected_subitem = subitems.at(subitem_id);
    //        QList<MLAlbumTrack*> tracks = selected_subitem->getPLTracks();
    //
    //        for (int i=0 ; i<tracks.size() ; i++)
    //        {
    //            std::shared_ptr<PLItem> pl_item = std::make_shared<PLItem>(tracks.at(i));
    //            m_PLModel->appendItem(pl_item, false);
    //        }
    //    }
    //}
}

// A specific item has been asked to be played,
// so it's added to the playlist and played
void MCMediaLib::addAndPlay( const int & )
{
    //if (item_id >= 0 && item_id <= m_currentObj.count())
    //{
    //    std::shared_ptr<MLItem> selected_item = m_currentObj.at(item_id);
    //    QList<MLAlbumTrack*> tracks = selected_item->getPLTracks();
    //
    //    for (int i=0 ; i<tracks.size() ; i++)
    //    {
    //        std::shared_ptr<PLItem> pl_item = std::make_shared<PLItem>(tracks.at(i));
    //        m_PLModel->appendItem(pl_item, i==0);
    //    }
    //}
}

// A specific sub-item has been asked to be played,
// so it's added to the playlist and played
void MCMediaLib::addAndPlay( const int &, const int & )
{
    //if (item_id >= 0 && item_id <= m_currentObj.count())
    //{
    //    std::shared_ptr<MLItem> selected_item = m_currentObj.at(item_id);
    //    QList<std::shared_ptr<MLItem>> subitems = selected_item->getDetailsObjects(m_currentSort, m_isDesc);
    //    if (subitem_id >= 0 && subitem_id <= subitems.count())
    //    {
    //        std::shared_ptr<MLItem> selected_subitem = subitems.at(subitem_id);
    //        QList<MLAlbumTrack*> tracks = selected_subitem->getPLTracks();
    //
    //        for (int i=0 ; i<tracks.size() ; i++)
    //        {
    //            std::shared_ptr<PLItem> pl_item = std::make_shared<PLItem>(tracks.at(i));
    //            m_PLModel->appendItem(pl_item, i==0);
    //        }
    //    }
    //}
}

// When a source (or sub-source) is selected (Music / Music>Albums / Videos / ...)
void MCMediaLib::selectSource( const QString & )
{
    //if (name == "music" && m_currentCat != CAT_MUSIC_ALBUM)
    //{
    //    msg_Dbg( m_intf, "Switching to music-general view");
    //    m_currentCat = CAT_MUSIC_ALBUM;
    //    if (m_currentMainObj) m_currentMainObj = nullptr;
    //    invokeQML("reloadPresentation()");
    //    emit categoryChanged();
    //}
    //else if (name == "music-albums" && m_currentCat != CAT_MUSIC_ALBUM)
    //{
    //    msg_Dbg( m_intf, "Switching to music-albums view");
    //    m_currentCat = CAT_MUSIC_ALBUM;
    //    if (m_currentMainObj) m_currentMainObj = nullptr;
    //    invokeQML("reloadPresentation()");
    //    emit categoryChanged();
    //}
    //else if (name == "music-artists" && m_currentCat != CAT_MUSIC_ARTIST)
    //{
    //    msg_Dbg( m_intf, "Switching to music-artists view");
    //    m_currentCat = CAT_MUSIC_ARTIST;
    //    if (m_currentMainObj) m_currentMainObj = nullptr;
    //    invokeQML("reloadPresentation()");
    //    emit categoryChanged();
    //}
    //else if (name == "music-genre" && m_currentCat != CAT_MUSIC_GENRE)
    //{
    //    msg_Dbg( m_intf, "Switching to music-genre view");
    //    m_currentCat = CAT_MUSIC_GENRE;
    //    if (m_currentMainObj) m_currentMainObj = nullptr;
    //    invokeQML("reloadPresentation()");
    //    emit categoryChanged();
    //}
    //else if (name == "music-tracks" && m_currentCat != CAT_MUSIC_TRACKS)
    //{
    //    msg_Dbg( m_intf, "Switching to music-track view");
    //    m_currentCat = CAT_MUSIC_TRACKS;
    //    if (m_currentMainObj) m_currentMainObj = nullptr;
    //    invokeQML("reloadPresentation()");
    //    emit categoryChanged();
    //}
    //else if (name == "video" && m_currentCat != CAT_VIDEO)
    //{
    //    msg_Dbg( m_intf, "Switching to video-general view");
    //    m_currentCat = CAT_VIDEO;
    //    if (m_currentMainObj) m_currentMainObj = nullptr;
    //    invokeQML("reloadPresentation()");
    //    emit categoryChanged();
    //}
    //else if (name == "network" && m_currentCat != CAT_NETWORK)
    //{
    //    msg_Dbg( m_intf, "Switching to network-general view");
    //    m_currentCat = CAT_NETWORK;
    //    if (m_currentMainObj) m_currentMainObj = nullptr;
    //    invokeQML("reloadPresentation()");
    //    emit categoryChanged();
    //}
}

vlc_medialibrary_t* MCMediaLib::vlcMl()
{
    return vlc_ml_get( m_intf );
}

MLGenreModel* MCMediaLib::getGenres()
{
    return m_genreModel;
}

MLAlbumTrackModel* MCMediaLib::getTracks()
{
    return m_trackModel;
}

void MCMediaLib::getMovies()
{

}

void MCMediaLib::getSeries()
{

}

// Invoke a given QML function (used to notify the view part of a change)
void MCMediaLib::invokeQML( const char* func ) {
    QQuickItem *root = m_qmlItem->rootObject();
    int methodIndex = root->metaObject()->indexOfMethod(func);
    QMetaMethod method = root->metaObject()->method(methodIndex);
    method.invoke(root);
}
