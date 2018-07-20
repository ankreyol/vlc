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
#include "mcmedialib.hpp"

class MCMediaLib;

class MLBaseModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit MLBaseModel(QObject *parent = nullptr);
    virtual ~MLBaseModel() = default;

    virtual void sort(int column, Qt::SortOrder order) override;

    Q_INVOKABLE void sortByColumn(QByteArray name, Qt::SortOrder order);

    Q_PROPERTY( ParentType::ParentTypes parentType READ parentType WRITE setParentType )
    Q_PROPERTY( MLId parentId READ parentId WRITE setParentId )
    Q_PROPERTY( MCMediaLib* ml READ ml WRITE setMl )

    Q_INVOKABLE int rowCount(const QModelIndex &parent) const override;
    Q_INVOKABLE bool canFetchMore(const QModelIndex&) const override;
    Q_INVOKABLE void fetchMore(const QModelIndex &parent) override;

signals:
    void sortRoleChanged();

private:
    virtual size_t countTotalElements() const = 0;
    virtual size_t nbElementsInModel() const = 0;
    virtual void fetchMoreInner(const QModelIndex&) = 0;

protected:
    virtual void clear() = 0;
    virtual vlc_ml_sorting_criteria_t roleToCriteria(int role) const = 0;
    virtual vlc_ml_sorting_criteria_t nameToCriteria(QByteArray) const {
        return VLC_ML_SORTING_DEFAULT;
    }

    ParentType::ParentTypes parentType() const;
    void setParentType(ParentType::ParentTypes parentType);
    MLId parentId() const;
    void setParentId(MLId parentId);
    MCMediaLib* ml() const;
    void setMl(MCMediaLib* ml);

    int m_parent_type = -1;
    MLId m_parent_id = 0;

    vlc_medialibrary_t* m_ml;
    MCMediaLib* m_mcMediaLib;
    int m_sort_role;
    vlc_ml_query_params_t m_query_param;

    bool m_initialized;
    size_t m_total_count;
};

#endif // MLBASEMODEL_HPP
