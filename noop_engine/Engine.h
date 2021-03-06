#ifndef DSTORM_NOOP_ENGINE_ENGINE_H
#define DSTORM_NOOP_ENGINE_ENGINE_H

#include <memory>
#include <dStorm/output/LocalizedImage_decl.h>
#include <dStorm/input/Source.h>
#include <dStorm/engine/Image.h>

namespace dStorm {
namespace noop_engine {

   class Engine
   : public input::Source<output::LocalizedImage>
   {
      public:
        typedef input::Source< engine::ImageStack > Input;
        typedef input::Source<output::LocalizedImage>::TraitsPtr TraitsPtr;
      private:
        typedef input::Source<output::LocalizedImage> Base;

        std::auto_ptr<Input> input;

        class _iterator;
        void attach_ui_( simparm::NodeHandle n ) { input->attach_ui( n ); }

      public:
        Engine(std::auto_ptr<Input> input);
        void dispatch(Messages m);
        Base::iterator begin();
        Base::iterator end();
        TraitsPtr get_traits(Wishes);
        Capabilities capabilities() const { return input->capabilities(); }

        static TraitsPtr convert_traits( const Input::Traits& i );
   };
}
}

#endif
