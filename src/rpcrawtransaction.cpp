// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2013-2025 CureCoin Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/assign/list_of.hpp>

#include "base58.h"
#include "curecoinrpc.h"
#include "db.h"
#include "init.h"
#include "main.h"
#include "net.h"
#include "wallet.h"

#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

using namespace boost;
using namespace boost::assign;

void ScriptPubKeyToJSON(const CScript& scriptPubKey, json_spirit::Object& out)
{
    txnouttype type;
    std::vector<CTxDestination> addresses;
    int nRequired;

    out.push_back(json_spirit::Pair("asm", scriptPubKey.ToString()));
    out.push_back(json_spirit::Pair("hex", HexStr(scriptPubKey.begin(), scriptPubKey.end())));

    if (!ExtractDestinations(scriptPubKey, type, addresses, nRequired))
    {
        out.push_back(json_spirit::Pair("type", GetTxnOutputType(TX_NONSTANDARD)));
        return;
    }

    out.push_back(json_spirit::Pair("reqSigs", nRequired));
    out.push_back(json_spirit::Pair("type", GetTxnOutputType(type)));

    json_spirit::Array a;
    BOOST_FOREACH(const CTxDestination& addr, addresses)
        a.push_back(CcurecoinAddress(addr).ToString());
    out.push_back(json_spirit::Pair("addresses", a));
}

void TxToJSON(const CTransaction& tx, const uint256 hashBlock, json_spirit::Object& entry)
{
    entry.push_back(json_spirit::Pair("txid", tx.GetHash().GetHex()));
    entry.push_back(json_spirit::Pair("version", tx.nVersion));
    entry.push_back(json_spirit::Pair("time", (boost::int64_t)tx.nTime));
    entry.push_back(json_spirit::Pair("locktime", (boost::int64_t)tx.nLockTime));
    if (tx.nVersion >= 2)
    {
        entry.push_back(json_spirit::Pair("tx-comment", tx.strTxComment));
    }

    json_spirit::Array vin;
    BOOST_FOREACH(const CTxIn& txin, tx.vin)
    {
        json_spirit::Object in;
        if (tx.IsCoinBase())
            in.push_back(json_spirit::Pair("coinbase", HexStr(txin.scriptSig.begin(), txin.scriptSig.end())));
        else
        {
            in.push_back(json_spirit::Pair("txid", txin.prevout.hash.GetHex()));
            in.push_back(json_spirit::Pair("vout", (boost::int64_t)txin.prevout.n));
            json_spirit::Object o;
            o.push_back(json_spirit::Pair("asm", txin.scriptSig.ToString()));
            o.push_back(json_spirit::Pair("hex", HexStr(txin.scriptSig.begin(), txin.scriptSig.end())));
            in.push_back(json_spirit::Pair("scriptSig", o));
        }
        in.push_back(json_spirit::Pair("sequence", (boost::int64_t)txin.nSequence));
        vin.push_back(in);
    }
    entry.push_back(json_spirit::Pair("vin", vin));
    json_spirit::Array vout;
    for (unsigned int i = 0; i < tx.vout.size(); i++)
    {
        const CTxOut& txout = tx.vout[i];
        json_spirit::Object out;
        out.push_back(json_spirit::Pair("value", ValueFromAmount(txout.nValue)));
        out.push_back(json_spirit::Pair("n", (boost::int64_t)i));
        json_spirit::Object o;
        ScriptPubKeyToJSON(txout.scriptPubKey, o);
        out.push_back(json_spirit::Pair("scriptPubKey", o));
        vout.push_back(out);
    }
    entry.push_back(json_spirit::Pair("vout", vout));

    if (hashBlock != 0)
    {
        entry.push_back(json_spirit::Pair("blockhash", hashBlock.GetHex()));
        std::map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(hashBlock);
        if (mi != mapBlockIndex.end() && (*mi).second)
        {
            CBlockIndex* pindex = (*mi).second;
            if (pindex->IsInMainChain())
            {
                entry.push_back(json_spirit::Pair("confirmations", 1 + nBestHeight - pindex->nHeight));
                entry.push_back(json_spirit::Pair("time", (boost::int64_t)pindex->nTime));
                entry.push_back(json_spirit::Pair("blocktime", (boost::int64_t)pindex->nTime));
            }
            else
                entry.push_back(json_spirit::Pair("confirmations", 0));
        }
    }
}

