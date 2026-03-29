// Copyright (c) 2011-present The Bitcoin Core developers
// Copyright (c) 2013-2025 The Curecoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "trafficgraphwidget.h"
#include "clientmodel.h"

#include <QPainter>
#include <QColor>
#include <QTimer>
#include <cmath>

TrafficGraphWidget::TrafficGraphWidget(QWidget *parent)
    : QWidget(parent),
      m_timer(nullptr),
      m_fMax(0.0f),
      m_mins(5),
      m_nLastBytesIn(0),
      m_nLastBytesOut(0),
      m_clientModel(nullptr)
{
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &TrafficGraphWidget::updateRates);
    setGraphRangeMins(m_mins);
}

void TrafficGraphWidget::setClientModel(ClientModel *model)
{
    m_clientModel = model;
    if (model) {
        m_nLastBytesIn = model->getTotalBytesRecv();
        m_nLastBytesOut = model->getTotalBytesSent();
    }
}

void TrafficGraphWidget::paintPath(QPainterPath &path, QQueue<float> &samples)
{
    int sampleCount = samples.size();
    if (sampleCount > 0 && m_fMax > 0.0f) {
        int h = height() - YMARGIN * 2;
        int w = width() - XMARGIN * 2;
        int x = XMARGIN + w;
        path.moveTo(x, YMARGIN + h);
        for (int i = 0; i < sampleCount; ++i) {
            x = XMARGIN + w - w * i / DESIRED_SAMPLES;
            int y = YMARGIN + h - static_cast<int>(h * samples.at(i) / m_fMax);
            path.lineTo(x, y);
        }
        path.lineTo(x, YMARGIN + h);
    }
}

void TrafficGraphWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.fillRect(rect(), Qt::black);

    if (m_fMax <= 0.0f)
        return;

    QColor axisCol(Qt::gray);
    int h = height() - YMARGIN * 2;
    painter.setPen(axisCol);
    painter.drawLine(XMARGIN, YMARGIN + h, width() - XMARGIN, YMARGIN + h);

    int base = static_cast<int>(std::floor(std::log10(m_fMax)));
    float val = std::pow(10.0f, static_cast<float>(base));
    const QString units = tr("kB/s");
    const float yMarginText = 2.0f;

    painter.setPen(axisCol);
    painter.drawText(XMARGIN, YMARGIN + h - static_cast<int>(h * val / m_fMax) - static_cast<int>(yMarginText),
                     QString("%1 %2").arg(static_cast<double>(val)).arg(units));
    for (float y = val; y < m_fMax; y += val) {
        int yy = YMARGIN + h - static_cast<int>(h * y / m_fMax);
        painter.drawLine(XMARGIN, yy, width() - XMARGIN, yy);
    }
    if (m_fMax / val <= 3.0f) {
        axisCol = axisCol.darker();
        val = std::pow(10.0f, static_cast<float>(base - 1));
        painter.setPen(axisCol);
        painter.drawText(XMARGIN, YMARGIN + h - static_cast<int>(h * val / m_fMax) - static_cast<int>(yMarginText),
                         QString("%1 %2").arg(static_cast<double>(val)).arg(units));
        int count = 1;
        for (float y = val; y < m_fMax; y += val, count++) {
            if (count % 10 == 0)
                continue;
            int yy = YMARGIN + h - static_cast<int>(h * y / m_fMax);
            painter.drawLine(XMARGIN, yy, width() - XMARGIN, yy);
        }
    }

    painter.setRenderHint(QPainter::Antialiasing);
    if (!m_vSamplesIn.empty()) {
        QPainterPath p;
        paintPath(p, m_vSamplesIn);
        painter.fillPath(p, QColor(0, 255, 0, 128));
        painter.setPen(Qt::green);
        painter.drawPath(p);
    }
    if (!m_vSamplesOut.empty()) {
        QPainterPath p;
        paintPath(p, m_vSamplesOut);
        painter.fillPath(p, QColor(255, 0, 0, 128));
        painter.setPen(Qt::red);
        painter.drawPath(p);
    }
}

void TrafficGraphWidget::updateRates()
{
    if (!m_clientModel)
        return;

    quint64 bytesIn = m_clientModel->getTotalBytesRecv();
    quint64 bytesOut = m_clientModel->getTotalBytesSent();
    int intervalMs = m_timer->interval();
    if (intervalMs <= 0)
        intervalMs = 1000;
    float intervalSec = intervalMs / 1000.0f;
    float inRateKbps = static_cast<float>(bytesIn - m_nLastBytesIn) / 1024.0f / intervalSec;
    float outRateKbps = static_cast<float>(bytesOut - m_nLastBytesOut) / 1024.0f / intervalSec;
    m_vSamplesIn.push_front(inRateKbps);
    m_vSamplesOut.push_front(outRateKbps);
    m_nLastBytesIn = bytesIn;
    m_nLastBytesOut = bytesOut;

    while (m_vSamplesIn.size() > DESIRED_SAMPLES)
        m_vSamplesIn.pop_back();
    while (m_vSamplesOut.size() > DESIRED_SAMPLES)
        m_vSamplesOut.pop_back();

    float tmax = 0.0f;
    for (float f : m_vSamplesIn)
        if (f > tmax) tmax = f;
    for (float f : m_vSamplesOut)
        if (f > tmax) tmax = f;
    m_fMax = tmax;
    update();
}

void TrafficGraphWidget::setGraphRangeMins(int mins)
{
    m_mins = (mins > 0) ? mins : 5;
    int msecsPerSample = (m_mins * 60 * 1000) / DESIRED_SAMPLES;
    if (msecsPerSample < 100)
        msecsPerSample = 100;
    m_timer->stop();
    m_timer->setInterval(msecsPerSample);
    clear();
}

void TrafficGraphWidget::clear()
{
    m_timer->stop();
    m_vSamplesOut.clear();
    m_vSamplesIn.clear();
    m_fMax = 0.0f;
    if (m_clientModel) {
        m_nLastBytesIn = m_clientModel->getTotalBytesRecv();
        m_nLastBytesOut = m_clientModel->getTotalBytesSent();
    }
    m_timer->start();
}
