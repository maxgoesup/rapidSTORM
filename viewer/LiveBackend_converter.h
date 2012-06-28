#ifndef DSTORM_VIEWER_LIVEBACKEND_CONVERTER_H
#define DSTORM_VIEWER_LIVEBACKEND_CONVERTER_H

#include <dStorm/outputs/BinnedLocalizations.h>
#include "ImageDiscretizer_converter.h"
#include "Display.h"
#include "TerminalBackend.h"
#include "LiveBackend.h"
#include "Status_decl.h"
#include "Status.h"

namespace dStorm {
namespace viewer {

template <typename Hueing>
LiveBackend<Hueing>::LiveBackend(const TerminalBackend<Hueing>& other, Status& s)
: status(s), 
  image( NULL, other.image ),
  colorizer( other.colorizer ),
  discretization( other.discretization,
                  image(), colorizer ),
  cache( 4096, other.cache.getSize().size ),
  cia( discretization, s, *this, colorizer, other.get_result() )
{
    cia.set_job_name( other.get_job_name() );
    image.set_listener(&discretization);
    discretization.setListener(&cache);
    cache.setListener(&cia);
    cia.show_window();
}

template <typename Hueing>
std::auto_ptr<Backend>
TerminalBackend<Hueing>::change_liveness( Status& s ) {
    if ( s.config.showOutput() ) {
        return std::auto_ptr<Backend>( new LiveBackend<Hueing>(*this, s) );
    } else {
        return std::auto_ptr<Backend>();
    }
}

}
}

#endif
