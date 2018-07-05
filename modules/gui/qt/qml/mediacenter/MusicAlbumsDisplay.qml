/*****************************************************************************
 * MusicAlbumsDisplay.qml : Component to display when category is "albums"
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

    // Force the data to be reloaded
    function reloadData() {
        viewLoader.item.model = medialib.albums;
        console.log( "Data reloaded" );
    }

    //sourceComponent: medialib.gridView ? gridViewComponent_id : listViewComponent_id
    sourceComponent: gridViewComponent_id
    /* Grid View */

    Component {
        id: gridViewComponent_id

        Flickable {
            ScrollBar.vertical: ScrollBar { }
            width: parent.width
            height: parent.height
            contentHeight: gridView_id.height
            //contentWidth: gridView_id.width

            Utils.ExpandGridView {

                id: gridView_id

                model: medialib.albums

                cellWidth: VLCStyle.cover_normal
                cellHeight: VLCStyle.cover_normal + VLCStyle.fontSize_small + VLCStyle.margin_xsmall
                expandHeight: VLCStyle.heightBar_xxlarge

                rowSpacing: VLCStyle.margin_xxxsmall
                columnSpacing: VLCStyle.margin_xxxsmall
                expandSpacing: VLCStyle.margin_xxxsmall
                expandCompact: true

                expandDuration: VLCStyle.timingGridExpandOpen

                delegate : Utils.GridItem {
                    width: gridView_id.cellWidth
                    height: gridView_id.cellHeight

                    cover : Image { source: model.cover || VLCStyle.noArtCover }
                    name : model.title || "Unknown title"
                    date : model.release_year !== "0" ? model.release_year : ""
                    infos : model.duration + " - " + model.nb_tracks + " tracks"

                    onItemClicked : console.log('Clicked on details : '+ model.title)
                    onPlayClicked: {
                        console.log('Clicked on play : '+model.title);
                        medialib.addAndPlay(currentIndex)
                    }
                    onAddToPlaylistClicked : {
                        console.log('Clicked on addToPlaylist : '+model.title);
                        medialib.addToPlaylist(currentIndex);
                    }
                }

                expandDelegate: MusicAlbumsGridExpandDelegate {
                    height: gridView_id.expandHeight
                }

            }
        }
    }


    /* List View */
    /*
    Component {
        id: listViewComponent_id

        ListView {
            spacing: VLCStyle.margin_xxxsmall

            model: medialib.albums
            delegate : Utils.ListExpandItem {
                height: VLCStyle.icon_normal
                width: parent.width

                cover: Image {
                    id: cover_obj

                    width: VLCStyle.icon_normal
                    height: VLCStyle.icon_normal

                    source: cover || VLCStyle.noArtCover

                    states: State {
                        name: "expanded"
                        PropertyChanges {target: cover_obj; width: VLCStyle.icon_xlarge; height: VLCStyle.icon_xlarge}
                    }
                    Behavior on height { PropertyAnimation { duration: VLCStyle.timingListExpandOpen } }
                    Behavior on width { PropertyAnimation { duration: VLCStyle.timingListExpandOpen } }
                }
                line1: Text{
                    text: (title || "Unknown title")+" ["+duration+"]"
                    font.bold: true
                    elide: Text.ElideRight
                    color: VLCStyle.textColor
                    font.pixelSize: VLCStyle.fontSize_normal
                }
                line2: Text{
                    text: main_artist || "Unknown artist"
                    elide: Text.ElideRight
                    color: VLCStyle.textColor
                    font.pixelSize: VLCStyle.fontSize_xsmall
                }
                expand: Utils.TracksDisplay {
                    height: nb_tracks * (VLCStyle.fontSize_normal + VLCStyle.margin_xxxsmall) - VLCStyle.margin_xxxsmall
                    width: parent.width

                    tracks: tracks
                    parentIndex: index
                }

                onPlayClicked: {
                    console.log('Clicked on play : '+title);
                    medialib.addAndPlay(index)
                }
                onAddToPlaylistClicked: {
                    console.log('Clicked on addToPlaylist : '+title);
                    medialib.addToPlaylist(index);
                }
            }

            ScrollBar.vertical: ScrollBar { }
        }
    }
    */
}
