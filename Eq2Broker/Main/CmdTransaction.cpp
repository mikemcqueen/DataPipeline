////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2009 Mike McQueen.  All rights reserved.
//
// CmdTransaction.cpp
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Eq2Broker_t.h"
#include "Eq2BrokerImpl_t.h"
#include "PipelineManager.h"
#include "TransactionManager.h"
#include "StringSet_t.h"
#include "CmdParser_t.h"
#include "BrokerId.h"
#include "Log.h"

using namespace Broker;
using namespace Transaction;

////////////////////////////////////////////////////////////////////////////////
//
// CmdTxExecute_t
//
////////////////////////////////////////////////////////////////////////////////

class CmdTxExecute_t :
    public CmdParser_t
{
public:

    static po::options_description m_all;
    static po::options_description m_commands;
    static po::options_description m_options;
    static bool s_hasInit;

public:

    CmdTxExecute_t()
    {
    }

    virtual
    bool
    Parse(
        const StringVector_t& args) override;

    bool
    Validate(
        const po::variables_map& vm,
              ItemId_t& itemId);

    void
    Execute(
        const po::variables_map& vm,
              ItemId_t itemId);

private:

    void
    Init();
};

////////////////////////////////////////////////////////////////////////////////

/*static*/ po::options_description CmdTxExecute_t::m_all("TxExecute <command> [options]");
/*static*/ po::options_description CmdTxExecute_t::m_commands("Commands:");
/*static*/ po::options_description CmdTxExecute_t::m_options("Options:");
/*static*/ bool CmdTxExecute_t::s_hasInit = false;

////////////////////////////////////////////////////////////////////////////////

