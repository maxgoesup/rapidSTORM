#define CImgBuffer_ANDORDIRECT_CPP

#include "debug.h"

#include "CameraConnection.h"
#include "AndorDirect.h"
#include <string.h>
#include <sstream>
#include <iomanip>
#include <dStorm/output/Basename.h>
#include <dStorm/input/Source.h>
#include <boost/utility.hpp>
#include <dStorm/engine/InputTraits.h>

#include "LiveView.h"

using namespace std;
using namespace dStorm::input;
using namespace dStorm;
using namespace simparm;

namespace dStorm {
namespace AndorCamera {

Source::Source
    (std::auto_ptr<CameraConnection> connection, bool show_live, LiveView::Resolution res )
: connection(connection),
  has_ended(false), show_live(show_live), resolution(res),
  status("CameraStatus", "Camera status", "")
{
    status.freeze();

    DEBUG("Built AndorDirect source " << this);
}

Source::~Source() {
    DEBUG( "Destructing source " << this );
}

void Source::attach_ui_( simparm::NodeHandle n ) {
    status.attach_ui( n );
}

Source::TraitsPtr Source::get_traits( Wishes ) 
{
    CamTraits cam_traits;
    DEBUG("Starting acquisition");
    connection->start_acquisition( cam_traits, status );
    TraitsPtr rv( new TraitsPtr::element_type(cam_traits) );
    DEBUG("Showing live view");
    live_view.reset( new LiveView(show_live, resolution ) );
    DEBUG("Returning traits");
    traits = rv;
    assert( rv.get() ); /* Make sure noone changed type to auto_ptr */
    assert( cam_traits.plane_count() > 0 );
    assert( rv->plane_count() > 0 );
    return rv;
}

}
}
