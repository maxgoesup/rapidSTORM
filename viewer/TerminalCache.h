#ifndef DSTORM_VIEWER_TERMINALCACHE_H
#define DSTORM_VIEWER_TERMINALCACHE_H

#include <dStorm/display/DataSource.h>
#include "ImageDiscretizer.h"

namespace dStorm {
namespace viewer {

class TerminalCache 
: public DummyDiscretizationListener
{
    display::ResizeChange size;

  public:
    TerminalCache();

    const display::ResizeChange& getSize() const 
        { return size; }
    void setSize(const input::Traits< Image<int,Im::Dim> >&);
    void setSize(const dStorm::display::ResizeChange& size);
};

}
}
#endif
