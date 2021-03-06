#include <simparm/BoostUnits.h>
#include <dStorm/engine/SpotFinder.h>
#include <dStorm/engine/SpotFinderBuilder.h>
#include <dStorm/engine/Image.h>
#include "averageSmooth.h"

#include <simparm/Object.h>
#include <simparm/Entry.h>
#include <simparm/GUILabelTable.h>

namespace dStorm {
namespace spalttiefpass_smoother {

struct Config {
    simparm::Entry< quantity<camera::length,int> > mask_size;
    void attach_ui( simparm::NodeHandle at ) { mask_size.attach_ui( at ); }
    Config() 
      : mask_size("SmoothingMaskSize", 5 * camera::pixel) 
    {
        mask_size.set_user_level( simparm::Intermediate );
    }
    static std::string get_name() { return "Average"; }
    static std::string get_description() { return simparm::GUILabelTable::get_singleton().get_description( get_name() ); }
};

class Spalttiefpass : public engine::spot_finder::Base {
    const Config config;
public:
    Spalttiefpass (const Config& config, const engine::spot_finder::Job& job )
        : Base(job), config(config) {}
    Spalttiefpass* clone() const { return new Spalttiefpass(*this); }

    void smooth( const engine::Image2D &in ) {
        spotFinders::smoothByAverage( in, smoothed, 
            config.mask_size() / camera::pixel / 2,
            config.mask_size() / camera::pixel / 2 );
    }
};

std::auto_ptr<engine::spot_finder::Factory> make_spot_finder_factory() { 
    return std::auto_ptr<engine::spot_finder::Factory>(
        new engine::spot_finder::Builder<Config,Spalttiefpass>()); 
}

}
}
