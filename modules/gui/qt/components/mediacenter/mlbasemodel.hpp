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
    vlc_ml_query_params_t m_query_param;

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
        , m_offset(0)
        , m_initialized(false)
    {
    }

    int rowCount(const QModelIndex &parent) const override
    {
        if (parent.isValid())
            return 0;
        if ( m_initialized == false )
        {
            m_total_count = countTotalElements();
            m_initialized = true;
            m_item_list = const_cast<MLSlidingWindowModel<T>*>(this)->fetch( m_offset, BatchSize );
        }
        return m_total_count;
    }

    const QObject* get(unsigned int idx) const override
    {
        return item( idx );
    }

    void clear() override
    {
        m_offset = 0;
        m_initialized = false;
        m_item_list.clear();
    }

protected:
    const T* item(unsigned int idx) const
    {
        if ( m_initialized == false )
        {
            m_total_count = countTotalElements();
            if ( m_total_count > 0 )
                m_item_list = const_cast<MLSlidingWindowModel<T>*>(this)->fetch( m_offset, BatchSize );
            m_initialized = true;
        }

        if ( m_total_count == 0 || idx >= m_total_count || idx < 0 )
            return nullptr;

        if ( idx < m_offset )
        {
            // Backward scroll, we need to reload previous elements
            // Special case if we are reading from the beginning again (for instance
            // after switching views or display type)
            if ( idx == 0 )
                m_offset = 0;
            else
            {
                // Otherwise read a new batch which ends with the new index
                if ( idx > BatchSize )
                    m_offset = idx - BatchSize + 1; // Add 1 to include the queried item
                else
                    m_offset = 0;
            }
            m_item_list = const_cast<MLSlidingWindowModel<T>*>(this)->fetch( m_offset, BatchSize );
        }
        else if ( idx >= m_offset + m_item_list.size() )
        {
            // Forward scroll, fetch a new complete window of items
            m_offset += BatchSize;
            m_item_list = const_cast<MLSlidingWindowModel<T>*>(this)->fetch( m_offset, BatchSize );
        }
        return m_item_list[idx - m_offset].get();
    }

private:
    virtual size_t countTotalElements() const = 0;
    virtual std::vector<std::unique_ptr<T>> fetch(int offset, int nbElems) = 0;

protected:
    mutable std::vector<std::unique_ptr<T>> m_item_list;

private:
    mutable unsigned int m_offset;
    mutable bool m_initialized;
    mutable size_t m_total_count;
};

#endif // MLBASEMODEL_HPP
