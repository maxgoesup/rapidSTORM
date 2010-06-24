#include "main.h"
#include <dStorm/engine/Image.h>
#include <dStorm/engine/Config.h>
#include <dStorm/engine/JobInfo.h>
#include <dStorm/engine/Spot.h>
#include <dStorm/Localization.h>
#include "Config.h"

template <typename Ty>
Ty sq(const Ty& a) { return a*a; }

namespace dStorm {
namespace gauss_2d_fitter {
namespace no_analysis {

using namespace fitpp::Exponential2D;
using dStorm::engine::Spot;
using dStorm::engine::BaseImage;

template <int FF>
CommonInfo<FF>::CommonInfo( 
   const Config& c, const engine::JobInfo& info
) 
: amplitude_threshold( info.config.amplitude_threshold() / cs_units::camera::ad_counts ),
  start_sx( info.config.sigma_x() / cs_units::camera::pixel ),
  start_sy( info.config.sigma_y() / cs_units::camera::pixel ),
  start_sxy( info.config.sigma_xy() ),
  params( NULL, &constants)
{
    fit_function.
        setStartLambda( c.marquardtStartLambda() );
    fit_function.
        setMaximumIterationSteps( c.maximumIterationSteps() );
    fit_function.
        setSuccessiveNegligibleStepLimit( 
            c.successiveNegligibleSteps() );

    FitGroup::template set_absolute_epsilon<MeanX,0>
        (fit_function, c.negligibleStepLength());
    FitGroup::template set_absolute_epsilon<MeanY,0>
        (fit_function, c.negligibleStepLength());

    if ( ! ( FF & ( 1 << fitpp::Exponential2D::SigmaX ) ) )
        params.template setSigmaX<0>( start_sx );
    if ( ! ( FF & ( 1 << fitpp::Exponential2D::SigmaY ) ) )
        params.template setSigmaY<0>( start_sy );
    if ( ! ( FF & ( 1 << fitpp::Exponential2D::SigmaXY ) ) )
        params.template setSigmaXY<0>( start_sxy );
}

template <int FF>
void
CommonInfo<FF>::set_start(
    const Spot& spot, 
    const BaseImage& image,
    double shift_estimate,
    typename FitGroup::Variables* variables 
) 
{
    params.change_variable_set( variables );
    params.template setMeanX<0>( spot.x() );
    params.template setMeanY<0>( spot.y() );
    if ( FF & ( 1 << fitpp::Exponential2D::SigmaX ) )
        params.template setSigmaX<0>(start_sx);
    if ( FF & ( 1 << fitpp::Exponential2D::SigmaY ) )
        params.template setSigmaY<0>(start_sy);
    if ( FF & ( 1 << fitpp::Exponential2D::SigmaXY ) )
        params.template setSigmaXY<0>(start_sxy);

    int xc = round(spot.x()), yc = round(spot.y());
    double center = image(xc,yc);
    
    params.setShift( shift_estimate );
    params.template setAmplitude<0>( 
        max<double>(center - shift_estimate, 10)
        * 2 * M_PI 
        * params.template getSigmaX<0>() * params.template getSigmaY<0>());

    maxs.x() = image.width_in_pixels()-1 - 1;
    maxs.y() = image.height_in_pixels()-1 - 1;
    start.x() = spot.x();
    start.y() = spot.y();
}

template <int FF>
bool 
CommonInfo<FF>::check_result(
    typename FitGroup::Variables* variables,
    Localization* target
)
{
    params.change_variable_set( variables );

    Localization::Position p;
    p.x() = params.template getMeanX<0>() * cs_units::camera::pixel;
    p.y() = params.template getMeanY<0>() * cs_units::camera::pixel;

    new(target) Localization( 
        p, float( params.template getAmplitude<0>() )
                * cs_units::camera::ad_counts );

    bool sx_correct = ( ! (FF & ( 1 << fitpp::Exponential2D::SigmaX )))
        || ( params.template getSigmaX<0>() >= start_sx/4
          && params.template getSigmaX<0>() <= start_sx*4 );
    bool sy_correct = ( ! (FF & ( 1 << fitpp::Exponential2D::SigmaY )))
        || ( params.template getSigmaX<0>() >= start_sy/4
          && params.template getSigmaX<0>() <= start_sy*4 );
    bool sigmas_correct = sx_correct && sy_correct;

    bool good = sigmas_correct
        && target->strength() > amplitude_threshold 
                                * cs_units::camera::ad_counts
        && target->x() >= 1*cs_units::camera::pixel
        && target->y() >= 1*cs_units::camera::pixel
        && target->x() < maxs.x() * cs_units::camera::pixel
        && target->y() < maxs.y() * cs_units::camera::pixel
        && sq(target->x().value() - start.x()) + 
           sq(target->y().value() - start.y()) < 4;
    if ( (FF != fitpp::Exponential2D::FixedForm) ) {
        double sx = params.template getSigmaX<0>(),
               sy = params.template getSigmaY<0>(),
               corr = params.template getSigmaXY<0>();
        target->fit_covariance_matrix()(0,0) 
            = sx*sx * cs_units::camera::pixel *
                      cs_units::camera::pixel;
        target->fit_covariance_matrix()(0,1)
            = target->fit_covariance_matrix()(1,0)
            = corr*sx*sy * cs_units::camera::pixel
                         * cs_units::camera::pixel;
        target->fit_covariance_matrix()(1,1) = sy*sy
            * cs_units::camera::pixel 
            * cs_units::camera::pixel;
    }

    target->unset_source_trace();
    return good;
}


}
}
}
