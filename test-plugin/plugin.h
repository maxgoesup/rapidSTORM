#ifndef TESTPLUGIN_PLUGIN_H
#define TESTPLUGIN_PLUGIN_H

#include <dStorm/Config.h>
#include <dStorm/display/Manager.h>

namespace dStorm {
namespace test {

void input_modules ( dStorm::Config* config );
void output_modules ( dStorm::Config* config );

}
}

#endif
