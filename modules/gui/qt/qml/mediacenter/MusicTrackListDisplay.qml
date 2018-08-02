import QtQuick 2.0
import QtQuick.Controls 1.4 as QC14
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.1
import org.videolan.medialib 0.1
import "qrc:///utils/" as Utils
import "qrc:///style/"

QC14.TableView
{
    sortIndicatorVisible: true
    selectionMode: QC14.SelectionMode.ExtendedSelection

    model: MLAlbumTrackModel {
        id: albumModel
        ml: medialib
        onParentIdChanged: {
            selection.clear()
        }
    }
    property alias parentId: albumModel.parentId

    property var columnModel: ListModel {
        ListElement{ role: "title"; title: qsTr("TITLE") }
        ListElement{ role: "main_artist"; title: qsTr("ARTIST") }
        ListElement{ role: "album_title"; title: qsTr("ALBUM") }
        ListElement{ role: "duration"; title: qsTr("DURATION") }
    }

    Component {
        id: tablecolumn_model
        QC14.TableViewColumn {
        }
    }

    Component.onCompleted: {
        for( var i=0; i < columnModel.count; i++ )
        {
            var col = addColumn(tablecolumn_model)
            col.role = columnModel.get(i).role
            col.title = columnModel.get(i).title
        }
    }

    itemDelegate: Text {
        text: styleData.value
        elide: Text.ElideRight
        font.pixelSize: VLCStyle.fontSize_normal
        color: VLCStyle.textColor
    }

    headerDelegate: Rectangle {
        height: textItem.implicitHeight * 1.2
        color: VLCStyle.buttonColor

        Text {
            id: textItem
            text: styleData.value
            elide: Text.ElideRight
            font {
                bold: true
                pixelSize: VLCStyle.fontSize_normal
            }

            anchors {
                fill: parent
                leftMargin: VLCStyle.margin_xxsmall
                rightMargin: VLCStyle.margin_xxsmall
            }
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignLeft

            color: VLCStyle.buttonTextColor
        }

        Text {
            anchors {
                right: parent.right
                leftMargin: VLCStyle.margin_xxsmall
                rightMargin: VLCStyle.margin_xxsmall
            }
            visible: styleData.column === sortIndicatorColumn
            text: sortIndicatorOrder === Qt.AscendingOrder ? "⯆" : "⯅"
            color: VLCStyle.vlc_orange
        }
        //right handle
        Rectangle {
            color: VLCStyle.buttonBorderColor
            height: parent.height * 0.8
            width: 1
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
        }
        //line below
        Rectangle {
            color: VLCStyle.buttonBorderColor
            height: 1
            width: parent.width
            anchors.bottom: parent.bottom
        }
    }

    onSortIndicatorColumnChanged: {
        model.sortByColumn(getColumn(sortIndicatorColumn).role, sortIndicatorOrder)
    }
    onSortIndicatorOrderChanged: {
        model.sortByColumn(getColumn(sortIndicatorColumn).role, sortIndicatorOrder)
    }

    //section.property : "title"
    //section.criteria: ViewSection.FirstCharacter
    //section.delegate: Text {
    //    text: section
    //    elide: Text.ElideRight
    //    font.pixelSize: VLCStyle.fontSize_xlarge
    //    color: VLCStyle.textColor
    //}
}

