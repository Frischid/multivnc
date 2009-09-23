

#include "VNCCanvas.h"


BEGIN_EVENT_TABLE(VNCCanvas, wxScrolledWindow)
    EVT_PAINT  (VNCCanvas::onPaint)
    EVT_MOUSE_EVENTS (VNCCanvas::onMouseAction)
    EVT_CHAR (VNCCanvas::onKeyAction)
END_EVENT_TABLE();

#define VNCCANVAS_SCROLL_RATE 10



/*
  constructor/destructor
*/

VNCCanvas::VNCCanvas(wxWindow* parent, VNCConn* c):
  wxScrolledWindow(parent)
{
  conn = c;
  framebuffer_rect.x = 0;
  framebuffer_rect.y = 0;
  framebuffer_rect.width = c->getFrameBufferWidth();
  framebuffer_rect.height = c->getFrameBufferHeight();

  SetVirtualSize(framebuffer_rect.width, framebuffer_rect.height);
  SetScrollRate(VNCCANVAS_SCROLL_RATE, VNCCANVAS_SCROLL_RATE);
}



/*
  private members
*/

void VNCCanvas::onPaint(wxPaintEvent &WXUNUSED(event))
{
  wxPaintDC dc(this);

  // find out where the window is scrolled to
  static int rx, ry;                    
  GetViewStart(&rx, &ry);
  rx *= VNCCANVAS_SCROLL_RATE;
  ry *= VNCCANVAS_SCROLL_RATE;

  
  // get the update rect list
  wxRegionIterator upd(GetUpdateRegion()); 
  while(upd)
    {
      wxRect viewport_rect(upd.GetRect());
     
      wxLogDebug(wxT("VNCCanvas %p: got repaint event: on screen (%i,%i,%i,%i)"),
		 this,
		 viewport_rect.x,
		 viewport_rect.y,
		 viewport_rect.width,
		 viewport_rect.height);

      wxRect fromframebuffer_rect = viewport_rect;
      fromframebuffer_rect.x += rx;
      fromframebuffer_rect.y += ry;
      fromframebuffer_rect.Intersect(framebuffer_rect);
      
      wxLogDebug(wxT("VNCCanvas %p: got repaint event: in framebuffer (%i,%i,%i,%i)"),
		 this,
		 fromframebuffer_rect.x,
		 fromframebuffer_rect.y,
		 fromframebuffer_rect.width,
		 fromframebuffer_rect.height);
      
      if(! fromframebuffer_rect.IsEmpty())
	{
      	  wxBitmap region = conn->getFrameBufferRegion(fromframebuffer_rect);
	  dc.DrawBitmap(region, viewport_rect.x, viewport_rect.y);
	}

      ++upd;
    }
}



void VNCCanvas::onMouseAction(wxMouseEvent &event)
{
  if(event.Entering())
    SetFocus();

#ifdef __WXDEBUG__
  wxClientDC dc(this);
  PrepareDC(dc);

  long x = dc.DeviceToLogicalX( event.GetX() );
  long y = dc.DeviceToLogicalY( event.GetY() );
  
  wxLogDebug(wxT("VNCCanvas %p: mouse event at (%d,%d), button %d"), this, (int)x, (int)y, event.GetButton());
#endif

  
  //wxPostEvent((wxEvtHandler*)conn, event);
}


void VNCCanvas::onKeyAction(wxKeyEvent &event)
{
#ifdef __WXDEBUG__
  wxChar c = event.GetUnicodeKey();

  wxLogDebug(wxT("VNCCanvas %p: got char: %c"), this, c);
#endif

  //wxPostEvent((wxEvtHandler*)conn, event);
}




/*
  public members
*/

void VNCCanvas::drawRegion(wxRect& rect)
{
  wxClientDC dc(this);
  DoPrepareDC(dc); // this adjusts coordinates when window is scrolled
  wxBitmap region = conn->getFrameBufferRegion(rect);
  dc.DrawBitmap(region, rect.x, rect.y);
}