json_spirit::Value getrawtransaction(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 2)
        throw std::runtime_error(
            "getrawtransaction <txid> [verbose=0]\n"
            "If verbose=0, returns a string that is\n"
            "serialized, hex-encoded data for <txid>.\n"
            "If verbose is non-zero, returns an Object\n"
            "with information about <txid>.");

    uint256 hash;
    hash.SetHex(params[0].get_str());

    bool fVerbose = false;
    if (params.size() > 1)
        fVerbose = (params[1].get_int() != 0);

    CTransaction tx;
    uint256 hashBlock = 0;
    if (!GetTransaction(hash, tx, hashBlock))
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "No information available about transaction");

    CDataStream ssTx(SER_NETWORK, PROTOCOL_VERSION);
    ssTx << tx;
    std::string strHex = HexStr(ssTx.begin(), ssTx.end());

    if (!fVerbose)
        return strHex;

    json_spirit::Object result;
    result.push_back(json_spirit::Pair("hex", strHex));
    TxToJSON(tx, hashBlock, result);
    return result;
}

json_spirit::Value listunspent(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp || params.size() > 3)
        throw std::runtime_error(
            "listunspent [minconf=1] [maxconf=9999999]  [\"address\",...]\n"
            "Returns array of unspent transaction outputs\n"
            "with between minconf and maxconf (inclusive) confirmations.\n"
            "Optionally filtered to only include txouts paid to specified addresses.\n"
            "Results are an array of Objects, each of which has:\n"
            "{txid, vout, scriptPubKey, amount, confirmations}");

    RPCTypeCheck(params, list_of(json_spirit::int_type)(json_spirit::int_type)(json_spirit::array_type));

    int nMinDepth = 1;
    if (params.size() > 0)
        nMinDepth = params[0].get_int();

    int nMaxDepth = 9999999;
    if (params.size() > 1)
        nMaxDepth = params[1].get_int();

    std::set<CcurecoinAddress> setAddress;
    if (params.size() > 2)
    {
        json_spirit::Array inputs = params[2].get_array();
        BOOST_FOREACH(json_spirit::Value& input, inputs)
        {
            CcurecoinAddress address(input.get_str());
            if (!address.IsValid())
                throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, std::string("Invalid curecoin address: ")+input.get_str());
            if (setAddress.count(address))
                throw JSONRPCError(RPC_INVALID_PARAMETER, std::string("Invalid parameter, duplicated address: ")+input.get_str());
           setAddress.insert(address);
        }
    }

    json_spirit::Array results;
    std::vector<COutput> vecOutputs;
    pwalletMain->AvailableCoins(vecOutputs, false);
    BOOST_FOREACH(const COutput& out, vecOutputs)
    {
        if (out.nDepth < nMinDepth || out.nDepth > nMaxDepth)
            continue;

        if(setAddress.size())
        {
            CTxDestination address;
            if(!ExtractDestination(out.tx->vout[out.i].scriptPubKey, address))
                continue;

            if (!setAddress.count(address))
                continue;
        }

        int64 nValue = out.tx->vout[out.i].nValue;
        const CScript& pk = out.tx->vout[out.i].scriptPubKey;
        json_spirit::Object entry;
        entry.push_back(json_spirit::Pair("txid", out.tx->GetHash().GetHex()));
        entry.push_back(json_spirit::Pair("vout", out.i));
        entry.push_back(json_spirit::Pair("scriptPubKey", HexStr(pk.begin(), pk.end())));
        entry.push_back(json_spirit::Pair("amount",ValueFromAmount(nValue)));
        entry.push_back(json_spirit::Pair("confirmations",out.nDepth));
        results.push_back(entry);
    }

    return results;
}

