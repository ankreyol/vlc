#ifndef MLTRACKMODEL_HPP
#define MLTRACKMODEL_HPP

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <QObject>
#include "mlbasemodel.hpp"
#include "mlalbumtrack.hpp"


class MLAlbumTrackModel : public MLSlidingWindowModel<MLAlbumTrack>
{
    Q_OBJECT

public:
    explicit MLAlbumTrackModel(QObject *parent = nullptr);

    virtual ~MLAlbumTrackModel() = default;

    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    std::vector<std::unique_ptr<MLAlbumTrack>> fetch() override;
    size_t countTotalElements() const override;
    vlc_ml_sorting_criteria_t roleToCriteria(int role) const override;
    vlc_ml_sorting_criteria_t nameToCriteria(QByteArray name) const override;

    static QHash<QByteArray, vlc_ml_sorting_criteria_t> M_names_to_criteria;
};
#endif // MLTRACKMODEL_HPP
