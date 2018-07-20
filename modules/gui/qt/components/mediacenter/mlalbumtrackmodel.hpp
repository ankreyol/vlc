#ifndef MLTRACKMODEL_HPP
#define MLTRACKMODEL_HPP

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <QObject>
#include "mlbasemodel.hpp"
#include "mlalbumtrack.hpp"


class MLAlbumTrackModel : public MLBaseModel
{
    Q_OBJECT

public:
    explicit MLAlbumTrackModel(QObject *parent = nullptr);

    virtual ~MLAlbumTrackModel() = default;

    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    void fetchMoreInner(const QModelIndex &parent) override;
    size_t nbElementsInModel() const override;
    size_t countTotalElements() const override;
    void clear() override;
    vlc_ml_sorting_criteria_t roleToCriteria(int role) const override;
    const MLAlbumTrack* getItem(const QModelIndex &index) const;

    std::vector<std::unique_ptr<MLAlbumTrack>> m_item_list;
};
#endif // MLTRACKMODEL_HPP