json_spirit::Value createrawtransaction(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp || params.size() != 2)
        throw std::runtime_error(
            "createrawtransaction [{\"txid\":txid,\"vout\":n},...] {address:amount,...}\n"
            "Create a transaction spending given inputs\n"
            "(array of objects containing transaction id and output number),\n"
            "sending to given address(es).\n"
            "Returns hex-encoded raw transaction.\n"
            "Note that the transaction's inputs are not signed, and\n"
            "it is not stored in the wallet or transmitted to the network.");

    RPCTypeCheck(params, list_of(json_spirit::array_type)(json_spirit::obj_type));

    json_spirit::Array inputs = params[0].get_array();
    json_spirit::Object sendTo = params[1].get_obj();

    CTransaction rawTx;

    BOOST_FOREACH(json_spirit::Value& input, inputs)
    {
        const json_spirit::Object& o = input.get_obj();

        const json_spirit::Value& txid_v = find_value(o, "txid");
        if (txid_v.type() != json_spirit::str_type)
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter, missing txid key");
        std::string txid = txid_v.get_str();
        if (!IsHex(txid))
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter, expected hex txid");

        const json_spirit::Value& vout_v = find_value(o, "vout");
        if (vout_v.type() != json_spirit::int_type)
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter, missing vout key");
        int nOutput = vout_v.get_int();
        if (nOutput < 0)
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter, vout must be positive");

        CTxIn in(COutPoint(uint256(txid), nOutput));
        rawTx.vin.push_back(in);
    }

    std::set<CcurecoinAddress> setAddress;
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

        CTxOut out(nAmount, scriptPubKey);
        rawTx.vout.push_back(out);
    }

    CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);
    ss << rawTx;
    return HexStr(ss.begin(), ss.end());
}

json_spirit::Value decoderawtransaction(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw std::runtime_error(
            "decoderawtransaction <hex string>\n"
            "Return a JSON object representing the serialized, hex-encoded transaction.");

    RPCTypeCheck(params, list_of(json_spirit::str_type));

    std::vector<unsigned char> txData(ParseHex(params[0].get_str()));
    CDataStream ssData(txData, SER_NETWORK, PROTOCOL_VERSION);
    CTransaction tx;
    try {
        ssData >> tx;
    }
    catch (std::exception &e) {
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "TX decode failed");
    }

    json_spirit::Object result;
    TxToJSON(tx, 0, result);

    return result;
}

