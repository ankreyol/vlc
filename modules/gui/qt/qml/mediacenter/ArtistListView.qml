import QtQuick 2.0
import QtQuick.Controls 2.0

import "qrc:///utils/" as Utils
import "qrc:///style/"

ListView {
    id: artistListView
    property var onItemClicked

    spacing: 2
    delegate : Utils.ListItem {
        height: VLCStyle.icon_normal
        width: parent.width

        cover: Image {
            height: VLCStyle.icon_normal
            width: VLCStyle.icon_normal

            source: model.cover || VLCStyle.noArtCover
        }
        line1: Text{
            text: model.name || "Unknown artist"
            font.bold: true
            elide: Text.ElideRight
            color: VLCStyle.textColor
        }

        onItemClicked: {
            artistListView.onItemClicked(model)
        }

        onPlayClicked: {
            console.log('Clicked on play : '+model.name);
            medialib.addAndPlay( index )
        }
        onAddToPlaylistClicked: {
            console.log('Clicked on addToPlaylist : '+model.name);
            medialib.addToPlaylist( index );
        }
    }

    ScrollBar.vertical: ScrollBar { }
}
