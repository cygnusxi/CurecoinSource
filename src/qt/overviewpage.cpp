#include "overviewpage.h"
#include "ui_overviewpage.h"

#include "clientmodel.h"
#include "walletmodel.h"
#include "curecoinunits.h"
#include "optionsmodel.h"
#include "transactiontablemodel.h"
#include "transactionfilterproxy.h"
#include "guiutil.h"
#include "guiconstants.h"

#include <QAbstractItemDelegate>
#include <QDateTime>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QSizePolicy>
#include <QStyle>
#include <QTimer>
#include <QVBoxLayout>

#include <cmath>

#define DECORATION_SIZE 54
#define NUM_ITEMS 7

class NetworkSyncPanel : public QWidget
{
public:
    explicit NetworkSyncPanel(QWidget *parent = 0):
        QWidget(parent),
        connections(0),
        blocks(0),
        peerBlocks(0),
        syncing(true)
    {
        setObjectName("overviewNetworkPanel");
        setMinimumHeight(156);
        QTimer *timer = new QTimer(this);
        connect(timer, SIGNAL(timeout()), this, SLOT(update()));
        timer->start(50);
    }

    void setNetworkState(int connectionCount, int blockCount, int peerBlockCount, bool isSyncing)
    {
        connections = connectionCount;
        blocks = blockCount;
        peerBlocks = peerBlockCount;
        syncing = isSyncing;
        update();
    }

protected:
    void paintEvent(QPaintEvent *)
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        QRectF panelRect = rect().adjusted(1, 1, -1, -1);
        painter.setPen(QPen(QColor(46, 182, 232, 120), 1));
        painter.setBrush(QColor(7, 18, 28, 210));
        painter.drawRoundedRect(panelRect, 10, 10);

        const qreal phase = std::fmod(QDateTime::currentMSecsSinceEpoch() / 1000.0, 8.0);
        QRectF visualRect(18, 18, qMin(width() * 0.42, 170.0), height() - 36);
        QPointF hub(visualRect.center().x(), visualRect.center().y());
        qreal radius = qMin(visualRect.width(), visualRect.height()) * 0.34;
        int nodeCount = qMax(3, qMin(10, connections + 3));

        QColor linkColor(46, 182, 232, connections > 0 ? 135 : 45);
        QColor nodeColor(connections > 0 ? QColor(46, 182, 232) : QColor(95, 111, 122));
        QColor pulseColor(syncing ? QColor(46, 182, 232) : QColor(64, 230, 165));

        painter.setPen(QPen(linkColor, 1));
        for(int i = 0; i < nodeCount; ++i)
        {
            qreal angle = phase * 0.65 + (6.28318530718 * i / nodeCount);
            QPointF node(hub.x() + std::cos(angle) * radius, hub.y() + std::sin(angle) * radius);
            painter.drawLine(hub, node);
        }

        qreal pulse = 10.0 + std::fmod(phase * 24.0, 28.0);
        QColor pulseOuter(pulseColor);
        pulseOuter.setAlpha(70);
        painter.setPen(QPen(pulseOuter, 2));
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(hub, pulse, pulse);

        painter.setPen(Qt::NoPen);
        QColor hubGlow(pulseColor);
        hubGlow.setAlpha(55);
        painter.setBrush(hubGlow);
        painter.drawEllipse(hub, 28, 28);
        painter.setBrush(pulseColor);
        painter.drawEllipse(hub, 9, 9);

        for(int i = 0; i < nodeCount; ++i)
        {
            qreal angle = phase * 0.65 + (6.28318530718 * i / nodeCount);
            QPointF node(hub.x() + std::cos(angle) * radius, hub.y() + std::sin(angle) * radius);
            QColor glow(nodeColor);
            glow.setAlpha(60);
            painter.setBrush(glow);
            painter.drawEllipse(node, 9, 9);
            painter.setBrush(nodeColor);
            painter.drawEllipse(node, 4, 4);
        }