json_spirit::Value signrawtransaction(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 4)
        throw std::runtime_error(
            "signrawtransaction <hex string> [{\"txid\":txid,\"vout\":n,\"scriptPubKey\":hex},...] [<privatekey1>,...] [sighashtype=\"ALL\"]\n"
            "Sign inputs for raw transaction (serialized, hex-encoded).\n"
            "Second optional argument (may be null) is an array of previous transaction outputs that\n"
            "this transaction depends on but may not yet be in the blockchain.\n"
            "Third optional argument (may be null) is an array of base58-encoded private\n"
            "keys that, if given, will be the only keys used to sign the transaction.\n"
            "Fourth optional argument is a string that is one of six values; ALL, NONE, SINGLE or\n"
            "ALL|ANYONECANPAY, NONE|ANYONECANPAY, SINGLE|ANYONECANPAY.\n"
            "Returns json object with keys:\n"
            "  hex : raw transaction with signature(s) (hex-encoded string)\n"
            "  complete : 1 if transaction has a complete set of signature (0 if not)"
            + HelpRequiringPassphrase());

    RPCTypeCheck(params, list_of(json_spirit::str_type)(json_spirit::array_type)(json_spirit::array_type)(json_spirit::str_type), true);

    std::vector<unsigned char> txData(ParseHex(params[0].get_str()));
    CDataStream ssData(txData, SER_NETWORK, PROTOCOL_VERSION);
    std::vector<CTransaction> txVariants;
    while (!ssData.empty())
    {
        try {
            CTransaction tx;
            ssData >> tx;
            txVariants.push_back(tx);
        }
        catch (std::exception &e) {
            throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "TX decode failed");
        }
    }

    if (txVariants.empty())
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Missing transaction");

    // mergedTx will end up with all the signatures; it
    // starts as a clone of the rawtx:
    CTransaction mergedTx(txVariants[0]);
    bool fComplete = true;

    // Fetch previous transactions (inputs):
    std::map<COutPoint, CScript> mapPrevOut;
    for (unsigned int i = 0; i < mergedTx.vin.size(); i++)
    {
        CTransaction tempTx;
        MapPrevTx mapPrevTx;
        CTxDB txdb("r");
        std::map<uint256, CTxIndex> unused;
        bool fInvalid;

        // FetchInputs aborts on failure, so we go one at a time.
        tempTx.vin.push_back(mergedTx.vin[i]);
        tempTx.FetchInputs(txdb, unused, false, false, mapPrevTx, fInvalid);

        // Copy results into mapPrevOut:
        BOOST_FOREACH(const CTxIn& txin, tempTx.vin)
        {
            const uint256& prevHash = txin.prevout.hash;
            if (mapPrevTx.count(prevHash) && mapPrevTx[prevHash].second.vout.size()>txin.prevout.n)
                mapPrevOut[txin.prevout] = mapPrevTx[prevHash].second.vout[txin.prevout.n].scriptPubKey;
        }
    }

    // Add previous txouts given in the RPC call:
    if (params.size() > 1 && params[1].type() != json_spirit::null_type)
    {
        json_spirit::Array prevTxs = params[1].get_array();
        BOOST_FOREACH(json_spirit::Value& p, prevTxs)
        {
            if (p.type() != json_spirit::obj_type)
                throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "expected object with {\"txid'\",\"vout\",\"scriptPubKey\"}");

            json_spirit::Object prevOut = p.get_obj();

            RPCTypeCheck(prevOut, map_list_of("txid", json_spirit::str_type)("vout", json_spirit::int_type)("scriptPubKey", json_spirit::str_type));

            std::string txidHex = find_value(prevOut, "txid").get_str();
            if (!IsHex(txidHex))
                throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "txid must be hexadecimal");
            uint256 txid;
            txid.SetHex(txidHex);

            int nOut = find_value(prevOut, "vout").get_int();
            if (nOut < 0)
                throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "vout must be positive");

            std::string pkHex = find_value(prevOut, "scriptPubKey").get_str();
            if (!IsHex(pkHex))
                throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "scriptPubKey must be hexadecimal");
            std::vector<unsigned char> pkData(ParseHex(pkHex));
            CScript scriptPubKey(pkData.begin(), pkData.end());

            COutPoint outpoint(txid, nOut);
            if (mapPrevOut.count(outpoint))
            {
                // Complain if scriptPubKey doesn't match
                if (mapPrevOut[outpoint] != scriptPubKey)
                {
                    std::string err("Previous output scriptPubKey mismatch:\n");
                    err = err + mapPrevOut[outpoint].ToString() + "\nvs:\n"+
                        scriptPubKey.ToString();
                    throw JSONRPCError(RPC_DESERIALIZATION_ERROR, err);
                }
            }
            else
                mapPrevOut[outpoint] = scriptPubKey;
        }
    }

    bool fGivenKeys = false;
    CBasicKeyStore tempKeystore;
    if (params.size() > 2 && params[2].type() != json_spirit::null_type)
    {
        fGivenKeys = true;
        json_spirit::Array keys = params[2].get_array();
        BOOST_FOREACH(json_spirit::Value k, keys)
        {
            CcurecoinSecret vchSecret;
            bool fGood = vchSecret.SetString(k.get_str());
            if (!fGood)
                throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY,"Invalid private key");
            CKey key;
            bool fCompressed;
            CSecret secret = vchSecret.GetSecret(fCompressed);
            key.SetSecret(secret, fCompressed);
            tempKeystore.AddKey(key);
        }
    }
    else
        EnsureWalletIsUnlocked();

    const CKeyStore& keystore = (fGivenKeys ? tempKeystore : *pwalletMain);

    int nHashType = SIGHASH_ALL;
    if (params.size() > 3 && params[3].type() != json_spirit::null_type)
    {
        static std::map<std::string, int> mapSigHashValues =
            boost::assign::map_list_of
            (std::string("ALL"), int(SIGHASH_ALL))
            (std::string("ALL|ANYONECANPAY"), int(SIGHASH_ALL|SIGHASH_ANYONECANPAY))
            (std::string("NONE"), int(SIGHASH_NONE))
            (std::string("NONE|ANYONECANPAY"), int(SIGHASH_NONE|SIGHASH_ANYONECANPAY))
            (std::string("SINGLE"), int(SIGHASH_SINGLE))
            (std::string("SINGLE|ANYONECANPAY"), int(SIGHASH_SINGLE|SIGHASH_ANYONECANPAY))
            ;
        std::string strHashType = params[3].get_str();
        if (mapSigHashValues.count(strHashType))
            nHashType = mapSigHashValues[strHashType];
        else
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid sighash param");
    }

    bool fHashSingle = ((nHashType & ~SIGHASH_ANYONECANPAY) == SIGHASH_SINGLE);

    // Sign what we can:
    for (unsigned int i = 0; i < mergedTx.vin.size(); i++)
    {
        CTxIn& txin = mergedTx.vin[i];
        if (mapPrevOut.count(txin.prevout) == 0)
        {
            fComplete = false;
            continue;
        }
        const CScript& prevPubKey = mapPrevOut[txin.prevout];

        txin.scriptSig.clear();
        // Only sign SIGHASH_SINGLE if there's a corresponding output:
        if (!fHashSingle || (i < mergedTx.vout.size()))
            SignSignature(keystore, prevPubKey, mergedTx, i, nHashType);

        // ... and merge in other signatures:
        BOOST_FOREACH(const CTransaction& txv, txVariants)
        {
            txin.scriptSig = CombineSignatures(prevPubKey, mergedTx, i, txin.scriptSig, txv.vin[i].scriptSig);
        }
        if (!VerifyScript(txin.scriptSig, prevPubKey, mergedTx, i, true, 0))
            fComplete = false;
    }

    json_spirit::Object result;
    CDataStream ssTx(SER_NETWORK, PROTOCOL_VERSION);
    ssTx << mergedTx;
    result.push_back(json_spirit::Pair("hex", HexStr(ssTx.begin(), ssTx.end())));
    result.push_back(json_spirit::Pair("complete", fComplete));

    return result;
}

