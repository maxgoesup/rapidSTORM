#include "Manager.h"
#include "DataSource.h"
#include <dStorm/Image_impl.h>
#include <limits>

namespace dStorm {
namespace display {

void DataSource::notice_user_key_limits(int, bool, std::string) {}

static Manager* m;

void Manager::setSingleton(Manager& manager) {
    m = &manager;
}

Manager& Manager::getSingleton() {
    return *m;
}

std::auto_ptr<Manager::WindowHandle> Manager::register_data_source
    (const WindowProperties& properties,
        DataSource& handler)
{
    return register_data_source_impl( properties, handler );
}

void Manager::store_image( std::string filename, const Change& image )
{
    store_image_impl( StorableImage( filename, image ) );
}
void Manager::store_image( const StorableImage& i ) {
    store_image_impl(i);
}

std::vector<KeyChange> 
KeyChange::make_linear_key( std::pair<float,float> range )
{
    std::vector<KeyChange> rv;
    rv.reserve( 256 );
    for (int i = 0; i <= 255; i++)
        rv.push_back( KeyChange(
            i, Color(i),
            range.first + i * (range.second - range.first) / 255.0) );
    return rv;
}

void Change::make_linear_key(Image::PixelPair range) {
    if ( changed_keys.empty() ) changed_keys.push_back( std::vector<KeyChange>() );
    changed_keys[0] = KeyChange::make_linear_key( std::pair<float,float>( range.first, range.second ) );
}

void DataSource::look_up_key_values( const PixelInfo& info, std::vector<float>& targets )
{
    for (std::vector<float>::iterator 
         i = targets.begin(); i != targets.end(); ++i)
    {
        *i = std::numeric_limits<float>::quiet_NaN();
    }
}

SaveRequest::SaveRequest() 
: scale_bar( 1E-6 * boost::units::si::meter ) {}

StorableImage::StorableImage( const std::string& filename, const Change& image )
: image(image), filename(filename), scale_bar( 1E-6 * boost::units::si::meter )
{
}

}
}