        QRect textRect(qMax(210, static_cast<int>(visualRect.right() + 18)), 20,
                       width() - qMax(228, static_cast<int>(visualRect.right() + 36)), height() - 36);
        painter.setPen(QColor(130, 220, 255));
        QFont titleFont = font();
        titleFont.setBold(true);
        titleFont.setPointSize(titleFont.pointSize() + 1);
        painter.setFont(titleFont);
        painter.drawText(textRect, Qt::AlignLeft | Qt::AlignTop, tr("NETWORK SYNC"));

        int progress = 0;
        if(peerBlocks > 0)
            progress = qMin(100, qMax(0, blocks * 100 / peerBlocks));
        QString status = syncing ? tr("Synchronizing") : tr("Synchronized");
        if(connections == 0)
            status = tr("Offline");

        QFont detailFont = font();
        painter.setFont(detailFont);
        painter.setPen(QColor(218, 238, 247));
        bool showProgress = syncing && connections > 0 && peerBlocks > 0 && progress < 100;
        QRect detailRect = textRect.adjusted(0, 32, 0, showProgress ? -24 : 0);
        painter.drawText(detailRect, Qt::AlignLeft | Qt::AlignTop,
                         tr("%1 peers  |  %2% complete\nBlock %3 of %4\n%5")
                         .arg(connections)
                         .arg(progress)
                         .arg(blocks)
                         .arg(peerBlocks > 0 ? QString::number(peerBlocks) : tr("unknown"))
                         .arg(status));

        if(!showProgress)
            return;

        QRectF progressRect(textRect.left(), textRect.bottom() - 18, textRect.width(), 8);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(18, 43, 58));
        painter.drawRoundedRect(progressRect, 4, 4);
        QRectF fillRect = progressRect;
        fillRect.setWidth(progressRect.width() * (connections == 0 ? 0 : progress) / 100.0);
        painter.setBrush(pulseColor);
        painter.drawRoundedRect(fillRect, 4, 4);
    }

private:
    int connections;
    int blocks;
    int peerBlocks;
    bool syncing;
};

class TxViewDelegate : public QAbstractItemDelegate
{
    Q_OBJECT
public:
    TxViewDelegate(): QAbstractItemDelegate(), unit(curecoinUnits::BTC)
    {

    }

    inline void paint(QPainter *painter, const QStyleOptionViewItem &option,
                      const QModelIndex &index ) const
    {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);

        QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
        QRect mainRect = option.rect.adjusted(6, 4, -6, -4);
        QColor baseCard = option.palette.color(QPalette::Base);
        QColor textColor = option.palette.color(QPalette::Text);
        QColor mutedText = option.palette.color(QPalette::Mid);
        if(!mutedText.isValid())
            mutedText = textColor.darker(135);

        QDateTime date = index.data(TransactionTableModel::DateRole).toDateTime();
        QString address = index.data(Qt::DisplayRole).toString();
        qint64 amount = index.data(TransactionTableModel::AmountRole).toLongLong();
        bool confirmed = index.data(TransactionTableModel::ConfirmedRole).toBool();
        QVariant value = index.data(Qt::ForegroundRole);
        QColor foreground = option.palette.color(QPalette::Text);
        if(qVariantCanConvert<QColor>(value))
        {
            foreground = qvariant_cast<QColor>(value);
        }

        QColor directionColor = amount < 0 ? QColor(255, 75, 95) : QColor(80, 230, 170);
        if(!confirmed)
            directionColor = QColor(160, 170, 180);

        QColor cardColor = baseCard;
        cardColor = cardColor.lighter(108);
        painter->setPen(QPen(directionColor, 1));
        painter->setBrush(cardColor);
        painter->drawRoundedRect(mainRect, 8, 8);

        int timelineX = mainRect.left() + 18;
        painter->setPen(QPen(directionColor, 2));
        painter->drawLine(timelineX, option.rect.top(), timelineX, option.rect.bottom());

        QColor dotGlow = directionColor;
        dotGlow.setAlpha(70);
        painter->setPen(Qt::NoPen);
        painter->setBrush(dotGlow);
        painter->drawEllipse(QPoint(timelineX, mainRect.center().y()), 9, 9);
        painter->setBrush(directionColor);
        painter->drawEllipse(QPoint(timelineX, mainRect.center().y()), 4, 4);

