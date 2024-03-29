// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2013  The Curecoin developer
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "main.h"
#include "db.h"
#include "init.h"
#include "curecoinrpc.h"

#include <algorithm>
#include <map>
#include <stdexcept>
#include <utility>
#include <vector>

extern unsigned int nStakeTargetSpacing;

json_spirit::Value getgenerate(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp || params.size() != 0)
        throw std::runtime_error(
            "getgenerate\n"
            "Returns true or false.");

    return GetBoolArg("-gen");
}


json_spirit::Value setgenerate(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 2)
        throw std::runtime_error(
            "setgenerate <generate> [genproclimit]\n"
            "<generate> is true or false to turn generation on or off.\n"
            "Generation is limited to [genproclimit] processors, -1 is unlimited.");

    bool fGenerate = true;
    if (params.size() > 0)
        fGenerate = params[0].get_bool();

    if (params.size() > 1)
    {
        int nGenProcLimit = params[1].get_int();
        mapArgs["-genproclimit"] = itostr(nGenProcLimit);
        if (nGenProcLimit == 0)
            fGenerate = false;
    }
    mapArgs["-gen"] = (fGenerate ? "1" : "0");

    Generatecurecoins(fGenerate, pwalletMain);
    return json_spirit::Value::null;
}


json_spirit::Value gethashespersec(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp || params.size() != 0)
        throw std::runtime_error(
            "gethashespersec\n"
            "Returns a recent hashes per second performance measurement while generating.");

    if (GetTimeMillis() - nHPSTimerStart > 8000)
        return (boost::int64_t)0;
    return (boost::int64_t)dHashesPerSec;
}

// Litecoin: Return average network hashes per second based on last number of blocks.
json_spirit::Value GetNetworkHashPS(int lookup) {
    if (pindexBest == NULL)
        return 0;

    // If lookup is -1, then use blocks since last difficulty change.
    if (lookup <= 0)
        lookup = pindexBest->nHeight % 2016 + 1;

    // If lookup is larger than chain, then set it to chain length.
    if (lookup > pindexBest->nHeight)
        lookup = pindexBest->nHeight;

    CBlockIndex* pindexPrev = pindexBest;
    for (int i = 0; i < lookup; i++)
        pindexPrev = pindexPrev->pprev;

    double timeDiff = pindexBest->GetBlockTime() - pindexPrev->GetBlockTime();
    double timePerBlock = timeDiff / lookup;

    return (boost::int64_t)(((double)GetDifficulty() * pow(2.0, 32)) / timePerBlock);
}


json_spirit::Value getnetworkhashps(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp || params.size() > 1)
        throw std::runtime_error(
            "getnetworkhashps [blocks]\n"
            "Returns the estimated network hashes per second based on the last 120 blocks.\n"
            "Pass in [blocks] to override # of blocks, -1 specifies since last difficulty change.");

    return GetNetworkHashPS(params.size() > 0 ? params[0].get_int() : 120);
}


json_spirit::Value getmininginfo(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp || params.size() != 0)
        throw std::runtime_error(
            "getmininginfo\n"
            "Returns an object containing mining-related information.");

    json_spirit::Object obj;
    obj.push_back(json_spirit::Pair("blocks",        (int)nBestHeight));
    obj.push_back(json_spirit::Pair("currentblocksize",(uint64_t)nLastBlockSize));
    obj.push_back(json_spirit::Pair("currentblocktx",(uint64_t)nLastBlockTx));
    obj.push_back(json_spirit::Pair("difficulty",    (double)GetDifficulty()));
