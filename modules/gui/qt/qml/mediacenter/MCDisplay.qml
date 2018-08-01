/*****************************************************************************
 * MCDisplay.qml : The main component to display the mediacenter
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
import QtQuick.Layouts 1.3
import "qrc:///style/"

import org.videolan.medialib 0.1

Rectangle {
    // Notify the view has beeen changed
    function changedView() {
        viewLoader.item.changedView();
    }

    // Notify the category has been changed
    function changedCategory() {
        viewLoader.sourceComponent = chooseCat();
        reloadData();
        console.log( "Changed category : "+medialib.category );
    }

    // Force the data inside the displayed view to de reloaded
    function reloadData() {
        viewLoader.item.reloadData();
    }

    // Force to recalculate if the presentation needs to be displayed
    function reloadPresentation() {
        if ( medialib.hasPresentation() ) {
            presentationLoader_id.sourceComponent = presentationComponent_id;
        } else {
            presentationLoader_id.sourceComponent = noPresentationComponent_id;
       }
    }

    color: VLCStyle.bgColor

    ColumnLayout {
        anchors.fill : parent

        RowLayout {

            anchors.left: parent.left
            anchors.right: parent.right
            TabBar {
                id: bar
                anchors.left: parent.left
                anchors.right: parent.right
                /* List of sub-sources for Music */
                Repeater {
                    id: model_music_id

                    model: ListModel {
                        ListElement { displayText: "Albums" ; name: "music-albums" }
                        ListElement { displayText: "Artistes" ; name: "music-artists" }
                        ListElement { displayText: "Genre" ; name: "music-genre" }
                        ListElement { displayText: "Tracks" ; name: "music-tracks" }
                    }

                    //Column {
                    TabButton {
                        id: control
                        text: model.displayText
                        background: Rectangle {
                            color: control.hovered ? VLCStyle.hoverBannerColor : VLCStyle.bannerColor
                        }
                        contentItem: Label {
                            text: control.text
                            font: control.font
                            color:  control.checked ?
                                        VLCStyle.textColor_activeSource :
                                        (control.hovered ?  VLCStyle.textColor_activeSource : VLCStyle.textColor)
                            verticalAlignment: Text.AlignVCenter
                            horizontalAlignment: Text.AlignHCenter
                        }
                    }
                }
            }

            /* Spacer */
            Item {
                Layout.fillWidth: true
            }

            /* Selector to choose a specific sorting operation */
            ComboBox {
                id: combo

                //Layout.fillHeight: true
                Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                Layout.preferredWidth: width
                width: VLCStyle.widthSortBox
                height: parent.height
                textRole: "text"
                model: viewLoader.children[viewLoader.currentIndex].sortModel
                onCurrentIndexChanged: {
                    var sorting = model.get(currentIndex);
                    viewLoader.children[viewLoader.currentIndex].model.sortByColumn(sorting.criteria, sorting.desc)
                }
            }
        }


        /* The data elements */
        StackLayout  {
            id: viewLoader
            Layout.fillWidth: true
            //Layout.fillHeight: true
            currentIndex: bar.currentIndex

            // Display some 'Albums' items
            MusicAlbumsDisplay {
                active: viewLoader.currentIndex == 0

                model: MLAlbumModel {
                    ml: medialib
                }

                property var sortModel: ListModel {
                    ListElement { text: qsTr("Alphabetic asc");  criteria: "title"; desc: Qt.AscendingOrder}
                    ListElement { text: qsTr("Alphabetic desc"); criteria: "title"; desc: Qt.DescendingOrder }
                    ListElement { text: qsTr("Duration asc");    criteria: "duration"; desc: Qt.AscendingOrder}
                    ListElement { text: qsTr("Duration desc");   criteria: "duration"; desc: Qt.DescendingOrder }
                    ListElement { text: qsTr("Date asc");        criteria: "release_year"; desc: Qt.AscendingOrder }
                    ListElement { text: qsTr("Date desc");       criteria: "release_year"; desc: Qt.DescendingOrder}
                    ListElement { text: qsTr("Artist asc");      criteria: "main_artist"; desc: Qt.AscendingOrder }
                    ListElement { text: qsTr("Artist desc");     criteria: "main_artist"; desc: Qt.DescendingOrder }
                }
            }

            // Display some 'Artists' items
            MusicArtistsDisplay {
                active: viewLoader.currentIndex == 1
                model: MLArtistModel {
                    ml: medialib
                }

                property var sortModel: ListModel {
                    ListElement { text: qsTr("Alphabetic asc");  criteria: "title"; desc: Qt.AscendingOrder}
                    ListElement { text: qsTr("Alphabetic desc"); criteria: "title"; desc: Qt.DescendingOrder }
                }
            }
            // Display some 'Genres' items
            MusicGenresDisplay {
                active: viewLoader.currentIndex == 2

                model: MLGenreModel {
                    ml: medialib
                }

                property var sortModel: ListModel {
                    ListElement { text: qsTr("Alphabetic asc");  criteria: "title"; desc: Qt.AscendingOrder}
                    ListElement { text: qsTr("Alphabetic desc"); criteria: "title"; desc: Qt.DescendingOrder }
                }
            }
            // Display some 'Tracks' items
            MusicTracksDisplay {
                active: viewLoader.currentIndex == 3

                model:  MLAlbumTrackModel {
                    ml: medialib
                }

                property var sortModel: ListModel {
                    ListElement { text: qsTr("Alphabetic asc");  criteria: "title"; desc: Qt.AscendingOrder}
                    ListElement { text: qsTr("Alphabetic desc"); criteria: "title"; desc: Qt.DescendingOrder }
                    ListElement { text: qsTr("Album asc");      criteria: "album_title"; desc: Qt.AscendingOrder }
                    ListElement { text: qsTr("Album desc");     criteria: "album_title"; desc: Qt.DescendingOrder }
                    ListElement { text: qsTr("Artist asc");      criteria: "main_artist"; desc: Qt.AscendingOrder }
                    ListElement { text: qsTr("Artist desc");     criteria: "main_artist"; desc: Qt.DescendingOrder }
                    ListElement { text: qsTr("Date asc");        criteria: "release_year"; desc: Qt.AscendingOrder }
                    ListElement { text: qsTr("Date desc");       criteria: "release_year"; desc: Qt.DescendingOrder}
                    ListElement { text: qsTr("Duration asc");    criteria: "duration"; desc: Qt.AscendingOrder}
                    ListElement { text: qsTr("Duration desc");   criteria: "duration"; desc: Qt.DescendingOrder }
                    ListElement { text: qsTr("Track number asc");  criteria: "track_number"; desc: Qt.AscendingOrder}
                    ListElement { text: qsTr("Track number desc"); criteria: "track_number"; desc: Qt.DescendingOrder }
                }
            }
        }
    }
}