        QRect iconRect(timelineX + 14, mainRect.top() + (mainRect.height() - 34) / 2, 34, 34);
        icon.paint(painter, iconRect);

        int contentLeft = iconRect.right() + 12;
        int amountWidth = qMin(170, qMax(100, mainRect.width() / 3));
        QRect amountRect(mainRect.right() - amountWidth - 12, mainRect.top() + 12, amountWidth, 24);
        QRect dateRect(contentLeft, mainRect.top() + 10, amountRect.left() - contentLeft - 10, 22);
        QRect addressRect(contentLeft, mainRect.top() + 35, amountRect.left() - contentLeft - 10, 22);

        painter->setPen(textColor);
        QFont titleFont = painter->font();
        titleFont.setBold(true);
        painter->setFont(titleFont);
        painter->drawText(dateRect, Qt::AlignLeft|Qt::AlignVCenter,
                          painter->fontMetrics().elidedText(GUIUtil::dateTimeStr(date), Qt::ElideRight, dateRect.width()));

        QFont regularFont = painter->font();
        regularFont.setBold(false);
        painter->setFont(regularFont);
        painter->setPen(foreground);
        painter->drawText(addressRect, Qt::AlignLeft|Qt::AlignVCenter,
                          painter->fontMetrics().elidedText(address, Qt::ElideRight, addressRect.width()));

        if(amount < 0)
        {
            foreground = COLOR_NEGATIVE;
        }
        else if(!confirmed)
        {
            foreground = COLOR_UNCONFIRMED;
        }
        else
        {
            foreground = option.palette.color(QPalette::Text);
        }
        painter->setPen(foreground);
        QString amountText = curecoinUnits::formatWithUnit(unit, amount, true);
        if(!confirmed)
        {
            amountText = QString("[") + amountText + QString("]");
        }

        QFont amountFont = GUIUtil::tabularAmountFont();
        amountFont.setBold(true);
        painter->setFont(amountFont);
        painter->drawText(amountRect, Qt::AlignRight|Qt::AlignVCenter, amountText);

        QString statusText = confirmed ? tr("CONFIRMED") : tr("PENDING");
        QRect pillRect(mainRect.right() - amountWidth - 12, mainRect.top() + 42, amountWidth, 20);
        painter->setPen(QPen(directionColor, 1));
        QColor pillFill = directionColor;
        pillFill.setAlpha(38);
        painter->setBrush(pillFill);
        painter->drawRoundedRect(pillRect.adjusted(0, 0, -1, -1), 9, 9);
        painter->setPen(directionColor);
        painter->setFont(regularFont);
        painter->drawText(pillRect, Qt::AlignCenter, statusText);

        painter->restore();
    }

    inline QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        return QSize(DECORATION_SIZE, DECORATION_SIZE + 34);
    }

    int unit;

};
#include "overviewpage.moc"

