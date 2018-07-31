import QtQuick 2.0

import "qrc:///utils/" as Utils
import "qrc:///style/"

import org.videolan.medialib 0.1

Utils.ExpandGridView {
    id: artistGridView
    cellWidth: VLCStyle.cover_normal
    cellHeight: VLCStyle.cover_normal + VLCStyle.fontSize_small + VLCStyle.margin_xsmall

    delegate : Utils.GridItem {
        width: artistGridView.cellWidth
        height: artistGridView.cellHeight

        cover: Utils.ArtistCover {
            albums: MLAlbumModel {
                ml: medialib
                parentId: model.id
                maxItems: 4
            }
            nb_albums: model.nb_albums
        }
        name: model.name || "Unknown Artist"

        onItemClicked: {
            console.log('Clicked on details : '+model.name);
            medialib.select( index );
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
}
