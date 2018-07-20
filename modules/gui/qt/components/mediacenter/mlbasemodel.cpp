#include <cassert>
#include "mlbasemodel.hpp"

MLBaseModel::MLBaseModel(vlc_medialibrary_t* ml, vlc_ml_parent_type parent_type, uint64_t parent_id, QObject *parent)
    : QAbstractListModel(parent)
    , m_parent_type (parent_type)
    , m_parent_id   (parent_id)
    , m_ml(ml)
{
    memset(&m_query_param, 0, sizeof(vlc_ml_query_params_t));
    m_query_param.b_desc = false;
    m_query_param.i_nbResults = 20; //FIXME: value for test
    m_query_param.i_sort = VLC_ML_SORTING_DEFAULT;
}

MLBaseModel::MLBaseModel(vlc_medialibrary_t* ml, QObject *parent)
    : MLBaseModel(ml, static_cast<vlc_ml_parent_type>( -1 ), 0, parent)
{
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