OverviewPage::OverviewPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OverviewPage),
    clientModel(0),
    currentBalance(-1),
    currentStake(0),
    currentUnconfirmedBalance(-1),
    currentImmatureBalance(-1),
    txdelegate(new TxViewDelegate()),
    filter(0),
    labelHeroBalance(0),
    labelHeroStake(0),
    labelHeroUnconfirmed(0),
    labelHeroTransactions(0),
    labelHeroWalletStatus(0),
    labelHeroResearchStatus(0),
    labelRecentEmptyState(0),
    networkSyncPanel(0),
    currentNumConnections(0),
    currentNumBlocks(0),
    currentNumBlocksOfPeers(0)
{
    ui->setupUi(this);
    createHeroPanel();
    createNetworkSyncPanel();
    ui->overviewWalletFrame->hide();
    ui->label_wallet_bgcoin->hide();

    QFont amountFont = GUIUtil::tabularAmountFont();
    amountFont.setBold(true);
    ui->labelBalance->setFont(amountFont);
    ui->labelStake->setFont(amountFont);
    ui->labelUnconfirmed->setFont(amountFont);
    ui->labelImmature->setFont(amountFont);
    ui->labelNumTransactions->setFont(amountFont);

    // Recent transactions
    ui->listTransactions->setItemDelegate(txdelegate);
    ui->listTransactions->setIconSize(QSize(DECORATION_SIZE, DECORATION_SIZE));
    ui->listTransactions->setMinimumHeight(NUM_ITEMS * (DECORATION_SIZE + 38));
    ui->listTransactions->setAttribute(Qt::WA_MacShowFocusRect, false);
    labelRecentEmptyState = new QLabel(tr("No recent transactions yet.\nNew wallet activity will appear here as a live timeline."));
    labelRecentEmptyState->setObjectName("overviewEmptyState");
    labelRecentEmptyState->setAlignment(Qt::AlignCenter);
    labelRecentEmptyState->setWordWrap(true);
    labelRecentEmptyState->hide();
    ui->verticalLayout->addWidget(labelRecentEmptyState);

    connect(ui->listTransactions, SIGNAL(clicked(QModelIndex)), this, SLOT(handleTransactionClicked(QModelIndex)));

    // init "out of sync" warning labels
    ui->labelWalletStatus->setText("(" + tr("out of sync") + ")");
    ui->labelTransactionsStatus->setText("(" + tr("out of sync") + ")");

    QPalette  p;
    p.setColor(QPalette::WindowText,Qt::red);
    ui->label_curecoin_Intro->setPalette(p);


    // start with displaying the "out of sync" warnings
    showOutOfSyncWarning(true);
}

static QLabel *createHeroLabel(const QString &objectName, const QString &text)
{
    QLabel *label = new QLabel(text);
    label->setObjectName(objectName);
    label->setTextFormat(Qt::PlainText);
    return label;
}

static QFrame *createMetricCard(const QString &title, QLabel *valueLabel)
{
    QFrame *card = new QFrame();
    card->setObjectName("overviewMetricCard");
    card->setFrameShape(QFrame::StyledPanel);
    QVBoxLayout *layout = new QVBoxLayout(card);
    layout->setContentsMargins(12, 10, 12, 10);
    layout->setSpacing(4);

    QLabel *titleLabel = createHeroLabel("overviewMetricTitle", title);
    valueLabel->setObjectName("overviewMetricValue");
    layout->addWidget(titleLabel);
    layout->addWidget(valueLabel);
    return card;
}