//    obj.push_back(json_spirit::Pair("netstakeweight", GetPoSKernelPS()));
    obj.push_back(json_spirit::Pair("errors",        GetWarnings("statusbar")));
    obj.push_back(json_spirit::Pair("generate",      GetBoolArg("-gen")));
    obj.push_back(json_spirit::Pair("genproclimit",  (int)GetArg("-genproclimit", -1)));
    obj.push_back(json_spirit::Pair("hashespersec",  gethashespersec(params, false)));
    obj.push_back(json_spirit::Pair("networkhashps", getnetworkhashps(params, false)));
    obj.push_back(json_spirit::Pair("pooledtx",      (uint64_t)mempool.size()));
    obj.push_back(json_spirit::Pair("testnet",       fTestNet));
    return obj;
}

json_spirit::Value getstakinginfo(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp || params.size() != 0)
        throw std::runtime_error(
            "getstakinginfo\n"
            "Returns an object containing staking-related information.");

    uint64 nMinWeight = 0, nMaxWeight = 0, nWeight = 0;
    pwalletMain->GetStakeWeight(*pwalletMain, nMinWeight, nMaxWeight, nWeight);

    uint64 nNetworkWeight = GetPoSKernelPS();
    bool staking = nLastCoinStakeSearchInterval && nWeight;
    int nExpectedTime = staking ? (nStakeTargetSpacing * nNetworkWeight / nWeight) : -1;

    json_spirit::Object obj;

    obj.push_back(json_spirit::Pair("enabled", GetBoolArg("-staking", true)));
    obj.push_back(json_spirit::Pair("staking", staking));
    obj.push_back(json_spirit::Pair("errors", GetWarnings("statusbar")));

    obj.push_back(json_spirit::Pair("currentblocksize", (uint64_t)nLastBlockSize));
    obj.push_back(json_spirit::Pair("currentblocktx", (uint64_t)nLastBlockTx));
    obj.push_back(json_spirit::Pair("pooledtx", (uint64_t)mempool.size()));

    obj.push_back(json_spirit::Pair("difficulty", GetDifficulty(GetLastBlockIndex(pindexBest, true))));
    obj.push_back(json_spirit::Pair("search-interval", (int)nLastCoinStakeSearchInterval));

    obj.push_back(json_spirit::Pair("weight", (uint64_t)nWeight));
    obj.push_back(json_spirit::Pair("netstakeweight", (uint64_t)nNetworkWeight));

    obj.push_back(json_spirit::Pair("expectedtime", nExpectedTime));

    return obj;
}

