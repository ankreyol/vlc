#include "mlartistmodel.hpp"

namespace {
    enum Roles
    {
        ARTIST_ID  =Qt::UserRole + 1,
        ARTIST_NAME,
        ARTIST_SHORT_BIO,
        ARTIST_COVER,
        ARTIST_NB_ALBUMS
    };
}

MLArtistModel::MLArtistModel(QObject *parent)
    : MLBaseModel(parent)
{
}

QVariant MLArtistModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const MLArtist* ml_artist = getItem(index);
    if ( !ml_artist )
        return QVariant();

    switch (role)
    {
    case ARTIST_ID :
        return QVariant::fromValue( ml_artist->getId() );
    case ARTIST_NAME :
        return QVariant::fromValue( ml_artist->getName() );
    case ARTIST_SHORT_BIO :
        return QVariant::fromValue( ml_artist->getShortBio() );
    case ARTIST_COVER :
        return QVariant::fromValue( ml_artist->getCover() );
    case ARTIST_NB_ALBUMS :
        return QVariant::fromValue( ml_artist->getNbAlbums() );
    default :
        return QVariant();
    }
}

QHash<int, QByteArray> MLArtistModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    // Artists
    roles[ARTIST_ID] = "id";
    roles[ARTIST_NAME] = "name";
    roles[ARTIST_SHORT_BIO] = "short_bio";
    roles[ARTIST_COVER] = "cover";
    roles[ARTIST_NB_ALBUMS] = "nb_albums";
    return roles;
}

void MLArtistModel::fetchMoreInner(const QModelIndex &)
{
    ml_unique_ptr<vlc_ml_artist_list_t> artist_list;

    if ( m_parent_id == 0 )
        artist_list.reset( vlc_ml_list_artists(m_ml, &m_query_param, false) );
    else
        artist_list.reset( vlc_ml_list_artist_of(m_ml, &m_query_param, m_parent_type, m_parent_id.value ) );
    m_query_param.i_offset += 20;

    beginInsertRows(QModelIndex(), m_item_list.size(), m_item_list.size() + artist_list->i_nb_items - 1);
    for( const vlc_ml_artist_t& artist: ml_range_iterate<vlc_ml_artist_t>( artist_list ) )
        m_item_list.emplace_back( new MLArtist( &artist, this ) );
    endInsertRows();
}

void MLArtistModel::clear()
{
    m_query_param.i_offset = 0;
    m_query_param.i_nbResults = 0;
    m_item_list.clear();
    m_total_count = countTotalElements();
}

size_t MLArtistModel::countTotalElements() const
{
    if ( m_parent_id == 0 )
        return vlc_ml_count_artists(m_ml, &m_query_param, false);
    return vlc_ml_count_artists_of(m_ml, &m_query_param, m_parent_type, m_parent_id.value );
}

size_t MLArtistModel::nbElementsInModel() const
{
    return m_item_list.size();
}

vlc_ml_sorting_criteria_t MLArtistModel::roleToCriteria(int role) const
{
    switch (role)
    {
    case ARTIST_NAME :
        return VLC_ML_SORTING_ALPHA;
    default :
        return VLC_ML_SORTING_DEFAULT;
    }
}

const MLArtist* MLArtistModel::getItem(const QModelIndex &index) const
{
    int r = index.row();
    if (index.isValid())
        return m_item_list.at(r).get();
    else
        return NULL;
}
