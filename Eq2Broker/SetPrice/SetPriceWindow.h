////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008-2009 Mike McQueen.  All rights reserved.
//
// SetPriceWindow.h
//
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "UiWindow.h"
#include "BrokerUi.h"
#include "DDUtil.h"

namespace Broker::SetPrice {
  class Window_t : public Ui::Window_t {
  public:
    explicit Window_t(const Ui::Window_t& parent);

    // Ui::Window_t virtual:
/*    bool GetWidgetRect(
      Ui::WidgetId_t WidgetId,
      Rect_t* WidgetRect) const override;
*/
    // UI::Window_t virtual:
    const CSurface* GetOriginSurface() const override { return &origin_surface_; }
    void GetOriginSearchRect(const CSurface& surface, Rect_t& rect) const override;

    bool FindBorder(
      const CSurface& Surface,
      const POINT& ptTab,
      Rect_t& SurfaceRect) const;

  private:
    void LoadSurfaces();

    // TODO: make this a unique_ptr to ensure we instantiate/load
    CSurface origin_surface_;
  };

} // namespace Broker::SetPrice