json_spirit::Value getworkex(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp || params.size() > 2)
        throw std::runtime_error(
            "getworkex [data, coinbase]\n"
            "If [data, coinbase] is not specified, returns extended work data.\n"
        );

    if (vNodes.empty())
        throw JSONRPCError(-9, "Curecoin is not connected!");

    if (IsInitialBlockDownload())
        throw JSONRPCError(-10, "Curecoin is downloading blocks...");

    typedef std::map<uint256, std::pair<CBlock*, CScript> > mapNewBlock_t;
    static mapNewBlock_t mapNewBlock;
    static std::vector<CBlock*> vNewBlock;
    static CReserveKey reservekey(pwalletMain);

    if (params.size() == 0)
    {
        // Update block
        static unsigned int nTransactionsUpdatedLast;
        static CBlockIndex* pindexPrev;
        static int64 nStart;
        static CBlock* pblock;
        if (pindexPrev != pindexBest ||
            (nTransactionsUpdated != nTransactionsUpdatedLast && GetTime() - nStart > 60))
        {
            if (pindexPrev != pindexBest)
            {
                // Deallocate old blocks since they're obsolete now
                mapNewBlock.clear();
                BOOST_FOREACH(CBlock* pblock, vNewBlock)
                    delete pblock;
                vNewBlock.clear();
            }
            nTransactionsUpdatedLast = nTransactionsUpdated;
            pindexPrev = pindexBest;
            nStart = GetTime();

            // Create new block
            pblock = CreateNewBlock(pwalletMain);
            if (!pblock)
                throw JSONRPCError(-7, "Out of memory");
            vNewBlock.push_back(pblock);
        }

        // Update nTime
        pblock->nTime = std::max(pindexPrev->GetMedianTimePast()+1, GetAdjustedTime());
        pblock->nNonce = 0;

        // Update nExtraNonce
        static unsigned int nExtraNonce = 0;
        IncrementExtraNonce(pblock, pindexPrev, nExtraNonce);

        // Save
        mapNewBlock[pblock->hashMerkleRoot] = make_pair(pblock, pblock->vtx[0].vin[0].scriptSig);

        // Prebuild hash buffers
        char pmidstate[32];
        char pdata[128];
        char phash1[64];
        FormatHashBuffers(pblock, pmidstate, pdata, phash1);

        uint256 hashTarget = CBigNum().SetCompact(pblock->nBits).getuint256();

        CTransaction coinbaseTx = pblock->vtx[0];
        std::vector<uint256> merkle = pblock->GetMerkleBranch(0);

        json_spirit::Object result;
        result.push_back(json_spirit::Pair("data",     HexStr(BEGIN(pdata), END(pdata))));
        result.push_back(json_spirit::Pair("target",   HexStr(BEGIN(hashTarget), END(hashTarget))));

        CDataStream ssTx(SER_NETWORK, PROTOCOL_VERSION);
        ssTx << coinbaseTx;
        result.push_back(json_spirit::Pair("coinbase", HexStr(ssTx.begin(), ssTx.end())));

        json_spirit::Array merkle_arr;

        BOOST_FOREACH(uint256 merkleh, merkle) {
            merkle_arr.push_back(HexStr(BEGIN(merkleh), END(merkleh)));
        }

        result.push_back(json_spirit::Pair("merkle", merkle_arr));


        return result;
    }
    else
    {
        // Parse parameters
        std::vector<unsigned char> vchData = ParseHex(params[0].get_str());
        std::vector<unsigned char> coinbase;

        if(params.size() == 2)
            coinbase = ParseHex(params[1].get_str());

        if (vchData.size() != 128)
            throw JSONRPCError(-8, "Invalid parameter");

        CBlock* pdata = (CBlock*)&vchData[0];

        // Byte reverse
        for (int i = 0; i < 128/4; i++)
            ((unsigned int*)pdata)[i] = ByteReverse(((unsigned int*)pdata)[i]);

        // Get saved block
        if (!mapNewBlock.count(pdata->hashMerkleRoot))
            return false;
        CBlock* pblock = mapNewBlock[pdata->hashMerkleRoot].first;

        pblock->nTime = pdata->nTime;
        pblock->nNonce = pdata->nNonce;

        if(coinbase.size() == 0)
            pblock->vtx[0].vin[0].scriptSig = mapNewBlock[pdata->hashMerkleRoot].second;
        else
            CDataStream(coinbase, SER_NETWORK, PROTOCOL_VERSION) >> pblock->vtx[0]; // FIXME - HACK!

        pblock->hashMerkleRoot = pblock->BuildMerkleTree();

        if (!pblock->SignBlock(*pwalletMain))
            throw JSONRPCError(-100, "Unable to sign block, wallet locked?");

        return CheckWork(pblock, *pwalletMain, reservekey);
    }
}


