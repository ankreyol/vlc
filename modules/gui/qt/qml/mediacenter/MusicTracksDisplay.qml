/*****************************************************************************
 * MusicTracksDisplay.qml : Component to display when category is "tracks"
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

import "qrc:///utils/" as Utils
import "qrc:///style/"

Loader {
    id: viewLoader
    property var model: []

    // Force the data to be reloaded
    function reloadData() {
        viewLoader.item.model = medialib.getObjects();
        console.log( "Data reloaded" );
    }

    sourceComponent: medialib.gridView ? gridViewComponent_id : listViewComponent_id

    /* Grid View */
    Component {
        id: gridViewComponent_id

        Flickable {
            ScrollBar.vertical: ScrollBar { }
            anchors.fill: parent
            contentHeight: gridView_id.height
            clip: true
            Utils.ExpandGridView {
            id: gridView_id

            cellWidth: VLCStyle.cover_normal
            cellHeight: VLCStyle.cover_normal + VLCStyle.fontSize_small + VLCStyle.margin_xsmall

            model: viewLoader.model
            delegate : Utils.GridItem {
                width: gridView_id.cellWidth
                height: gridView_id.cellHeight

                cover: Image { source: model.cover || VLCStyle.noArtCover }
                name: model.title || "Unknown track"

                onItemClicked: console.log('Clicked on details : '+model.title)
                onPlayClicked: {
                    console.log('Clicked on play : '+model.title);
                    medialib.addAndPlay(index)
                }
                onAddToPlaylistClicked: {
                    console.log('Clicked on addToPlaylist : '+model.title);
                    medialib.addToPlaylist(index);
                }
            }
            }
        }
    }

    /* List View */
    Component {
        id: listViewComponent_id
        ListView {
            spacing: VLCStyle.margin_xxxsmall

            model: viewLoader.model
            delegate : Utils.ListItem {
                height: VLCStyle.heightBar_small
                width: parent.width

                line1: Text{
                    text: (model.title || "Unknown track")+" - "+model.duration
                    font.bold: true
                    elide: Text.ElideRight
                    color: VLCStyle.textColor
                }

                onItemClicked: console.log("Clicked on : "+model.title)
                onPlayClicked: {
                    console.log('Clicked on play : '+model.title);
                    medialib.addAndPlay(index)
                }
                onAddToPlaylistClicked: {
                    console.log('Clicked on addToPlaylist : '+model.title);
                    medialib.addToPlaylist(index);
                }
            }

            ScrollBar.vertical: ScrollBar { }
        }
    }
}
