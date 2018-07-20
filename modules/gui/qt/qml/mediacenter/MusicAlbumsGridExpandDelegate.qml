/*****************************************************************************
 * MusicAlbumsGridExpandDelegate.qml : Item in the expanded zone displayed
 *     when an album is clicked
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

import QtQuick 2.2
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3

import org.videolan.medialib 0.1

import "qrc:///utils/" as Utils
import "qrc:///style/"

RowLayout {
    id: root
    spacing: VLCStyle.margin_xsmall

    property var albumModel: model
    /* A bigger cover for the album */
    Image {
        id: expand_cover_id

        Layout.preferredHeight: VLCStyle.cover_large
        Layout.preferredWidth: VLCStyle.cover_large
        Layout.margins: VLCStyle.margin_xsmall

        source: model.cover || VLCStyle.noArtCover
    }

    ColumnLayout {
        id: expand_infos_id

        spacing: VLCStyle.margin_xsmall
        Layout.fillWidth:  true

        /* The title of the albums */
        // Needs a rectangle too prevent the tracks from overlapping the title when scrolled
        Rectangle {
            id: expand_infos_titleRect_id
            height: expand_infos_title_id.implicitHeight
            Layout.fillWidth:  true
            color: "transparent"
            Text {
                id: expand_infos_title_id
                text: "<b>"+(model.title || "Unknown title")+"</b>"
                color: VLCStyle.textColor
            }
        }

        /* The list of the tracks available */
        ListView {
            id: expand_track_id

            Layout.leftMargin: VLCStyle.margin_large
            Layout.rightMargin: VLCStyle.margin_large
            Layout.topMargin: VLCStyle.margin_xsmall
            Layout.bottomMargin: VLCStyle.margin_small
            Layout.fillHeight: true
            Layout.fillWidth: true

            spacing: VLCStyle.margin_xxxsmall
            clip: true
            focus: true
            ScrollBar.vertical: ScrollBar { }

            model: MLAlbumTrackModel {
                ml: medialib
                parentId: model.id
            }

            delegate: Rectangle {
                height: txt_id.height
                width: parent.width
                color: {
                    if ( mouse.containsMouse)
                        VLCStyle.hoverBgColor
                    else if (index % 2)
                        VLCStyle.bgColor
                    else VLCStyle.bgColorAlt
                }

                MouseArea {
                    id: mouse
                    anchors.fill: parent
                    hoverEnabled: true
                }

                Text {
                    id: txt_id
                    text: (model.title || "Unknown track")+" - "+model.duration
                    elide: Text.ElideRight
                    font.pixelSize: VLCStyle.fontSize_normal
                    color: VLCStyle.textColor
                }
            }

        }
    }
}
