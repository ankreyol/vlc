#ifndef MLGENREMODEL_HPP
#define MLGENREMODEL_HPP

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <memory>
#include <QObject>
#include "mlbasemodel.hpp"
#include "mlgenre.hpp"

class MLGenreModel : public MLBaseModel
{
    Q_OBJECT

public:
    explicit MLGenreModel(QObject *parent = nullptr);
    virtual ~MLGenreModel() = default;

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role) const override;

private:
    void fetchMoreInner(const QModelIndex&) override;
    void clear() override;
    size_t countTotalElements() const override;
    size_t nbElementsInModel() const override;

    vlc_ml_sorting_criteria_t roleToCriteria(int role) const override;

    const MLGenre* getItem(const QModelIndex &index) const;

    std::vector<std::unique_ptr<MLGenre>> m_item_list;
};


#endif // MLGENREMODEL_HPP
