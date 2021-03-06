#ifndef DSTORM_CARCONFIG_H
#define DSTORM_CARCONFIG_H

#include <dStorm/Config.h>
#include <dStorm/output/Config.h>
#include <dStorm/input/Link.h>
#include <memory>
#include <list>
#include <simparm/Group.h>
#include <simparm/Menu.h>
#include <simparm/FileEntry.h>
#include <simparm/Entry.h>
#include <simparm/Message.h>
#include <dStorm/output/BasenameAdjustedFileEntry.h>
#include <boost/function/function1.hpp>

#include "OutputTreeRoot.h"

namespace dStorm {
namespace output { class OutputSource; }
namespace job {

/** Configuration that summarises all
    *  configuration items offered by the dStorm library. */
class Config : public dStorm::Config
{
  private:
    friend class EngineChoice;

    simparm::Group car_config;
    simparm::NodeHandle current_ui;

    std::auto_ptr<OutputTreeRoot> outputRoot;
    std::auto_ptr<input::Link> input;
    input::Link::Connection input_listener;
    bool localization_replay_mode;
    simparm::Group outputBox;

    void traits_changed( boost::shared_ptr<const input::MetaInfo> );
    void create_input( std::auto_ptr<input::Link> );
    void attach_children_ui( simparm::NodeHandle at );

public:
    Config( bool localization_replay_mode );
    Config(const Config &c);
    ~Config();
    Config *clone() const { return new Config(*this); }

    simparm::NodeHandle attach_ui( simparm::NodeHandle at );
    void send( simparm::Message& m ) { m.send( current_ui ); }

    output::BasenameAdjustedFileEntry configTarget;
    simparm::BoolEntry auto_terminate;
    /** Number of parallel computation threads to run. */
    simparm::Entry<unsigned long> pistonCount;

    const output::OutputSource& output_tree() const { return *outputRoot; }

    void add_input( std::auto_ptr<input::Link>, InsertionPlace );
    void add_spot_finder( std::auto_ptr<engine::spot_finder::Factory> );
    void add_spot_fitter( std::auto_ptr<engine::spot_fitter::Factory> );
    void add_output( std::auto_ptr<output::OutputSource> );

    const input::MetaInfo& get_meta_info() const;
    std::auto_ptr<input::BaseSource> makeSource() const;

    std::auto_ptr< Job > make_job();
};

}
}

#endif
