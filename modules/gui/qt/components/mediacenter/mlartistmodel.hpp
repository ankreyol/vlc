#ifndef MLARTISTMODEL_HPP
#define MLARTISTMODEL_HPP

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <QObject>
#include "mlbasemodel.hpp"
#include "mlartist.hpp"

class MLArtistModel : public MLBaseModel
{
    Q_OBJECT

public:
    explicit MLArtistModel(std::shared_ptr<vlc_medialibrary_t> &ml, QObject *parent = nullptr);
    explicit MLArtistModel(std::shared_ptr<vlc_medialibrary_t> &ml, vlc_ml_parent_type parent_type, uint64_t parent_id, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE bool canFetchMore(const QModelIndex &parent) const override;
    Q_INVOKABLE void fetchMore(const QModelIndex &parent) override;
private:
    vlc_ml_sorting_criteria_t roleToCriteria(int role) const override;

    const MLArtist* getItem(const QModelIndex &index) const;

    unsigned int m_total_count;
    std::vector<MLArtist*> m_item_list;
};

#endif // MLARTISTMODEL_HPP