void OverviewPage::createHeroPanel()
{
    QFrame *heroFrame = new QFrame(this);
    heroFrame->setObjectName("overviewHeroFrame");
    heroFrame->setFrameShape(QFrame::StyledPanel);
    heroFrame->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);

    QVBoxLayout *heroLayout = new QVBoxLayout(heroFrame);
    heroLayout->setContentsMargins(18, 16, 18, 16);
    heroLayout->setSpacing(12);

    QHBoxLayout *headerLayout = new QHBoxLayout();
    headerLayout->setSpacing(10);

    QLabel *eyebrowLabel = createHeroLabel("overviewHeroEyebrow", tr("CURECOIN RESEARCH WALLET"));
    headerLayout->addWidget(eyebrowLabel);
    headerLayout->addStretch();

    labelHeroWalletStatus = createHeroLabel("overviewHeroStatusWarning", tr("SYNCING"));
    labelHeroResearchStatus = createHeroLabel("overviewHeroStatusInfo", tr("RESEARCH READY"));
    headerLayout->addWidget(labelHeroWalletStatus);
    headerLayout->addWidget(labelHeroResearchStatus);
    heroLayout->addLayout(headerLayout);

    QLabel *captionLabel = createHeroLabel("overviewHeroCaption", tr("Available Balance"));
    heroLayout->addWidget(captionLabel);

    labelHeroBalance = createHeroLabel("overviewHeroBalance", QString("0 CURE"));
    labelHeroBalance->setCursor(Qt::IBeamCursor);
    labelHeroBalance->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
    heroLayout->addWidget(labelHeroBalance);

    QLabel *subtitleLabel = createHeroLabel("overviewHeroSubtitle", tr("Monitor wallet funds, staking readiness, and research activity from one control surface."));
    subtitleLabel->setWordWrap(true);
    heroLayout->addWidget(subtitleLabel);

    QGridLayout *metricsLayout = new QGridLayout();
    metricsLayout->setContentsMargins(0, 0, 0, 0);
    metricsLayout->setHorizontalSpacing(10);
    metricsLayout->setVerticalSpacing(10);

    labelHeroStake = createHeroLabel("overviewMetricValue", QString("0 CURE"));
    labelHeroUnconfirmed = createHeroLabel("overviewMetricValue", QString("0 CURE"));
    labelHeroTransactions = createHeroLabel("overviewMetricValue", QString("0"));
    metricsLayout->addWidget(createMetricCard(tr("Stake"), labelHeroStake), 0, 0);
    metricsLayout->addWidget(createMetricCard(tr("Unconfirmed"), labelHeroUnconfirmed), 0, 1);
    metricsLayout->addWidget(createMetricCard(tr("Transactions"), labelHeroTransactions), 0, 2);
    heroLayout->addLayout(metricsLayout);

    QFont heroFont = GUIUtil::tabularAmountFont();
    heroFont.setBold(true);
    heroFont.setPointSize(qMax(heroFont.pointSize() + 8, 18));
    labelHeroBalance->setFont(heroFont);

    QFont metricFont = GUIUtil::tabularAmountFont();
    metricFont.setBold(true);
    labelHeroStake->setFont(metricFont);
    labelHeroUnconfirmed->setFont(metricFont);
    labelHeroTransactions->setFont(metricFont);

    ui->verticalLayout_2->insertWidget(0, heroFrame);
}

void OverviewPage::createNetworkSyncPanel()
{
    networkSyncPanel = new NetworkSyncPanel(this);
    ui->verticalLayout_2->insertWidget(1, networkSyncPanel);
    refreshNetworkSyncPanel();
}

void OverviewPage::refreshHeroStatus(bool outOfSync)
{
    if(!labelHeroWalletStatus)
        return;

    labelHeroWalletStatus->setObjectName(outOfSync ? "overviewHeroStatusWarning" : "overviewHeroStatusOk");
    labelHeroWalletStatus->setText(outOfSync ? tr("SYNCING") : tr("SYNCED"));
    labelHeroWalletStatus->style()->unpolish(labelHeroWalletStatus);
    labelHeroWalletStatus->style()->polish(labelHeroWalletStatus);
    labelHeroWalletStatus->update();
}

void OverviewPage::refreshNetworkSyncPanel()
{
    if(!networkSyncPanel)
        return;

    bool syncing = true;
    if(clientModel)
        syncing = clientModel->inInitialBlockDownload() || (currentNumBlocksOfPeers > 0 && currentNumBlocks < currentNumBlocksOfPeers);
    networkSyncPanel->setNetworkState(currentNumConnections, currentNumBlocks, currentNumBlocksOfPeers, syncing);
}

void OverviewPage::handleTransactionClicked(const QModelIndex &index)
{
    if(filter)
        emit transactionClicked(filter->mapToSource(index));
}

OverviewPage::~OverviewPage()
{
    delete ui;
}

void OverviewPage::setClientModel(ClientModel *model)
{
    this->clientModel = model;
    if(model)
    {
        updateNetworkConnections(model->getNumConnections());
        updateNetworkBlocks(model->getNumBlocks(), model->getNumBlocksOfPeers());
        connect(model, SIGNAL(numConnectionsChanged(int)), this, SLOT(updateNetworkConnections(int)));
        connect(model, SIGNAL(numBlocksChanged(int,int)), this, SLOT(updateNetworkBlocks(int,int)));
    }
}

