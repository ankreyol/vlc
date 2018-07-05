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
        ALBUM_ARTISTS,
        ALBUM_NB_TRACKS,
        ALBUM_DURATION
    };
}

MLAlbumModel::MLAlbumModel(std::shared_ptr<vlc_medialibrary_t> &ml, QObject *parent)
    : MLBaseModel(ml, parent)
{
    reload();
}

int MLAlbumModel::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid())
        return 0;
    printf("*** MLAlbumModel::rowCount %lu\n", m_item_list.size() );
    return m_item_list.size();
}

QVariant MLAlbumModel::data(const QModelIndex &index, int role) const
{
    printf("*** MLAlbumModel::data \n");
    if (!index.isValid())
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
    case ALBUM_ARTISTS :
        return QVariant::fromValue( ml_item->getArtists() );
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
    roles[ALBUM_ARTISTS] = "artists";
    roles[ALBUM_NB_TRACKS] = "nb_tracks";
    roles[ALBUM_DURATION] = "duration";

    return roles;
}

QObject *MLAlbumModel::get(int idx)
{
    printf("Get %i\n");
    if (idx >= m_item_list.size())
        return NULL;
    return m_item_list.at(idx);
}

void MLAlbumModel::reload()
{
    for (MLAlbum* album : m_item_list )
        delete album;
    m_item_list.clear();

    ml_unique_ptr<ml_album_list_t> album_list( ml_list_albums(m_ml.get(), &m_query_param) );
    printf("*** MLAlbumModel::reload %lu \n", album_list->i_nb_items);
    for( const ml_album_t& album: ml_range_iterate<ml_album_t>( album_list ) )
        m_item_list.push_back( new MLAlbum( &album, this ) );
}

ml_sorting_criteria_t MLAlbumModel::roleToCriteria(int role) const
{
    switch (role)
    {
    case ALBUM_TITLE :
        return ML_SORTING_ALPHA;
    case ALBUM_RELEASE_YEAR :
        return ML_SORTING_RELEASEDATE;
    case ALBUM_MAIN_ARTIST :
        return ML_SORTING_ARTIST;
    case ALBUM_DURATION:
        return ML_SORTING_DURATION;
    default:
        return ML_SORTING_DEFAULT;
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
