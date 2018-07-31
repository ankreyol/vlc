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
                model: ListModel {
                    id: sortModel
                    ListElement { text: "Alphabetic asc";  criteria: "title"; desc: Qt.AscendingOrder}
                    ListElement { text: "Alphabetic desc"; criteria: "title"; desc: Qt.DescendingOrder }
                    ListElement { text: "Duration asc";    criteria: "duration"; desc: Qt.AscendingOrder}
                    ListElement { text: "Duration desc";   criteria: "duration"; desc: Qt.DescendingOrder }
                    ListElement { text: "Date asc";        criteria: "release_year"; desc: Qt.AscendingOrder }
                    ListElement { text: "Date desc";       criteria: "release_year"; desc: Qt.DescendingOrder}
                    ListElement { text: "Artist asc";      criteria: "main_artist"; desc: Qt.AscendingOrder }
                    ListElement { text: "Artist desc";     criteria: "main_artist"; desc: Qt.DescendingOrder }
                }
                onActivated: {
                    var sorting = sortModel.get(index);
                    medialib.albums.sortByColumn(sorting.criteria, sorting.desc)
                }
            }
        }

        /* The Presentation Bar */
        //Loader {
        //    id: presentationLoader_id
        //
        //    z:10
        //    Layout.fillWidth: true
        //    height: item.height
        //    Layout.preferredHeight: height
        //    Layout.minimumHeight: height
        //    Layout.maximumHeight: height
        //
        //    sourceComponent: medialib.hasPresentation() ? presentationComponent_id : noPresentationComponent_id
        //
        //    // If the presentation bar should be displayed
        //    Component {
        //        id: presentationComponent_id
        //
        //        Presentation {
        //            height: VLCStyle.heightBar_xlarge
        //            Layout.preferredHeight: height
        //            Layout.minimumHeight: height
        //            Layout.maximumHeight: height
        //
        //            obj: medialib.getPresObject();
        //        }
        //    }
        //    // If the presentation bar should be hidden
        //    Component {
        //        id: noPresentationComponent_id
        //
        //        Item {
        //            height: 0
        //            Layout.preferredHeight: height
        //            Layout.minimumHeight: height
        //            Layout.maximumHeight: height
        //        }
        //    }
        //}

        /* The data elements */
        StackLayout  {
            id: viewLoader

            Layout.fillWidth: true
            //Layout.fillHeight: true
            currentIndex: bar.currentIndex

            // Display some 'Albums' items
            MusicAlbumsDisplay {
                //enabled: viewLoader.currentIndex == 0
                active: viewLoader.currentIndex == 0
            }
            // Display some 'Artists' items
            MusicArtistsDisplay {
                //enabled: viewLoader.currentIndex == 1
                active: viewLoader.currentIndex == 1
            }
            // Display some 'Genres' items
            MusicGenresDisplay {
                //enabled: viewLoader.currentIndex == 2
                active: viewLoader.currentIndex == 2
            }
            // Display some 'Tracks' items
            MusicTracksDisplay {
                //enabled: viewLoader.currentIndex == 3
                active: viewLoader.currentIndex == 3
            }
        }
    }
}
