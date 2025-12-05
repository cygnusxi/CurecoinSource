// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2013-2025 CureCoin Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "wallet.h"
#include "walletdb.h"
#include "curecoinrpc.h"
#include "init.h"
#include "base58.h"

#include <algorithm>
#include <list>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

int64 nWalletUnlockTime;
static CCriticalSection cs_nWalletUnlockTime;

extern void TxToJSON(const CTransaction& tx, const uint256 hashBlock, json_spirit::Object& entry);

std::string HelpRequiringPassphrase()
{
    return pwalletMain->IsCrypted()
        ? "\nrequires wallet passphrase to be set with walletpassphrase first"
        : "";
}

void EnsureWalletIsUnlocked()
{
    if (pwalletMain->IsLocked())
        throw JSONRPCError(RPC_WALLET_UNLOCK_NEEDED, "Error: Please enter the wallet passphrase with walletpassphrase first.");
    if (fWalletUnlockMintOnly)
        throw JSONRPCError(RPC_WALLET_UNLOCK_NEEDED, "Error: Wallet unlocked for block minting only.");
}

void WalletTxToJSON(const CWalletTx& wtx, json_spirit::Object& entry)
{
    int confirms = wtx.GetDepthInMainChain();
    entry.push_back(json_spirit::Pair("confirmations", confirms));
    if (wtx.IsCoinBase() || wtx.IsCoinStake())
        entry.push_back(json_spirit::Pair("generated", true));
    if (confirms)
    {
        entry.push_back(json_spirit::Pair("blockhash", wtx.hashBlock.GetHex()));
        entry.push_back(json_spirit::Pair("blockindex", wtx.nIndex));
        entry.push_back(json_spirit::Pair("blocktime", (boost::int64_t)(mapBlockIndex[wtx.hashBlock]->nTime)));
    }
    entry.push_back(json_spirit::Pair("txid", wtx.GetHash().GetHex()));
    entry.push_back(json_spirit::Pair("time", (boost::int64_t)wtx.GetTxTime()));
    entry.push_back(json_spirit::Pair("timereceived", (boost::int64_t)wtx.nTimeReceived));
    entry.push_back(json_spirit::Pair("tx-comment", wtx.strTxComment));
    BOOST_FOREACH(const PAIRTYPE(std::string,std::string)& item, wtx.mapValue)
        entry.push_back(json_spirit::Pair(item.first, item.second));
}

std::string AccountFromValue(const json_spirit::Value& value)
{
    std::string strAccount = value.get_str();
    if (strAccount == "*")
        throw JSONRPCError(RPC_WALLET_INVALID_ACCOUNT_NAME, "Invalid account name");
    return strAccount;
}

json_spirit::Value getinfo(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp || params.size() != 0)
        throw std::runtime_error(
            "getinfo\n"
            "Returns an object containing various state info.");

    proxyType proxy;
    GetProxy(NET_IPV4, proxy);

    json_spirit::Object obj;
    obj.push_back(json_spirit::Pair("version",       FormatFullVersion()));
    obj.push_back(json_spirit::Pair("protocolversion",(int)PROTOCOL_VERSION));
    obj.push_back(json_spirit::Pair("walletversion", pwalletMain->GetVersion()));
    obj.push_back(json_spirit::Pair("balance",       ValueFromAmount(pwalletMain->GetBalance())));
    obj.push_back(json_spirit::Pair("newmint",       ValueFromAmount(pwalletMain->GetNewMint())));
    obj.push_back(json_spirit::Pair("stake",         ValueFromAmount(pwalletMain->GetStake())));
    obj.push_back(json_spirit::Pair("blocks",        (int)nBestHeight));
    obj.push_back(json_spirit::Pair("moneysupply",   ValueFromAmount(pindexBest->nMoneySupply)));
    obj.push_back(json_spirit::Pair("connections",   (int)vNodes.size()));
    obj.push_back(json_spirit::Pair("proxy",         (proxy.first.IsValid() ? proxy.first.ToStringIPPort() : std::string())));
    obj.push_back(json_spirit::Pair("ip",            addrSeenByPeer.ToStringIP()));
    obj.push_back(json_spirit::Pair("difficulty",    (double)GetDifficulty()));
    obj.push_back(json_spirit::Pair("testnet",       fTestNet));
    obj.push_back(json_spirit::Pair("keypoololdest", (boost::int64_t)pwalletMain->GetOldestKeyPoolTime()));
    obj.push_back(json_spirit::Pair("keypoolsize",   pwalletMain->GetKeyPoolSize()));
    obj.push_back(json_spirit::Pair("paytxfee",      ValueFromAmount(nTransactionFee)));
    if (pwalletMain->IsCrypted())
        obj.push_back(json_spirit::Pair("unlocked_until", (boost::int64_t)nWalletUnlockTime / 1000));
    obj.push_back(json_spirit::Pair("errors",        GetWarnings("statusbar")));
    return obj;
}


json_spirit::Value getnewpubkey(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp || params.size() > 1)
        throw std::runtime_error(
            "getnewpubkey [account]\n"
            "Returns new public key for coinbase generation.");

    // Parse the account first so we don't generate a key if there's an error
    std::string strAccount;
    if (params.size() > 0)
        strAccount = AccountFromValue(params[0]);

    if (!pwalletMain->IsLocked())
        pwalletMain->TopUpKeyPool();

    // Generate a new key that is added to wallet
    CPubKey newKey;
    if (!pwalletMain->GetKeyFromPool(newKey, false))
        throw JSONRPCError(RPC_WALLET_KEYPOOL_RAN_OUT, "Error: Keypool ran out, please call keypoolrefill first");
    CKeyID keyID = newKey.GetID();

    pwalletMain->SetAddressBookName(keyID, strAccount);
    std::vector<unsigned char> vchPubKey = newKey.Raw();

    return HexStr(vchPubKey.begin(), vchPubKey.end());
}


json_spirit::Value getnewaddress(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp || params.size() > 1)
        throw std::runtime_error(
            "getnewaddress [account]\n"
            "Returns a new curecoin address for receiving payments.  "
            "If [account] is specified (recommended), it is added to the address book "
            "so payments received with the address will be credited to [account].");

    // Parse the account first so we don't generate a key if there's an error
    std::string strAccount;
    if (params.size() > 0)
        strAccount = AccountFromValue(params[0]);

    if (!pwalletMain->IsLocked())
        pwalletMain->TopUpKeyPool();

    // Generate a new key that is added to wallet
    CPubKey newKey;
    if (!pwalletMain->GetKeyFromPool(newKey, false))
        throw JSONRPCError(RPC_WALLET_KEYPOOL_RAN_OUT, "Error: Keypool ran out, please call keypoolrefill first");
    CKeyID keyID = newKey.GetID();

    pwalletMain->SetAddressBookName(keyID, strAccount);

    return CcurecoinAddress(keyID).ToString();
}


CcurecoinAddress GetAccountAddress(std::string strAccount, bool bForceNew=false)
{
    CWalletDB walletdb(pwalletMain->strWalletFile);

    CAccount account;
    walletdb.ReadAccount(strAccount, account);

    bool bKeyUsed = false;

    // Check if the current key has been used
    if (account.vchPubKey.IsValid())
    {
        CScript scriptPubKey;
        scriptPubKey.SetDestination(account.vchPubKey.GetID());
        for (std::map<uint256, CWalletTx>::iterator it = pwalletMain->mapWallet.begin();
             it != pwalletMain->mapWallet.end() && account.vchPubKey.IsValid();
             ++it)
        {
            const CWalletTx& wtx = (*it).second;
            BOOST_FOREACH(const CTxOut& txout, wtx.vout)
                if (txout.scriptPubKey == scriptPubKey)
                    bKeyUsed = true;
        }
    }

    // Generate a new key
    if (!account.vchPubKey.IsValid() || bForceNew || bKeyUsed)
    {
        if (!pwalletMain->GetKeyFromPool(account.vchPubKey, false))
            throw JSONRPCError(RPC_WALLET_KEYPOOL_RAN_OUT, "Error: Keypool ran out, please call keypoolrefill first");

        pwalletMain->SetAddressBookName(account.vchPubKey.GetID(), strAccount);
        walletdb.WriteAccount(strAccount, account);
    }

    return CcurecoinAddress(account.vchPubKey.GetID());
}

