#ifndef DSTORM_CARCONFIG_H
#define DSTORM_CARCONFIG_H

#include <dStorm/Config.h>
#include <dStorm/output/Config.h>
#include <dStorm/input/Config.h>
#include <memory>
#include <list>
#include <simparm/Set.hh>
#include <simparm/Menu.hh>
#include <simparm/Callback.hh>
#include <simparm/FileEntry.hh>
#include <simparm/Entry.hh>
#include <boost/ptr_container/ptr_list.hpp>

namespace dStorm {
    namespace output { class OutputSource; }

    /** Configuration that summarises all
     *  configuration items offered by the dStorm library. */
    class GrandConfig : public simparm::Set, public Config
    {
      private:
        class TreeRoot;
        class InputListener;
        class EngineChoice;

        std::auto_ptr<input::Config> _inputConfig;
        std::auto_ptr<TreeRoot> outputRoot;
        std::auto_ptr<InputListener> input_listener;
        std::auto_ptr<EngineChoice> engine_choice;

        void registerNamedEntries();

      public:
        GrandConfig();
        GrandConfig(const GrandConfig &c);
        ~GrandConfig();
        GrandConfig *clone() const { return new GrandConfig(*this); }

        input::Config& inputConfig;
        output::OutputSource& outputSource;
        output::Config& outputConfig;

        simparm::Menu helpMenu;
        simparm::Set outputBox;
        simparm::FileEntry configTarget;
        simparm::BoolEntry auto_terminate;
        /** Number of parallel computation threads to run. */
        simparm::Entry<unsigned long> pistonCount;

        void add_input( std::auto_ptr<input::chain::Link>, InsertionPlace );
        void add_input( std::auto_ptr<input::chain::Filter>, InsertionPlace );
        void add_engine( std::auto_ptr<input::chain::Filter> );
        void add_spot_finder( std::auto_ptr<engine::spot_finder::Factory> );
        void add_spot_finder( engine::spot_finder::Factory* f ) 
            { add_spot_finder( std::auto_ptr<engine::spot_finder::Factory>(f) ); }
        void add_spot_fitter( std::auto_ptr<engine::spot_fitter::Factory> );
        void add_spot_fitter( engine::spot_fitter::Factory* f ) 
            { add_spot_fitter( std::auto_ptr<engine::spot_fitter::Factory>(f) ); }
        void add_output( std::auto_ptr<output::OutputSource> );

        const input::chain::MetaInfo& get_meta_info() const;
        std::auto_ptr<input::BaseSource> makeSource();
    };
}

#endif