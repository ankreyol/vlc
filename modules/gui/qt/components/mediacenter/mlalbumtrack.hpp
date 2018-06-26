/*****************************************************************************
 * mlalbumtrack.hpp : Medialibrary's album track
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
#include <memory>

#include <vlc_media_library.h>
#include "mlhelper.hpp"

class MLAlbumTrack : public QObject
{
    Q_OBJECT

    Q_PROPERTY(uint64_t id READ getId)
    Q_PROPERTY(QString title READ getTitle)
    Q_PROPERTY(QString albumtitle READ getAlbumTitle)
    Q_PROPERTY(QString cover READ getCover)
    Q_PROPERTY(unsigned int tracknumber READ getTrackNumber)
    Q_PROPERTY(unsigned int duration READ getDuration)
    Q_PROPERTY(QString mrl READ getMRL)

public:
    MLAlbumTrack(const ml_media_t *_data, QObject *_parent = nullptr);

    uint64_t getId() const;
    QString getTitle() const;
    QString getAlbumTitle() const;
    QString getCover() const;
    unsigned int getTrackNumber() const;
    unsigned int getDuration() const;
    QString getMRL() const;

    Q_INVOKABLE QString getPresName() const;
    Q_INVOKABLE QString getPresImage() const;
    Q_INVOKABLE QString getPresInfo() const;

private:
    uint64_t m_id;
    QString m_title;
    QString m_albumTitle;
    QString m_cover;
    unsigned int m_trackNumber;
    unsigned int m_duration;
    QString m_mrl;

   ml_unique_ptr<ml_media_t> m_data;

signals:

public slots:
};
