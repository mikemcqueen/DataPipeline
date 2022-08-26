////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2009 Mike McQueen.  All rights reserved.
//
////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

namespace boost { namespace program_options {
    typedef vector< shared_ptr<option_description> > option_desc_vector_t;
} }

class CmdParser_t
{
public:

    virtual
    bool
    Parse(
        const StringVector_t& args) = 0;

    static
    bool
    Parse(
        const StringVector_t&          args,
              po::variables_map&       vm,
              po::options_description& all,
        const po::options_description* commands = 0)
    {
        locale::global(locale(""));
        po::wcommand_line_parser parser(args);
        po::store(parser.options(all).run(), vm);
        po::notify(vm);
        return Validate(vm, all, commands);
    }

    static
    size_t
    GetCount(
        const po::variables_map&       vm,
        const po::options_description& optionsDesc)
    {
        size_t count = 0;
        const po::option_desc_vector_t& options = optionsDesc.options();
        po::option_desc_vector_t::const_iterator it = options.begin();
        for (; options.end() != it; ++it)
        {
            try
            {
                count += vm[(*it)->long_name()].as<bool>();
            }
            catch (exception&)
            {
            }
        }
        return count;
    }

    static
    bool
    Validate
    (
        const po::variables_map& vm,
        const po::options_description& all,
        const po::options_description* commands)
    {
        if (vm.count("help"))
        {
            cout << endl << all << endl;
            return false;
        }
        else if ((NULL != commands) && (0 == GetCount(vm, *commands)))
        {
            LogError(L"Missing command: -? for help");
            return false;
        }
        return true;
    }

    static
    ItemId_t
    GetItemId(
        const po::variables_map& vm)
    {
        long id = 0;
        if (1 != vm.count("name") + vm.count("id"))
        {
            LogError(L"You must specify either Name or Id");
        }
        else if (vm.count("name"))
        {
            const wstring& name = vm["name"].as<wstring>(); // safe?
            id = (long)Accounts::Db::Items_t::GetItemId(name.c_str());
            if (0 == id)
            {
                LogError(L"Unknown item name(%s)", name.c_str());
            }
        }
        else
        {
            id = vm["id"].as<long>();
            if ((0 >= id) || (NULL == Accounts::Db::Items_t::GetItemName(id)))
            {
                LogError(L"Invalid item id(%d)", id);
                id = 0;
            }
        }
        return id;
    }
};

////////////////////////////////////////////////////////////////////////////////
