#ifndef MLBASEMODEL_HPP
#define MLBASEMODEL_HPP

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <memory>
#include <QObject>
#include <QAbstractListModel>
#include "vlc_media_library.h"

class MLBaseModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int sortRole READ sortRole WRITE setSortRole NOTIFY sortRoleChanged)

public:
    explicit MLBaseModel(std::shared_ptr<vlc_medialibrary_t>& ml, QObject *parent = nullptr);
    explicit MLBaseModel(std::shared_ptr<vlc_medialibrary_t>& ml, vlc_ml_parent_type parent_type, uint64_t parent_id, QObject *parent = nullptr);
    virtual ~MLBaseModel();

    //virtual void sort(int column, Qt::SortOrder order) override;

    int sortRole() const;
    void setSortRole(int role);

signals:
    void sortRoleChanged();

protected:
    virtual vlc_ml_sorting_criteria_t roleToCriteria(int role) const = 0;

    int m_parent_type = -1;
    uint64_t m_parent_id;

    std::shared_ptr<vlc_medialibrary_t> m_ml;
    int m_sort_role;
    vlc_ml_query_params_t m_query_param;
};

#endif // MLBASEMODEL_HPP
