// Copyright (c) 2011-present The Bitcoin Core developers
// Copyright (c) 2013-2025 The Curecoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "trafficgraphwidget.h"
#include "clientmodel.h"

#include <QPainter>
#include <QColor>
#include <QLinearGradient>
#include <QPen>
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
    setMinimumHeight(220);
    setAutoFillBackground(false);
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

void TrafficGraphWidget::buildGraphPath(QPainterPath &path, const QQueue<float> &samples, bool closeToBaseline) const
{
    int sampleCount = samples.size();
    if (sampleCount > 0 && m_fMax > 0.0f) {
        int h = height() - TOP_MARGIN - BOTTOM_MARGIN;
        int w = width() - LEFT_MARGIN - RIGHT_MARGIN;
        int baseline = TOP_MARGIN + h;
        int firstX = LEFT_MARGIN + w - w * (sampleCount - 1) / DESIRED_SAMPLES;
        int lastX = LEFT_MARGIN + w;

        if(closeToBaseline)
            path.moveTo(firstX, baseline);

        bool firstPoint = true;
        for (int i = sampleCount - 1; i >= 0; --i) {
            int x = LEFT_MARGIN + w - w * i / DESIRED_SAMPLES;
            int y = TOP_MARGIN + h - static_cast<int>(h * samples.at(i) / m_fMax);
            y = qMax(TOP_MARGIN, qMin(baseline, y));
            if(firstPoint && !closeToBaseline)
                path.moveTo(x, y);
            else
                path.lineTo(x, y);
            firstPoint = false;
        }

        if(closeToBaseline) {
            path.lineTo(lastX, baseline);
            path.closeSubpath();
        }
    }
}

void TrafficGraphWidget::drawGlowLine(QPainter &painter, const QPainterPath &path, const QColor &color) const
{
    QColor wideGlow(color);
    wideGlow.setAlpha(45);
    painter.setPen(QPen(wideGlow, 11, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.drawPath(path);

    QColor midGlow(color);
    midGlow.setAlpha(95);
    painter.setPen(QPen(midGlow, 6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.drawPath(path);

    QColor core(color);
    core.setAlpha(245);
    painter.setPen(QPen(core, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.drawPath(path);
}

void TrafficGraphWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QLinearGradient backgroundGradient(rect().topLeft(), rect().bottomLeft());
    backgroundGradient.setColorAt(0.0, QColor(5, 20, 34));
    backgroundGradient.setColorAt(1.0, QColor(2, 7, 13));
    painter.fillRect(rect(), backgroundGradient);

    QRect plotRect(LEFT_MARGIN, TOP_MARGIN, width() - LEFT_MARGIN - RIGHT_MARGIN, height() - TOP_MARGIN - BOTTOM_MARGIN);
    if(plotRect.width() <= 0 || plotRect.height() <= 0)
        return;

    painter.save();
    painter.setPen(QPen(QColor(52, 156, 205, 120), 1));
    painter.setBrush(QColor(0, 0, 0, 105));
    painter.drawRoundedRect(plotRect.adjusted(0, 0, -1, -1), 6, 6);
    painter.restore();

    QFont labelFont = font();
    labelFont.setBold(true);
    painter.setFont(labelFont);
    QRect legendRect(plotRect.right() - 190, plotRect.top() + 10, 178, 24);
    painter.setPen(QPen(QColor(52, 156, 205, 95), 1));
    painter.setBrush(QColor(3, 12, 20, 185));
    painter.drawRoundedRect(legendRect, 10, 10);
    painter.setPen(QColor(0, 255, 170));
    painter.drawText(legendRect.adjusted(12, 0, 0, 0), Qt::AlignLeft | Qt::AlignVCenter, tr("IN"));
    painter.setPen(QColor(255, 95, 45));
    painter.drawText(legendRect.adjusted(72, 0, 0, 0), Qt::AlignLeft | Qt::AlignVCenter, tr("OUT"));
    painter.setPen(QColor(205, 232, 244, 210));
    painter.drawText(legendRect.adjusted(132, 0, -8, 0), Qt::AlignRight | Qt::AlignVCenter, QString("%1m").arg(m_mins));

    if (m_fMax <= 0.0f)
        return;

    QColor axisCol(145, 198, 226, 140);
    int h = plotRect.height();
    int baseline = plotRect.bottom();
    painter.setPen(QPen(axisCol, 1));
    painter.drawLine(plotRect.left(), baseline, plotRect.right(), baseline);

    int base = static_cast<int>(std::floor(std::log10(m_fMax)));
    float val = std::pow(10.0f, static_cast<float>(base));
    const QString units = tr("kB/s");
    const float yMarginText = 2.0f;

    painter.setPen(QColor(205, 232, 244, 210));
    painter.drawText(8, TOP_MARGIN + h - static_cast<int>(h * val / m_fMax) - static_cast<int>(yMarginText),
                     QString("%1 %2").arg(static_cast<double>(val)).arg(units));
    painter.setPen(QPen(axisCol, 1));
    for (float y = val; y < m_fMax; y += val) {
        int yy = TOP_MARGIN + h - static_cast<int>(h * y / m_fMax);
        painter.drawLine(plotRect.left(), yy, plotRect.right(), yy);
    }
    if (m_fMax / val <= 3.0f) {
        axisCol = QColor(90, 135, 160, 95);
        val = std::pow(10.0f, static_cast<float>(base - 1));
        painter.setPen(QColor(172, 208, 226, 190));
        painter.drawText(8, TOP_MARGIN + h - static_cast<int>(h * val / m_fMax) - static_cast<int>(yMarginText),
                         QString("%1 %2").arg(static_cast<double>(val)).arg(units));
        painter.setPen(QPen(axisCol, 1));
        int count = 1;
        for (float y = val; y < m_fMax; y += val, count++) {
            if (count % 10 == 0)
                continue;
            int yy = TOP_MARGIN + h - static_cast<int>(h * y / m_fMax);
            painter.drawLine(plotRect.left(), yy, plotRect.right(), yy);
        }
    }

    if (!m_vSamplesIn.empty()) {
        QPainterPath areaPath;
        QPainterPath linePath;
        buildGraphPath(areaPath, m_vSamplesIn, true);
        buildGraphPath(linePath, m_vSamplesIn, false);
        painter.fillPath(areaPath, QColor(0, 255, 190, 24));
        drawGlowLine(painter, linePath, QColor(0, 255, 170));
    }
    if (!m_vSamplesOut.empty()) {
        QPainterPath areaPath;
        QPainterPath linePath;
        buildGraphPath(areaPath, m_vSamplesOut, true);
        buildGraphPath(linePath, m_vSamplesOut, false);
        painter.fillPath(areaPath, QColor(255, 95, 50, 22));
        drawGlowLine(painter, linePath, QColor(255, 95, 45));
    }

    painter.setFont(labelFont);
    painter.setPen(QColor(205, 232, 244, 210));
    painter.drawText(plotRect.adjusted(12, 10, -12, -10), Qt::AlignLeft | Qt::AlignTop,
                     tr("PEAK %1 kB/s").arg(QString::number(m_fMax, 'f', m_fMax >= 10.0f ? 0 : 1)));
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
