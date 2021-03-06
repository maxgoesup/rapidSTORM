#include "debug.h"

#include "ChainLink.h"
#include "Engine.h"
#include <dStorm/input/MetaInfo.h>
#include <dStorm/input/Method.hpp>
#include <dStorm/output/LocalizedImage_traits.h>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/for_each.hpp>

#include <dStorm/input/Source.h>
#include <dStorm/input/MetaInfo.h>
#include <dStorm/output/LocalizedImage.h>

namespace dStorm {
namespace noop_engine {

using namespace input;

class ChainLink
: public input::Method< ChainLink >
{
    simparm::Object config;

    friend class input::Method< ChainLink >;
    typedef boost::mpl::vector<engine::ImageStack> SupportedTypes;

    boost::shared_ptr< Traits<output::LocalizedImage> >
    create_traits( MetaInfo& my_info,
                   const Traits<engine::ImageStack>& orig_traits ) 
    {
        return Engine::convert_traits(orig_traits);
    }

    BaseSource* make_source( std::auto_ptr< Source<engine::ImageStack> > p )
        { return new Engine(p); }

  public:
    ChainLink()
        : config(getName(), "Do not localize") {}

    static std::string getName() { return "Noop"; }
    void attach_ui( simparm::NodeHandle at ) { config.attach_ui( at ); }
};

std::auto_ptr<input::Link>
makeLink()
{
    return std::auto_ptr<input::Link>( new ChainLink( ) );
}

}
}
