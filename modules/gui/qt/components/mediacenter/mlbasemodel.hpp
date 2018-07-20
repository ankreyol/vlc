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
#include "mlqmltypes.hpp"

class MCMediaLib;

class MLBaseModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit MLBaseModel(QObject *parent = nullptr);
    virtual ~MLBaseModel() = default;

    virtual void sort(int column, Qt::SortOrder order) override;

    Q_INVOKABLE void sortByColumn(QByteArray name, Qt::SortOrder order);

signals:
    void sortRoleChanged();

protected:
    virtual void clear() = 0;
    virtual vlc_ml_sorting_criteria_t roleToCriteria(int role) const = 0;
    virtual vlc_ml_sorting_criteria_t nameToCriteria(QByteArray) const {
        return VLC_ML_SORTING_DEFAULT;
    }

    ParentType::ParentTypes parentType() const;
    void setParentType(ParentType::ParentTypes parentType);
    int64_t parentId() const;
    void setParentId(int64_t parentId);
    MCMediaLib* ml() const;
    void setMl(MCMediaLib* ml);

    int m_parent_type = -1;
    uint64_t m_parent_id = 0;

    vlc_medialibrary_t* m_ml;
    MCMediaLib* m_mcMediaLib;
    int m_sort_role;
    vlc_ml_query_params_t m_query_param;
};

#endif // MLBASEMODEL_HPP
