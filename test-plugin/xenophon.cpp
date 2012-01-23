#include "xenophon.h"
#include <dStorm/output/LocalizedImage.h>
#include <dStorm/Localization.h>

namespace dStorm {
namespace test {

using namespace boost::units;

input::Traits< output::LocalizedImage > xenophon_traits() {
    input::Traits< output::LocalizedImage > traits("Xenophon", "Xenophon");
    for (int i = 0; i < 3; ++i)
        traits.position().range()[i].first = 0 * si::meter;
    traits.amplitude().range().first = 0 * camera::ad_count;
    traits.image_number().range().first = 0 * camera::frame;

    traits.position().range()[0].second = float(10000E-9) * si::meter;
    traits.position().range()[1].second = float(5000E-9) * si::meter;
    traits.position().range()[2].second = float(2000E-9) * si::meter;
    traits.amplitude().range().second = float(RAND_MAX * M_PI * 1E-9) * camera::ad_count;
    traits.image_number().range().second = 99 * camera::frame;
    return traits;
}

std::vector< output::LocalizedImage > xenophon()
{
    float scale = 1E-5 / RAND_MAX;
    srand( 42 );
    std::vector< output::LocalizedImage > rv;
    for (int i = 0; i < 100; ++i) {
        output::LocalizedImage result( i * camera::frame );
        for (int j = 0; j < 100; ++j) {
            Localization l;
            l.frame_number() = result.frame_number();
            l.position().x() = (rand()*scale) * si::meter;
            l.position().y() = (rand()*scale) * 0.5f * si::meter;
            l.position().z() = (rand()*scale) * 0.2f * si::meter;
            l.amplitude() = rand() * M_PI * 1E-6 * camera::ad_count;
        }
        rv.push_back( result );
    }
    return rv;
}

}
}
