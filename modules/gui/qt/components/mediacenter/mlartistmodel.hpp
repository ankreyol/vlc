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

    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    void fetchMoreInner(const QModelIndex &parent) override;
    void clear() override;
    size_t countTotalElements() const override;
    size_t nbElementsInModel() const override;
    vlc_ml_sorting_criteria_t roleToCriteria(int role) const override;

    const MLArtist* getItem(const QModelIndex &index) const;

    std::vector<std::unique_ptr<MLArtist>> m_item_list;
};

#endif // MLARTISTMODEL_HPP