json_spirit::Value sendrawtransaction(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 1)
        throw std::runtime_error(
            "sendrawtransaction <hex string>\n"
            "Submits raw transaction (serialized, hex-encoded) to local node and network.");

    RPCTypeCheck(params, list_of(json_spirit::str_type));

    // parse hex string from parameter
    std::vector<unsigned char> txData(ParseHex(params[0].get_str()));
    CDataStream ssData(txData, SER_NETWORK, PROTOCOL_VERSION);
    CTransaction tx;

    // deserialize binary data stream
    try {
        ssData >> tx;
    }
    catch (std::exception &e) {
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "TX decode failed");
    }
    uint256 hashTx = tx.GetHash();

    // See if the transaction is already in a block
    // or in the memory pool:
    CTransaction existingTx;
    uint256 hashBlock = 0;
    if (GetTransaction(hashTx, existingTx, hashBlock))
    {
        if (hashBlock != 0)
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, std::string("transaction already in block ")+hashBlock.GetHex());
        // Not in block, but already in the memory pool; will drop
        // through to re-relay it.
    }
    else
    {
        // push to local node
        CTxDB txdb("r");
        if (!tx.AcceptToMemoryPool(txdb))
            throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "TX rejected");

        SyncWithWallets(tx, NULL, true);
    }
    RelayMessage(CInv(MSG_TX, hashTx), tx);

    return hashTx.GetHex();
}
