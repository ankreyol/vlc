import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.1
import org.videolan.medialib 0.1
import "qrc:///utils/" as Utils
import "qrc:///style/"

/* The list of the tracks available */
ListView {
    id: expand_track_id

    spacing: VLCStyle.margin_xxxsmall
    clip: true
    focus: true
    ScrollBar.vertical: ScrollBar { }

    model: null

    header: Rectangle {
        height: titleheader.height * 2
        width: parent.width
        color: VLCStyle.bgColor

        RowLayout{
            anchors.fill: parent
            anchors.verticalCenter: parent

            Text {
                id: titleheader
                Layout.preferredWidth: parent.width / 4
                text: qsTr("TITLE")
                elide: Text.ElideRight
                font.bold:  true
                font.pixelSize: VLCStyle.fontSize_normal
                color: VLCStyle.textColor
            }

            Text {
                Layout.preferredWidth: parent.width / 4
                text: qsTr("ARTIST")
                elide: Text.ElideRight
                font.bold:  true
                font.pixelSize: VLCStyle.fontSize_normal
                color: VLCStyle.textColor
            }

            Text {
                Layout.preferredWidth: parent.width / 4
                text: qsTr("ALBUM")
                elide: Text.ElideRight
                font.bold:  true
                font.pixelSize: VLCStyle.fontSize_normal
                color: VLCStyle.textColor
            }

            Text {
                Layout.preferredWidth: parent.width / 4
                id: duration_id
                text: qsTr("DURATION")
                elide: Text.ElideRight
                font.bold:  true
                font.pixelSize: VLCStyle.fontSize_normal
                color: VLCStyle.textColor
            }
        }
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

        RowLayout{
            anchors.fill: parent

            Text {
                Layout.preferredWidth: parent.width / 4
                id: txt_id
                text: (model.title || "Unknown track")
                elide: Text.ElideRight
                font.pixelSize: VLCStyle.fontSize_normal
                color: VLCStyle.textColor
            }

            Text {
                Layout.preferredWidth: parent.width / 4
                id: artist_id
                text: model.main_artist
                elide: Text.ElideRight
                font.pixelSize: VLCStyle.fontSize_normal
                color: VLCStyle.textColor
            }

            Text {
                Layout.preferredWidth: parent.width / 4
                id: album_id
                text: model.album_title
                elide: Text.ElideRight
                font.pixelSize: VLCStyle.fontSize_normal
                color: VLCStyle.textColor
            }

            Text {
                Layout.preferredWidth: parent.width / 4
                id: duration_id
                text: model.duration
                elide: Text.ElideRight
                font.pixelSize: VLCStyle.fontSize_normal
                color: VLCStyle.textColor
            }
        }
    }
}