json_spirit::Value getaccountaddress(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw std::runtime_error(
            "getaccountaddress <account>\n"
            "Returns the current curecoin address for receiving payments to this account.");

    // Parse the account first so we don't generate a key if there's an error
    std::string strAccount = AccountFromValue(params[0]);

    json_spirit::Value ret;

    ret = GetAccountAddress(strAccount).ToString();

    return ret;
}



json_spirit::Value setaccount(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 2)
        throw std::runtime_error(
            "setaccount <curecoinAddress> <account>\n"
            "Sets the account associated with the given address.");

    CcurecoinAddress address(params[0].get_str());
    if (!address.IsValid())
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid curecoin address");


    std::string strAccount;
    if (params.size() > 1)
        strAccount = AccountFromValue(params[1]);

    // Detect when changing the account of an address that is the 'unused current key' of another account:
    if (pwalletMain->mapAddressBook.count(address.Get()))
    {
        std::string strOldAccount = pwalletMain->mapAddressBook[address.Get()];
        if (address == GetAccountAddress(strOldAccount))
            GetAccountAddress(strOldAccount, true);
    }

    pwalletMain->SetAddressBookName(address.Get(), strAccount);

    return json_spirit::Value::null;
}


json_spirit::Value getaccount(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw std::runtime_error(
            "getaccount <curecoinAddress>\n"
            "Returns the account associated with the given address.");

    CcurecoinAddress address(params[0].get_str());
    if (!address.IsValid())
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid curecoin address");

    std::string strAccount;
    std::map<CTxDestination, std::string>::iterator mi = pwalletMain->mapAddressBook.find(address.Get());
    if (mi != pwalletMain->mapAddressBook.end() && !(*mi).second.empty())
        strAccount = (*mi).second;
    return strAccount;
}


json_spirit::Value getaddressesbyaccount(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw std::runtime_error(
            "getaddressesbyaccount <account>\n"
            "Returns the list of addresses for the given account.");

    std::string strAccount = AccountFromValue(params[0]);

    // Find all addresses that have the given account
    json_spirit::Array ret;
    BOOST_FOREACH(const PAIRTYPE(CcurecoinAddress, std::string)& item, pwalletMain->mapAddressBook)
    {
        const CcurecoinAddress& address = item.first;
        const std::string& strName = item.second;
        if (strName == strAccount)
            ret.push_back(address.ToString());
    }
    return ret;
}

json_spirit::Value sendtoaddress(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp || params.size() < 2 || params.size() > 4)
        throw std::runtime_error(
            "sendtoaddress <curecoinAddress> <amount> [comment] [comment-to]\n"
            "<amount> is a real and is rounded to the nearest 0.000001"
            + HelpRequiringPassphrase());

    CcurecoinAddress address(params[0].get_str());
    if (!address.IsValid())
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid curecoin address");

    // Amount
    int64 nAmount = AmountFromValue(params[1]);

    if (nAmount < MIN_TXOUT_AMOUNT)
        throw JSONRPCError(-101, "Send amount too small");

    // Wallet comments
    CWalletTx wtx;
    if (params.size() > 2 && params[2].type() != json_spirit::null_type && !params[2].get_str().empty())
        wtx.mapValue["comment"] = params[2].get_str();
    if (params.size() > 3 && params[3].type() != json_spirit::null_type && !params[3].get_str().empty())
        wtx.mapValue["to"]      = params[3].get_str();

    // Transaction comment
    std::string txcomment;
    if (params.size() > 4 && params[4].type() != json_spirit::null_type && !params[4].get_str().empty())
    {
        txcomment = params[4].get_str();
        if (txcomment.length() > MAX_TX_COMMENT_LEN)
            txcomment.resize(MAX_TX_COMMENT_LEN);
    }

    if (pwalletMain->IsLocked())
        throw JSONRPCError(RPC_WALLET_UNLOCK_NEEDED, "Error: Please enter the wallet passphrase with walletpassphrase first.");

    //string strError = pwalletMain->SendMoneyToDestination(address.Get(), nAmount, wtx);
    std::string strError = pwalletMain->SendMoneyToDestination(address.Get(), nAmount, wtx, false, txcomment);

    if (strError != "")
        throw JSONRPCError(RPC_WALLET_ERROR, strError);

    return wtx.GetHash().GetHex();
}

json_spirit::Value listaddressgroupings(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp)
        throw std::runtime_error(
            "listaddressgroupings\n"
            "Lists groups of addresses which have had their common ownership\n"
            "made public by common use as inputs or as the resulting change\n"
            "in past transactions");

    json_spirit::Array jsonGroupings;
    std::map<CTxDestination, int64> balances = pwalletMain->GetAddressBalances();
    BOOST_FOREACH(std::set<CTxDestination> grouping, pwalletMain->GetAddressGroupings())
    {
        json_spirit::Array jsonGrouping;
        BOOST_FOREACH(CTxDestination address, grouping)
        {
            json_spirit::Array addressInfo;
            addressInfo.push_back(CcurecoinAddress(address).ToString());
            addressInfo.push_back(ValueFromAmount(balances[address]));
            {
                LOCK(pwalletMain->cs_wallet);
                if (pwalletMain->mapAddressBook.find(CcurecoinAddress(address).Get()) != pwalletMain->mapAddressBook.end())
                    addressInfo.push_back(pwalletMain->mapAddressBook.find(CcurecoinAddress(address).Get())->second);
            }
            jsonGrouping.push_back(addressInfo);
        }
        jsonGroupings.push_back(jsonGrouping);
    }
    return jsonGroupings;
}

json_spirit::Value signmessage(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp || params.size() != 2)
        throw std::runtime_error(
            "signmessage <curecoinAddress> <message>\n"
            "Sign a message with the private key of an address");

    EnsureWalletIsUnlocked();

    std::string strAddress = params[0].get_str();
    std::string strMessage = params[1].get_str();

    CcurecoinAddress addr(strAddress);
    if (!addr.IsValid())
        throw JSONRPCError(RPC_TYPE_ERROR, "Invalid address");

    CKeyID keyID;
    if (!addr.GetKeyID(keyID))
        throw JSONRPCError(RPC_TYPE_ERROR, "Address does not refer to key");

    CKey key;
    if (!pwalletMain->GetKey(keyID, key))
        throw JSONRPCError(RPC_WALLET_ERROR, "Private key not available");

    CDataStream ss(SER_GETHASH, 0);
    ss << strMessageMagic;
    ss << strMessage;

    std::vector<unsigned char> vchSig;
    if (!key.SignCompact(Hash(ss.begin(), ss.end()), vchSig))
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Sign failed");

    return EncodeBase64(&vchSig[0], vchSig.size());
}

json_spirit::Value verifymessage(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp || params.size() != 3)
        throw std::runtime_error(
            "verifymessage <curecoinAddress> <signature> <message>\n"
            "Verify a signed message");

    std::string strAddress  = params[0].get_str();
    std::string strSign     = params[1].get_str();
    std::string strMessage  = params[2].get_str();

    CcurecoinAddress addr(strAddress);
    if (!addr.IsValid())
        throw JSONRPCError(RPC_TYPE_ERROR, "Invalid address");

    CKeyID keyID;
    if (!addr.GetKeyID(keyID))
        throw JSONRPCError(RPC_TYPE_ERROR, "Address does not refer to key");

    bool fInvalid = false;
    std::vector<unsigned char> vchSig = DecodeBase64(strSign.c_str(), &fInvalid);

    if (fInvalid)
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Malformed base64 encoding");

    CDataStream ss(SER_GETHASH, 0);
    ss << strMessageMagic;
    ss << strMessage;

    CKey key;
    if (!key.SetCompactSignature(Hash(ss.begin(), ss.end()), vchSig))
        return false;

    return (key.GetPubKey().GetID() == keyID);
}


