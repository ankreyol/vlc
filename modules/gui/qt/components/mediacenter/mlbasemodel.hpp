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

    Q_PROPERTY( MLParentId parentId MEMBER m_parent )
    Q_PROPERTY( MCMediaLib* ml READ ml WRITE setMl )
    Q_PROPERTY( unsigned int maxItems MEMBER m_nb_max_items )

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

    MCMediaLib* ml() const;
    void setMl(MCMediaLib* ml);

    MLParentId m_parent;

    vlc_medialibrary_t* m_ml;
    MCMediaLib* m_mcMediaLib;
    int m_sort_role;
    vlc_ml_query_params_t m_query_param;

    bool m_initialized;
    size_t m_total_count;
    unsigned int m_nb_max_items;
};

#endif // MLBASEMODEL_HPP
