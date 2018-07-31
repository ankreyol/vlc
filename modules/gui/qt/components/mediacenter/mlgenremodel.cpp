#include "mlgenremodel.hpp"

#include "mlartistmodel.hpp"

namespace {
    enum Roles
    {
        GENRE_ID = Qt::UserRole + 1,
        GENRE_NAME,
        GENRE_NB_TRACKS,
        GENRE_ARTISTS,
        GENRE_TRACKS,
        GENRE_ALBUMS,
    };
}

MLGenreModel::MLGenreModel(QObject *parent)
    : MLSlidingWindowModel<MLGenre>(parent)
{
}

QVariant MLGenreModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0)
        return QVariant();

    const MLGenre* ml_genre = item(static_cast<unsigned int>(index.row()));
    if (!ml_genre)
        return QVariant();

    switch (role)
    {
        // Genres
    case GENRE_ID:
        return QVariant::fromValue( ml_genre->getId() );
    case GENRE_NAME:
        return QVariant::fromValue( ml_genre->getName() );
    case GENRE_NB_TRACKS:
        return QVariant::fromValue( ml_genre->getNbTracks() );
    default :
        return QVariant();
    }
}

QHash<int, QByteArray> MLGenreModel::roleNames() const
{
    return {
        { GENRE_ID, "id" },
        { GENRE_NAME, "name" },
        { GENRE_NB_TRACKS, "nb_tracks" },
        { GENRE_ARTISTS, "artists" },
        { GENRE_TRACKS, "tracks" },
        { GENRE_ALBUMS, "albums" }
    };
}

std::vector<std::unique_ptr<MLGenre>> MLGenreModel::fetch()
{
    ml_unique_ptr<vlc_ml_genre_list_t> genre_list(
        vlc_ml_list_genres(m_ml, &m_query_param)
    );

    std::vector<std::unique_ptr<MLGenre>> res;
    for( const vlc_ml_genre_t& genre: ml_range_iterate<vlc_ml_genre_t>( genre_list ) )
        res.emplace_back( std::unique_ptr<MLGenre>{ new MLGenre( &genre ) } );
    return res;
}

size_t MLGenreModel::countTotalElements() const
{
    auto queryParams = m_query_param;
    queryParams.i_offset = 0;
    queryParams.i_nbResults = 0;
    return vlc_ml_count_genres( m_ml, nullptr );
}

vlc_ml_sorting_criteria_t MLGenreModel::roleToCriteria(int role) const
{
    switch (role)
    {
    case GENRE_NAME:
        return VLC_ML_SORTING_ALPHA;
    default :
        return VLC_ML_SORTING_DEFAULT;
    }
}
