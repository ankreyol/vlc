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

    model: null

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

