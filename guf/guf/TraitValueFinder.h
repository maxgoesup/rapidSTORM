#ifndef DSTORM_FITTER_GUF_TRAIT_VALUE_FINDER_H
#define DSTORM_FITTER_GUF_TRAIT_VALUE_FINDER_H

#include <dStorm/engine/JobInfo.h>
#include "FitAnalysis.h"
#include "Config.h"
#include <dStorm/engine/Input.h>
#include <dStorm/traits/optics.h>
#include <dStorm/engine/InputTraits.h>
#include <boost/variant/get.hpp>
#include "guf/psf/parameters.h"

namespace dStorm {
namespace guf {

struct TraitValueFinder {
    const dStorm::engine::JobInfo& info;
    const dStorm::traits::Optics& plane;
    const boost::optional<traits::Optics::PSF> psf;
    const quantity<si::length> zero_wavelength;
    const bool is_3d;

  public:
    typedef void result_type;
    TraitValueFinder( 
        const dStorm::engine::JobInfo& info, const dStorm::traits::Optics& plane );

    template <int Dim, typename Structure>
    void operator()( PSF::BestSigma<Dim> p, Structure& m ) const 
        { m(p) = (*psf)[Dim] / zero_wavelength; }
    template <int Dim, typename Structure>
    void operator()( PSF::DeltaSigma<Dim> p, Structure& m ) const {
        if ( is_3d )
            m(p) = boost::get<traits::Zhuang3D>(*info.traits.depth_info).widening[Dim] / zero_wavelength;
    }

    template <typename Structure>
    void operator()( PSF::ZPosition p, Structure& m ) const { 
        if ( is_3d )
            m( p ) = *plane.z_position; 
    }

    template <int Dim, typename Structure>
    void operator()( PSF::ZOffset<Dim> p, Structure& m ) const { 
        m(p) = ( plane.offsets[Dim].is_initialized() ) 
           ? *plane.offsets[Dim]
           : 0;
    }
    template <typename Structure>
    void operator()( PSF::Wavelength p, Structure& m ) const { 
        m(p) = info.traits.fluorophores.at(info.fluorophore).wavelength;
    }

    template <typename Structure>
    void operator()( PSF::Prefactor  p, Structure& m ) const {
        m(p) = plane.transmission_coefficient(info.fluorophore); 
    }
};

}
}

#endif