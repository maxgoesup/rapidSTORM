#ifndef DSTORM_GUF_DATAPLANE_IMPL_H
#define DSTORM_GUF_DATAPLANE_IMPL_H

#include "DataPlane.h"
#include "InputPlane.h"

#include "TransformedImage.hpp"
#include <nonlinfit/index_of.h>
#include <boost/mpl/for_each.hpp>
#include <boost/bind/bind.hpp>
#include "EvaluationTags.h"
#include "mle_converter.h"
#include <boost/type_traits/is_same.hpp>
#include "Statistics.h"
#include <dStorm/engine/InputTraits.h>

#include "dejagnu.h"
#include <dStorm/traits/ScaledProjection.h>
#include <boost/units/cmath.hpp>

using namespace nonlinfit::plane;

namespace dStorm {
namespace guf {

#if 0
template <typename Data>
const Statistics<2> InputPlane::set_data( Data& d, const Image& i, const Spot& s ) const
{
    mle_converter t(dark_current, photon_response_);
    return transformation.set_data( d, i, s, t );
}
#endif

std::auto_ptr<DataPlane> InputPlane::set_image( const Image& image, const Spot& position ) const {
    int index = index_finder.get_evaluation_tag_index( position );
    return extractor_table.get( index ).extract_data( image, position );
}

InputPlane::InputPlane( const Config& c, const engine::InputPlane& plane )
: optics( Spot::Constant( Spot::Scalar( c.fit_window_size() ) ), plane ),
  index_finder( c.allow_disjoint(), ! c.double_computation(), optics ),
  extractor_table( optics )
{
}

void test_DataPlane_scaled( TestState& state )
{
    Config config;
    config.fit_window_size = 1600 * si::nanometre;
    config.allow_disjoint = true;

    engine::InputPlane traits;
    traits.image.size.fill( 100 * camera::pixel );
    traits.image.set_resolution( 0, 230 * si::nanometre / camera::pixel );
    traits.image.set_resolution( 1, 120 * si::nanometre / camera::pixel );
    traits.optics.set_projection_factory( traits::test_scaled_projection() );
    traits.create_projection();

    dStorm::engine::Image2D image( traits.image.size );
    for (int x = 0; x < traits.image.size.x().value(); ++x)
        for (int y = 0; y < traits.image.size.y().value(); ++y)
            image(x,y) = x + y;

    Spot position;
    position.x() = 5600.0E-9 * si::metre;
    position.y() = 3000.0E-9 * si::metre;

    InputPlane scaled( config, traits );
    std::auto_ptr<DataPlane> data1 = scaled.set_image( image , position );
    state( data1->tag_index() == nonlinfit::index_of< 
        evaluation_tags, 
        nonlinfit::plane::xs_disjoint<double,PSF::LengthUnit,14>::type >::value,
        "DataPlane selects disjoint fitting when applicable" );
    state( abs( data1->pixel_size() - 230E-9 * si::metre * 120E-9 * si::metre ) < pow<2>(1E-9 * si::metre),
        "Disjoint fitting has correct pixel size" ); 
    Statistics<2> stats = data1->get_statistics();
    state( abs( stats.highest_pixel.x() - 7130E-9 * si::metre ) < 1E-9 * si::metre,
        "Highest pixel is located correctly (X)" );
    state( abs( stats.highest_pixel.y() - 4560E-9 * si::metre ) < 1E-9 * si::metre,
        "Highest pixel is located correctly (Y)" );
    state( abs( stats.integral - 18711 * camera::ad_count ) < 1 * camera::ad_count,
        "Integral is correctly computed" );
    state( abs( stats.peak_intensity - 69 * camera::ad_count ) < 1 * camera::ad_count,
        "Peak intensity is correctly computed" );
    state( abs( stats.quarter_percentile_pixel - 43 * camera::ad_count ) < 1 * camera::ad_count,
        "Quarter percentile pixel is correctly computed" );
    state( abs( stats.sigma[0] - 870E-9 * si::meter ) < 1E-9 * si::meter,
        "Sigma X is correctly computed" );
    state( abs( stats.sigma[1] - 636E-9 * si::meter ) < 1E-9 * si::meter,
        "Sigma Y is correctly computed" );
    state( stats.pixel_count == 378, "Pixel count is correct" );
}

void test_DataPlane( TestState& state ) {
    test_DataPlane_scaled( state );
}

}
}

#endif
