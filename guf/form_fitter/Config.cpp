#include "Config.h"

namespace dStorm {
namespace form_fitter {


Config::Config()
:   simparm::Object("FitPSFForm", "Estimate PSF form"),
    auto_disable("AutoDisable", "Raise no error for missing source images", false), 
    mle("FormMLE", "Use MLE to fit PSF form", false), 
    number_of_spots("EstimationSpots", "Number of spots used in estimation", 40),
    visual_selection("SelectSpots", "Manually select good spots", true),
    circular_psf("CircularPSF", "Assume circular PSF", true),
    laempi_fit("LaempiPosition", "Laempi fit for positions", false),
    disjoint_amplitudes("LaempiAmplitudes", "Disjoint amplitude fit", false),
    linear_term("LinearTerm", "Fit linear term of polynomial 3D", false),
    quadratic_term("QuadraticTerm", "Fit quadratic term of polynomial 3D", true),
    cubic_term("CubicTerm", "Fit cubic term of polynomial 3D", false),
    quartic_term("QuarticTerm", "Fit quartic term of polynomial 3D", false)
{
    auto_disable.userLevel = simparm::Object::Intermediate;
}

void Config::registerNamedEntries() {
    push_back( auto_disable );
    push_back( mle );
    push_back( number_of_spots );
    push_back( visual_selection );
    push_back( circular_psf );
    push_back( laempi_fit );
    push_back( disjoint_amplitudes );
    push_back( linear_term );
    push_back( quadratic_term );
    push_back( cubic_term );
    push_back( quartic_term );
}

}
}
