#ifndef LOCPREC_REGIONSEGMENTER_H
#define LOCPREC_REGIONSEGMENTER_H

#include <memory>
#include <dStorm/output/OutputSource.h>

namespace dStorm { 
namespace outputs {
    std::auto_ptr< dStorm::output::OutputSource > make_segmenter_source();
}
}

#endif
