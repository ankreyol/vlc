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
import QtQuick.Layouts 1.1

import "qrc:///utils/" as Utils
import "qrc:///style/"

Loader {
    id: viewLoader
    property var model: []

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

                model: viewLoader.model

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

    Component {
        id: listViewComponent_id

        ListView {
            id: listview_id
            ScrollBar.vertical: ScrollBar { id: scroll_id }

            spacing: VLCStyle.margin_xxxsmall
            anchors.fill: parent
            model: viewLoader.model

            clip: true
            focus: true

            delegate : Rectangle {
                property bool hovered: false
                width: parent.width
                height: VLCStyle.icon_normal
                color: {
                    if ( mouse.containsMouse || index == listview_id.currentIndex )
                        VLCStyle.hoverBgColor
                    else if (index % 2)
                        VLCStyle.bgColor
                    else VLCStyle.bgColorAlt
                }
                MouseArea {
                    id: mouse
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: {
                        listview_id.currentIndex = index
                    }
                }

                RowLayout {
                    anchors.fill: parent
                    Image {
                        id: cover_obj
                        Layout.preferredWidth: VLCStyle.icon_normal
                        Layout.preferredHeight: VLCStyle.icon_normal
                        width: VLCStyle.icon_normal
                        height: VLCStyle.icon_normal
                        fillMode: Image.PreserveAspectFit
                        source: model.cover || VLCStyle.noArtCover
                    }
                    Column {
                        Text{
                            text: (model.title || "Unknown title")+" ["+model.duration+"]"
                            font.bold: true
                            elide: Text.ElideRight
                            color: VLCStyle.textColor
                            font.pixelSize: VLCStyle.fontSize_normal
                        }
                        Text{
                            text: model.main_artist || "Unknown artist"
                            elide: Text.ElideRight
                            color: VLCStyle.textColor
                            font.pixelSize: VLCStyle.fontSize_xsmall
                        }
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    Image {
                        id: add_to_playlist_icon

                        anchors.verticalCenter: parent.verticalCenter
                        Layout.preferredWidth: VLCStyle.icon_small
                        Layout.preferredHeight: VLCStyle.icon_small

                        visible: mouse.containsMouse
                        source: "qrc:///buttons/playlist/playlist_add.svg"
                        MouseArea {
                            anchors.fill: parent
                            onClicked: medialib.addAlbumToPlaylist(model.id, false);
                        }
                    }

                    /* The icon to add to playlist and play */
                    Image {
                        id: add_and_play_icon

                        Layout.preferredWidth: VLCStyle.icon_small
                        Layout.preferredHeight: VLCStyle.icon_small
                        Layout.rightMargin: VLCStyle.margin_large
                        visible: mouse.containsMouse
                        source: "qrc:///toolbar/play_b.svg"
                        MouseArea {
                            anchors.fill: parent
                            onClicked: medialib.addAlbumToPlaylist(model.id, true);
                        }
                    }
                }
            }

            //delegate: Utils.ListExpandItem {
            //       height: VLCStyle.icon_normal
            //       width: parent.width
            //
            //       cover: Image {
            //           id: cover_obj
            //
            //           width: VLCStyle.icon_normal
            //           height: VLCStyle.icon_normal
            //
            //           source: model.cover || VLCStyle.noArtCover
            //
            //           states: State {
            //               name: "expanded"
            //               PropertyChanges {target: cover_obj; width: VLCStyle.icon_xlarge; height: VLCStyle.icon_xlarge}
            //           }
            //           Behavior on height { PropertyAnimation { duration: VLCStyle.timingListExpandOpen } }
            //           Behavior on width { PropertyAnimation { duration: VLCStyle.timingListExpandOpen } }
            //       }
            //       line1: Text{
            //           text: (title || "Unknown title")+" ["+duration+"]"
            //           font.bold: true
            //           elide: Text.ElideRight
            //           color: VLCStyle.textColor
            //           font.pixelSize: VLCStyle.fontSize_normal
            //       }
            //       line2: Text{
            //           text: model.main_artist || "Unknown artist"
            //           elide: Text.ElideRight
            //           color: VLCStyle.textColor
            //           font.pixelSize: VLCStyle.fontSize_xsmall
            //       }
            //       expand: MusicAlbumsGridExpandDelegate {
            //           height: VLCStyle.heightBar_xxlarge
            //       }
            //   }
        }
    }
}