json_spirit::Value getreceivedbyaddress(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 2)
        throw std::runtime_error(
            "getreceivedbyaddress <curecoinaddress> [minconf=1]\n"
            "Returns the total amount received by <curecoinaddress> in transactions with at least [minconf] confirmations.");

    // curecoin address
    CcurecoinAddress address = CcurecoinAddress(params[0].get_str());
    CScript scriptPubKey;
    if (!address.IsValid())
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid curecoin address");
    scriptPubKey.SetDestination(address.Get());
    if (!IsMine(*pwalletMain,scriptPubKey))
        return (double)0.0;

    // Minimum confirmations
    int nMinDepth = 1;
    if (params.size() > 1)
        nMinDepth = params[1].get_int();

    // Tally
    int64 nAmount = 0;
    for (std::map<uint256, CWalletTx>::iterator it = pwalletMain->mapWallet.begin(); it != pwalletMain->mapWallet.end(); ++it)
    {
        const CWalletTx& wtx = (*it).second;
        if (wtx.IsCoinBase() || wtx.IsCoinStake() || !wtx.IsFinal())
            continue;

        BOOST_FOREACH(const CTxOut& txout, wtx.vout)
            if (txout.scriptPubKey == scriptPubKey)
                if (wtx.GetDepthInMainChain() >= nMinDepth)
                    nAmount += txout.nValue;
    }

    return  ValueFromAmount(nAmount);
}


void GetAccountAddresses(std::string strAccount, std::set<CTxDestination>& setAddress)
{
    BOOST_FOREACH(const PAIRTYPE(CTxDestination, std::string)& item, pwalletMain->mapAddressBook)
    {
        const CTxDestination& address = item.first;
        const std::string& strName = item.second;
        if (strName == strAccount)
            setAddress.insert(address);
    }
}

json_spirit::Value getreceivedbyaccount(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 2)
        throw std::runtime_error(
            "getreceivedbyaccount <account> [minconf=1]\n"
            "Returns the total amount received by addresses with <account> in transactions with at least [minconf] confirmations.");

    // Minimum confirmations
    int nMinDepth = 1;
    if (params.size() > 1)
        nMinDepth = params[1].get_int();

    // Get the set of pub keys assigned to account
    std::string strAccount = AccountFromValue(params[0]);
    std::set<CTxDestination> setAddress;
    GetAccountAddresses(strAccount, setAddress);

    // Tally
    int64 nAmount = 0;
    for (std::map<uint256, CWalletTx>::iterator it = pwalletMain->mapWallet.begin(); it != pwalletMain->mapWallet.end(); ++it)
    {
        const CWalletTx& wtx = (*it).second;
        if (wtx.IsCoinBase() || wtx.IsCoinStake() || !wtx.IsFinal())
            continue;

        BOOST_FOREACH(const CTxOut& txout, wtx.vout)
        {
            CTxDestination address;
            if (ExtractDestination(txout.scriptPubKey, address) && IsMine(*pwalletMain, address) && setAddress.count(address))
                if (wtx.GetDepthInMainChain() >= nMinDepth)
                    nAmount += txout.nValue;
        }
    }

    return (double)nAmount / (double)COIN;
}


int64 GetAccountBalance(CWalletDB& walletdb, const std::string& strAccount, int nMinDepth)
{
    int64 nBalance = 0;

    // Tally wallet transactions
    for (std::map<uint256, CWalletTx>::iterator it = pwalletMain->mapWallet.begin(); it != pwalletMain->mapWallet.end(); ++it)
    {
        const CWalletTx& wtx = (*it).second;
        if (!wtx.IsFinal())
            continue;

        int64 nGenerated, nReceived, nSent, nFee;
        wtx.GetAccountAmounts(strAccount, nGenerated, nReceived, nSent, nFee);

        if (nReceived != 0 && wtx.GetDepthInMainChain() >= nMinDepth)
            nBalance += nReceived;
        nBalance += nGenerated - nSent - nFee;
    }

    // Tally internal accounting entries
    nBalance += walletdb.GetAccountCreditDebit(strAccount);

    return nBalance;
}

int64 GetAccountBalance(const std::string& strAccount, int nMinDepth)
{
    CWalletDB walletdb(pwalletMain->strWalletFile);
    return GetAccountBalance(walletdb, strAccount, nMinDepth);
}


json_spirit::Value getbalance(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp || params.size() > 2)
        throw std::runtime_error(
            "getbalance [account] [minconf=1]\n"
            "If [account] is not specified, returns the server's total available balance.\n"
            "If [account] is specified, returns the balance in the account.");

    if (params.size() == 0)
        return  ValueFromAmount(pwalletMain->GetBalance());

    int nMinDepth = 1;
    if (params.size() > 1)
        nMinDepth = params[1].get_int();

    if (params[0].get_str() == "*") {
        // Calculate total balance a different way from GetBalance()
        // (GetBalance() sums up all unspent TxOuts)
        // getbalance and getbalance '*' should always return the same number.
        int64 nBalance = 0;
        for (std::map<uint256, CWalletTx>::iterator it = pwalletMain->mapWallet.begin(); it != pwalletMain->mapWallet.end(); ++it)
        {
            const CWalletTx& wtx = (*it).second;
            if (!wtx.IsFinal())
                continue;

            int64 allGeneratedImmature, allGeneratedMature, allFee;
            allGeneratedImmature = allGeneratedMature = allFee = 0;

            std::string strSentAccount;
            std::list<std::pair<CTxDestination, int64> > listReceived;
            std::list<std::pair<CTxDestination, int64> > listSent;
            wtx.GetAmounts(allGeneratedImmature, allGeneratedMature, listReceived, listSent, allFee, strSentAccount);
            if (wtx.GetDepthInMainChain() >= nMinDepth)
            {
                BOOST_FOREACH(const PAIRTYPE(CTxDestination,int64)& r, listReceived)
                    nBalance += r.second;
            }
            BOOST_FOREACH(const PAIRTYPE(CTxDestination,int64)& r, listSent)
                nBalance -= r.second;
            nBalance -= allFee;
            nBalance += allGeneratedMature;
        }
        return  ValueFromAmount(nBalance);
    }

    std::string strAccount = AccountFromValue(params[0]);

    int64 nBalance = GetAccountBalance(strAccount, nMinDepth);

    return ValueFromAmount(nBalance);
}


json_spirit::Value movecmd(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp || params.size() < 3 || params.size() > 5)
        throw std::runtime_error(
            "move <fromaccount> <toaccount> <amount> [minconf=1] [comment]\n"
            "Move from one account in your wallet to another.");

    std::string strFrom = AccountFromValue(params[0]);
    std::string strTo = AccountFromValue(params[1]);
    int64 nAmount = AmountFromValue(params[2]);

    if (nAmount < MIN_TXOUT_AMOUNT)
        throw JSONRPCError(-101, "Send amount too small");

    if (params.size() > 3)
        // unused parameter, used to be nMinDepth, keep type-checking it though
        (void)params[3].get_int();
    std::string strComment;
    if (params.size() > 4)
        strComment = params[4].get_str();

    CWalletDB walletdb(pwalletMain->strWalletFile);
    if (!walletdb.TxnBegin())
        throw JSONRPCError(RPC_DATABASE_ERROR, "database error");

    int64 nNow = GetAdjustedTime();

    // Debit
    CAccountingEntry debit;
    debit.nOrderPos = pwalletMain->IncOrderPosNext(&walletdb);
    debit.strAccount = strFrom;
    debit.nCreditDebit = -nAmount;
    debit.nTime = nNow;
    debit.strOtherAccount = strTo;
    debit.strComment = strComment;
    walletdb.WriteAccountingEntry(debit);

    // Credit
    CAccountingEntry credit;
    credit.nOrderPos = pwalletMain->IncOrderPosNext(&walletdb);
    credit.strAccount = strTo;
    credit.nCreditDebit = nAmount;
    credit.nTime = nNow;
    credit.strOtherAccount = strFrom;
    credit.strComment = strComment;
    walletdb.WriteAccountingEntry(credit);

    if (!walletdb.TxnCommit())
        throw JSONRPCError(RPC_DATABASE_ERROR, "database error");

    return true;
}


