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
class pool_item
{
    typedef pool_item<T>    PoolItemT;
    typedef pool<PoolItemT> PoolT;

protected:
            volatile LONG   m_iRefCount;
    mutable volatile LONG   m_state;
                     PoolT* m_pPool;
                     T      m_item;

public:
    pool_item(
        PoolT* pPool,
        T&     item)
    :   m_pPool(pPool),
        m_iRefCount(0),
        m_item(item),
        m_state(0)
    {
    }

    const T& get( void ) const
    {
        return m_item;
    }

    int addref( void )
    {
        return InterlockedIncrement(&m_iRefCount);
    }

    int release( void )
    {
        LONG lRef = InterlockedDecrement(&m_iRefCount);
        if (0 == lRef)
        {
            m_pPool->release(*this);
        }
        else if (0 > lRef)
        {
//            LogInfo(L"pool_item::release(): refcount = %d", lRef);
            // InterlockedExchange(&m_iRefCount, 0);
            m_iRefCount = 0;
        }
        return lRef;
    }

    bool operator ==(const pool_item<T>& x)
    {
        return m_item == x.m_item;
    }

    void set_state(LONG state)
    {
        InterlockedExchange(&m_state, state);
    }

    int get_state() const
    {
        return InterlockedCompareExchange(&m_state, 0, 0);
    }

#if 1
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
protected:
    std::vector<T> m_Items;

    //private:
    size_t               m_sizeMin;
private:
    mutable CAutoCritSec m_cs;

protected:
    void Lock() { m_cs.lock(); }

public:
    pool(size_t size = 0)
    {
        m_Items.reserve(size);
        m_sizeMin = 0;
    }

    T& at(int iPos)
    {
        CLock lock(m_cs);
        return m_Items.at(iPos);
    }

    size_t minsize() const
    {
        return m_sizeMin;
    }

    size_t size() const
    {
        CLock lock(m_cs);
        return m_Items.size();
    }

    void reserve(size_t count)
    {
        CLock lock(m_cs);
        m_Items.reserve(count);
    }

    void add(T& item)
    {
        CLock lock(m_cs);
        item.set_state(PF_UNUSED);
        m_Items.push_back(item);
        m_sizeMin = size();
    }

    T* get_unused()
    {
        CLock lock(m_cs);
        for (size_t i = 0; i < size(); ++i )
        {
            if (PF_UNUSED == m_Items[i].get_state())
            {
                m_Items[i].set_state(PF_USED);
                m_Items[i].addref();
                return &m_Items[i];
            }
        }
        return NULL;
    }

    T& get_ready(size_t first = 0)
    {
        CLock lock(m_cs);
        for (size_t i = first; i < size(); ++i)
        {
            if (PF_READY == m_Items[i].get_state())
            {
                m_Items[i].addref();
                return m_Items[i];
            }
        }
        throw E_FAIL;
    }

    void release(const T& item)
    {
        CLock lock(m_cs);
        for (size_t i = 0; i < size(); ++i)
        {
            if (m_Items[i] == item)
            {
                m_Items[i].set_state(PF_UNUSED);
                return;
            }
        }
        throw E_FAIL;
    }

    size_t
    unused() const
    {
        long count = 0;
        CLock lock(m_cs);
        for (size_t i = 0; i < size(); ++i )
        {
            if (PF_UNUSED == m_Items[i].get_state())
                ++count;
        }
        return count;
    }

    bool
    all_unused() const
    {
        return unused() == size();
    }
};

/////////////////////////////////////////////////////////////////////////////

#endif // Include_POOL_H

/////////////////////////////////////////////////////////////////////////////
