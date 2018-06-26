/*****************************************************************************
 * mlalbum.hcp : Medialibrary's album
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
#include <cassert>
#include "mlalbum.hpp"

MLAlbum::MLAlbum(const ml_album_t *_data, QObject *_parent)
    : QObject( _parent )
    , m_id          ( _data->i_id )
    , m_title       ( QString::fromUtf8( _data->psz_title ) )
    , m_releaseYear ( _data->i_year )
    , m_shortSummary( QString::fromUtf8( _data->psz_summary ) )
    , m_cover       ( QString::fromUtf8( _data->psz_artwork_mrl ) )
    , m_mainArtist  ( QString::fromUtf8( _data->psz_artist ) )
    , m_otherArtists( QList<QString>() )
    , m_nbTracks    ( _data->i_nb_tracks )
    , m_duration    ( _data->i_duration )
{
    assert(_data);
    // Fill m_otherArtists
    if (_data->p_featuring)
        for (int i=0 ; i < _data->p_featuring->i_nb_items ; i++ )
            m_otherArtists.append( QString::fromUtf8( _data->p_featuring->p_items[i].psz_name ) );
}

int64_t MLAlbum::getId() const
{
    return m_id;
}

QString MLAlbum::getTitle() const
{
    return m_title;
}

unsigned int MLAlbum::getReleaseYear() const
{
    return  m_releaseYear;
}

QString MLAlbum::getShortSummary() const
{
    return m_shortSummary;
}

QString MLAlbum::getCover() const
{
    return m_cover;
}


QString MLAlbum::getArtist() const
{
    return m_mainArtist;
}

QList<QString> MLAlbum::getArtists() const
{
    return m_otherArtists;
}

unsigned int MLAlbum::getNbTracks() const
{
    return m_nbTracks;
}

unsigned int MLAlbum::getDuration() const
{
    return m_duration;
}

QString MLAlbum::getPresName() const
{
    return m_title;
}

QString MLAlbum::getPresImage() const
{
    return m_cover;
}

QString MLAlbum::getPresInfo() const
{
    return m_shortSummary;
}


