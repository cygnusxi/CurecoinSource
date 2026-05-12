#include "overviewpage.h"
#include "ui_overviewpage.h"

#include "walletmodel.h"
#include "curecoinunits.h"
#include "optionsmodel.h"
#include "transactiontablemodel.h"
#include "transactionfilterproxy.h"
#include "guiutil.h"
#include "guiconstants.h"

#include <QAbstractItemDelegate>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QSizePolicy>
#include <QStyle>
#include <QVBoxLayout>

#define DECORATION_SIZE 54
#define NUM_ITEMS 7

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

        QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
        QRect mainRect = option.rect;
        QRect decorationRect(mainRect.left() + 4, mainRect.top() + (mainRect.height() - DECORATION_SIZE) / 2, DECORATION_SIZE, DECORATION_SIZE);
        int xspace = DECORATION_SIZE + 18;
        int ypad = 9;
        int halfheight = (mainRect.height() - 2*ypad)/2;
        QRect topLineRect(mainRect.left() + xspace, mainRect.top()+ypad, mainRect.width() - xspace - 6, halfheight);
        QRect addressRect(mainRect.left() + xspace, mainRect.top()+ypad+halfheight, mainRect.width() - xspace, halfheight);
        icon.paint(painter, decorationRect);

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

        painter->setPen(foreground);
        painter->drawText(addressRect, Qt::AlignLeft|Qt::AlignVCenter, address);

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
        QFont regularFont = painter->font();
        QFont amountFont = GUIUtil::tabularAmountFont();
        QFontMetrics amountMetrics(amountFont);
        int amountWidth = qMin(topLineRect.width(), amountMetrics.width(amountText) + 10);
        QRect amountRect(topLineRect.right() - amountWidth + 1, topLineRect.top(), amountWidth, topLineRect.height());
        QRect dateRect(topLineRect.left(), topLineRect.top(), qMax(0, topLineRect.width() - amountWidth - 10), topLineRect.height());

        painter->setFont(amountFont);
        painter->drawText(amountRect, Qt::AlignRight|Qt::AlignVCenter, amountText);
        painter->setFont(regularFont);

        painter->setPen(option.palette.color(QPalette::Text));
        painter->drawText(dateRect, Qt::AlignLeft|Qt::AlignVCenter,
                          painter->fontMetrics().elidedText(GUIUtil::dateTimeStr(date), Qt::ElideRight, dateRect.width()));

        painter->restore();
    }

    inline QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        return QSize(DECORATION_SIZE, DECORATION_SIZE + 18);
    }

    int unit;

};
#include "overviewpage.moc"

OverviewPage::OverviewPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OverviewPage),
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
    labelHeroResearchStatus(0)
{
    ui->setupUi(this);
    createHeroPanel();

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
    ui->listTransactions->setMinimumHeight(NUM_ITEMS * (DECORATION_SIZE + 20));
    ui->listTransactions->setAttribute(Qt::WA_MacShowFocusRect, false);

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

void OverviewPage::handleTransactionClicked(const QModelIndex &index)
{
    if(filter)
        emit transactionClicked(filter->mapToSource(index));
}

OverviewPage::~OverviewPage()
{
    delete ui;
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
