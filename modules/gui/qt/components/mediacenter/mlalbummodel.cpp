#include "mlalbummodel.hpp"
#include "mcmedialib.hpp"

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

QHash<int, QByteArray> MLAlbumModel::m_role_names = {
    {ALBUM_ID,"id"},
    {ALBUM_TITLE, "title"},
    {ALBUM_RELEASE_YEAR, "release_year"},
    {ALBUM_SHORT_SUMMARY, "shortsummary"},
    {ALBUM_COVER, "cover"},
    {ALBUM_TRACKS, "tracks"},
    {ALBUM_MAIN_ARTIST, "main_artist"},
    {ALBUM_NB_TRACKS, "nb_tracks"},
    {ALBUM_DURATION, "duration"}
};

QHash<QByteArray, vlc_ml_sorting_criteria_t> MLAlbumModel::m_names_to_criteria = {
    {"id", VLC_ML_SORTING_DEFAULT},
    {"title", VLC_ML_SORTING_ALBUM},
    {"release_year", VLC_ML_SORTING_RELEASEDATE},
    {"main_artist", VLC_ML_SORTING_ARTIST},
    //{"nb_tracks"},
    {"duration", VLC_ML_SORTING_DURATION}
};

MLAlbumModel::MLAlbumModel(QObject *parent)
    : MLBaseModel(nullptr, static_cast<vlc_ml_parent_type>( -1 ), 0, parent)
    , m_initialized( false )
{
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
    return m_role_names;
}

QObject *MLAlbumModel::get(unsigned int idx)
{
    if (idx >= m_item_list.size())
        return NULL;
    return m_item_list.at(idx);
}

bool MLAlbumModel::canFetchMore(const QModelIndex &) const
{
    if ( m_initialized == false )
        return true;
    return m_item_list.size() < m_total_count;
}

void MLAlbumModel::fetchMore(const QModelIndex &)
{
    if ( m_initialized == false )
    {
        if ( m_parent_id == 0 )
            m_total_count = vlc_ml_count_albums(m_ml, &m_query_param);
        else
            m_total_count = vlc_ml_count_albums_of(m_ml, &m_query_param,
                                                   m_parent_type, m_parent_id);
        m_initialized = true;
    }
    ml_unique_ptr<vlc_ml_album_list_t> album_list;
    if ( m_parent_id == 0 )
        album_list.reset( vlc_ml_list_albums(m_ml, &m_query_param) );
    else
        album_list.reset( vlc_ml_list_albums_of(m_ml, &m_query_param, m_parent_type, m_parent_id) );

    beginInsertRows(QModelIndex(), m_item_list.size(), m_item_list.size() + album_list->i_nb_items - 1);
    for( const vlc_ml_album_t& album: ml_range_iterate<vlc_ml_album_t>( album_list ) )
        m_item_list.push_back( new MLAlbum( m_ml, &album, this ) );
    m_query_param.i_offset += album_list->i_nb_items;
    endInsertRows();
}

ParentType::ParentTypes MLAlbumModel::parentType() const
{
    return static_cast<ParentType::ParentTypes>( m_parent_type );
}

void MLAlbumModel::setParentType( ParentType::ParentTypes parentType)
{
    // FIXME: Store as the enum
    m_parent_type = static_cast<int>( parentType );
}

int64_t MLAlbumModel::parentId() const
{
    return m_parent_id;
}

void MLAlbumModel::setParentId(int64_t parentId)
{
    m_parent_id = parentId;
}

MCMediaLib* MLAlbumModel::ml() const
{
    return m_mcMediaLib;
}

void MLAlbumModel::setMl(MCMediaLib* mcMl)
{
    m_ml = mcMl->vlcMl();
    m_mcMediaLib = mcMl;
}

void MLAlbumModel::clear()
{
    beginResetModel();
    for ( MLAlbum* album : m_item_list )
        delete album;
    m_item_list.clear();
    m_query_param.i_offset = 0;
    endResetModel();
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
        return m_item_list.at(r);
    else
        return NULL;
}