json_spirit::Value getwork(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp || params.size() > 1)
        throw std::runtime_error(
            "getwork [data]\n"
            "If [data] is not specified, returns formatted hash data to work on:\n"
            "  \"midstate\" : precomputed hash state after hashing the first half of the data (DEPRECATED)\n" // deprecated
            "  \"data\" : block data\n"
            "  \"hash1\" : formatted hash buffer for second hash (DEPRECATED)\n" // deprecated
            "  \"target\" : little endian hash target\n"
            "If [data] is specified, tries to solve the block and returns true if it was successful.");

    if (vNodes.empty())
        throw JSONRPCError(RPC_CLIENT_NOT_CONNECTED, "Curecoin is not connected!");

    if (IsInitialBlockDownload())
        throw JSONRPCError(RPC_CLIENT_IN_INITIAL_DOWNLOAD, "Curecoin is downloading blocks...");

    typedef std::map<uint256, std::pair<CBlock*, CScript> > mapNewBlock_t;
    static mapNewBlock_t mapNewBlock;    // FIXME: thread safety
    static std::vector<CBlock*> vNewBlock;
    static CReserveKey reservekey(pwalletMain);

    if (params.size() == 0)
    {
        // Update block
        static unsigned int nTransactionsUpdatedLast;
        static CBlockIndex* pindexPrev;
        static int64 nStart;
        static CBlock* pblock;
        if (pindexPrev != pindexBest ||
            (nTransactionsUpdated != nTransactionsUpdatedLast && GetTime() - nStart > 60))
        {
            if (pindexPrev != pindexBest)
            {
                // Deallocate old blocks since they're obsolete now
                mapNewBlock.clear();
                BOOST_FOREACH(CBlock* pblock, vNewBlock)
                    delete pblock;
                vNewBlock.clear();
            }

            // Clear pindexPrev so future getworks make a new block, despite any failures from here on
            pindexPrev = NULL;

            // Store the pindexBest used before CreateNewBlock, to avoid races
            nTransactionsUpdatedLast = nTransactionsUpdated;
            CBlockIndex* pindexPrevNew = pindexBest;
            nStart = GetTime();

            // Create new block
            pblock = CreateNewBlock(pwalletMain);
            if (!pblock)
                throw JSONRPCError(RPC_OUT_OF_MEMORY, "Out of memory");
            vNewBlock.push_back(pblock);

            // Need to update only after we know CreateNewBlock succeeded
            pindexPrev = pindexPrevNew;
        }

        // Update nTime
        pblock->UpdateTime(pindexPrev);
        pblock->nNonce = 0;

        // Update nExtraNonce
        static unsigned int nExtraNonce = 0;
        IncrementExtraNonce(pblock, pindexPrev, nExtraNonce);

        // Save
        mapNewBlock[pblock->hashMerkleRoot] = make_pair(pblock, pblock->vtx[0].vin[0].scriptSig);

        // Pre-build hash buffers
        char pmidstate[32];
        char pdata[128];
        char phash1[64];
        FormatHashBuffers(pblock, pmidstate, pdata, phash1);

        uint256 hashTarget = CBigNum().SetCompact(pblock->nBits).getuint256();

        json_spirit::Object result;
        result.push_back(json_spirit::Pair("midstate", HexStr(BEGIN(pmidstate), END(pmidstate)))); // deprecated
        result.push_back(json_spirit::Pair("data",     HexStr(BEGIN(pdata), END(pdata))));
        result.push_back(json_spirit::Pair("hash1",    HexStr(BEGIN(phash1), END(phash1)))); // deprecated
        result.push_back(json_spirit::Pair("target",   HexStr(BEGIN(hashTarget), END(hashTarget))));
        return result;
    }
    else
    {
        // Parse parameters
        std::vector<unsigned char> vchData = ParseHex(params[0].get_str());
        if (vchData.size() != 128)
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter");
        CBlock* pdata = (CBlock*)&vchData[0];

        // Byte reverse
        for (int i = 0; i < 128/4; i++)
            ((unsigned int*)pdata)[i] = ByteReverse(((unsigned int*)pdata)[i]);

        // Get saved block
        if (!mapNewBlock.count(pdata->hashMerkleRoot))
            return false;
        CBlock* pblock = mapNewBlock[pdata->hashMerkleRoot].first;

        pblock->nTime = pdata->nTime;
        pblock->nNonce = pdata->nNonce;
        pblock->vtx[0].vin[0].scriptSig = mapNewBlock[pdata->hashMerkleRoot].second;
        pblock->hashMerkleRoot = pblock->BuildMerkleTree();

        if (!pblock->SignBlock(*pwalletMain))
            throw JSONRPCError(-100, "Unable to sign block, wallet locked?");

        return CheckWork(pblock, *pwalletMain, reservekey);
    }
}


