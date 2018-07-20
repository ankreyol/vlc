#include <cassert>
#include "mlbasemodel.hpp"
#include "mcmedialib.hpp"

MLBaseModel::MLBaseModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_parent_type ( -1 )
    , m_parent_id   (0)
    , m_ml(nullptr)
{
    memset(&m_query_param, 0, sizeof(vlc_ml_query_params_t));
    m_query_param.b_desc = false;
    m_query_param.i_nbResults = 20; //FIXME: value for test
    m_query_param.i_sort = VLC_ML_SORTING_DEFAULT;
}

void MLBaseModel::sort(int column, Qt::SortOrder order)
{
    //setSortRole(column);
    m_query_param.b_desc = (order == Qt::SortOrder::DescendingOrder);
    beginResetModel();
    clear();
    endResetModel();
}

void MLBaseModel::sortByColumn(QByteArray name, Qt::SortOrder order)
{
    m_query_param.b_desc = (order == Qt::SortOrder::DescendingOrder);
    m_query_param.i_sort = nameToCriteria(name);
    clear();
}

ParentType::ParentTypes MLBaseModel::parentType() const
{
    return static_cast<ParentType::ParentTypes>( m_parent_type );
}

void MLBaseModel::setParentType( ParentType::ParentTypes parentType)
{
    // FIXME: Store as the enum
    m_parent_type = static_cast<int>( parentType );
}

MLId MLBaseModel::parentId() const
{
    return m_parent_id;
}

void MLBaseModel::setParentId(MLId parentId)
{
    m_parent_id = parentId;
}

MCMediaLib* MLBaseModel::ml() const
{
    return m_mcMediaLib;
}

void MLBaseModel::setMl(MCMediaLib* mcMl)
{
    m_ml = mcMl->vlcMl();
    m_mcMediaLib = mcMl;
}

int MLBaseModel::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid())
        return 0;
    return nbElementsInModel();
}

bool MLBaseModel::canFetchMore(const QModelIndex&) const
{
    if ( m_initialized == false )
        return true;
    return nbElementsInModel() < m_total_count;
}

void MLBaseModel::fetchMore(const QModelIndex& parent)
{
    if ( m_initialized == false )
    {
        m_total_count = countTotalElements();
        m_initialized = true;
    }
    return fetchMoreInner(parent);
}
