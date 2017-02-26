////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2009 Mike McQueen.  All rights reserved.
//
////////////////////////////////////////////////////////////////////////////////

#pragma once

class Account_t;

typedef unsigned AccountId_t;
typedef unsigned ServerId_t;
typedef unsigned SellerId_t;
typedef unsigned CharacterId_t;
typedef unsigned ItemId_t;

namespace Game
{
    namespace Id
    {
        enum E : unsigned
        {
            Undefined = 0,
            Eq2
        };
    }
} // Game

typedef Game::Id::E GameId_t;

namespace Eq2
{
    namespace Server
    {
        namespace Id
        {
            enum : ServerId_t
            {
                Undefined = 0,
                Mistmoore,
            };
        }
    }
} // Eq2

