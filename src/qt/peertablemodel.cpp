// Copyright (c) 2013-2025 The Curecoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "peertablemodel.h"
#include "clientmodel.h"

PeerTableModel::PeerTableModel(QObject *parent)
    : QAbstractTableModel(parent),
      m_clientModel(nullptr)
{
}

int PeerTableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_peers.size();
}

int PeerTableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return 5;
}

QVariant PeerTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_peers.size())
        return QVariant();
    if (role != Qt::DisplayRole && role != Qt::TextAlignmentRole)
        return QVariant();
    const PeerTableRow &row = m_peers.at(index.row());
    switch (index.column()) {
    case 0: return row.address;
    case 1: return row.ping;
    case 2: return row.protocolVersion;
    case 3: return row.subversion;
    case 4: return row.direction;
    default: return QVariant();
    }
}

QVariant PeerTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return QVariant();
    switch (section) {
    case 0: return tr("Address");
    case 1: return tr("Ping");
    case 2: return tr("Protocol");
    case 3: return tr("Subversion");
    case 4: return tr("Direction");
    default: return QVariant();
    }
}

void PeerTableModel::setClientModel(ClientModel *model)
{
    m_clientModel = model;
}

void PeerTableModel::refresh()
{
    if (!m_clientModel)
        return;
    beginResetModel();
    m_peers.clear();
    QList<ClientModel::PeerInfo> info = m_clientModel->getPeerInfo();
    for (const ClientModel::PeerInfo &pi : info) {
        PeerTableRow row;
        row.address = pi.address;
        row.ping = pi.ping;
        row.protocolVersion = pi.protocolVersion;
        row.subversion = pi.subversion;
        row.direction = pi.inbound ? tr("Inbound") : tr("Outbound");
        m_peers.append(row);
    }
    endResetModel();
}
