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
import QtQuick.Layouts 1.3

import org.videolan.medialib 0.1

import "qrc:///utils/" as Utils
import "qrc:///style/"

Loader {
    id: artistViewLoader
    sourceComponent: artistView
    property var selectedArtistParentId
    property var model: MLArtistModel {
        ml: medialib
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
                model: artistViewLoader.model
                onItemClicked: function(model) {
                    artistViewLoader.selectedArtistParentId = model.id;
                }
            }

            StackLayout {
                currentIndex: artistViewLoader.selectedArtistParentId ? 1 : 0
                // Display all artists when none is selected from the side menu
                Flickable {
                    Layout.fillWidth: true
                    ScrollBar.vertical: ScrollBar { }
                    clip: true
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    anchors.right: parent.right
                    contentHeight: artistGridView.height
                    ArtistGridView {
                        id: artistGridView
                        anchors.fill: parent
                        model: MLArtistModel {
                            ml: medialib
                        }
                    }
                }
                // Display selected artist albums
                MusicAlbumsDisplay {
                    anchors.fill: parent
                    model: MLAlbumModel {
                        ml: medialib
                        parentId: artistViewLoader.selectedArtistParentId
                    }
                }
            }
        }
    }
}
