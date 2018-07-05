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
    explicit MLGenreModel(std::shared_ptr<vlc_medialibrary_t>& ml,  QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    void reload() override;
    vlc_ml_sorting_criteria_t roleToCriteria(int role) const override;

    const MLGenre* getItem(const QModelIndex &index) const;

    std::vector<MLGenre*> m_item_list;
};


#endif // MLGENREMODEL_HPP
