////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2009 Mike McQueen.  All rights reserved.
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Eq2Broker_t.h"
#include "Eq2BrokerImpl_t.h"
#include "PipelineManager.h"
#include "TransactionManager.h"
#include "Character_t.h"
#include "Log.h"
#include "Macros.h"
#include "BrokerId.h"
#include "MainWindow_t.h"
#include "Character_t.h"
#include "DbItems_t.h"
#include "CmdParser_t.h"
#include "DbItemsOwned_t.h"

using namespace Broker;

////////////////////////////////////////////////////////////////////////////////
//
// CmdBuySell_t
//
////////////////////////////////////////////////////////////////////////////////

class CmdBuySell_t :
    public CmdParser_t
{
public:

    static po::options_description m_all;
    static po::options_description m_commands;
    static po::options_description m_options;
    static bool s_hasInit;

public:

    CmdBuySell_t()
    {
    }

    virtual
    bool
    Parse(
        const StringVector_t& args) override;

    ItemId_t
    Validate(
        const po::variables_map& vm);

    void
    Execute(
        ItemId_t itemId,
        const po::variables_map& vm);

private:

    void
    Init();

};

////////////////////////////////////////////////////////////////////////////////

/*static*/ po::options_description CmdBuySell_t::m_all("BuySell <command> [options]");
/*static*/ po::options_description CmdBuySell_t::m_commands("Commands:");
/*static*/ po::options_description CmdBuySell_t::m_options("Options:");
/*static*/ bool CmdBuySell_t::s_hasInit = false;

////////////////////////////////////////////////////////////////////////////////

void
CmdBuySell_t::
Init()
{
    if (!s_hasInit)
    {
        m_commands.add_options()
            ("add",           po::bool_switch(),                      "add entry")
            ("delete",        po::bool_switch(),                      "delete entry")
            //("modify",        po::bool_switch(),                      "modify entry")
            ("dump",          po::bool_switch(),                      "dump buysell map")
            ("reload",        po::bool_switch(),                      "reload buysell map from db")
            ;
        m_options.add_options()
            ("help,?",                                               "show usage")
            ("name",          po::wvalue<wstring>(),                  "item name -or-")
            ("id",            po::value<long>(),                      "item id")
            ("lowbid",        po::value<long>()->default_value(0),    "low bid")
            ("highbid",       po::value<long>()->default_value(0),    "high bid")
            ("incbid",        po::value<long>()->default_value(0),    "bid increment")
            ("lowask",        po::value<long>()->default_value(0),    "low ask")
            ("highask",       po::value<long>()->default_value(0),    "high ask")
            ("incask",        po::value<long>()->default_value(0),    "ask increment")
            ("maxtoown",      po::value<long>()->default_value(1),    "max to own")
            ("maxtosell",     po::value<long>()->default_value(0),    "max to sell")
            ("maxforsale",    po::value<long>()->default_value(0),    "thread count")
            //        ("server",        po::value<long>()->default_value(0),    "server id")
            //        ("character",     po::value<long>()->default_value(0),    "character id")
            ;
        m_all.add(m_commands).add(m_options);
        s_hasInit = true;
    }
}

////////////////////////////////////////////////////////////////////////////////

ItemId_t
CmdBuySell_t::
Validate(
    const po::variables_map& vm)
{
    if (vm["dump"].as<bool>())
    {
        GetCharacter().BuySellDump();
        return 0;
    }
    else if (vm["reload"].as<bool>())
    {
        GetCharacter().Reload(); 
        return 0;
    }
    long id = GetItemId(vm);
    if (vm["add"].as<bool>())
    {
        long bid = vm["highbid"].as<long>();
        if (0 == bid)
        {
            LogError(L"CmdBuySell_t::Validate() Add: You must specify a high bid");
            return 0;
        }
    }
    return id;
}

////////////////////////////////////////////////////////////////////////////////

void
CmdBuySell_t::
Execute(
    ItemId_t itemId,
    const po::variables_map& vm)
{
    BuySellData_t data;
    if (vm["add"].as<bool>()) // || vm["modify"].as<bool>())
    {
        data.lowBid     = vm["lowbid"].as<long>();
        data.highBid    = vm["highbid"].as<long>();
        data.incBid     = vm["incbid"].as<long>();
        data.lowAsk     = vm["lowask"].as<long>();
        data.highAsk    = vm["highask"].as<long>();
        data.incAsk     = vm["incask"].as<long>();
        data.maxToOwn   = vm["maxtoown"].as<long>();
        data.maxToSell  = vm["maxtosell"].as<long>();
        data.maxForSale = vm["maxforsale"].as<long>();
    }
    if (vm["add"].as<bool>())
    {
        GetCharacter().BuySellAdd(itemId, data);
    }
    else if (vm["delete"].as<bool>())
    {
        GetCharacter().BuySellDelete(itemId); 
    }
    else
    {
        throw logic_error("CmdBuySell_t::Execute() invalid action");
        //GetCharacter().BuySellModify(itemId, data);
    }
}

////////////////////////////////////////////////////////////////////////////////

bool
CmdBuySell_t::
Parse(
    const std::vector<wstring>& args)
{
    Init();
    po::variables_map vm;
    if (CmdParser_t::Parse(args, vm, m_all, &m_commands))
    {
        ItemId_t itemId = Validate(vm);
        if (0 != itemId)
        {
            Execute(itemId, vm);
        }
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
Eq2Broker_t::
CmdCharacter(
    const wchar_t* pszCmd)
{
#if 0
    static CmdBuySell_t cmdBuySell;
    std::vector<wstring> args = po::split_winmain(pszCmd);

    size_t Pos = 0;
    switch (pszCmd[Pos++]) 
    {
    case L'b': // buysell items
        return cmdBuySell.Parse(args);

    case L'o': // owned items
        switch (pszCmd[Pos]) 
        {
        case L'd': // dump items owned
            // TODO: GetCharacter().GetItemsOwned().Dump();
            GetCharacter().DumpItemsOwned();
            return true;
        case L'i': // init items owned from items for sale
            if (wstring(L"init") == &pszCmd[Pos])
            {
                GetCharacter().InitItemsOwnedFromItemsForSale();
                return true;
            }
            break;
#if 0
        case L'w': // write items owned
            {
                using namespace Accounts::Db;
                ItemsOwned_t::Write(GetCharacter.GetId(), GetCharacter().GetItemsOwned());
            }
            return true;
#endif
        default:
            break;
        }
        break;
    default:
        break;
    }
#endif
    return false;
}

////////////////////////////////////////////////////////////////////////////////