json_spirit::Value sendfrom(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp || params.size() < 3 || params.size() > 6)
        throw std::runtime_error(
            "sendfrom <fromaccount> <TocurecoinAddress> <amount> [minconf=1] [comment] [comment-to]\n"
            "<amount> is a real and is rounded to the nearest 0.000001"
            + HelpRequiringPassphrase());

    std::string strAccount = AccountFromValue(params[0]);
    CcurecoinAddress address(params[1].get_str());
    if (!address.IsValid())
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid curecoin address");
    int64 nAmount = AmountFromValue(params[2]);

    if (nAmount < MIN_TXOUT_AMOUNT)
        throw JSONRPCError(-101, "Send amount too small");

    int nMinDepth = 1;
    if (params.size() > 3)
        nMinDepth = params[3].get_int();

    CWalletTx wtx;
    wtx.strFromAccount = strAccount;
    if (params.size() > 4 && params[4].type() != json_spirit::null_type && !params[4].get_str().empty())
        wtx.mapValue["comment"] = params[4].get_str();
    if (params.size() > 5 && params[5].type() != json_spirit::null_type && !params[5].get_str().empty())
        wtx.mapValue["to"]      = params[5].get_str();

    std::string txcomment;
    if (params.size() > 6 && params[6].type() != json_spirit::null_type && !params[6].get_str().empty())
    {
        txcomment = params[6].get_str();
        if (txcomment.length() > MAX_TX_COMMENT_LEN)
            txcomment.resize(MAX_TX_COMMENT_LEN);
    }

    EnsureWalletIsUnlocked();

    // Check funds
    int64 nBalance = GetAccountBalance(strAccount, nMinDepth);
    if (nAmount > nBalance)
        throw JSONRPCError(RPC_WALLET_INSUFFICIENT_FUNDS, "Account has insufficient funds");

    // Send
    std::string strError = pwalletMain->SendMoneyToDestination(address.Get(), nAmount, wtx, false, txcomment);
    if (strError != "")
        throw JSONRPCError(RPC_WALLET_ERROR, strError);

    return wtx.GetHash().GetHex();
}


json_spirit::Value sendmany(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp || params.size() < 2 || params.size() > 4)
        throw std::runtime_error(
            "sendmany <fromaccount> {address:amount,...} [minconf=1] [comment]\n"
            "amounts are double-precision floating point numbers"
            + HelpRequiringPassphrase());

    std::string strAccount = AccountFromValue(params[0]);
    json_spirit::Object sendTo = params[1].get_obj();
    int nMinDepth = 1;
    if (params.size() > 2)
        nMinDepth = params[2].get_int();

    CWalletTx wtx;
    std::string strTxComment;

    wtx.strFromAccount = strAccount;
    if (params.size() > 3 && params[3].type() != json_spirit::null_type && !params[3].get_str().empty())
        wtx.mapValue["comment"] = params[3].get_str();
    if (params.size() > 4 && params[4].type() != json_spirit::null_type && !params[4].get_str().empty())
        strTxComment = params[4].get_str();

    std::set<CcurecoinAddress> setAddress;
    std::vector<std::pair<CScript, int64> > vecSend;

    int64 totalAmount = 0;
    BOOST_FOREACH(const json_spirit::Pair& s, sendTo)
    {
        CcurecoinAddress address(s.name_);
        if (!address.IsValid())
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, std::string("Invalid curecoin address: ")+s.name_);

        if (setAddress.count(address))
            throw JSONRPCError(RPC_INVALID_PARAMETER, std::string("Invalid parameter, duplicated address: ")+s.name_);
        setAddress.insert(address);

        CScript scriptPubKey;
        scriptPubKey.SetDestination(address.Get());
        int64 nAmount = AmountFromValue(s.value_);

        if (nAmount < MIN_TXOUT_AMOUNT)
            throw JSONRPCError(-101, "Send amount too small");

        totalAmount += nAmount;

        vecSend.push_back(std::make_pair(scriptPubKey, nAmount));
    }

    EnsureWalletIsUnlocked();

    // Check funds
    int64 nBalance = GetAccountBalance(strAccount, nMinDepth);
    if (totalAmount > nBalance)
        throw JSONRPCError(RPC_WALLET_INSUFFICIENT_FUNDS, "Account has insufficient funds");

    // Send
    CReserveKey keyChange(pwalletMain);
    int64 nFeeRequired = 0;
    bool fCreated = pwalletMain->CreateTransaction(vecSend, wtx, keyChange, nFeeRequired, strTxComment);
    if (!fCreated)
    {
        if (totalAmount + nFeeRequired > pwalletMain->GetBalance())
            throw JSONRPCError(RPC_WALLET_INSUFFICIENT_FUNDS, "Insufficient funds");
        throw JSONRPCError(RPC_WALLET_ERROR, "Transaction creation failed");
    }
    if (!pwalletMain->CommitTransaction(wtx, keyChange))
        throw JSONRPCError(RPC_WALLET_ERROR, "Transaction commit failed");

    return wtx.GetHash().GetHex();
}

json_spirit::Value addmultisigaddress(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp || params.size() < 2 || params.size() > 3)
    {
        std::string msg = "addmultisigaddress <nrequired> <'[\"key\",\"key\"]'> [account]\n"
            "Add a nrequired-to-sign multisignature address to the wallet\"\n"
            "each key is a curecoin address or hex-encoded public key\n"
            "If [account] is specified, assign address to [account].";
        throw std::runtime_error(msg);
    }

    int nRequired = params[0].get_int();
    const json_spirit::Array& keys = params[1].get_array();
    std::string strAccount;
    if (params.size() > 2)
        strAccount = AccountFromValue(params[2]);

    // Gather public keys
    if (nRequired < 1)
        throw std::runtime_error("a multisignature address must require at least one key to redeem");
    if ((int)keys.size() < nRequired)
        throw std::runtime_error(
            strprintf("not enough keys supplied "
                      "(got %" PRIszu" keys, but need at least %d to redeem)", keys.size(), nRequired));
    std::vector<CKey> pubkeys;
    pubkeys.resize(keys.size());
    for (unsigned int i = 0; i < keys.size(); i++)
    {
        const std::string& ks = keys[i].get_str();

        // Case 1: curecoin address and we have full public key:
        CcurecoinAddress address(ks);
        if (address.IsValid())
        {
            CKeyID keyID;
            if (!address.GetKeyID(keyID))
                throw std::runtime_error(
                    strprintf("%s does not refer to a key",ks.c_str()));
            CPubKey vchPubKey;
            if (!pwalletMain->GetPubKey(keyID, vchPubKey))
                throw std::runtime_error(
                    strprintf("no full public key for address %s",ks.c_str()));
            if (!vchPubKey.IsValid() || !pubkeys[i].SetPubKey(vchPubKey))
                throw std::runtime_error(" Invalid public key: "+ks);
        }

        // Case 2: hex public key
        else if (IsHex(ks))
        {
            CPubKey vchPubKey(ParseHex(ks));
            if (!vchPubKey.IsValid() || !pubkeys[i].SetPubKey(vchPubKey))
                throw std::runtime_error(" Invalid public key: "+ks);
        }
        else
        {
            throw std::runtime_error(" Invalid public key: "+ks);
        }
    }

    // Construct using pay-to-script-hash:
    CScript inner;
    inner.SetMultisig(nRequired, pubkeys);
    CScriptID innerID = inner.GetID();
    pwalletMain->AddCScript(inner);

    pwalletMain->SetAddressBookName(innerID, strAccount);
    return CcurecoinAddress(innerID).ToString();
}


struct tallyitem
{
    int64 nAmount;
    int nConf;
    tallyitem()
    {
        nAmount = 0;
        nConf = std::numeric_limits<int>::max();
    }
};

