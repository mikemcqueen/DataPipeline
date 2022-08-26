///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// DpTransaction.h
//
///////////////////////////////////////////////////////////////////////////////

#if _MSC_VER >= 1000
#pragma once
#endif

#ifndef Include_DPTRANSACTION_H
#define Include_DPTRANSACTION_H

#include "DpEvent.h"

namespace DP
{

using TransactionId_t = MessageId_t;
namespace Transaction
{
    namespace Id
    {
        //using namespace Message;
        static const TransactionId_t Unknown    = TransactionId_t(0);
        static const TransactionId_t User_First = TransactionId_t(Message::MakeId(0x1000));
           // = TransactionId_t(Message::Id::Txn_First) + 0x00001000
    }

    typedef unsigned State_t;
    namespace State
    {
        enum E : State_t
        {
            New,
            Pending,
            Complete,
            // Error, ?
            User_First = 0x00040000
        };
    }

    typedef unsigned Error_t;
    namespace Error
    {
        enum E : Error_t
        {
            None       = 0,
            Aborted    = 0x80000001,
            Timeout    = 0x80000002,
            User_First = 0x80040000
        };
    }
    
    struct Data_t :
        public Event::Data_t
    {
    private:

        State_t   State;
        size_t    stateTimeout;

    public:

        Error_t   Error;

        Data_t(
            TransactionId_t transactionId,
            size_t          size = sizeof(Data_t),
            Stage_t         stage = Stage::Any)
            :
            Event::Data_t(
                stage,
                transactionId,
                size,
                0,
                Message::Type::Transaction),
            State(State::New),
            stateTimeout(0),
            Error(Error::None)
        { }

        State_t
        GetState() const
        {
            return State;
        }

        size_t
        GetStateTimeout() const
        {
            return stateTimeout;
        }

        size_t
        IncStateTimeout()
        {
            return ++stateTimeout;
        }

        bool
        Execute(
            bool fInterrupt = false);

        void
        Complete(
            Transaction::Error_t Error = Transaction::Error::None) const;

        void
        PrevState()
        {
            SetState(State - 1);
        }

        void
        NextState()
        {
            SetState(State + 1);
        }

        void
        SetState(State_t state);

    private:

        Data_t();
    };

} // Transaction
} // DP

///////////////////////////////////////////////////////////////////////////////

#endif // Include_DPTRANSACTION_H

///////////////////////////////////////////////////////////////////////////////
