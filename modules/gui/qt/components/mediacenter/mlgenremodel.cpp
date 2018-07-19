#include "mlgenremodel.hpp"

#include "mlartistmodel.hpp"

namespace {
    enum Roles
    {
        GENRE_ID  =Qt::UserRole + 1,
        GENRE_NAME,
        GENRE_NB_TRACKS,
        GENRE_ARTISTS,
        GENRE_TRACKS,
        GENRE_ALBUMS,
    };
}

MLGenreModel::MLGenreModel(vlc_medialibrary_t* ml, QObject *parent)
    : MLBaseModel(ml, parent)
{
}

int MLGenreModel::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid())
        return 0;

    return m_item_list.size();
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
    QHash<int, QByteArray> roles;
    roles[GENRE_ID] = "genre_id";
    roles[GENRE_NAME] = "genre_name";
    roles[GENRE_NB_TRACKS] = "genre_nb_tracks";
    roles[GENRE_ARTISTS] = "genre_artists";
    roles[GENRE_TRACKS] = "genre_tracks";
    roles[GENRE_ALBUMS] = "genre_albums";
    return roles;
}

bool MLGenreModel::canFetchMore(const QModelIndex &) const
{
    return m_item_list.size() < m_total_count;
}

void MLGenreModel::fetchMore(const QModelIndex &)
{
    ml_unique_ptr<vlc_ml_genre_list_t> genre_list(
        vlc_ml_list_genres(m_ml, &m_query_param)
    );

    beginInsertRows(QModelIndex(), m_item_list.size(), m_item_list.size() + genre_list->i_nb_items - 1);
    for( const vlc_ml_genre_t& genre: ml_range_iterate<vlc_ml_genre_t>( genre_list ) )
        m_item_list.emplace_back( new MLGenre( &genre ) );
    endInsertRows();
}

void MLGenreModel::clear()
{
    m_item_list.clear();
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
    else
        return NULL;
}
