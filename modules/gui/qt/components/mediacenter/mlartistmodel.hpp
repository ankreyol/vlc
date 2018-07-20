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
    explicit MLArtistModel(QObject *parent = nullptr);
    virtual ~MLArtistModel() = default;

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE bool canFetchMore(const QModelIndex &parent) const override;
    Q_INVOKABLE void fetchMore(const QModelIndex &parent) override;

private:
    void clear() override;
    size_t countTotalElement() const;
    vlc_ml_sorting_criteria_t roleToCriteria(int role) const override;

    const MLArtist* getItem(const QModelIndex &index) const;

    size_t m_total_count;
    bool m_initialized;
    std::vector<std::unique_ptr<MLArtist>> m_item_list;
};

#endif // MLARTISTMODEL_HPP
