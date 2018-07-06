#ifndef MLABLUMMODEL_H
#define MLABLUMMODEL_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <QObject>
#include "mlbasemodel.hpp"
#include "mlalbum.hpp"

class MLAlbumModel : public MLBaseModel
{
    Q_OBJECT

public:
    explicit MLAlbumModel(std::shared_ptr<vlc_medialibrary_t>& ml, QObject *parent = nullptr);
    explicit MLAlbumModel(std::shared_ptr<vlc_medialibrary_t>& ml,  vlc_ml_parent_type parent_type, uint64_t parent_id, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent) const override;
    Q_INVOKABLE QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Q_INVOKABLE QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE QObject *get(unsigned int idx);

    Q_INVOKABLE bool canFetchMore(const QModelIndex &parent) const override;
    Q_INVOKABLE void fetchMore(const QModelIndex &parent) override;
private:
    vlc_ml_sorting_criteria_t roleToCriteria(int role) const override;

    const MLAlbum *getItem(const QModelIndex &index) const;

    unsigned int m_total_count;
    std::vector<MLAlbum*> m_item_list;
};

#endif // MLABLUMMODEL_H
