#include <stdexcept>

#include <simparm/Eigen_decl.h>
#include <simparm/BoostUnits.h>
#include <simparm/Eigen.h>
#include <simparm/Object.h>
#include <simparm/Entry.h>

#include <boost/units/io.hpp>

#include <dStorm/LengthUnit.h>
#include <dStorm/units/nanolength.h>
#include "No3D.h"
#include "Config.h"

namespace dStorm {
namespace threed_info {

class No3DConfig : public Config {
    typedef  Eigen::Matrix< boost::units::quantity< boost::units::si::nanolength, double >, 2, 1, Eigen::DontAlign > PSFSize;
    simparm::Entry<PSFSize> psf_size;

    boost::shared_ptr<DepthInfo> make_traits( Direction dir ) const { 
        boost::shared_ptr<No3D> rv( new No3D() );
        rv->sigma = ToLengthUnit(psf_size()[dir] / 2.35);
        return rv;
    }
    void read_traits( const DepthInfo& dx, const DepthInfo& dy ) {
        PSFSize s;
        s[Direction_X] = PSFSize::Scalar( FromLengthUnit(dynamic_cast<const No3D&>(dx).sigma * 2.35f) );
        s[Direction_Y] = PSFSize::Scalar( FromLengthUnit(dynamic_cast<const No3D&>(dy).sigma * 2.35f) );
        psf_size = s;
    }
    void set_context() {}
    void attach_ui( simparm::NodeHandle to ) { 
        simparm::NodeHandle r = attach_parent(to);
        psf_size.attach_ui( r );
    }
  public:
    No3DConfig() 
        : Config("No3D"),
          psf_size("PSF", PSFSize::Constant(500.0 * boost::units::si::nanometre)) 
    { 
    }
    No3DConfig* clone() const { return new No3DConfig(*this); }
};

SigmaDerivative No3D::get_sigma_deriv_( ZPosition ) const
{ 
    throw std::logic_error("Attempted to get dSigma/dZ for no-3D model"); 
}

std::ostream& No3D::print_( std::ostream& o ) const
{
    return o << "no 3D information with PSF width " << sigma * 2.35f;
}

std::auto_ptr< Config > make_no_3d_config() 
    { return std::auto_ptr< Config >( new No3DConfig() ); }

}
}
