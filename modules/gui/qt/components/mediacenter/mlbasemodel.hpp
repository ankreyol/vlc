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
    Q_INVOKABLE virtual const QObject *get(unsigned int idx) const = 0;

    Q_PROPERTY( MLParentId parentId READ parentId WRITE setParentId NOTIFY parentIdChanged )
    Q_PROPERTY( MCMediaLib* ml READ ml WRITE setMl )
    Q_PROPERTY( unsigned int maxItems MEMBER m_nb_max_items )

signals:
    void sortRoleChanged();
    void parentIdChanged();

protected:
    virtual void clear() = 0;
    virtual vlc_ml_sorting_criteria_t roleToCriteria(int role) const = 0;
    virtual vlc_ml_sorting_criteria_t nameToCriteria(QByteArray) const {
        return VLC_ML_SORTING_DEFAULT;
    }

    MLParentId parentId() const;
    void setParentId(MLParentId parentId);
    MCMediaLib* ml() const;
    void setMl(MCMediaLib* ml);

    MLParentId m_parent;

    vlc_medialibrary_t* m_ml;
    MCMediaLib* m_mcMediaLib;
    int m_sort_role;
    mutable vlc_ml_query_params_t m_query_param;

    unsigned int m_nb_max_items;
};

/**
 * Implements a basic sliding window.
 * const_cast & immutable are unavoidable, since all access member functions
 * are marked as const. fetchMore & canFetchMore don't allow for the full size
 * to be known (so the scrollbar would grow as we scroll, until we displayed all
 * elements), and implies having all elements loaded in RAM at all time.
 */
template <typename T>
class MLSlidingWindowModel : public MLBaseModel
{
public:
    static constexpr size_t BatchSize = 100;

    MLSlidingWindowModel(QObject* parent = nullptr)
        : MLBaseModel(parent)
        , m_initialized(false)
    {
        m_query_param.i_nbResults = BatchSize;
    }

    int rowCount(const QModelIndex &parent) const override
    {
        if (parent.isValid())
            return 0;
        if ( m_initialized == false )
        {
            m_total_count = countTotalElements();
            m_initialized = true;
            m_item_list = const_cast<MLSlidingWindowModel<T>*>(this)->fetch();
        }
        return m_total_count;
    }

    const QObject* get(unsigned int idx) const override
    {
        return item( idx );
    }

    void clear() override
    {
        m_query_param.i_offset = 0;
        m_initialized = false;
        m_total_count = 0;
        m_item_list.clear();
    }

protected:
    const T* item(unsigned int idx) const
    {
        if ( m_initialized == false )
        {
            m_total_count = countTotalElements();
            if ( m_total_count > 0 )
                m_item_list = const_cast<MLSlidingWindowModel<T>*>(this)->fetch();
            m_initialized = true;
        }

        if ( m_total_count == 0 || idx >= m_total_count || idx < 0 )
            return nullptr;

        if ( idx < m_query_param.i_offset ||  idx >= m_query_param.i_offset + m_item_list.size() )
        {
            if (m_query_param.i_nbResults == 0)
                m_query_param.i_offset = 0;
            else
                m_query_param.i_offset = idx - idx % m_query_param.i_nbResults;
            m_item_list = const_cast<MLSlidingWindowModel<T>*>(this)->fetch();
        }

        //db as changed
        if ( idx - m_query_param.i_offset >= m_item_list.size() || idx - m_query_param.i_offset < 0 )
            return nullptr;
        return m_item_list[idx - m_query_param.i_offset].get();
    }

private:
    virtual size_t countTotalElements() const = 0;
    virtual std::vector<std::unique_ptr<T>> fetch() = 0;

protected:
    mutable std::vector<std::unique_ptr<T>> m_item_list;

private:
    mutable bool m_initialized;
    mutable size_t m_total_count;
};

#endif // MLBASEMODEL_HPP
