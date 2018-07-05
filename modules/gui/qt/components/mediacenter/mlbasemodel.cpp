#include <cassert>
#include "mlbasemodel.hpp"

MLBaseModel::MLBaseModel(std::shared_ptr<vlc_medialibrary_t> &ml, QObject *parent)
    : QAbstractListModel(parent)
    , m_ml(ml)
{
    assert(ml != nullptr);
    memset(&m_query_param, 0, sizeof(vlc_ml_query_params_t));
    m_query_param.b_desc = false;
    m_query_param.i_sort = VLC_ML_SORTING_DEFAULT;
}

MLBaseModel::~MLBaseModel()
{
}

//void MLBaseModel::sort(int column, Qt::SortOrder order)
//{
//    m_query_param.b_desc = (order == Qt::SortOrder::DescendingOrder) ? true : false;
//    beginResetModel();
//    reload();
//    endResetModel();
//}

int MLBaseModel::sortRole() const
{
    return m_sort_role;
}

void MLBaseModel::setSortRole(int role)
{
    m_sort_role = role;
    m_query_param.i_sort = roleToCriteria(m_sort_role);
    emit sortRoleChanged();
}
