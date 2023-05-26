////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2009 Mike McQueen.  All rights reserved.
//
// AccountManager_t.h
//
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Account_t.h"

class AccountManager_t
{
private:

    std::shared_ptr<Account_t> m_spAccount;

public:

    AccountManager_t() {}
    ~AccountManager_t() {}

    Account_t&
    GetAccount(
        GameId_t gameId);
};

////////////////////////////////////////////////////////////////////////////////
