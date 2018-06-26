/*****************************************************************************
 * mlalbum.hpp : Medialibrary's album
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <QObject>
#include <QString>
#include <QList>
#include <memory>
#include "vlc_media_library.h"
#include "mlhelper.hpp"

#include "mlalbumtrack.hpp"
#include "mlitem.hpp"

class MLAlbum : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int64_t id READ getId)
    Q_PROPERTY(QString title READ getTitle)
    Q_PROPERTY(unsigned int releaseyear READ getReleaseYear)
    Q_PROPERTY(QString shortsummary READ getShortSummary)
    Q_PROPERTY(QString cover READ getCover)
    Q_PROPERTY(QString artist READ getArtist)
    Q_PROPERTY(QList<QString> artists READ getArtists)
    Q_PROPERTY(unsigned int nbtracks READ getNbTracks)
    Q_PROPERTY(unsigned int duration READ getDuration)

public:
    MLAlbum(const ml_album_t *_data, QObject *_parent = nullptr);

    int64_t getId() const;
    QString getTitle() const;
    unsigned int getReleaseYear() const;
    QString getShortSummary() const;
    QString getCover() const;
    QString getArtist() const;
    QList<QString> getArtists() const;
    unsigned int getNbTracks() const;
    unsigned int getDuration() const;

    Q_INVOKABLE QString getPresName() const;
    Q_INVOKABLE QString getPresImage() const;
    Q_INVOKABLE QString getPresInfo() const;

private:
    int64_t m_id;
    QString m_title;
    unsigned int m_releaseYear;
    QString m_shortSummary;
    QString m_cover;
    QString m_mainArtist;
    QList<QString> m_otherArtists;
    unsigned int m_nbTracks;
    unsigned int m_duration;
};