json_spirit::Value ListReceived(const json_spirit::Array& params, bool fByAccounts)
{
    // Minimum confirmations
    int nMinDepth = 1;
    if (params.size() > 0)
        nMinDepth = params[0].get_int();

    // Whether to include empty accounts
    bool fIncludeEmpty = false;
    if (params.size() > 1)
        fIncludeEmpty = params[1].get_bool();

    // Tally
    std::map<CcurecoinAddress, tallyitem> mapTally;
    for (std::map<uint256, CWalletTx>::iterator it = pwalletMain->mapWallet.begin(); it != pwalletMain->mapWallet.end(); ++it)
    {
        const CWalletTx& wtx = (*it).second;

        if (wtx.IsCoinBase() || wtx.IsCoinStake() || !wtx.IsFinal())
            continue;

        int nDepth = wtx.GetDepthInMainChain();
        if (nDepth < nMinDepth)
            continue;

        BOOST_FOREACH(const CTxOut& txout, wtx.vout)
        {
            CTxDestination address;
            if (!ExtractDestination(txout.scriptPubKey, address) || !IsMine(*pwalletMain, address))
                continue;

            tallyitem& item = mapTally[address];
            item.nAmount += txout.nValue;
            item.nConf = std::min(item.nConf, nDepth);
        }
    }

    // Reply
    json_spirit::Array ret;
    std::map<std::string, tallyitem> mapAccountTally;
    BOOST_FOREACH(const PAIRTYPE(CcurecoinAddress, std::string)& item, pwalletMain->mapAddressBook)
    {
        const CcurecoinAddress& address = item.first;
        const std::string& strAccount = item.second;
        std::map<CcurecoinAddress, tallyitem>::iterator it = mapTally.find(address);
        if (it == mapTally.end() && !fIncludeEmpty)
            continue;

        int64 nAmount = 0;
        int nConf = std::numeric_limits<int>::max();
        if (it != mapTally.end())
        {
            nAmount = (*it).second.nAmount;
            nConf = (*it).second.nConf;
        }

        if (fByAccounts)
        {
            tallyitem& item = mapAccountTally[strAccount];
            item.nAmount += nAmount;
            item.nConf = std::min(item.nConf, nConf);
        }
        else
        {
            json_spirit::Object obj;
            obj.push_back(json_spirit::Pair("address",       address.ToString()));
            obj.push_back(json_spirit::Pair("account",       strAccount));
            obj.push_back(json_spirit::Pair("amount",        ValueFromAmount(nAmount)));
            obj.push_back(json_spirit::Pair("confirmations", (nConf == std::numeric_limits<int>::max() ? 0 : nConf)));
            ret.push_back(obj);
        }
    }

    if (fByAccounts)
    {
        for (std::map<std::string, tallyitem>::iterator it = mapAccountTally.begin(); it != mapAccountTally.end(); ++it)
        {
            int64 nAmount = (*it).second.nAmount;
            int nConf = (*it).second.nConf;
            json_spirit::Object obj;
            obj.push_back(json_spirit::Pair("account",       (*it).first));
            obj.push_back(json_spirit::Pair("amount",        ValueFromAmount(nAmount)));
            obj.push_back(json_spirit::Pair("confirmations", (nConf == std::numeric_limits<int>::max() ? 0 : nConf)));
            ret.push_back(obj);
        }
    }

    return ret;
}

json_spirit::Value listreceivedbyaddress(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp || params.size() > 2)
        throw std::runtime_error(
            "listreceivedbyaddress [minconf=1] [includeempty=false]\n"
            "[minconf] is the minimum number of confirmations before payments are included.\n"
            "[includeempty] whether to include addresses that haven't received any payments.\n"
            "Returns an array of objects containing:\n"
            "  \"address\" : receiving address\n"
            "  \"account\" : the account of the receiving address\n"
            "  \"amount\" : total amount received by the address\n"
            "  \"confirmations\" : number of confirmations of the most recent transaction included");

    return ListReceived(params, false);
}

json_spirit::Value listreceivedbyaccount(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp || params.size() > 2)
        throw std::runtime_error(
            "listreceivedbyaccount [minconf=1] [includeempty=false]\n"
            "[minconf] is the minimum number of confirmations before payments are included.\n"
            "[includeempty] whether to include accounts that haven't received any payments.\n"
            "Returns an array of objects containing:\n"
            "  \"account\" : the account of the receiving addresses\n"
            "  \"amount\" : total amount received by addresses with this account\n"
            "  \"confirmations\" : number of confirmations of the most recent transaction included");

    return ListReceived(params, true);
}

void ListTransactions(const CWalletTx& wtx, const std::string& strAccount, int nMinDepth, bool fLong, json_spirit::Array& ret)
{
    int64 nGeneratedImmature, nGeneratedMature, nFee;
    std::string strSentAccount;
    std::list<std::pair<CTxDestination, int64> > listReceived;
    std::list<std::pair<CTxDestination, int64> > listSent;

    wtx.GetAmounts(nGeneratedImmature, nGeneratedMature, listReceived, listSent, nFee, strSentAccount);

    bool fAllAccounts = (strAccount == std::string("*"));

    // Generated blocks assigned to account ""
    if ((nGeneratedMature+nGeneratedImmature) != 0 && (fAllAccounts || strAccount == ""))
    {
        json_spirit::Object entry;
        entry.push_back(json_spirit::Pair("account", std::string("")));
        if (nGeneratedImmature)
        {
            entry.push_back(json_spirit::Pair("category", wtx.GetDepthInMainChain() ? "immature" : "orphan"));
            entry.push_back(json_spirit::Pair("amount", ValueFromAmount(nGeneratedImmature)));
        }
        else
        {
            entry.push_back(json_spirit::Pair("category", "generate"));
            entry.push_back(json_spirit::Pair("amount", ValueFromAmount(nGeneratedMature)));
        }
        if (fLong)
            WalletTxToJSON(wtx, entry);
        ret.push_back(entry);
    }

    // Sent
    if ((!listSent.empty() || nFee != 0) && (fAllAccounts || strAccount == strSentAccount))
    {
        BOOST_FOREACH(const PAIRTYPE(CTxDestination, int64)& s, listSent)
        {
            json_spirit::Object entry;
            entry.push_back(json_spirit::Pair("account", strSentAccount));
            entry.push_back(json_spirit::Pair("address", CcurecoinAddress(s.first).ToString()));
            entry.push_back(json_spirit::Pair("category", "send"));
            entry.push_back(json_spirit::Pair("amount", ValueFromAmount(-s.second)));
            entry.push_back(json_spirit::Pair("fee", ValueFromAmount(-nFee)));
            if (fLong)
                WalletTxToJSON(wtx, entry);
            ret.push_back(entry);
        }
    }

    // Received
    if (listReceived.size() > 0 && wtx.GetDepthInMainChain() >= nMinDepth)
    {
        BOOST_FOREACH(const PAIRTYPE(CTxDestination, int64)& r, listReceived)
        {
            std::string account;
            if (pwalletMain->mapAddressBook.count(r.first))
                account = pwalletMain->mapAddressBook[r.first];
            if (fAllAccounts || (account == strAccount))
            {
                json_spirit::Object entry;
                entry.push_back(json_spirit::Pair("account", account));
                entry.push_back(json_spirit::Pair("address", CcurecoinAddress(r.first).ToString()));
                if (wtx.IsCoinBase())
                {
                    if (wtx.GetDepthInMainChain() < 1)
                        entry.push_back(json_spirit::Pair("category", "orphan"));
                    else if (wtx.GetBlocksToMaturity() > 0)
                        entry.push_back(json_spirit::Pair("category", "immature"));
                    else
                        entry.push_back(json_spirit::Pair("category", "generate"));
                }
                else
                    entry.push_back(json_spirit::Pair("category", "receive"));
                entry.push_back(json_spirit::Pair("amount", ValueFromAmount(r.second)));
                if (fLong)
                    WalletTxToJSON(wtx, entry);
                ret.push_back(entry);
            }
        }
    }
}

void AcentryToJSON(const CAccountingEntry& acentry, const std::string& strAccount, json_spirit::Array& ret)
{
    bool fAllAccounts = (strAccount == std::string("*"));

    if (fAllAccounts || acentry.strAccount == strAccount)
    {
        json_spirit::Object entry;
        entry.push_back(json_spirit::Pair("account", acentry.strAccount));
        entry.push_back(json_spirit::Pair("category", "move"));
        entry.push_back(json_spirit::Pair("time", (boost::int64_t)acentry.nTime));
        entry.push_back(json_spirit::Pair("amount", ValueFromAmount(acentry.nCreditDebit)));
        entry.push_back(json_spirit::Pair("otheraccount", acentry.strOtherAccount));
        entry.push_back(json_spirit::Pair("comment", acentry.strComment));
        ret.push_back(entry);
    }
}

