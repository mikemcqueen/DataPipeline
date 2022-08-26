////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008-2009 Mike McQueen.  All rights reserved.
//

#pragma once

#include "DiffDbTypes.h"

namespace DiffDb
{

class DiffDir_t
{
    struct Result_t
    {
        vector<HANDLE>       threadVector;
        vector<ThreadData_t> threadData;
    };

public:

    DiffDir_t() {}

    void
    Diff(
        const DbFiles_t& dbFiles,
              int        threadCount);

private:

    static
    void
    DumpHiloHeader();

    static
    DWORD
    DiffExecute(
        ThreadData_t* pData);

    static
    DWORD WINAPI
    DiffThreadProc(
        LPVOID pvParam);

    void
    WaitForAllThreads(
        Result_t& data);

    void
    DiffFiles(
        const DbFiles_t& dbFiles,
              Result_t&  result);

    HANDLE
    DoDiff(
        ThreadData_t& data,
        bool          bCreateThread);

    void
    DumpSaleData(
        const ThreadDataVector_t& threadData,
        const ItemCoalesceMap_t&  itemMap);

    size_t
    DumpHiLoData(
              ItemId_t         itemId,
        const ThreadDataVector_t&  threadVector,
        const ItemCoalesceMap_t&   itemMap) const;
};

} // namespace DiffDb
