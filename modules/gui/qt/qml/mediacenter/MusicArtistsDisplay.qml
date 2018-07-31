/*****************************************************************************
 * MusicAlbumsDisplay.qml : Component to display when category is "artists"
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

import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.0

import org.videolan.medialib 0.1

import "qrc:///utils/" as Utils
import "qrc:///style/"

Loader {
    id: viewLoader
    sourceComponent: artistView

    // Force the data to be reloaded
    function reloadData() {
        viewLoader.item.model = medialib.getArtists();
        console.log( "Data reloaded" );
    }

    Component {
        id: artistView
        SplitView {
            anchors.fill: parent

            ArtistListView  {
                id: artistList
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                Layout.minimumWidth: 250
                model: MLArtistModel {
                    ml: medialib
                }
            }
            Flickable {
                Layout.fillWidth: true
                ScrollBar.vertical: ScrollBar { }
                clip: true
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.right: parent.right
                anchors.left: artistList.right
                contentHeight: gridView_id.height
                ArtistGridView {
                    id: gridView_id
                    anchors.fill: parent
                    model: MLArtistModel {
                        ml: medialib
                    }
                }
            }
        }
    }
}