json_spirit::Value listtransactions(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp || params.size() > 3)
        throw std::runtime_error(
            "listtransactions [account] [count=10] [from=0]\n"
            "Returns up to [count] most recent transactions skipping the first [from] transactions for account [account].");

    std::string strAccount = "*";
    if (params.size() > 0)
        strAccount = params[0].get_str();
    int nCount = 10;
    if (params.size() > 1)
        nCount = params[1].get_int();
    int nFrom = 0;
    if (params.size() > 2)
        nFrom = params[2].get_int();

    if (nCount < 0)
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Negative count");
    if (nFrom < 0)
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Negative from");

    json_spirit::Array ret;

    std::list<CAccountingEntry> acentries;
    CWallet::TxItems txOrdered = pwalletMain->OrderedTxItems(acentries, strAccount);

    // iterate backwards until we have nCount items to return:
    for (CWallet::TxItems::reverse_iterator it = txOrdered.rbegin(); it != txOrdered.rend(); ++it)
    {
        CWalletTx *const pwtx = (*it).second.first;
        if (pwtx != 0)
            ListTransactions(*pwtx, strAccount, 0, true, ret);
        CAccountingEntry *const pacentry = (*it).second.second;
        if (pacentry != 0)
            AcentryToJSON(*pacentry, strAccount, ret);

        if ((int)ret.size() >= (nCount+nFrom)) break;
    }
    // ret is newest to oldest

    if (nFrom > (int)ret.size())
        nFrom = ret.size();
    if ((nFrom + nCount) > (int)ret.size())
        nCount = ret.size() - nFrom;
    json_spirit::Array::iterator first = ret.begin();
    std::advance(first, nFrom);
    json_spirit::Array::iterator last = ret.begin();
    std::advance(last, nFrom+nCount);

    if (last != ret.end()) ret.erase(last, ret.end());
    if (first != ret.begin()) ret.erase(ret.begin(), first);

    std::reverse(ret.begin(), ret.end()); // Return oldest to newest

    return ret;
}

json_spirit::Value listaccounts(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp || params.size() > 1)
        throw std::runtime_error(
            "listaccounts [minconf=1]\n"
            "Returns Object that has account names as keys, account balances as values.");

    int nMinDepth = 1;
    if (params.size() > 0)
        nMinDepth = params[0].get_int();

    std::map<std::string, int64> mapAccountBalances;
    BOOST_FOREACH(const PAIRTYPE(CTxDestination, std::string)& entry, pwalletMain->mapAddressBook) {
        if (IsMine(*pwalletMain, entry.first)) // This address belongs to me
            mapAccountBalances[entry.second] = 0;
    }

    for (std::map<uint256, CWalletTx>::iterator it = pwalletMain->mapWallet.begin(); it != pwalletMain->mapWallet.end(); ++it)
    {
        const CWalletTx& wtx = (*it).second;
        int64 nGeneratedImmature, nGeneratedMature, nFee;
        std::string strSentAccount;
        std::list<std::pair<CTxDestination, int64> > listReceived;
        std::list<std::pair<CTxDestination, int64> > listSent;
        wtx.GetAmounts(nGeneratedImmature, nGeneratedMature, listReceived, listSent, nFee, strSentAccount);
        mapAccountBalances[strSentAccount] -= nFee;
        BOOST_FOREACH(const PAIRTYPE(CTxDestination, int64)& s, listSent)
            mapAccountBalances[strSentAccount] -= s.second;
        if (wtx.GetDepthInMainChain() >= nMinDepth)
        {
            mapAccountBalances[""] += nGeneratedMature;
            BOOST_FOREACH(const PAIRTYPE(CTxDestination, int64)& r, listReceived)
                if (pwalletMain->mapAddressBook.count(r.first))
                    mapAccountBalances[pwalletMain->mapAddressBook[r.first]] += r.second;
                else
                    mapAccountBalances[""] += r.second;
        }
    }

    std::list<CAccountingEntry> acentries;
    CWalletDB(pwalletMain->strWalletFile).ListAccountCreditDebit("*", acentries);
    BOOST_FOREACH(const CAccountingEntry& entry, acentries)
        mapAccountBalances[entry.strAccount] += entry.nCreditDebit;

    json_spirit::Object ret;
    BOOST_FOREACH(const PAIRTYPE(std::string, int64)& accountBalance, mapAccountBalances) {
        ret.push_back(json_spirit::Pair(accountBalance.first, ValueFromAmount(accountBalance.second)));
    }
    return ret;
}

json_spirit::Value listsinceblock(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp)
        throw std::runtime_error(
            "listsinceblock [blockhash] [target-confirmations]\n"
            "Get all transactions in blocks since block [blockhash], or all transactions if omitted");

    CBlockIndex *pindex = NULL;
    int target_confirms = 1;

    if (params.size() > 0)
    {
        uint256 blockId = 0;

        blockId.SetHex(params[0].get_str());
        pindex = CBlockLocator(blockId).GetBlockIndex();
    }

    if (params.size() > 1)
    {
        target_confirms = params[1].get_int();

        if (target_confirms < 1)
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter");
    }

    int depth = pindex ? (1 + nBestHeight - pindex->nHeight) : -1;

    json_spirit::Array transactions;

    for (std::map<uint256, CWalletTx>::iterator it = pwalletMain->mapWallet.begin(); it != pwalletMain->mapWallet.end(); it++)
    {
        CWalletTx tx = (*it).second;

        if (depth == -1 || tx.GetDepthInMainChain() < depth)
            ListTransactions(tx, "*", 0, true, transactions);
    }

    uint256 lastblock;

    if (target_confirms == 1)
    {
        lastblock = hashBestChain;
    }
    else
    {
        int target_height = pindexBest->nHeight + 1 - target_confirms;

        CBlockIndex *block;
        for (block = pindexBest;
             block && block->nHeight > target_height;
             block = block->pprev)  { }

        lastblock = block ? block->GetBlockHash() : 0;
    }

    json_spirit::Object ret;
    ret.push_back(json_spirit::Pair("transactions", transactions));
    ret.push_back(json_spirit::Pair("lastblock", lastblock.GetHex()));

    return ret;
}

json_spirit::Value gettransaction(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw std::runtime_error(
            "gettransaction <txid>\n"
            "Get detailed information about <txid>");

    uint256 hash;
    hash.SetHex(params[0].get_str());

    json_spirit::Object entry;

    if (pwalletMain->mapWallet.count(hash))
    {
        const CWalletTx& wtx = pwalletMain->mapWallet[hash];

        TxToJSON(wtx, 0, entry);

        int64 nCredit = wtx.GetCredit();
        int64 nDebit = wtx.GetDebit();
        int64 nNet = nCredit - nDebit;
        int64 nFee = (wtx.IsFromMe() ? wtx.GetValueOut() - nDebit : 0);

        entry.push_back(json_spirit::Pair("amount", ValueFromAmount(nNet - nFee)));
        if (wtx.IsFromMe())
            entry.push_back(json_spirit::Pair("fee", ValueFromAmount(nFee)));

        WalletTxToJSON(wtx, entry);

        json_spirit::Array details;
        ListTransactions(pwalletMain->mapWallet[hash], "*", 0, false, details);
        entry.push_back(json_spirit::Pair("details", details));
    }
    else
    {
        CTransaction tx;
        uint256 hashBlock = 0;
        if (GetTransaction(hash, tx, hashBlock))
        {
            entry.push_back(json_spirit::Pair("txid", hash.GetHex()));
            TxToJSON(tx, 0, entry);
            if (hashBlock == 0)
                entry.push_back(json_spirit::Pair("confirmations", 0));
            else
            {
                entry.push_back(json_spirit::Pair("blockhash", hashBlock.GetHex()));
                std::map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(hashBlock);
                if (mi != mapBlockIndex.end() && (*mi).second)
                {
                    CBlockIndex* pindex = (*mi).second;
                    if (pindex->IsInMainChain())
                    {
                        entry.push_back(json_spirit::Pair("confirmations", 1 + nBestHeight - pindex->nHeight));
                        entry.push_back(json_spirit::Pair("txntime", (boost::int64_t)tx.nTime));
                        entry.push_back(json_spirit::Pair("time", (boost::int64_t)pindex->nTime));
                    }
                    else
                        entry.push_back(json_spirit::Pair("confirmations", 0));
                }
            }
        }
        else
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "No information available about transaction");
    }

    return entry;
}


