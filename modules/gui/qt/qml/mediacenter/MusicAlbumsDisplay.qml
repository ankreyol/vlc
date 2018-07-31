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

            GridView {
                property int currentId: -1;
                onCurrentIdChanged: {
                    footer_overlay.model = gridView_id.model.get(gridView_id.currentId)
                }

                ScrollBar.vertical: ScrollBar { }
                anchors.fill: parent
                clip: true

                model: viewLoader.model
                id: gridView_id

                cellWidth: VLCStyle.cover_normal
                cellHeight: VLCStyle.cover_normal + VLCStyle.fontSize_small + VLCStyle.margin_xsmall


                delegate: Utils.GridItem {
                    width: gridView_id.cellWidth
                    height: gridView_id.cellHeight

                    cover : Image { source: model.cover || VLCStyle.noArtCover }
                    name : model.title || "Unknown title"
                    date : model.release_year !== "0" ? model.release_year : ""
                    infos : model.duration + " - " + model.nb_tracks + " tracks"

                    onItemClicked : {
                        currentId =  (currentId === index) ? -1 : index
                        console.log('Clicked on details : '+ model.title + " " + currentId)
                    }
                    onPlayClicked: {
                        console.log('Clicked on play : '+model.title);
                        medialib.addAndPlay(currentIndex)
                    }
                    onAddToPlaylistClicked : {
                        console.log('Clicked on addToPlaylist : '+model.title);
                        medialib.addToPlaylist(currentIndex);
                    }
                }

                footer:  Rectangle {
                    visible: currentId == null
                    height: currentId == null ? 0 : footer_overlay.height
                }

                MusicAlbumsGridExpandDelegate {
                    id: footer_overlay
                    height: VLCStyle.heightBar_xxlarge
                    width: gridView_id.width
                    anchors.bottom: parent.bottom
                    visible: false
                }

                states: [
                    State {
                        name: "DETAILS_HIDDEN"
                        PropertyChanges { target: footer_overlay; height: 0; visible: false }
                        when: gridView_id.currentId < 0
                    },
                    State {
                        name: "DETAILS_VISIBLE"
                        PropertyChanges { target: footer_overlay; height: VLCStyle.heightBar_xxlarge; visible: true}
                        when: gridView_id.currentId >= 0
                    }
                ]

                transitions: [
                    Transition {
                        from: "DETAILS_HIDDEN"
                        to: "DETAILS_VISIBLE"
                        SequentialAnimation {
                            PropertyAnimation { properties: "visible" }
                            NumberAnimation { properties: "height"; easing.type: Easing.InOutQuad }
                        }
                    },
                    Transition {
                        from: "DETAILS_VISIBLE"
                        to: "DETAILS_HIDDEN"
                        SequentialAnimation {
                            NumberAnimation { properties: "height"; easing.type: Easing.InOutQuad }
                            PropertyAnimation { properties: "visible" }
                        }
                    }
                ]
            }
    }

    /* ListView */
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
        }
    }
}
