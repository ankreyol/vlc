#ifndef MLABLUMMODEL_H
#define MLABLUMMODEL_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <QObject>
#include "mlbasemodel.hpp"
#include "mlalbum.hpp"
#include "mlqmltypes.hpp"
#include "mcmedialib.hpp"

class MLAlbumModel : public MLBaseModel
{
    Q_OBJECT

public:
    explicit MLAlbumModel(QObject *parent = nullptr);
    virtual ~MLAlbumModel();

    int rowCount(const QModelIndex &parent) const override;
    Q_INVOKABLE QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Q_INVOKABLE QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE QObject *get(unsigned int idx);

    Q_INVOKABLE bool canFetchMore(const QModelIndex &parent) const override;
    Q_INVOKABLE void fetchMore(const QModelIndex &parent) override;

    Q_PROPERTY( ParentType::ParentTypes parentType READ parentType WRITE setParentType )
    Q_PROPERTY( int64_t parentId READ parentId WRITE setParentId )
    Q_PROPERTY( MCMediaLib* ml READ ml WRITE setMl )

    ParentType::ParentTypes parentType() const;
    void setParentType(ParentType::ParentTypes parentType);
    int64_t parentId() const;
    void setParentId(int64_t parentId);
    MCMediaLib* ml() const;
    void setMl(MCMediaLib* ml);

private:
    void clear() override;
    vlc_ml_sorting_criteria_t roleToCriteria(int role) const;
    vlc_ml_sorting_criteria_t nameToCriteria(QByteArray name) const override;

    const MLAlbum *getItem(const QModelIndex &index) const;

    unsigned int m_total_count;
    bool m_initialized;
    std::vector<MLAlbum*> m_item_list;

    static  QHash<int, QByteArray> m_role_names;
    static  QHash<QByteArray, vlc_ml_sorting_criteria_t> m_names_to_criteria;
};


#endif // MLABLUMMODEL_H
