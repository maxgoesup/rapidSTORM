#ifndef DSTORM_DISPLAY_APPWINDOW_H
#define DSTORM_DISPLAY_APPWINDOW_H

#include <wx/wx.h>
#include "dStorm/helpers/DisplayDataSource.h"
#include "wxManager.h"
#include "Canvas.h"

namespace dStorm {
namespace Display {

class ZoomSlider;
class Key;
class ScaleBar;

class Window : public wxFrame, public Canvas::Listener
{
  private:
    Canvas* canvas;
    Key* key;
    ZoomSlider *zoom;
    ScaleBar *scale_bar;
    wxStaticText *position_label;
    wxTimer timer;

    DataSource* source;
    wxManager::WindowHandle *handle;

    Color background;

    bool close_on_completion, notify_for_zoom;

    DECLARE_EVENT_TABLE();

    void OnTimer(wxTimerEvent& event);

    template <typename Drawer>
    void draw_image_window( const Change& changes );
    void commit_changes(const Change& changes);

    void drawn_rectangle( wxRect rect );
    void zoom_changed( int to );
    void mouse_over_pixel( wxPoint );

  public:
    Window( const Manager::WindowProperties& props,
            DataSource* data_source,
            wxManager::WindowHandle *my_handle );
    ~Window(); 

    void remove_data_source();

    std::auto_ptr<Change> getState();
};

}
}

#endif