json_spirit::Value getblocktemplate(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp || params.size() > 1)
        throw std::runtime_error(
            "getblocktemplate [params]\n"
            "Returns data needed to construct a block to work on:\n"
            "  \"version\" : block version\n"
            "  \"previousblockhash\" : hash of current highest block\n"
            "  \"transactions\" : contents of non-coinbase transactions that should be included in the next block\n"
            "  \"coinbaseaux\" : data that should be included in coinbase\n"
            "  \"coinbasevalue\" : maximum allowable input to coinbase transaction, including the generation award and transaction fees\n"
            "  \"target\" : hash target\n"
            "  \"mintime\" : minimum timestamp appropriate for next block\n"
            "  \"curtime\" : current timestamp\n"
            "  \"mutable\" : list of ways the block template may be changed\n"
            "  \"noncerange\" : range of valid nonces\n"
            "  \"sigoplimit\" : limit of sigops in blocks\n"
            "  \"sizelimit\" : limit of block size\n"
            "  \"bits\" : compressed target of next block\n"
            "  \"height\" : height of the next block\n"
            "See https://en.bitcoin.it/wiki/BIP_0022 for full specification.");

    std::string strMode = "template";
    if (params.size() > 0)
    {
        const json_spirit::Object& oparam = params[0].get_obj();
        const json_spirit::Value& modeval = find_value(oparam, "mode");
        if (modeval.type() == json_spirit::str_type)
            strMode = modeval.get_str();
        else if (modeval.type() == json_spirit::null_type)
        {
            /* Do nothing */
        }
        else
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid mode");
    }

    if (strMode != "template")
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid mode");

    if (vNodes.empty())
        throw JSONRPCError(RPC_CLIENT_NOT_CONNECTED, "Curecoin is not connected!");

    if (IsInitialBlockDownload())
        throw JSONRPCError(RPC_CLIENT_IN_INITIAL_DOWNLOAD, "Curecoin is downloading blocks...");

    static CReserveKey reservekey(pwalletMain);

    // Update block
    static unsigned int nTransactionsUpdatedLast;
    static CBlockIndex* pindexPrev;
    static int64 nStart;
    static CBlock* pblock;
    if (pindexPrev != pindexBest ||
        (nTransactionsUpdated != nTransactionsUpdatedLast && GetTime() - nStart > 5))
    {
        // Clear pindexPrev so future calls make a new block, despite any failures from here on
        pindexPrev = NULL;

        // Store the pindexBest used before CreateNewBlock, to avoid races
        nTransactionsUpdatedLast = nTransactionsUpdated;
        CBlockIndex* pindexPrevNew = pindexBest;
        nStart = GetTime();

        // Create new block
        if(pblock)
        {
            delete pblock;
            pblock = NULL;
        }
        pblock = CreateNewBlock(pwalletMain);
        if (!pblock)
            throw JSONRPCError(RPC_OUT_OF_MEMORY, "Out of memory");

        // Need to update only after we know CreateNewBlock succeeded
        pindexPrev = pindexPrevNew;
    }

    // Update nTime
    pblock->UpdateTime(pindexPrev);
    pblock->nNonce = 0;

    json_spirit::Array transactions;
    std::map<uint256, int64_t> setTxIndex;
    int i = 0;
    CTxDB txdb("r");
    BOOST_FOREACH (CTransaction& tx, pblock->vtx)
    {
        uint256 txHash = tx.GetHash();
        setTxIndex[txHash] = i++;

        if (tx.IsCoinBase() || tx.IsCoinStake())
            continue;

        json_spirit::Object entry;

        CDataStream ssTx(SER_NETWORK, PROTOCOL_VERSION);
        ssTx << tx;
        entry.push_back(json_spirit::Pair("data", HexStr(ssTx.begin(), ssTx.end())));

        entry.push_back(json_spirit::Pair("hash", txHash.GetHex()));

        MapPrevTx mapInputs;
        std::map<uint256, CTxIndex> mapUnused;
        bool fInvalid = false;
        if (tx.FetchInputs(txdb, mapUnused, false, false, mapInputs, fInvalid))
        {
            entry.push_back(json_spirit::Pair("fee", (int64_t)(tx.GetValueIn(mapInputs) - tx.GetValueOut())));

            json_spirit::Array deps;
            BOOST_FOREACH (MapPrevTx::value_type& inp, mapInputs)
            {
                if (setTxIndex.count(inp.first))
                    deps.push_back(setTxIndex[inp.first]);
            }
            entry.push_back(json_spirit::Pair("depends", deps));

            int64_t nSigOps = tx.GetLegacySigOpCount();
            nSigOps += tx.GetP2SHSigOpCount(mapInputs);
            entry.push_back(json_spirit::Pair("sigops", nSigOps));
        }

        transactions.push_back(entry);
    }

    json_spirit::Object aux;
    aux.push_back(json_spirit::Pair("flags", HexStr(COINBASE_FLAGS.begin(), COINBASE_FLAGS.end())));

    uint256 hashTarget = CBigNum().SetCompact(pblock->nBits).getuint256();

    static json_spirit::Array aMutable;
    if (aMutable.empty())
    {
        aMutable.push_back("time");
        aMutable.push_back("transactions");
        aMutable.push_back("prevblock");
    }

    json_spirit::Object result;
    result.push_back(json_spirit::Pair("version", pblock->nVersion));
    result.push_back(json_spirit::Pair("previousblockhash", pblock->hashPrevBlock.GetHex()));
    result.push_back(json_spirit::Pair("transactions", transactions));
    result.push_back(json_spirit::Pair("coinbaseaux", aux));
    result.push_back(json_spirit::Pair("coinbasevalue", (int64_t)pblock->vtx[0].vout[0].nValue));
    result.push_back(json_spirit::Pair("target", hashTarget.GetHex()));
    result.push_back(json_spirit::Pair("mintime", (int64_t)pindexPrev->GetMedianTimePast()+1));
    result.push_back(json_spirit::Pair("mutable", aMutable));
    result.push_back(json_spirit::Pair("noncerange", "00000000ffffffff"));
    result.push_back(json_spirit::Pair("sigoplimit", (int64_t)MAX_BLOCK_SIGOPS));
    result.push_back(json_spirit::Pair("sizelimit", (int64_t)MAX_BLOCK_SIZE));
    result.push_back(json_spirit::Pair("curtime", (int64_t)pblock->nTime));
    result.push_back(json_spirit::Pair("bits", HexBits(pblock->nBits)));
    result.push_back(json_spirit::Pair("height", (int64_t)(pindexPrev->nHeight+1)));

    return result;
}

json_spirit::Value submitblock(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 2)
        throw std::runtime_error(
            "submitblock <hex data> [optional-params-obj]\n"
            "[optional-params-obj] parameter is currently ignored.\n"
            "Attempts to submit new block to network.\n"
            "See https://en.bitcoin.it/wiki/BIP_0022 for full specification.");

    std::vector<unsigned char> blockData(ParseHex(params[0].get_str()));
    CDataStream ssBlock(blockData, SER_NETWORK, PROTOCOL_VERSION);
    CBlock block;
    try {
        ssBlock >> block;
    }
    catch (std::exception &e) {
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Block decode failed");
    }

    if (!block.SignBlock(*pwalletMain))
        throw JSONRPCError(-100, "Unable to sign block, wallet locked?");

    bool fAccepted = ProcessBlock(NULL, &block);
    if (!fAccepted)
        return "rejected";

    return json_spirit::Value::null;
}

