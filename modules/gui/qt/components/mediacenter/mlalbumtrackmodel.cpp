#include "mlalbumtrackmodel.hpp"

namespace {

enum Role {
    TRACK_TITLE = Qt::UserRole + 1,
    TRACK_COVER,
    TRACK_NUMBER,
    TRACK_DURATION
};

}

QHash<QByteArray, vlc_ml_sorting_criteria_t> MLAlbumTrackModel::M_names_to_criteria = {
    {"id", VLC_ML_SORTING_DEFAULT},
    {"title", VLC_ML_SORTING_ALPHA},
    {"track_number", VLC_ML_SORTING_TRACKNUMBER},
    {"release_year", VLC_ML_SORTING_RELEASEDATE},
    {"main_artist", VLC_ML_SORTING_ARTIST},
    {"duration", VLC_ML_SORTING_DURATION}
};

MLAlbumTrackModel::MLAlbumTrackModel(QObject *parent)
    : MLSlidingWindowModel<MLAlbumTrack>(parent)
{
}

QVariant MLAlbumTrackModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0)
        return QVariant();

    const MLAlbumTrack* ml_track = item(static_cast<unsigned int>(index.row()));
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

size_t MLAlbumTrackModel::countTotalElements() const
{
    auto queryParams = m_query_param;
    queryParams.i_offset = 0;
    queryParams.i_nbResults = 0;
    if ( m_parent.id <= 0 )
        return vlc_ml_count_audio_media(m_ml, &queryParams);
    return vlc_ml_count_media_of(m_ml, &queryParams, m_parent.type, m_parent.id );
}

std::vector<std::unique_ptr<MLAlbumTrack>> MLAlbumTrackModel::fetch()
{
    ml_unique_ptr<vlc_ml_media_list_t> media_list;

    if ( m_parent.id <= 0 )
        media_list.reset( vlc_ml_list_audio_media(m_ml, &m_query_param) );
    else
        media_list.reset( vlc_ml_list_media_of(m_ml, &m_query_param, m_parent.type, m_parent.id ) );

    std::vector<std::unique_ptr<MLAlbumTrack>> res;
    for( const vlc_ml_media_t& media: ml_range_iterate<vlc_ml_media_t>( media_list ) )
        res.emplace_back( std::unique_ptr<MLAlbumTrack>{ new MLAlbumTrack( &media ) } );
    return res;
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

vlc_ml_sorting_criteria_t MLAlbumTrackModel::nameToCriteria(QByteArray name) const
{
    return M_names_to_criteria.value(name, VLC_ML_SORTING_DEFAULT);
}
