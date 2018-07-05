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
    explicit MLAlbumTrackModel(std::shared_ptr<vlc_medialibrary_t>& ml, QObject *parent = nullptr);

    explicit MLAlbumTrackModel(std::shared_ptr<vlc_medialibrary_t>& ml, vlc_ml_parent_type parent_type, uint64_t parent_id, QObject *parent = nullptr);

    ~MLAlbumTrackModel();

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;
private:
    void reload() override;
    vlc_ml_sorting_criteria_t roleToCriteria(int role) const override;
    const MLAlbumTrack* getItem(const QModelIndex &index) const;

    int m_parent_type = -1;
    uint64_t m_parent_id;
    std::vector<MLAlbumTrack*> m_item_list;
};
#endif // MLTRACKMODEL_HPP
