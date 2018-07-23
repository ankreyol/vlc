#include <cassert>
#include "mlbasemodel.hpp"
#include "mcmedialib.hpp"

MLBaseModel::MLBaseModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_ml(nullptr)
    , m_initialized(false)
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

MLParentId MLBaseModel::parentId() const
{
    return m_parent;
}

void MLBaseModel::setParentId(MLParentId parentId)
{
    m_parent = parentId;
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
