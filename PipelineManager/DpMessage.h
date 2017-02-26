////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008-2009 Mike McQueen.  All rights reserved.
//
// DpMessage.h
//
////////////////////////////////////////////////////////////////////////////////

#if _MSC_VER >= 1000
#pragma once
#endif

#ifndef Include_DPMESSAGE_H
#define Include_DPMESSAGE_H

namespace DP
{

////////////////////////////////////////////////////////////////////////////////

typedef unsigned Stage_t;
namespace Stage
{
    enum : Stage_t
    {
        None       = 0,
        Acquire    = 0x00000001,
        Translate  = 0x00000002,
        Interpret  = 0x00000004,
        Analyze    = 0x00000008,
        Execute    = 0x00000010,
        Any        = 0x0000001F,
    };
}

typedef unsigned MessageType_t;
typedef unsigned MessageId_t;

namespace Message
{
    namespace Type
    {
        enum : MessageType_t
        {
            Unknown = 0,
            Message,
            Event,
            Transaction,
        };
    }

    namespace Id
    {
        enum : MessageId_t
        {
            Unknown    = 0,
            Screenshot = 1,
            User_First = 0x00010000,
        };
    }

    struct Data_t
    {
        static const size_t ClassLength = 32;

        Stage_t       Stage;
        MessageId_t   Id;
        size_t        Size;
        wchar_t       Class[ClassLength];
        MessageType_t Type;

        Data_t(
            Stage_t        InitStage = Stage::Any,
            MessageId_t    InitId    = Id::Unknown,
            size_t         InitSize  = sizeof(Data_t),
            const wchar_t* InitClass = NULL,
            MessageType_t  InitType  = Type::Message)
        :
            Stage(InitStage),
            Type(InitType),
            Id(InitId),
            Size(InitSize)
        {
            Class[0] = L'\0';
            if (NULL != InitClass)
            {
                wcscpy_s(Class, InitClass);
            }
        }

        virtual
        ~Data_t()
        {
        }
    };

////////////////////////////////////////////////////////////////////////////////

} // Message
} // DP

#endif // Include_DPMESSAGE_H

////////////////////////////////////////////////////////////////////////////////