json_spirit::Value backupwallet(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw std::runtime_error(
            "backupwallet <destination>\n"
            "Safely copies wallet.dat to destination, which can be a directory or a path with filename.");

    std::string strDest = params[0].get_str();
    if (!BackupWallet(*pwalletMain, strDest))
        throw JSONRPCError(RPC_WALLET_ERROR, "Error: Wallet backup failed!");

    return json_spirit::Value::null;
}


json_spirit::Value keypoolrefill(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp || params.size() > 0)
        throw std::runtime_error(
            "keypoolrefill\n"
            "Fills the keypool."
            + HelpRequiringPassphrase());

    EnsureWalletIsUnlocked();

    pwalletMain->TopUpKeyPool();

    if (pwalletMain->GetKeyPoolSize() < GetArg("-keypool", 100))
        throw JSONRPCError(RPC_WALLET_ERROR, "Error refreshing keypool.");

    return json_spirit::Value::null;
}


void ThreadTopUpKeyPool(void* parg)
{
    // Make this thread recognisable as the key-topping-up thread
    RenameThread("curecoin-key-top");

    pwalletMain->TopUpKeyPool();
}

void ThreadCleanWalletPassphrase(void* parg)
{
    // Make this thread recognisable as the wallet relocking thread
    RenameThread("curecoin-lock-wa");

    int64 nMyWakeTime = GetTimeMillis() + *((int64*)parg) * 1000;

    ENTER_CRITICAL_SECTION(cs_nWalletUnlockTime);

    if (nWalletUnlockTime == 0)
    {
        nWalletUnlockTime = nMyWakeTime;

        do
        {
            if (nWalletUnlockTime==0)
                break;
            int64 nToSleep = nWalletUnlockTime - GetTimeMillis();
            if (nToSleep <= 0)
                break;

            LEAVE_CRITICAL_SECTION(cs_nWalletUnlockTime);
            Sleep(nToSleep);
            ENTER_CRITICAL_SECTION(cs_nWalletUnlockTime);

        } while(1);

        if (nWalletUnlockTime)
        {
            nWalletUnlockTime = 0;
            pwalletMain->Lock();
        }
    }
    else
    {
        if (nWalletUnlockTime < nMyWakeTime)
            nWalletUnlockTime = nMyWakeTime;
    }

    LEAVE_CRITICAL_SECTION(cs_nWalletUnlockTime);

    delete (int64*)parg;
}

json_spirit::Value walletpassphrase(const json_spirit::Array& params, bool fHelp)
{
    if (pwalletMain->IsCrypted() && (fHelp || params.size() < 2 || params.size() > 3))
        throw std::runtime_error(
            "walletpassphrase <passphrase> <timeout> [mintonly]\n"
            "Stores the wallet decryption key in memory for <timeout> seconds.\n"
            "mintonly is optional true/false allowing only block minting.");
    if (fHelp)
        return true;
    if (!pwalletMain->IsCrypted())
        throw JSONRPCError(RPC_WALLET_WRONG_ENC_STATE, "Error: running with an unencrypted wallet, but walletpassphrase was called.");

    if (!pwalletMain->IsLocked())
        throw JSONRPCError(RPC_WALLET_ALREADY_UNLOCKED, "Error: Wallet is already unlocked, use walletlock first if need to change unlock settings.");
    // Note that the walletpassphrase is stored in params[0] which is not mlock()ed
    SecureString strWalletPass;
    strWalletPass.reserve(100);
    // TODO: get rid of this .c_str() by implementing SecureString::operator=(std::string)
    // Alternately, find a way to make params[0] mlock()'d to begin with.
    strWalletPass = params[0].get_str().c_str();

    if (strWalletPass.length() > 0)
    {
        if (!pwalletMain->Unlock(strWalletPass))
            throw JSONRPCError(RPC_WALLET_PASSPHRASE_INCORRECT, "Error: The wallet passphrase entered was incorrect.");
    }
    else
        throw std::runtime_error(
            "walletpassphrase <passphrase> <timeout>\n"
            "Stores the wallet decryption key in memory for <timeout> seconds.");

    NewThread(ThreadTopUpKeyPool, NULL);
    int64* pnSleepTime = new int64(params[1].get_int64());
    NewThread(ThreadCleanWalletPassphrase, pnSleepTime);

    // ppcoin: if user OS account compromised prevent trivial sendmoney commands
    if (params.size() > 2)
        fWalletUnlockMintOnly = params[2].get_bool();
    else
        fWalletUnlockMintOnly = false;

    return json_spirit::Value::null;
}


json_spirit::Value walletpassphrasechange(const json_spirit::Array& params, bool fHelp)
{
    if (pwalletMain->IsCrypted() && (fHelp || params.size() != 2))
        throw std::runtime_error(
            "walletpassphrasechange <oldpassphrase> <newpassphrase>\n"
            "Changes the wallet passphrase from <oldpassphrase> to <newpassphrase>.");
    if (fHelp)
        return true;
    if (!pwalletMain->IsCrypted())
        throw JSONRPCError(RPC_WALLET_WRONG_ENC_STATE, "Error: running with an unencrypted wallet, but walletpassphrasechange was called.");

    // TODO: get rid of these .c_str() calls by implementing SecureString::operator=(std::string)
    // Alternately, find a way to make params[0] mlock()'d to begin with.
    SecureString strOldWalletPass;
    strOldWalletPass.reserve(100);
    strOldWalletPass = params[0].get_str().c_str();

    SecureString strNewWalletPass;
    strNewWalletPass.reserve(100);
    strNewWalletPass = params[1].get_str().c_str();

    if (strOldWalletPass.length() < 1 || strNewWalletPass.length() < 1)
        throw std::runtime_error(
            "walletpassphrasechange <oldpassphrase> <newpassphrase>\n"
            "Changes the wallet passphrase from <oldpassphrase> to <newpassphrase>.");

    if (!pwalletMain->ChangeWalletPassphrase(strOldWalletPass, strNewWalletPass))
        throw JSONRPCError(RPC_WALLET_PASSPHRASE_INCORRECT, "Error: The wallet passphrase entered was incorrect.");

    return json_spirit::Value::null;
}


json_spirit::Value walletlock(const json_spirit::Array& params, bool fHelp)
{
    if (pwalletMain->IsCrypted() && (fHelp || params.size() != 0))
        throw std::runtime_error(
            "walletlock\n"
            "Removes the wallet encryption key from memory, locking the wallet.\n"
            "After calling this method, you will need to call walletpassphrase again\n"
            "before being able to call any methods which require the wallet to be unlocked.");
    if (fHelp)
        return true;
    if (!pwalletMain->IsCrypted())
        throw JSONRPCError(RPC_WALLET_WRONG_ENC_STATE, "Error: running with an unencrypted wallet, but walletlock was called.");

    {
        LOCK(cs_nWalletUnlockTime);
        pwalletMain->Lock();
        nWalletUnlockTime = 0;
    }

    return json_spirit::Value::null;
}


json_spirit::Value encryptwallet(const json_spirit::Array& params, bool fHelp)
{
    if (!pwalletMain->IsCrypted() && (fHelp || params.size() != 1))
        throw std::runtime_error(
            "encryptwallet <passphrase>\n"
            "Encrypts the wallet with <passphrase>.");
    if (fHelp)
        return true;
    if (pwalletMain->IsCrypted())
        throw JSONRPCError(RPC_WALLET_WRONG_ENC_STATE, "Error: running with an encrypted wallet, but encryptwallet was called.");

    // TODO: get rid of this .c_str() by implementing SecureString::operator=(std::string)
    // Alternately, find a way to make params[0] mlock()'d to begin with.
    SecureString strWalletPass;
    strWalletPass.reserve(100);
    strWalletPass = params[0].get_str().c_str();

    if (strWalletPass.length() < 1)
        throw std::runtime_error(
            "encryptwallet <passphrase>\n"
            "Encrypts the wallet with <passphrase>.");

    if (!pwalletMain->EncryptWallet(strWalletPass))
        throw JSONRPCError(RPC_WALLET_ENCRYPTION_FAILED, "Error: Failed to encrypt the wallet.");

    // BDB seems to have a bad habit of writing old data into
    // slack space in .dat files; that is bad if the old data is
    // unencrypted private keys. So:
    StartShutdown();
    return "wallet encrypted; curecoin server stopping, restart to run with encrypted wallet.  The keypool has been flushed, you need to make a new backup.";
}

