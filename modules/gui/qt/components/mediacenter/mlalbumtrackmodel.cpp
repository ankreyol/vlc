#include "mlalbumtrackmodel.hpp"

namespace {

enum Role {
    TRACK_TITLE = Qt::UserRole + 1,
    TRACK_COVER,
    TRACK_NUMBER,
    TRACK_DURATION
};

}

MLAlbumTrackModel::MLAlbumTrackModel(QObject *parent)
    : MLBaseModel(parent)
{
}

QVariant MLAlbumTrackModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const MLAlbumTrack* ml_track = getItem(index);
    if ( !ml_track )
        return QVariant();

    switch (role)
    {
    // Tracks
    case TRACK_TITLE :
        return QVariant::fromValue( ml_track->getTitle() );
    case TRACK_COVER :
        return QVariant::fromValue( ml_track->getCover() );
    case TRACK_NUMBER :
        return QVariant::fromValue( ml_track->getTrackNumber() );
    case TRACK_DURATION :
        return QVariant::fromValue( ml_track->getDuration() );
    default :
        return QVariant();
    }
}

QHash<int, QByteArray> MLAlbumTrackModel::roleNames() const
{
    return {
        { TRACK_TITLE, "title" },
        { TRACK_COVER, "cover" },
        { TRACK_NUMBER, "number" },
        { TRACK_DURATION, "duration" }
    };
}

size_t MLAlbumTrackModel::nbElementsInModel() const
{
    return m_item_list.size();
}

size_t MLAlbumTrackModel::countTotalElements() const
{
    if ( m_parent_id == 0 )
        return vlc_ml_count_audio_media(m_ml, &m_query_param);
    return vlc_ml_count_media_of(m_ml, &m_query_param, m_parent_type, m_parent_id);
}

void MLAlbumTrackModel::fetchMoreInner(const QModelIndex &)
{
    ml_unique_ptr<vlc_ml_media_list_t> media_list;

    if ( m_parent_id == 0 )
        media_list.reset( vlc_ml_list_audio_media(m_ml, &m_query_param) );
    else
        media_list.reset( vlc_ml_list_media_of(m_ml, &m_query_param, m_parent_type, m_parent_id ) );

    m_query_param.i_offset += m_query_param.i_nbResults;
    beginInsertRows(QModelIndex(), m_item_list.size(), m_item_list.size() + media_list->i_nb_items - 1);
    for( const vlc_ml_media_t& media: ml_range_iterate<vlc_ml_media_t>( media_list ) )
        m_item_list.emplace_back( std::unique_ptr<MLAlbumTrack>{ new MLAlbumTrack( &media ) } );
    endInsertRows();
}

void MLAlbumTrackModel::clear()
{
    m_item_list.clear();
    m_query_param.i_offset = 0;
}

vlc_ml_sorting_criteria_t MLAlbumTrackModel::roleToCriteria(int role) const
{
    switch (role) {
    case TRACK_TITLE :
        return VLC_ML_SORTING_ALPHA;
    case TRACK_NUMBER :
        return VLC_ML_SORTING_TRACKNUMBER;
    case TRACK_DURATION :
        return VLC_ML_SORTING_DURATION;
    default:
        return VLC_ML_SORTING_DEFAULT;
    }
}


const MLAlbumTrack* MLAlbumTrackModel::getItem(const QModelIndex &index) const
{
    int r = index.row();
    if (index.isValid())
        return m_item_list.at(r).get();
    return nullptr;
}
