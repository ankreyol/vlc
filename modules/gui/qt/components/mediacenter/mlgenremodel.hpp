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

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE bool canFetchMore(const QModelIndex &parent) const override;
    Q_INVOKABLE void fetchMore(const QModelIndex &parent) override;

private:
    void clear() override;
    size_t countTotalElement() const;

    vlc_ml_sorting_criteria_t roleToCriteria(int role) const override;

    const MLGenre* getItem(const QModelIndex &index) const;

    unsigned int m_total_count;
    bool m_initialized;
    std::vector<std::unique_ptr<MLGenre>> m_item_list;
};


#endif // MLGENREMODEL_HPP