class DescribeAddressVisitor : public boost::static_visitor<json_spirit::Object>
{
public:
    json_spirit::Object operator()(const CNoDestination &dest) const { return json_spirit::Object(); }

    json_spirit::Object operator()(const CKeyID &keyID) const {
        json_spirit::Object obj;
        CPubKey vchPubKey;
        pwalletMain->GetPubKey(keyID, vchPubKey);
        obj.push_back(json_spirit::Pair("isscript", false));
        obj.push_back(json_spirit::Pair("pubkey", HexStr(vchPubKey.Raw())));
        obj.push_back(json_spirit::Pair("iscompressed", vchPubKey.IsCompressed()));
        return obj;
    }

    json_spirit::Object operator()(const CScriptID &scriptID) const {
        json_spirit::Object obj;
        obj.push_back(json_spirit::Pair("isscript", true));
        CScript subscript;
        pwalletMain->GetCScript(scriptID, subscript);
        std::vector<CTxDestination> addresses;
        txnouttype whichType;
        int nRequired;
        ExtractDestinations(subscript, whichType, addresses, nRequired);
        obj.push_back(json_spirit::Pair("script", GetTxnOutputType(whichType)));
        json_spirit::Array a;
        BOOST_FOREACH(const CTxDestination& addr, addresses)
            a.push_back(CcurecoinAddress(addr).ToString());
        obj.push_back(json_spirit::Pair("addresses", a));
        if (whichType == TX_MULTISIG)
            obj.push_back(json_spirit::Pair("sigsrequired", nRequired));
        return obj;
    }
};

json_spirit::Value validateaddress(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw std::runtime_error(
            "validateaddress <curecoinAddress>\n"
            "Return information about <curecoinAddress>.");

    CcurecoinAddress address(params[0].get_str());
    bool isValid = address.IsValid();

    json_spirit::Object ret;
    ret.push_back(json_spirit::Pair("isvalid", isValid));
    if (isValid)
    {
        CTxDestination dest = address.Get();
        std::string currentAddress = address.ToString();
        ret.push_back(json_spirit::Pair("address", currentAddress));
        bool fMine = IsMine(*pwalletMain, dest);
        ret.push_back(json_spirit::Pair("ismine", fMine));
        if (fMine) {
            json_spirit::Object detail = boost::apply_visitor(DescribeAddressVisitor(), dest);
            ret.insert(ret.end(), detail.begin(), detail.end());
        }
        if (pwalletMain->mapAddressBook.count(dest))
            ret.push_back(json_spirit::Pair("account", pwalletMain->mapAddressBook[dest]));
    }
    return ret;
}

json_spirit::Value validatepubkey(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp || !params.size() || params.size() > 2)
        throw std::runtime_error(
            "validatepubkey <curecoinPubkey>\n"
            "Return information about <curecoinPubkey>.");

    std::vector<unsigned char> vchPubKey = ParseHex(params[0].get_str());
    CPubKey pubKey(vchPubKey);

    bool isValid = pubKey.IsValid();
    bool isCompressed = pubKey.IsCompressed();
    CKeyID keyID = pubKey.GetID();

    CcurecoinAddress address;
    address.Set(keyID);

    json_spirit::Object ret;
    ret.push_back(json_spirit::Pair("isvalid", isValid));
    if (isValid)
    {
        CTxDestination dest = address.Get();
        std::string currentAddress = address.ToString();
        ret.push_back(json_spirit::Pair("address", currentAddress));
        bool fMine = IsMine(*pwalletMain, dest);
        ret.push_back(json_spirit::Pair("ismine", fMine));
        ret.push_back(json_spirit::Pair("iscompressed", isCompressed));
        if (fMine) {
            json_spirit::Object detail = boost::apply_visitor(DescribeAddressVisitor(), dest);
            ret.insert(ret.end(), detail.begin(), detail.end());
        }
        if (pwalletMain->mapAddressBook.count(dest))
            ret.push_back(json_spirit::Pair("account", pwalletMain->mapAddressBook[dest]));
    }
    return ret;
}

// ppcoin: reserve balance from being staked for network protection
json_spirit::Value reservebalance(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp || params.size() > 2)
        throw std::runtime_error(
            "reservebalance [<reserve> [amount]]\n"
            "<reserve> is true or false to turn balance reserve on or off.\n"
            "<amount> is a real and rounded to cent.\n"
            "Set reserve amount not participating in network protection.\n"
            "If no parameters provided current setting is printed.\n");

    if (params.size() > 0)
    {
        bool fReserve = params[0].get_bool();
        if (fReserve)
        {
            if (params.size() == 1)
                throw std::runtime_error("must provide amount to reserve balance.\n");
            int64 nAmount = AmountFromValue(params[1]);
            nAmount = (nAmount / CENT) * CENT;  // round to cent
            if (nAmount < 0)
                throw std::runtime_error("amount cannot be negative.\n");
            mapArgs["-reservebalance"] = FormatMoney(nAmount).c_str();
        }
        else
        {
            if (params.size() > 1)
                throw std::runtime_error("cannot specify amount to turn off reserve.\n");
            mapArgs["-reservebalance"] = "0";
        }
    }

    json_spirit::Object result;
    int64 nReserveBalance = 0;
    if (mapArgs.count("-reservebalance") && !ParseMoney(mapArgs["-reservebalance"], nReserveBalance))
        throw std::runtime_error("invalid reserve balance amount\n");
    result.push_back(json_spirit::Pair("reserve", (nReserveBalance > 0)));
    result.push_back(json_spirit::Pair("amount", ValueFromAmount(nReserveBalance)));
    return result;
}


// ppcoin: check wallet integrity
json_spirit::Value checkwallet(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp || params.size() > 0)
        throw std::runtime_error(
            "checkwallet\n"
            "Check wallet for integrity.\n");

    int nMismatchSpent;
    int64 nBalanceInQuestion;
    pwalletMain->FixSpentCoins(nMismatchSpent, nBalanceInQuestion, true);
    json_spirit::Object result;
    if (nMismatchSpent == 0)
        result.push_back(json_spirit::Pair("wallet check passed", true));
    else
    {
        result.push_back(json_spirit::Pair("mismatched spent coins", nMismatchSpent));
        result.push_back(json_spirit::Pair("amount in question", ValueFromAmount(nBalanceInQuestion)));
    }
    return result;
}


// ppcoin: repair wallet
json_spirit::Value repairwallet(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp || params.size() > 0)
        throw std::runtime_error(
            "repairwallet\n"
            "Repair wallet if checkwallet reports any problem.\n");

    int nMismatchSpent;
    int64 nBalanceInQuestion;
    pwalletMain->FixSpentCoins(nMismatchSpent, nBalanceInQuestion);
    json_spirit::Object result;
    if (nMismatchSpent == 0)
        result.push_back(json_spirit::Pair("wallet check passed", true));
    else
    {
        result.push_back(json_spirit::Pair("mismatched spent coins", nMismatchSpent));
        result.push_back(json_spirit::Pair("amount affected by repair", ValueFromAmount(nBalanceInQuestion)));
    }
    return result;
}

// curecoin: resend unconfirmed wallet transactions
json_spirit::Value resendtx(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp || params.size() > 1)
        throw std::runtime_error(
            "resendtx\n"
            "Re-send unconfirmed transactions.\n"
        );

    ResendWalletTransactions();

    return json_spirit::Value::null;
}

// ppcoin: make a public-private key pair
json_spirit::Value makekeypair(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp || params.size() > 1)
        throw std::runtime_error(
            "makekeypair [prefix]\n"
            "Make a public/private key pair.\n"
            "[prefix] is optional preferred prefix for the public key.\n");

    std::string strPrefix = "";
    if (params.size() > 0)
        strPrefix = params[0].get_str();
 
    CKey key;
    key.MakeNewKey(false);

    CPrivKey vchPrivKey = key.GetPrivKey();
    json_spirit::Object result;
    result.push_back(json_spirit::Pair("PrivateKey", HexStr<CPrivKey::iterator>(vchPrivKey.begin(), vchPrivKey.end())));
    result.push_back(json_spirit::Pair("PublicKey", HexStr(key.GetPubKey().Raw())));
    return result;
}
