#include "App.h"
#include "Window.h"

#include "debug.h"
#include <dStorm/GUIThread.h>

namespace simparm {
namespace wx_ui {

BEGIN_EVENT_TABLE(App, wxAppConsole)
    EVT_IDLE(App::OnIdle)
END_EVENT_TABLE()

App::App()
{
}

App::~App() {
}


bool App::OnInit()
{
    if ( !wxApp::OnInit() )
        return false;

    dStorm::GUIThread::get_singleton().perform_wx_tasks();
    return true;
}

void App::OnIdle(wxIdleEvent&) {
    DEBUG("Idling");
    dStorm::GUIThread::get_singleton().perform_wx_tasks();
    DEBUG("Idled");
}

}
}

IMPLEMENT_APP_NO_MAIN( simparm::wx_ui::App );
