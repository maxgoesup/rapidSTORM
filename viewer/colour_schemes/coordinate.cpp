#include "coordinate.h"
#include <dStorm/output/binning/binning.h>
#include <dStorm/Engine.h>

namespace dStorm {
namespace viewer {
namespace colour_schemes {

Coordinate::Coordinate( bool invert, std::auto_ptr< output::binning::UserScaled > scaled, float range )
: BaseType(invert), mixer(0,0), variable( scaled ), repeater(NULL),
  is_for_image_number( variable->field_number() == dStorm::Localization::Fields::ImageNumber ),
  range(range)
{
    currently_mapping = variable->is_bounded();
    mixer.set_base_tone( 0, (currently_mapping) ? 1 : 0 );
    variable->set_clipping( false );
}

Coordinate::Coordinate( const Coordinate& o )
: BaseType(o), mixer(o.mixer), variable( o.variable->clone() ), repeater(o.repeater),
  is_for_image_number(o.is_for_image_number), currently_mapping(o.currently_mapping),
  range(o.range)
{
}

display::KeyDeclaration Coordinate::create_key_declaration( int index ) const {
    if ( index != 1 ) throw std::logic_error("Request to create unknown key");

    display::KeyDeclaration rv = variable->key_declaration();
    rv.size = key_resolution;
    if ( ! repeater ) {
        rv.can_set_lower_limit = rv.can_set_upper_limit = false;
    }
    return rv;
}

void Coordinate::create_full_key( display::Change::Keys::value_type& into, int index ) const
{
    if ( index != 1 ) {
        BaseType::create_full_key( into, index );
        return;
    }

    if (currently_mapping) {
        const float max_saturation = 1;
        const BrightnessType max_brightness 
            = std::numeric_limits<BrightnessType>::max();
        const int key_count = key_resolution;
        into.reserve( key_count );
        for (int i = 0; i < key_count; ++i) {
            float hue = (i * range / key_count);
            RGBWeight weights;
            rgb_weights_from_hue_saturation
                ( hue, max_saturation, weights );

            /* Key value in frames */
            float value = variable->reverse_mapping( (1.0f * i + 0.5f) / key_count );

            into.push_back( display::KeyChange(
                /* index */ i,
                /* color */ weights * max_brightness,
                /* value */ value ) );
        }
    }
} 


void Coordinate::announce(const output::Output::Announcement& a)
{
    repeater = a.engine;
    variable->announce(a);
    currently_mapping = variable->is_bounded();
    mixer.set_base_tone( 0, (currently_mapping) ? 1 : 0 );
}

void Coordinate::announce(const output::Output::EngineResult& er)
{
    if ( currently_mapping && is_for_image_number && ! er.empty() ) {
        set_tone( er.front() );
    }
}

void Coordinate::announce(const Localization& l)
{
    if ( currently_mapping && ! is_for_image_number ) {
        set_tone( l );
    }
}

void Coordinate::set_tone( const Localization& l ) {
    boost::optional<float> v = variable->bin_point(l);
    if ( v.is_initialized() )
        mixer.set_tone( *v * range );
    else
        mixer.set_tone( 0 );
}

void Coordinate::notice_user_key_limits(int index, bool lower_limit, std::string s)
{
    if ( index == 1 ) {
        assert( repeater );
        if ( ! repeater ) throw std::runtime_error("Missing old localization data for re-keying");
        variable->set_user_limit( lower_limit, s );
        currently_mapping = variable->is_bounded();
        mixer.set_base_tone( 0, (currently_mapping) ? 1 : 0 );
        repeater->repeat_results();
    } else
        BaseType::notice_user_key_limits( index, lower_limit, s );
}

}
}
}
