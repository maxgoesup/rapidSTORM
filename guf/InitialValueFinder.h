#ifndef DSTORM_FITTER_GUF_INITIAL_VALUE_FINDER_H
#define DSTORM_FITTER_GUF_INITIAL_VALUE_FINDER_H

#include "Spot.h"
#include "Config.h"
#include <dStorm/engine/Input_decl.h>
#include <dStorm/engine/JobInfo_decl.h>
#include "fit_window/Stack.h"
#include <vector>
#include <dStorm/Direction.h>
#include <boost/smart_ptr/scoped_ptr.hpp>

namespace dStorm {
namespace threed_info { struct SigmaDiffLookup; }
namespace guf {

class MultiKernelModel;
class MultiKernelModelStack;

struct InitialValueFinder {
    typedef void result_type;

    const dStorm::engine::JobInfo& info;
    const bool disjoint_amplitudes, need_z_estimate;
    struct SigmaDiff {
        int minuend_plane, subtrahend_plane;
        Direction minuend_dir, subtrahend_dir;

        threed_info::SigmaDiffLookup lookup( const engine::InputTraits& info ) const;
    };
    boost::optional< SigmaDiff > most_discriminating_diff;
    boost::scoped_ptr< threed_info::SigmaDiffLookup > lookup_table;
    float correlation( const SigmaDiff& ) const;

    struct PlaneEstimate;
    std::vector<PlaneEstimate> estimate_bg_and_amp( const Spot& spot, const fit_window::Stack & ) const;
    void join_amp_estimates( std::vector<PlaneEstimate>& v ) const;
    void estimate_z( const fit_window::Stack&, std::vector<PlaneEstimate>& ) const;
    static bool determine_z_estimate_need( const engine::InputTraits& t );
    void create_z_lookup_table( const engine::InputTraits& t );

    class set_parameter;
    template <typename Parameter>
    inline void operator()( Parameter, MultiKernelModel&, const Spot&, const PlaneEstimate& ) const;

  public:
    InitialValueFinder( const Config& config, const dStorm::engine::JobInfo& info);
    ~InitialValueFinder();

    void operator()( MultiKernelModelStack& position, const Spot&, const fit_window::Stack& ) const;
};

}
}

#endif
