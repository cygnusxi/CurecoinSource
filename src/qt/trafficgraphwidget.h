// Copyright (c) 2011-present The Bitcoin Core developers
// Copyright (c) 2013-2025 The Curecoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CURECOIN_QT_TRAFFICGRAPHWIDGET_H
#define CURECOIN_QT_TRAFFICGRAPHWIDGET_H

#include <QWidget>
#include <QQueue>
#include <QPainterPath>

class ClientModel;

QT_BEGIN_NAMESPACE
class QPaintEvent;
class QTimer;
QT_END_NAMESPACE

class TrafficGraphWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TrafficGraphWidget(QWidget *parent = nullptr);
    void setClientModel(ClientModel *model);
    int getGraphRangeMins() const { return m_mins; }

protected:
    void paintEvent(QPaintEvent *) override;

public slots:
    void updateRates();
    void setGraphRangeMins(int mins);
    void clear();

private:
    void paintPath(QPainterPath &path, QQueue<float> &samples);

    static const int DESIRED_SAMPLES = 800;
    static const int XMARGIN = 10;
    static const int YMARGIN = 10;

    QTimer *m_timer;
    float m_fMax;
    int m_mins;
    QQueue<float> m_vSamplesIn;
    QQueue<float> m_vSamplesOut;
    quint64 m_nLastBytesIn;
    quint64 m_nLastBytesOut;
    ClientModel *m_clientModel;
};

#endif // CURECOIN_QT_TRAFFICGRAPHWIDGET_H
