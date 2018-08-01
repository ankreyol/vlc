#ifndef MLARTISTMODEL_HPP
#define MLARTISTMODEL_HPP

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <QObject>
#include "mlbasemodel.hpp"
#include "mlartist.hpp"

class MLArtistModel : public MLSlidingWindowModel<MLArtist>
{
    Q_OBJECT

public:
    explicit MLArtistModel(QObject *parent = nullptr);
    virtual ~MLArtistModel() = default;

    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    std::vector<std::unique_ptr<MLArtist>> fetch() override;
    size_t countTotalElements() const override;
    vlc_ml_sorting_criteria_t roleToCriteria(int role) const override;
    vlc_ml_sorting_criteria_t nameToCriteria(QByteArray name) const override;

    static QHash<QByteArray, vlc_ml_sorting_criteria_t> M_names_to_criteria;
};

#endif // MLARTISTMODEL_HPP
