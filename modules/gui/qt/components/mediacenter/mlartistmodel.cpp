#include "mlartistmodel.hpp"

namespace {
    enum Roles
    {
        ARTIST_ID = Qt::UserRole + 1,
        ARTIST_NAME,
        ARTIST_SHORT_BIO,
        ARTIST_COVER,
        ARTIST_NB_ALBUMS
    };
}

MLArtistModel::MLArtistModel(QObject *parent)
    : MLSlidingWindowModel(parent)
{
}

QVariant MLArtistModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0)
        return QVariant();

    const MLArtist* ml_artist = item(static_cast<unsigned int>(index.row()));
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
    return {
        { ARTIST_ID, "id" },
        { ARTIST_NAME, "name" },
        { ARTIST_SHORT_BIO, "short_bio" },
        { ARTIST_COVER, "cover" },
        { ARTIST_NB_ALBUMS, "nb_albums" },
    };
}

std::vector<std::unique_ptr<MLArtist>> MLArtistModel::fetch()
{
    ml_unique_ptr<vlc_ml_artist_list_t> artist_list;
    if ( m_parent.id <= 0 )
        artist_list.reset( vlc_ml_list_artists(m_ml, &m_query_param, false) );
    else
        artist_list.reset( vlc_ml_list_artist_of(m_ml, &m_query_param, m_parent.type, m_parent.id) );

    std::vector<std::unique_ptr<MLArtist>> res;
    for( const vlc_ml_artist_t& artist: ml_range_iterate<vlc_ml_artist_t>( artist_list ) )
        res.emplace_back( new MLArtist( &artist, this ) );
    return res;
}

size_t MLArtistModel::countTotalElements() const
{
    auto queryParams = m_query_param;
    queryParams.i_offset = 0;
    queryParams.i_nbResults = 0;

    if ( m_parent.id <= 0 )
        return vlc_ml_count_artists(m_ml, &m_query_param, false);
    return vlc_ml_count_artists_of(m_ml, &m_query_param, m_parent.type, m_parent.id );
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
