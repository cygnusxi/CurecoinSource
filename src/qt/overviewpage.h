#ifndef OVERVIEWPAGE_H
#define OVERVIEWPAGE_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QLabel;
class QModelIndex;
QT_END_NAMESPACE

namespace Ui {
    class OverviewPage;
}
class ClientModel;
class WalletModel;
class NetworkSyncPanel;
class TxViewDelegate;
class TransactionFilterProxy;

/** Overview ("home") page widget */
class OverviewPage : public QWidget
{
    Q_OBJECT

public:
    explicit OverviewPage(QWidget *parent = 0);
    ~OverviewPage();

    void setClientModel(ClientModel *model);
    void setModel(WalletModel *model);
    void showOutOfSyncWarning(bool fShow);

public slots:
    void setBalance(qint64 balance, qint64 stake, qint64 unconfirmedBalance, qint64 immatureBalance);
    void setNumTransactions(int count);

signals:
    void transactionClicked(const QModelIndex &index);

private:
    Ui::OverviewPage *ui;
    ClientModel *clientModel;
    WalletModel *model;
    qint64 currentBalance;
    qint64 currentStake;
    qint64 currentUnconfirmedBalance;
    qint64 currentImmatureBalance;

    TxViewDelegate *txdelegate;
    TransactionFilterProxy *filter;

    QLabel *labelHeroBalance;
    QLabel *labelHeroStake;
    QLabel *labelHeroUnconfirmed;
    QLabel *labelHeroTransactions;
    QLabel *labelHeroWalletStatus;
    QLabel *labelHeroResearchStatus;
    QLabel *labelRecentEmptyState;
    NetworkSyncPanel *networkSyncPanel;
    int currentNumConnections;
    int currentNumBlocks;
    int currentNumBlocksOfPeers;

    void createHeroPanel();
    void createNetworkSyncPanel();
    void refreshHeroStatus(bool outOfSync);
    void refreshNetworkSyncPanel();
    void updateNetworkSyncPanelVisibility(const QString &themeId);

private slots:
    void updateDisplayUnit();
    void handleTransactionClicked(const QModelIndex &index);
    void updateNetworkConnections(int count);
    void updateNetworkBlocks(int count, int countOfPeers);
    void updateGuiTheme(const QString &themeId);
};

#endif // OVERVIEWPAGE_H
