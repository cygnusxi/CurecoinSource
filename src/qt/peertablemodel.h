// Copyright (c) 2013-2025 The Curecoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CURECOIN_QT_PEERTABLEMODEL_H
#define CURECOIN_QT_PEERTABLEMODEL_H

#include <QAbstractTableModel>
#include <QStringList>
#include <QVariant>

class ClientModel;

struct PeerTableRow {
    QString address;
    QString ping;
    int protocolVersion;
    QString subversion;
    QString direction;
};

class PeerTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit PeerTableModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

public slots:
    void refresh();
    void setClientModel(ClientModel *model);

private:
    ClientModel *m_clientModel;
    QList<PeerTableRow> m_peers;
};

#endif // CURECOIN_QT_PEERTABLEMODEL_H
