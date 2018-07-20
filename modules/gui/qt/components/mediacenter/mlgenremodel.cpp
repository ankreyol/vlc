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
    : MLBaseModel(parent)
{
}

QVariant MLGenreModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const MLGenre* ml_genre = getItem(index);
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

void MLGenreModel::fetchMoreInner(const QModelIndex &)
{
    ml_unique_ptr<vlc_ml_genre_list_t> genre_list(
        vlc_ml_list_genres(m_ml, &m_query_param)
    );
    m_query_param.i_offset += m_query_param.i_nbResults;

    beginInsertRows(QModelIndex(), m_item_list.size(), m_item_list.size() + genre_list->i_nb_items - 1);
    for( const vlc_ml_genre_t& genre: ml_range_iterate<vlc_ml_genre_t>( genre_list ) )
        m_item_list.emplace_back( std::unique_ptr<MLGenre>{ new MLGenre( &genre ) } );
    endInsertRows();
}

void MLGenreModel::clear()
{
    m_item_list.clear();
}

size_t MLGenreModel::nbElementsInModel() const
{
    return m_item_list.size();
}

size_t MLGenreModel::countTotalElements() const
{
    return vlc_ml_count_genres( m_ml, &m_query_param );
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

const MLGenre* MLGenreModel::getItem(const QModelIndex &index) const
{
    int r = index.row();
    if (index.isValid())
        return m_item_list.at(r).get();
    return nullptr;
}