void OverviewPage::setBalance(qint64 balance, qint64 stake, qint64 unconfirmedBalance, qint64 immatureBalance)
{
    int unit = model->getOptionsModel()->getDisplayUnit();
    currentBalance = balance;
    currentStake = stake;
    currentUnconfirmedBalance = unconfirmedBalance;
    currentImmatureBalance = immatureBalance;
    ui->labelBalance->setText(curecoinUnits::formatWithUnit(unit, balance));
    ui->labelStake->setText(curecoinUnits::formatWithUnit(unit, stake));
    ui->labelUnconfirmed->setText(curecoinUnits::formatWithUnit(unit, unconfirmedBalance));
    ui->labelImmature->setText(curecoinUnits::formatWithUnit(unit, immatureBalance));
    if(labelHeroBalance)
        labelHeroBalance->setText(curecoinUnits::formatWithUnit(unit, balance));
    if(labelHeroStake)
        labelHeroStake->setText(curecoinUnits::formatWithUnit(unit, stake));
    if(labelHeroUnconfirmed)
        labelHeroUnconfirmed->setText(curecoinUnits::formatWithUnit(unit, unconfirmedBalance));

    // only show immature (newly mined) balance if it's non-zero, so as not to complicate things
    // for the non-mining users
    bool showImmature = immatureBalance != 0;
    ui->labelImmature->setVisible(showImmature);
    ui->labelImmatureText->setVisible(showImmature);
}

void OverviewPage::setNumTransactions(int count)
{
    ui->labelNumTransactions->setText(QLocale::system().toString(count));
    if(labelHeroTransactions)
        labelHeroTransactions->setText(QLocale::system().toString(count));
    if(labelRecentEmptyState)
    {
        bool isEmpty = count == 0;
        labelRecentEmptyState->setVisible(isEmpty);
        ui->listTransactions->setVisible(!isEmpty);
    }
}

void OverviewPage::updateNetworkConnections(int count)
{
    currentNumConnections = count;
    refreshNetworkSyncPanel();
}

void OverviewPage::updateNetworkBlocks(int count, int countOfPeers)
{
    currentNumBlocks = count;
    currentNumBlocksOfPeers = countOfPeers;
    refreshNetworkSyncPanel();
}

void OverviewPage::setModel(WalletModel *model)
{
    this->model = model;
    if(model && model->getOptionsModel())
    {
        // Set up transaction list
        filter = new TransactionFilterProxy();
        filter->setSourceModel(model->getTransactionTableModel());
        filter->setLimit(NUM_ITEMS);
        filter->setDynamicSortFilter(true);
        filter->setSortRole(Qt::EditRole);
        filter->sort(TransactionTableModel::Status, Qt::DescendingOrder);

        ui->listTransactions->setModel(filter);
        ui->listTransactions->setModelColumn(TransactionTableModel::ToAddress);

        // Keep up to date with wallet
        setBalance(model->getBalance(), model->getStake(), model->getUnconfirmedBalance(), model->getImmatureBalance());
        connect(model, SIGNAL(balanceChanged(qint64, qint64, qint64, qint64)), this, SLOT(setBalance(qint64, qint64, qint64, qint64)));

        setNumTransactions(model->getNumTransactions());
        connect(model, SIGNAL(numTransactionsChanged(int)), this, SLOT(setNumTransactions(int)));

        connect(model->getOptionsModel(), SIGNAL(displayUnitChanged(int)), this, SLOT(updateDisplayUnit()));
    }

    // update the display unit, to not use the default ("BTC")
    updateDisplayUnit();
}

void OverviewPage::updateDisplayUnit()
{
    if(model && model->getOptionsModel())
    {
        if(currentBalance != -1)
            setBalance(currentBalance, model->getStake(), currentUnconfirmedBalance, currentImmatureBalance);

        // Update txdelegate->unit with the current unit
        txdelegate->unit = model->getOptionsModel()->getDisplayUnit();

        ui->listTransactions->update();
    }
}

void OverviewPage::showOutOfSyncWarning(bool fShow)
{
    ui->labelWalletStatus->setVisible(fShow);
    ui->labelTransactionsStatus->setVisible(fShow);
    refreshHeroStatus(fShow);
}
