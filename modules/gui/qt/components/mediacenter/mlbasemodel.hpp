#ifndef MLBASEMODEL_HPP
#define MLBASEMODEL_HPP

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "vlc_common.h"

#include <memory>
#include <QObject>
#include <QAbstractListModel>
#include "vlc_media_library.h"

class MLBaseModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit MLBaseModel(std::shared_ptr<vlc_medialibrary_t>& ml, QObject *parent = nullptr);
    explicit MLBaseModel(std::shared_ptr<vlc_medialibrary_t>& ml, vlc_ml_parent_type parent_type, uint64_t parent_id, QObject *parent = nullptr);
    virtual ~MLBaseModel();

    virtual void sort(int column, Qt::SortOrder order) override;

    Q_INVOKABLE void sortByColumn(QByteArray name, Qt::SortOrder order);

signals:
    void sortRoleChanged();

protected:
    virtual void clear() = 0;
    virtual vlc_ml_sorting_criteria_t roleToCriteria(int role) const = 0;
    virtual vlc_ml_sorting_criteria_t nameToCriteria(QByteArray name) const {
        return VLC_ML_SORTING_DEFAULT;
    }

    int m_parent_type = -1;
    uint64_t m_parent_id;

    std::shared_ptr<vlc_medialibrary_t> m_ml;
    int m_sort_role;
    vlc_ml_query_params_t m_query_param;
};

#endif // MLBASEMODEL_HPP
