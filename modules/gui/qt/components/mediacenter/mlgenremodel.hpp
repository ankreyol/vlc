#ifndef MLGENREMODEL_HPP
#define MLGENREMODEL_HPP

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <memory>
#include <QObject>
#include "mlbasemodel.hpp"
#include "mlgenre.hpp"

class MLGenreModel : public MLSlidingWindowModel<MLGenre>
{
    Q_OBJECT

public:
    explicit MLGenreModel(QObject *parent = nullptr);
    virtual ~MLGenreModel() = default;

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role) const override;

private:
    std::vector<std::unique_ptr<MLGenre>> fetch() override;
    size_t countTotalElements() const override;

    vlc_ml_sorting_criteria_t roleToCriteria(int role) const override;
};


#endif // MLGENREMODEL_HPP