void
CmdTxExecute_t::
Init()
{
    if (s_hasInit)
    {
        return;
    }
    m_commands.add_options()
        ("info,i",        po::bool_switch(),         "show active transaction info")
        ("kill,k",        po::bool_switch(),         "kill active transaction")
        ("setwindow",     po::bool_switch(),         "TxSetActiveWindow (--windowid)")
        ("buytab",        po::bool_switch(),         "TxBuyTabGetItems")
        ("selltab",       po::bool_switch(),         "TxSellTabGetItems")
        ("getprices",     po::bool_switch(),         "TxGetItemPrices")
        ("getforsale",    po::bool_switch(),         "TxGetItemsForSale")
        ("reprice",       po::bool_switch(),         "TxRepriceItems")
        ("logon",         po::bool_switch(),         "TxLogon (--camp, --handle)")
        ("settext",       po::bool_switch(),         "TxSetWidgetText (--text)")
        ("buy",           po::bool_switch(),         "TxBuyItem (--name/--id)")
        ("buysell",       po::bool_switch(),         "TxBuySell")
        ("openbroker",    po::bool_switch(),         "TxOpenBroker")
        ;
    m_options.add_options()
        ("help,?",                                   "show usage")
        ("name",          po::wvalue<wstring>(),     "item name")
        ("handle",        po::wvalue<wstring>(),     "login handle")
        ("id",            po::value<long>(),         "item id")
        ("windowid",      po::value<long>(),         "setwindow windowid")
        ("price",         po::value<long>(),         "buy price")
        ("quantity",      po::value<long>()->default_value(1), "buy quantity")
        ("text",          po::wvalue<wstring>(),     "settext text")
        ("camp",          po::bool_switch(),         "camp after executing")
        ("all",           po::bool_switch(),         "kill all transactions")
        ;
    m_all.add(m_commands).add(m_options);
    s_hasInit = true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CmdTxExecute_t::
Parse(
    const StringVector_t& args)
{
    Init();
    po::variables_map vm;
    if (CmdParser_t::Parse(args, vm, m_all, &m_commands))
    {
        ItemId_t itemId;
        if (Validate(vm, itemId))
        {
            Execute(vm, itemId);
        }
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CmdTxExecute_t::
Validate(
    const po::variables_map& vm,
          ItemId_t&          itemId)
{
    itemId = 0;
    if (vm["buy"].as<bool>() ||
        vm["getprices"].as<bool>())
    {
        itemId = GetItemId(vm);
        if (0 == itemId)
        {
            return false;
        }
    }
    if (vm["buy"].as<bool>())
    {
        if (0 == vm.count("price"))
        {
            LogError(L"Missing count");
            return false;
        }
        if ((0 >= vm["quantity"].as<long>()) ||
            (0 >= vm["price"].as<long>()))
        {
            LogError(L"Bad price(%d) or quantity(%d)",
                     vm["price"].as<long>(), vm["quantity"].as<long>());
            return false;
        }
    }
    else if (vm["logon"].as<bool>())
    {
        if (0 == vm.count("handle"))
        {
            LogError(L"Missing handle");
            return false;
        }
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////

void
CmdTxExecute_t::
Execute(
    const po::variables_map& vm,
          ItemId_t           itemId)
{
    static ItemDataMap_t sellTabMap;

    DP::Transaction::Data_t* pTxData = nullptr;
    wstring itemName;
    if (0 != itemId)
    {
        itemName.assign(Accounts::Db::Items_t::GetItemName(itemId));
    }
    if (vm["info"].as<bool>())
    {
        DP::TransactionManager_t::AutoRelease_t tm(GetTransactionManager().Acquire());
        if (nullptr == tm.get())
        {
            LogAlways(L"No transaction active.");
        }
        else
        {
            const DP::Transaction::Data_t& data = *tm.get();
            LogAlways(L"Active transaction Id(%x) State(%x) Error(%d)",
                      data.Id, data.GetState(), data.Error);
        }
    }
    else if (vm["kill"].as<bool>())
    {
        DP::TransactionManager_t::AutoRelease_t txData(GetTransactionManager().Acquire());
        if (nullptr == txData.get())
        {
            LogAlways(L"No transaction active.");
        }
        else
        {
            GetTransactionManager().CompleteTransaction(txData.get()->Id,
                DP::Transaction::Error::Aborted);
        }
    }
    else if (vm["buy"].as<bool>())
    {
        long quantity = vm["quantity"].as<long>();
        long price = vm["price"].as<long>();
        LogAlways(L"TxBuyItem: %d x (%s) @ (%s)",
                  quantity, itemName.c_str(), GetCoinString(price));
        pTxData = new BuyItem::Data_t(itemName, price, quantity);
    }
    else if (vm["buysell"].as<bool>())
    {
        pTxData = new BuySellItems::Data_t;
    }
    else if (vm["buytab"].as<bool>())
    {
        static ItemDataMap_t map;
        pTxData = new BuyTabGetItems::Data_t(wstring(), map);
    }
    else if (vm["getprices"].as<bool>())
    {
        // TODO: foreach item in sellTabMap 
        static PriceCountMap_t map;
        pTxData = new GetItemPrices::Data_t(itemName, map);
    }
    else if (vm["getforsale"].as<bool>())
    {
        pTxData = new GetItemsForSale::Data_t;
    }
    else if (vm["logon"].as<bool>())
    {
        if (vm["camp"].as<bool>())
        {
            pTxData = new Logon::Data_t(Logon::Method::Camp);
        }
        else
        {
            pTxData = new Logon::Data_t(vm["handle"].as<wstring>());
        }
    }
    else if (vm["openbroker"].as<bool>())
    {
        pTxData = new OpenBroker::Data_t;
    }
    else if (vm["reprice"].as<bool>())
    {
        pTxData = new RepriceItems::Data_t(sellTabMap);
    }
    else if (vm["selltab"].as<bool>())
    {
        pTxData = new SellTabGetItems::Data_t(sellTabMap);
    }
    if (nullptr != pTxData)
    {
        GetTransactionManager().ExecuteTransaction(pTxData);
        if (vm["camp"].as<bool>() && !vm["logon"].as<bool>())
        {
            GetTransactionManager().ExecuteTransaction(
                new Logon::Data_t(Logon::Method::Camp));
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

static StringSet_t s_stringSet;

bool
AddNamesToStringSet(
    const wchar_t* pTextRow,
    const wstring& txItemName,
    ItemDataMap_t& itemDataMap)
{
    txItemName; itemDataMap;
    using namespace Buy::Table;
    Buy::Text_t text(CharColumnWidths, ColumnCount);
    const wstring itemName(text.GetItemName(pTextRow));
    if (!itemName.empty())
    {
        s_stringSet.insert(itemName);
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
Eq2Broker_t::
CmdTransaction(
    const wchar_t* pszCmd)
{
    // execute
    size_t Pos = 0;
    if (L'x' != pszCmd[Pos++])
    {
        return false;
    }
    static CmdTxExecute_t cmdTxExecute;
    StringVector_t args = po::split_winmain(pszCmd);

    using namespace Broker::Transaction;
    DP::Transaction::Data_t *pData = nullptr;
    switch (pszCmd[Pos++]) 
    {
    case L'b': // TxBuyItem, TxBuyTabGetItems
        {
            static ItemDataMap_t map;
            if (wstring(L"list") == &pszCmd[Pos])
            {
                LogAlways(L"TxBuyTabGetItems list test - building item list - use 'tplist' to price");
                s_stringSet.clear();
                pData = new BuyTabGetItems::Data_t(wstring(), map,
                    BuyTabGetItems::Data_t::FnAddRow_t(AddNamesToStringSet));
            }
        }
        break;

    case L't': // SetWidgetText transaction
        if (L'\0' == pszCmd[Pos])
        {
            LogError(L"Missing text");
        }
        else
        {
            using namespace Broker::Buy;
            pData = new SetWidgetText::Data_t(
                Broker::Window::Id::BrokerBuyTab,
                Buy::Widget::Id::SearchEdit,
                wstring(&pszCmd[Pos]),
                Buy::Widget::Id::SearchLabel);
        }
        break;

    default:
        return cmdTxExecute.Parse(args);
    }
    return false;
}

