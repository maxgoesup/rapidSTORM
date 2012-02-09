#include "debug.h"
#include "optics.h"
#include "optics_config.h"
#include <Eigen/Core>
#include <Eigen/LU>
#include <Eigen/Geometry>
#include <boost/units/Eigen/Array>
#include <boost/lexical_cast.hpp>
#include <boost/units/cmath.hpp>
#include <functional>
#include "ScaledProjection.h"

namespace dStorm {
namespace traits {

using namespace boost::units;

Optics<2>::Optics() {}

Optics<2>::Optics( const Optics &o ) 
: tmc(o.tmc),
  resolutions(o.resolutions),
  psf(o.psf),
  projection_(o.projection_),
  z_position(o.z_position),
  offsets(o.offsets),
  photon_response(o.photon_response),
  background_stddev(o.background_stddev),
  dark_current(o.dark_current)
{
    DEBUG("Copied " << this << " from " << &o);
}

Optics<2>& Optics<2>::operator=( const Optics<2> &o ) 
{
    DEBUG("Setting " << this << " to " << &o);
    tmc = o.tmc;
    psf = o.psf;
    projection_ = o.projection_;
    resolutions = o.resolutions;
    z_position = o.z_position;
    offsets[0] = o.offsets[0];
    offsets[1] = o.offsets[1];
    photon_response = o.photon_response;
    background_stddev = o.background_stddev;
    dark_current = o.dark_current;
    return *this;
}

Optics<2>::~Optics<2>() {}

void Optics<2>::set_resolution( const boost::array< ImageResolution, 2 >& f )
{
    resolutions[0] = f[0];
    resolutions[1] = f[1];
    set_resolution( resolutions );
}

void Optics<2>::set_resolution( const Resolutions& f )
{
    resolutions = f;
    if ( f[0].is_initialized() && f[0]->is_in_dpm() && f[1].is_initialized() && f[1]->is_in_dpm() )
    {
        Eigen::Affine2f trafo;
        trafo.setIdentity();
        projection_.reset( new ScaledProjection( f[0]->in_dpm(), f[1]->in_dpm() ) );
    } else {
        projection_.reset();
    }
}

CuboidConfig::CuboidConfig() 
: simparm::Object("Optics", "Optical pathway properties"),
  pixel_size("PixelSizeInNM", "Size of one input pixel",
                   PixelSize::Constant(105.0f * si::nanometre / camera::pixel))
{
    layers.push_back( new PlaneConfig(0) );
    set_number_of_planes( 1 );
    set_number_of_fluorophores( 1 );
}

void CuboidConfig::registerNamedEntries()
{
    layers[0].push_back( pixel_size );
    for ( Layers::iterator i = layers.begin(); i != layers.end(); ++i) {
        i->registerNamedEntries();
        push_back( *i );
    }
}

void CuboidConfig::set_number_of_planes(int number)
{
    while ( number > int( layers.size() ) ) {
        layers.push_back( new PlaneConfig( layers.size() ) );
        layers.back().registerNamedEntries();
        push_back( layers.back() );
    }
    while ( number < int( layers.size() ) )
        layers.pop_back();
}

void CuboidConfig::set_number_of_fluorophores(int number)
{
    for ( Layers::iterator i = layers.begin(); i != layers.end(); ++i)
        i->set_number_of_fluorophores( number, layers.size() > 1 );
}

int CuboidConfig::number_of_planes() const
    { return layers.size(); }

traits::Optics<2>::Resolutions
static make_resolution( const Eigen::Matrix< quantity< nanometer_pixel_size, float >, 2, 1, Eigen::DontAlign >& f ) {
    quantity< divide_typeof_helper<
        si::length,camera::length>::type, float > q1;
    traits::Optics<2>::Resolutions r;
    for (int i = 0; i < 2; ++i) {
        q1 = (f[i] / (1E9 * si::nanometre) * si::metre);
        r[i] = q1;
    }
    return r;
}


void CuboidConfig::set_traits(traits::Optics<3>& rv) const
{
    traits::Optics<2>::Resolutions defaults;
    defaults = make_resolution(pixel_size());

    for (int i = 0; i < int( layers.size() ) && i < rv.plane_count(); ++i) {
        layers[i].set_traits( rv.planes[i], defaults );
    }
}

traits::Optics<3> CuboidConfig::make_traits() const
{
    traits::Optics<3> rv;
    rv.planes.resize( layers.size() );
    set_traits(rv);
    return rv;
}

Position::ResolutionType CuboidConfig::make_localization_traits() const
{
    Position::ResolutionType rv;
    for (int i = 0; i < 2; ++i)
        rv[i] = 1.0f / (pixel_size()[i] / (1E9f * si::nanometre) * si::metre) ;
    return rv;
}

PlaneConfig::PlaneConfig(int number)
: simparm::Set("InputLayer" + boost::lexical_cast<std::string>(number), 
                  "Input layer " + boost::lexical_cast<std::string>(number+1)),
  is_first_layer(number==0),
  z_position("ZPosition", "Point of sharpest Z", ZPosition::Constant(0 * si::nanometre)),
  counts_per_photon( "CountsPerPhoton", "Camera response to photon" ),
  dark_current( "DarkCurrent", "Dark intensity" ),
  alignment( "Alignment", "Plane alignment" ),
  psf_size("PSF", "PSF FWHM", PSFSize::Constant(500.0 * boost::units::si::nanometre))
{
    alignment.addChoice( make_scaling_projection_config() );
    if ( ! is_first_layer ) {
        alignment.addChoice( make_affine_projection_config() );
        alignment.addChoice( make_support_point_projection_config() );
    }

    psf_size.helpID = "PSF.FWHM";
    z_position.setHelp("Z position where this layer is sharpest in this dimension");
    transmissions.push_back( new simparm::Entry<double>("Transmission0", "Transmission of fluorophore 0", 1) );

    counts_per_photon.userLevel = Object::Intermediate;
    dark_current.userLevel = Object::Intermediate;
}

PlaneConfig::PlaneConfig( const PlaneConfig& o )
: simparm::Set(o), is_first_layer(o.is_first_layer), 
  z_position(o.z_position), counts_per_photon(o.counts_per_photon), 
  dark_current(o.dark_current), alignment( o.alignment ),
  psf_size(o.psf_size)
{
    for (Transmissions::const_iterator i = o.transmissions.begin(), e = o.transmissions.end(); i != e; ++i)
    {
        transmissions.push_back( i->clone() );
    }
}

void PlaneConfig::registerNamedEntries()
{
    push_back( psf_size );
    push_back( z_position );
    push_back( counts_per_photon );
    push_back( dark_current );
    push_back( alignment );

    for (Transmissions::iterator i = transmissions.begin(), e = transmissions.end(); i != e; ++i)
        push_back( *i );
}

void PlaneConfig::set_number_of_fluorophores(int number, bool has_multiple_layers)
{
    while ( int(transmissions.size()) < number ) {
       std::string i = boost::lexical_cast<std::string>(transmissions.size());
       transmissions.push_back( new simparm::Entry<double>("Transmission" + i,
         	"Transmission of fluorophore " + i, 1) );
        push_back( transmissions.back() );
    }

    for (Transmissions::iterator i = transmissions.begin(); i != transmissions.end(); ++i)
	i->viewable = (i - transmissions.begin()) < number && (number > 1 || has_multiple_layers);
}

void PlaneConfig::set_traits( traits::Optics<2>& rv, const traits::Optics<2>::Resolutions& defaults ) const
{
    rv.resolutions = defaults;
    rv.projection_ = alignment().get_projection( rv.resolutions );
    rv.z_position = (z_position()[0] + z_position()[1]) / si::nanometre * (2.0 * 1E-9) * si::metre;
    rv.photon_response = counts_per_photon();
    rv.dark_current = dark_current();
    for (int i = 0; i < 2; ++i) {
        rv.offsets[i] = z_position()[i] / si::nanometre * 1E-9 * si::metre;
        rv.offsets[i] = *rv.offsets[i] - *rv.z_position;
    }
    rv.tmc.clear();
    for ( Transmissions::const_iterator i = transmissions.begin(); i != transmissions.end(); ++i) {
        DEBUG("Set transmission of " << rv.tmc.size() << " to " << i->value() << " at " << &rv);
        rv.tmc.push_back( i->value() );
    }
    rv.psf_size(0) = psf_size().cast< quantity<si::length,float> >();
    for (int i = 0; i < 2; ++i)
        (*rv.psf_size(0))[i] /= 2.35;
}

traits::Optics<2> PlaneConfig::make_traits( traits::Optics<2>::Resolutions defaults ) const
{
    traits::Optics<2> rv;
    set_traits(rv, defaults);
    return rv;
}

float traits::Optics<2>::transmission_coefficient( int fluorophore ) const
{
    DEBUG("Getting transmission of " << fluorophore << " of " << tmc.size() << " at " << this);
    if ( int(tmc.size()) > fluorophore )
        return tmc[fluorophore];
    else
        return 1.0f;
}

void traits::Optics<2>::set_fluorophore_transmission_coefficient( int fluorophore, float value ) 
{
    DEBUG("Did set transmission of " << fluorophore << " to " << value << " at " << this);
    while ( int(tmc.size()) <= fluorophore )
        tmc.push_back( 1.0f );
    tmc[fluorophore] = value;
}

void CuboidConfig::set_entries_to_traits( const traits::Optics<3>& t, int fc )
{
    set_number_of_planes( t.plane_count() );
    set_number_of_fluorophores(fc);
    for (int i = 0; i < t.plane_count(); ++i)
        layers[i].set_entries_to_traits( t.plane(i), fc );
}

void PlaneConfig::set_entries_to_traits( const traits::Optics<2>& t, int fc )
{
    for (int i = 0; i < fc && i < int(t.tmc.size()); ++i) {
        transmissions[i] = t.tmc[i];
    }
    if ( t.psf_size(0).is_initialized() ) {
        PSFSize s = (*t.psf_size(0) ).cast< PSFSize::Scalar >();
        for (int i = 0; i < 2; ++i) s[i] *= 2.35;
        psf_size = s;
    }

}

void CuboidConfig::set_3d_availability(bool available) {
    for (size_t i = 0; i < layers.size(); ++i)
        layers[i].set_3d_availability( available );
}

void PlaneConfig::set_3d_availability(bool available) {
    z_position.viewable = available;
}

ImageResolution Optics<2>::resolution(int r) const {
    assert( resolutions[r].is_initialized() );
    return *resolutions[r];
}

bool Optics<2>::has_resolution() const {
    return resolutions[0] && resolutions[1] && 
        dynamic_cast< const ScaledProjection* >( projection().get() );
}
const Optics<2>::Resolutions& 
Optics<2>::image_resolutions() const {
    return resolutions;
}

}
}
