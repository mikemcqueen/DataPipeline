/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// POOL.H
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER >= 1000
#pragma once
#endif

#ifndef Include_POOL_H
#define Include_POOL_H

/////////////////////////////////////////////////////////////////////////////

#include "AutoCs.h"

/////////////////////////////////////////////////////////////////////////////

#define PF_UNUSED       1
#define PF_USED         2
#define PF_READY        3

/////////////////////////////////////////////////////////////////////////////

template<class T> class pool;
//template<class T> class pool_item;

/////////////////////////////////////////////////////////////////////////////

template<class T>
class pool_item final
{
    //typedef pool_item<T>    PoolItemT;
    //typedef pool<PoolItemT> PoolT;

private:
    pool<T>* m_pPool;
    T* m_pData;
    LONG m_refCount;
    mutable LONG m_state;

public:
    pool_item(
        pool<T>* pPool,
        T* pData)
        :   
        m_pPool(pPool),
        m_pData(pData),
        m_refCount(0),
        m_state(0)
    {
    }

    T* get() const
    {
        return m_pData;
    }

    int addref()
    {
        return InterlockedIncrement(&m_refCount);
    }

    int release()
    {
        LONG refCount = InterlockedDecrement(&m_refCount);
        if (0L == refCount) {
            m_pPool->release(this);
        }
        else if (0L > refCount) {
            LogError(L"pool_item::release(): refcount = %d", refCount);
            // InterlockedExchange(&m_iRefCount, 0);
            m_refCount = 0;
        }
        return refCount;
    }

    // TODO: sketchy. remove this. 
    bool operator==(const pool_item<T>& x)
    {
        return m_pData == x.m_pData;
    }

    void set_state(LONG state)
    {
        InterlockedExchange(&m_state, state);
    }

    int get_state() const
    {
        return InterlockedCompareExchange(&m_state, 0, 0);
    }

#if 0
    const T& T() const
    {
        return m_item;
    }
#endif
};

/////////////////////////////////////////////////////////////////////////////

template<class T>
class pool
{
public:
    using item_t = pool_item<T>;

    explicit 
    pool(int size = 0)
    {
        m_items.reserve(size);
        m_minSize = 0;
    }

    item_t& at(int index)
    {
        CLock lock(m_cs);
        return m_items.at(index);
    }

    int minsize() const
    {
        return m_minSize;
    }

    int size() const
    {
        CLock lock(m_cs);
        return m_items.size();
    }

    void reserve(int count)
    {
        CLock lock(m_cs);
        m_items.reserve(count);
    }

    void add(const item_t& item)
    {
        CLock lock(m_cs);
        // TODO: possible allocation + copy here. With a CSurface* that gets lost
        // if we throw. Maybe a good spot for a unique_ptr?
        m_items.push_back(item);
        m_items.back().set_state(PF_UNUSED);
        m_minSize = size();
    }

    item_t* get_unused()
    {
        CLock lock(m_cs);
        for (auto& item: m_items) {
            if (PF_UNUSED == item.get_state()) {
                item.set_state(PF_USED);
                item.addref();
                return &item;
            }
        }
        return nullptr;
    }

    item_t* get_ready(int first = 0)
    {
        CLock lock(m_cs);
        for (int i = first; i < size(); ++i) {
            auto& item = m_items.at(i);
            if (PF_READY == item.get_state()) {
                item.addref();
                return &item;
            }
        }
        throw E_FAIL;
    }

    void release(const item_t* pItem)
    {
        CLock lock(m_cs);
        for (auto& item : m_items) {
            if (&item == pItem) {
                item.set_state(PF_UNUSED);
                return;
            }
        }
        throw E_FAIL;
    }

    int unused() const
    {
        int count = 0;
        CLock lock(m_cs);
        for (auto& item : m_items) {
            if (PF_UNUSED == item.get_state()) {
                ++count;
            }
        }
        return count;
    }

    bool all_unused() const
    {
        return unused() == size();
    }

protected:
    void Lock() { m_cs.lock(); }

private:
    std::vector<item_t> m_items;
    int m_minSize;
    mutable CAutoCritSec m_cs;
};

/////////////////////////////////////////////////////////////////////////////

#endif // Include_POOL_H

/////////////////////////////////////////////////////////////////////////////
