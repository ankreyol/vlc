#include <cassert>
#include "mlbasemodel.hpp"

MLBaseModel::MLBaseModel(std::shared_ptr<vlc_medialibrary_t> &ml, QObject *parent)
    : QAbstractListModel(parent)
    , m_ml(ml)
{
    assert(ml != nullptr);
    memset(&m_query_param, 0, sizeof(vlc_ml_query_params_t));
    m_query_param.b_desc = false;
    m_query_param.i_nbResults = 20; //FIXME: value for test
    m_query_param.i_sort = VLC_ML_SORTING_DEFAULT;
}

MLBaseModel::MLBaseModel(std::shared_ptr<vlc_medialibrary_t> &ml, vlc_ml_parent_type parent_type, uint64_t parent_id, QObject *parent)
    : QAbstractListModel(parent)
    , m_ml(ml)
    , m_parent_type (parent_type)
    , m_parent_id   (parent_id)
{
    assert(ml != nullptr);
    memset(&m_query_param, 0, sizeof(vlc_ml_query_params_t));
    m_query_param.b_desc = false;
    m_query_param.i_nbResults = 20; //FIXME: value for test
    m_query_param.i_sort = VLC_ML_SORTING_DEFAULT;
}

MLBaseModel::~MLBaseModel()
{
}

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
