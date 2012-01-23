#ifndef DSTORM_VIEWER_FWD_H
#define DSTORM_VIEWER_FWD_H

#include <memory>

class TestState;

namespace dStorm {
namespace output { class OutputSource; }
namespace viewer {

std::auto_ptr<output::OutputSource> make_output_source();
void unit_test( TestState& );

}
}

#endif
