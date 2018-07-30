#include "mlalbummodel.hpp"

namespace {
    enum Roles
    {
        ALBUM_ID = Qt::UserRole + 1,
        ALBUM_TITLE,
        ALBUM_RELEASE_YEAR,
        ALBUM_SHORT_SUMMARY,
        ALBUM_COVER,
        ALBUM_MAIN_ARTIST,
        ALBUM_NB_TRACKS,
        ALBUM_DURATION
    };
}

QHash<QByteArray, vlc_ml_sorting_criteria_t> MLAlbumModel::m_names_to_criteria = {
    {"id", VLC_ML_SORTING_DEFAULT},
    {"title", VLC_ML_SORTING_ALBUM},
    {"release_year", VLC_ML_SORTING_RELEASEDATE},
    {"main_artist", VLC_ML_SORTING_ARTIST},
    //{"nb_tracks"},
    {"duration", VLC_ML_SORTING_DURATION}
};

MLAlbumModel::MLAlbumModel(QObject *parent)
    : MLBaseModel(parent)
{
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
        return QVariant::fromValue( ml_item->getParentId() );
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
    case ALBUM_DURATION:
        return QVariant::fromValue( ml_item->getDuration() );
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> MLAlbumModel::roleNames() const
{
    return {
        {ALBUM_ID,"id"},
        {ALBUM_TITLE, "title"},
        {ALBUM_RELEASE_YEAR, "release_year"},
        {ALBUM_SHORT_SUMMARY, "shortsummary"},
        {ALBUM_COVER, "cover"},
        {ALBUM_MAIN_ARTIST, "main_artist"},
        {ALBUM_NB_TRACKS, "nb_tracks"},
        {ALBUM_DURATION, "duration"}
    };
}

QObject *MLAlbumModel::get(unsigned int idx)
{
    if (idx >= m_item_list.size())
        return NULL;
    return m_item_list.at(idx).get();
}

void MLAlbumModel::fetchMoreInner(const QModelIndex &)
{
    ml_unique_ptr<vlc_ml_album_list_t> album_list;
    if ( m_parent.id == 0 )
        album_list.reset( vlc_ml_list_albums(m_ml, &m_query_param) );
    else
        album_list.reset( vlc_ml_list_albums_of(m_ml, &m_query_param, m_parent.type, m_parent.id ) );

    beginInsertRows(QModelIndex(), m_item_list.size(), m_item_list.size() + album_list->i_nb_items - 1);
    for( const vlc_ml_album_t& album: ml_range_iterate<vlc_ml_album_t>( album_list ) )
        m_item_list.emplace_back( new MLAlbum( m_ml, &album, this ) );
    m_query_param.i_offset += album_list->i_nb_items;
    endInsertRows();
}

void MLAlbumModel::clear()
{
    beginResetModel();
    m_item_list.clear();
    m_query_param.i_offset = 0;
    endResetModel();
}

size_t MLAlbumModel::nbElementsInModel() const
{
    return m_item_list.size();
}

vlc_ml_sorting_criteria_t MLAlbumModel::nameToCriteria(QByteArray name) const
{
    return m_names_to_criteria.value(name, VLC_ML_SORTING_DEFAULT);
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
        return m_item_list.at(r).get();
    else
        return NULL;
}

size_t MLAlbumModel::countTotalElements() const
{
    if ( m_parent.id == 0 )
        return vlc_ml_count_albums(m_ml, &m_query_param);
    return vlc_ml_count_albums_of(m_ml, &m_query_param, m_parent.type, m_parent.id);
}
