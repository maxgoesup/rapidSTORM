#ifndef DSTORM_NULL_OUTPUT_H
#define DSTORM_NULL_OUTPUT_H

#include "../output/Output.h"

namespace dStorm {
namespace outputs {

struct NullOutput : public output::OutputObject 
{
    NullOutput();
    NullOutput* clone() const { return new NullOutput(); }
    AdditionalData announceStormSize(const Announcement&) 
        { return AdditionalData(); }
    void store_results() {}
    void receiveLocalizations(const EngineResult&) {}
};

}
}

#endif
