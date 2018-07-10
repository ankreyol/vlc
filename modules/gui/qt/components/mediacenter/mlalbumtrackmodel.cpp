#include "mlalbumtrackmodel.hpp"

namespace {

enum Role {
    TRACK_TITLE = Qt::UserRole + 1,
    TRACK_COVER,
    TRACK_NUMBER,
    TRACK_DURATION
};

}

MLAlbumTrackModel::MLAlbumTrackModel(std::shared_ptr<vlc_medialibrary_t> &ml, QObject *parent)
    : MLBaseModel( ml, parent )
{
    m_total_count = vlc_ml_count_audio_media(ml.get(), &m_query_param);
}

MLAlbumTrackModel::MLAlbumTrackModel(std::shared_ptr<vlc_medialibrary_t> &ml, vlc_ml_parent_type parent_type, uint64_t parent_id, QObject *parent)
    : MLBaseModel( ml, parent_type, parent_id, parent )
{
    m_total_count = vlc_ml_count_media_of(ml.get(), &m_query_param, m_parent_type, m_parent_id);
}

MLAlbumTrackModel::~MLAlbumTrackModel()
{
    clear();
}

int MLAlbumTrackModel::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid())
        return 0;

    return m_item_list.size();
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
    QHash<int, QByteArray> roles;
    roles[TRACK_TITLE] = "title";
    roles[TRACK_COVER] = "cover";
    roles[TRACK_NUMBER] = "number";
    roles[TRACK_DURATION] = "duration";
    return roles;
}

bool MLAlbumTrackModel::canFetchMore(const QModelIndex &parent) const
{
    return m_item_list.size() < m_total_count;
}

void MLAlbumTrackModel::fetchMore(const QModelIndex &)
{
    ml_unique_ptr<vlc_ml_media_list_t> media_list;

    if ( m_parent_type == -1 )
        media_list.reset( vlc_ml_list_audio_media(m_ml.get(), &m_query_param) );
    else
        media_list.reset( vlc_ml_list_media_of(m_ml.get(), &m_query_param, m_parent_type, m_parent_id ) );

    beginInsertRows(QModelIndex(), m_item_list.size(), m_item_list.size() + media_list->i_nb_items - 1);
    for( const vlc_ml_media_t& media: ml_range_iterate<vlc_ml_media_t>( media_list ) )
        m_item_list.push_back( new MLAlbumTrack( &media) );
    endInsertRows();
}

void MLAlbumTrackModel::clear()
{
    for ( MLAlbumTrack* track : m_item_list )
        delete track;
    m_item_list.clear();
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
        return m_item_list.at(r);
    else
        return NULL;
}
