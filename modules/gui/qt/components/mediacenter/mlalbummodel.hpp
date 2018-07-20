#ifndef MLABLUMMODEL_H
#define MLABLUMMODEL_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <QObject>
#include "mlbasemodel.hpp"
#include "mlalbum.hpp"
#include "mcmedialib.hpp"

class MLAlbumModel : public MLBaseModel
{
    Q_OBJECT

public:
    explicit MLAlbumModel(QObject *parent = nullptr);
    virtual ~MLAlbumModel();

    Q_INVOKABLE QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Q_INVOKABLE QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE QObject *get(unsigned int idx);

private:
    void fetchMoreInner(const QModelIndex &parent) override;
    void clear() override;
    size_t nbElementsInModel() const override;
    size_t countTotalElements() const override;
    vlc_ml_sorting_criteria_t roleToCriteria(int role) const;
    vlc_ml_sorting_criteria_t nameToCriteria(QByteArray name) const override;

    const MLAlbum *getItem(const QModelIndex &index) const;

    std::vector<MLAlbum*> m_item_list;

    static  QHash<int, QByteArray> m_role_names;
    static  QHash<QByteArray, vlc_ml_sorting_criteria_t> m_names_to_criteria;
};


#endif // MLABLUMMODEL_H
