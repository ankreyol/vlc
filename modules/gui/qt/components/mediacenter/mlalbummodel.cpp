#include "mlalbummodel.hpp"

namespace {
    enum Roles
    {
        ALBUM_ID = Qt::UserRole + 1,
        ALBUM_TITLE,
        ALBUM_RELEASE_YEAR,
        ALBUM_SHORT_SUMMARY,
        ALBUM_COVER,
        ALBUM_TRACKS,
        ALBUM_MAIN_ARTIST,
        ALBUM_NB_TRACKS,
        ALBUM_DURATION
    };
}

MLAlbumModel::MLAlbumModel(std::shared_ptr<vlc_medialibrary_t> &ml, QObject *parent)
    : MLBaseModel(ml, parent)
{
    m_query_param.i_nbResults = 10;
    m_total_count = vlc_ml_count_albums(ml.get(), &m_query_param);
}

MLAlbumModel::MLAlbumModel(std::shared_ptr<vlc_medialibrary_t> &ml, vlc_ml_parent_type parent_type, uint64_t parent_id, QObject *parent)
    : MLBaseModel(ml, parent_type, parent_id, parent)
{
    m_query_param.i_nbResults = 10;
    m_total_count = vlc_ml_count_albums_of(ml.get(), &m_query_param, m_parent_type, m_parent_id);
}

MLAlbumModel::~MLAlbumModel()
{
    clear();
}

int MLAlbumModel::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid())
        return 0;
    return m_item_list.size();
}

QVariant MLAlbumModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= m_item_list.size() || index.row() < 0)
            return QVariant();

    const MLAlbum* ml_item = getItem(index);
    if ( ml_item == NULL )
        return QVariant();

    switch (role)
    {
    case ALBUM_ID :
        return QVariant::fromValue( ml_item->getId() );
    case ALBUM_TITLE :
        return QVariant::fromValue( ml_item->getTitle() );
    case ALBUM_RELEASE_YEAR :
        return QVariant::fromValue( ml_item->getReleaseYear() );
    case ALBUM_SHORT_SUMMARY :
        return QVariant::fromValue( ml_item->getShortSummary() );
    case ALBUM_COVER :
        return QVariant::fromValue( ml_item->getCover() );
    case ALBUM_MAIN_ARTIST :
        return QVariant::fromValue( ml_item->getArtist() );
    case ALBUM_NB_TRACKS :
        return QVariant::fromValue( ml_item->getNbTracks() );
    case ALBUM_TRACKS :
        return QVariant::fromValue( (QObject*)ml_item->getTracks());
    case ALBUM_DURATION:
        return QVariant::fromValue( ml_item->getDuration() );
    default:
        return QVariant();
    }
}


QHash<int, QByteArray> MLAlbumModel::roleNames() const
{
    printf("*** MLAlbumModel::roleNames \n");
    QHash<int, QByteArray> roles;

    // Albums
    roles[ALBUM_ID] = "id";
    roles[ALBUM_TITLE] = "title";
    roles[ALBUM_RELEASE_YEAR] = "release_year";
    roles[ALBUM_SHORT_SUMMARY] = "shortsummary";
    roles[ALBUM_COVER] = "cover";
    roles[ALBUM_TRACKS] = "tracks";
    roles[ALBUM_MAIN_ARTIST] = "main_artist";
    roles[ALBUM_NB_TRACKS] = "nb_tracks";
    roles[ALBUM_DURATION] = "duration";

    return roles;
}

QObject *MLAlbumModel::get(unsigned int idx)
{
    if (idx >= m_item_list.size())
        return NULL;
    return m_item_list.at(idx);
}

bool MLAlbumModel::canFetchMore(const QModelIndex &parent) const
{
    return m_item_list.size() < m_total_count;
}

void MLAlbumModel::fetchMore(const QModelIndex &)
{
    ml_unique_ptr<vlc_ml_album_list_t> album_list;
    if ( m_parent_type == -1 )
        album_list.reset( vlc_ml_list_albums(m_ml.get(), &m_query_param) );
    else
        album_list.reset( vlc_ml_list_albums_of(m_ml.get(), &m_query_param, m_parent_type, m_parent_id) );

    beginInsertRows(QModelIndex(), m_item_list.size(), m_item_list.size() + album_list->i_nb_items - 1);
    for( const vlc_ml_album_t& album: ml_range_iterate<vlc_ml_album_t>( album_list ) )
        m_item_list.push_back( new MLAlbum( m_ml, &album, this ) );
    m_query_param.i_offset += album_list->i_nb_items;
    endInsertRows();
}

void MLAlbumModel::clear()
{
    for ( MLAlbum* album : m_item_list )
        delete album;
    m_item_list.clear();
}

vlc_ml_sorting_criteria_t MLAlbumModel::roleToCriteria(int role) const
{
    switch (role)
    {
    case ALBUM_TITLE :
        return VLC_ML_SORTING_ALPHA;
    case ALBUM_RELEASE_YEAR :
        return VLC_ML_SORTING_RELEASEDATE;
    case ALBUM_MAIN_ARTIST :
        return VLC_ML_SORTING_ARTIST;
    case ALBUM_DURATION:
        return VLC_ML_SORTING_DURATION;
    default:
        return VLC_ML_SORTING_DEFAULT;
    }
}

const MLAlbum* MLAlbumModel::getItem(const QModelIndex &index) const
{
    int r = index.row();
    if (index.isValid())
        return m_item_list.at(r);
    else
        return NULL;
}
