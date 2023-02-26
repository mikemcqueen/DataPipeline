/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// DCR.H
//
// Digital Character Recognition (a.k.a. ConvertBitmapToText)
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_DCR_H
#define Include_DCR_H

class Rect_t;
class CSurface;
class TextTable_i;
class TableInfo_t;

/////////////////////////////////////////////////////////////////////////////

enum class DcrImpl {
    Legacy,
    Tesseract,

    Default = Tesseract
};

class TableInfo_t;

/////////////////////////////////////////////////////////////////////////////

class DCR
{
private:
    struct ImplInterface_t;
    template <typename T> class Impl_t;

    static std::unordered_map<DcrImpl, std::unique_ptr<ImplInterface_t>> impl_map_;

    int id_;
    DcrImpl method_;

public:

    DCR(int id, std::optional<DcrImpl> method);
    DCR(const DCR&) = delete;
    DCR(const DCR&&) = delete;
    virtual ~DCR();

    //
    // DCR virtual:
    //

    virtual bool Initialize()
    {
        return true;
    }

    virtual bool TranslateSurface(
        CSurface* /*pSurface*/,
        const Rect_t& /*Rect*/)
    {
        return true;
    }

    //

    const auto id() {
        return id_;
    }

    const ImplInterface_t& Impl() {
        return DCR::Impl(method_);
    }

    static void WriteBadBmp(
        const CSurface* pSurface,
        const RECT& rc,
        const wchar_t* pszText);

private:
    struct ImplInterface_t {
        virtual ~ImplInterface_t() noexcept {}

        virtual std::string GetText(
            const CSurface* pSurface,
            const Rect_t& rect) const = 0;

        virtual int GetTableText(
            const CSurface* pSurface,
            const Rect_t& rcTable,
            const TableInfo_t& tableInfo,
            const std::vector<Rect_t>& columnRects,
            const std::vector<std::unique_ptr<CSurface>>& columnSurfaces,
            TextTable_i* pText) const = 0;
    };

    template <typename T>
    class Impl_t : public ImplInterface_t {
    private:
        std::unique_ptr<T> impl_;

    public:
        Impl_t(std::unique_ptr<T> impl)
            : impl_(std::move(impl)) {}

        constexpr Impl_t(Impl_t&&) noexcept = default;
        constexpr Impl_t& operator=(Impl_t&&) noexcept = default;
        ~Impl_t() noexcept override = default;

        std::string GetText(
            const CSurface* pSurface,
            const Rect_t& rect) const override
        {
            return impl_->GetText(pSurface, rect);
        }

        int GetTableText(
            const CSurface* pSurface,
            const Rect_t& rcTable,
            const TableInfo_t& tableInfo,
            const std::vector<Rect_t>& columnRects,
            const std::vector<std::unique_ptr<CSurface>>& columnSurfaces,
            TextTable_i* pText) const override
        {
            return impl_->GetTableText(pSurface, rcTable, tableInfo, columnRects, columnSurfaces, pText);
        }
    };

public:
    template<typename T>
    static void AddImpl(
        DcrImpl method,
        std::unique_ptr<T> pImpl)
    {
        if (impl_map_.contains(method))
            throw std::invalid_argument(std::format("DcrImpl added twice, {}", int(method)));
        impl_map_.emplace(method, make_unique<Impl_t<T>>(std::move(pImpl)));
       
    }

    static const ImplInterface_t& Impl(
        DcrImpl method)
    {
        if (!impl_map_.contains(method))
            throw std::logic_error(std::format("DcrImpl doesn't exist, {}", int(method)));
        return *impl_map_.at(method).get();
    }

};

#endif // Include_DCR_H_
