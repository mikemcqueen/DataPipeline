/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_COMMONTYPES_H_
#define Include_COMMONTYPES_H_

//typedef unsigned long Flag_t;

template<typename T>
concept underlying_type_int = std::is_same_v<std::underlying_type_t<T>, int>;

int intValue(underlying_type_int auto val) {
    return static_cast<int>(val);
}

#endif // Include_COMMONTYPES_H_

/////////////////////////////////////////////////////////////////////////////
