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

MLArtistModel::MLArtistModel(std::shared_ptr<vlc_medialibrary_t>& ml, QObject *parent)
    : MLBaseModel(ml, parent)
{
    reload();
}

int MLArtistModel::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid())
        return 0;
    return m_item_list.size();
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

void MLArtistModel::reload()
{
    for ( MLArtist* artist : m_item_list )
        delete artist;

    m_item_list.clear();
    ml_unique_ptr<ml_artist_list_t> artist_list( ml_list_artists(m_ml.get(), &m_query_param) );
    for( const ml_artist_t& artist: ml_range_iterate<ml_artist_t>( artist_list ) )
        m_item_list.push_back( new MLArtist( &artist, this ) );
}

ml_sorting_criteria_t MLArtistModel::roleToCriteria(int role) const
{
    switch (role)
    {
    case ARTIST_NAME :
        return ML_SORTING_ALPHA;
    default :
        return ML_SORTING_DEFAULT;
    }
}

const MLArtist* MLArtistModel::getItem(const QModelIndex &index) const
{
    int r = index.row();
    if (index.isValid())
        return m_item_list.at(r);
    else
        return NULL;
}
