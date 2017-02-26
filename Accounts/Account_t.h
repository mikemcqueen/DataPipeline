////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2009 Mike McQueen.  All rights reserved.
//
// Account_t.h
//
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Character_t.h"
#include "GameId.h"

////////////////////////////////////////////////////////////////////////////////

class AccountManager_t;

class Account_t
{
private:

    AccountManager_t& m_manager;
    GameId_t          m_gameId;
    AccountId_t       m_accountId;
    wstring           m_username;
    wstring           m_password;
    shared_ptr<Character_t> m_spCharacter; // TODO: vector

public:

    Account_t(
        AccountManager_t& manager,
        GameId_t          gameId,
        AccountId_t       accountId,
        const wchar_t*    pUsername,
        const wchar_t*    pPassword)
    :
        m_manager(manager),
        m_gameId(gameId),
        m_accountId(accountId),
        m_username(pUsername),
        m_password(pPassword)
    {
    }

    const wstring&
    GetUsername() const { return m_username; }

    const wstring&
    GetPassword() const { return m_password; }

    Character_t&
    GetCharacter(
        ServerId_t     serverId,
        const wchar_t* pCharacterName);
};

////////////////////////////////////////////////////////////////////////////////
