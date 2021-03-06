#define DSTORM_ENGINE_CPP

#include "debug.h"

#include "Engine.h"

#include <cassert>

#include <dStorm/input/Source.h>
#include <dStorm/output/Traits.h>
#include <dStorm/output/LocalizedImage_traits.h>
#include <dStorm/output/LocalizedImage.h>

namespace dStorm {
namespace noop_engine {

Engine::Engine( std::auto_ptr<Input> input )
: input(input)
{
}

Engine::TraitsPtr Engine::get_traits(Wishes w) {
    Engine::TraitsPtr rv = convert_traits( *input->get_traits(w) );
    rv->carburettor = input.get();
    return rv;
}

Engine::TraitsPtr Engine::convert_traits( const Input::Traits& p ) {
    Base::TraitsPtr prv( new input::Traits<output::LocalizedImage>("Noop", "Dummy engine data") );
    prv->in_sequence = true;
    prv->source_image_is_set = true;
    prv->image_number() = p.image_number();
    prv->input_image_traits.reset( p.clone() );

    return prv;
}

void Engine::dispatch(Messages m) {
    input->dispatch(m);
}

class Engine::_iterator
: public boost::iterator_facade< 
    _iterator, 
    output::LocalizedImage,
    std::input_iterator_tag>
{
    Input::iterator base;
    mutable output::LocalizedImage resultStructure;
    bool created;
    void create() {
        resultStructure.forImage = base->frame_number();
        resultStructure.clear();
        resultStructure.source = *base; 
        resultStructure.candidates = NULL;
    }

    output::LocalizedImage& dereference() const {
        if ( !created ) const_cast<_iterator&>(*this).create();
        return resultStructure; 
    }
    bool equal(const _iterator& o) const { return base == o.base; }
    void increment() { ++base; created = false; }
    friend class boost::iterator_core_access;

  public:
    _iterator( Input::iterator base ) : base(base) { created = false; }
};

Engine::Base::iterator Engine::begin() {
    return Base::iterator( _iterator( input->begin() ) );
}

Engine::Base::iterator Engine::end() {
    return Base::iterator( _iterator( input->end() ) );
}

}
}
