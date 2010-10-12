#include "debug.h"

#include <simparm/Message.hh>
#include "CommandLine.h"
#include <vector>
#include <string>
#include <dStorm/Config.h>
#include "JobStarter.h"
#include <fstream>
#include <dStorm/output/OutputSource.h>
#include <dStorm/output/FilterSource.h>
#include "InputStream.h"
#include <dStorm/JobMaster.h>
#include "ModuleLoader.h"
#include <simparm/IO.hh>
#include <dStorm/helpers/DisplayManager.h>
#include <dStorm/input/FileMethod.h>

namespace dStorm {

using namespace output;

class TransmissionTreePrinter 
: public simparm::TriggerEntry,
  simparm::Listener,
  boost::noncopyable
{
    const dStorm::Config &config;
    void operator()( const simparm::Event& );
    void printNode( 
        const output::OutputSource& src,
        int indent );
  public:
    TransmissionTreePrinter(const dStorm::Config&);
};

class TwiddlerLauncher
: public simparm::TriggerEntry,
  simparm::Listener,
  boost::noncopyable
{
    dStorm::Config &config;
    std::list<Job*> &jobs;
    void operator()( const simparm::Event& );
  public:
    TwiddlerLauncher(dStorm::Config&, std::list<Job*>&);
};

class CommandLine::Pimpl
: public JobMaster,
  public simparm::IO
{
    int argc;
    char **argv;
    dStorm::Config config;
    JobStarter starter;
    ost::Mutex mutex;
    ost::Condition jobs_empty;
    std::list<Job*> jobs;

    bool load_config_file(const std::string& filename);
    void find_config_file();

  public:
    Pimpl(int argc, char *argv[]);
    ~Pimpl();

    void run();
    void register_node( dStorm::Job& j ) { 
        ost::MutexLock lock(mutex);
        this->push_back(j.get_config());
        DEBUG("Pushed back " << j.get_config().getName());
        jobs.push_back( &j ); 
    }
    void erase_node( dStorm::Job& j ) {
        ost::MutexLock lock(mutex);
        this->erase(j.get_config());  
        DEBUG("Erased " << j.get_config().getName());
        jobs.remove( &j ); 
        if ( jobs.empty() ) jobs_empty.broadcast();
    }
};

CommandLine::CommandLine(int argc, char *argv[])
: Thread("CommandLineInterpreter"),
  pimpl(new Pimpl(argc, argv))
{
}
CommandLine::~CommandLine()
{}

void CommandLine::run() {
    pimpl->run();
}

void CommandLine::abnormal_termination(std::string reason) {
    std::cerr << "Had an unexpected error while processing command line "
                 "arguments: " << reason << " Aborting." << std::endl;
    this->exit();
}
void CommandLine::Pimpl::run() {
    for ( int i = 0; i < argc; i++ ) {
        DEBUG("Argument " << i << " is '" << argv[i] << "'");
    }

    simparm::Set cmd_line_args
        ("dSTORM", "dSTORM command line");
    cmd_line_args.push_back( config );
    cmd_line_args.push_back(
        std::auto_ptr<simparm::Node>(new
            TransmissionTreePrinter(config)));
    cmd_line_args.push_back(
        std::auto_ptr<simparm::Node>(new
            TwiddlerLauncher(config, jobs)));
    cmd_line_args.push_back( starter );
    if ( Display::Manager::getSingleton().getConfig() )
        cmd_line_args.push_back( *Display::Manager::getSingleton().getConfig() );

    find_config_file();

    DEBUG("Reading command line arguments");
    int first_nonoption = 0;
    if (argc > 0) {
        first_nonoption = 
            cmd_line_args.readConfig(argc, argv);
    }

    DEBUG("Processing nonoption arguments from " <<first_nonoption << " to " <<  argc );
    for (int arg = first_nonoption; arg < argc; arg++) {
        config.inputConfig.file_method.input_file = std::string(argv[arg]);
        starter.trigger();
    }
    DEBUG("Finished processing commandline arguments");
}

void CommandLine::Pimpl::find_config_file() {
    DEBUG("Checking for relevant environment variables");
    const char *home = getenv("HOME"),
               *homedrive = getenv("HOMEDRIVE"),
               *homepath = getenv("HOMEPATH");
    bool have_file = false;
    DEBUG("Checking for command line config file");
    if ( !have_file && argc > 2 && std::string(argv[1]) == "--config" ) {
        have_file = load_config_file(std::string(argv[2]));
        if ( !have_file )
            DEBUG("Skipped unreadable config file '" << argv[2] << "'");
        argc -= 2;
        argv = argv + 2;
    }
    DEBUG("Checking for home directory config file");
    if ( !have_file && home != NULL )
        have_file = load_config_file( std::string(home) + "/.dstorm");
    if ( !have_file && homedrive != NULL && homepath != NULL )
        have_file = load_config_file( 
            std::string(homedrive) + std::string(homepath) + "/dstorm.txt");

}

bool CommandLine::Pimpl::load_config_file(
    const std::string& name
) {
    std::ifstream config_file( name.c_str() );
    if ( !config_file )
        return false;
    else {
        while ( config_file ) {
            DEBUG("Processing command from " << name.c_str());
            try {
                config.processCommand( config_file );
            } catch (const std::exception& e) {
                simparm::Message m("Error in initialization file",
                    "Unable to read initialization file: " + std::string(e.what()) );
                config.send(m);
            }
        }
        return true;
    }
}

CommandLine::Pimpl::Pimpl(int argc, char *argv[])
: IO(NULL, NULL), argc(argc), argv(argv), starter(this),
  jobs_empty(mutex)
{
    push_back(config);
    starter.setConfig(config);
    ModuleLoader::getSingleton().add_modules( config );
}
CommandLine::Pimpl::~Pimpl() {
    ost::MutexLock lock(mutex);
    while ( ! jobs.empty() )
        jobs_empty.wait();
}

TransmissionTreePrinter::TransmissionTreePrinter
    ( const dStorm::Config& c )
: simparm::TriggerEntry("ShowTransmissionTree", 
                        "Print tree view of outputs"),
  simparm::Listener( simparm::Event::ValueChanged ),
  config(c)
{
    receive_changes_from( value );
}

void TransmissionTreePrinter::operator()( const simparm::Event& )
{
    printNode( config.outputSource, 0 );
}

void TransmissionTreePrinter::printNode( 
    const output::OutputSource& src, int indent 
) {
    const std::string& name = src.getNode().getName();
    if ( indent < 2 )
        std::cout << ((indent)?" ": name ) << "\n";
    else {
        std::cout << std::string(indent-2, ' ') << "|-" << name << "\n";
    }
    const FilterSource* fwd = dynamic_cast<const FilterSource*>(&src);
    if (fwd != NULL) {
        for (FilterSource::const_iterator
                            i = fwd->begin(); i != fwd->end(); i++)
            printNode(**i, indent+2);
    }
}

TwiddlerLauncher::TwiddlerLauncher
    ( dStorm::Config& c, std::list<Job*>& j )
: simparm::TriggerEntry("TwiddlerControl", 
                "Read stdin/out for simparm control commands"),
  simparm::Listener( simparm::Event::ValueChanged ),
  config(c),
  jobs(j)
{
    receive_changes_from( value );
}

void TwiddlerLauncher::operator()( const simparm::Event& )
{
    DEBUG("Launching command stream");
    std::auto_ptr<InputStream> is(new InputStream(config, std::cin, std::cout));
    for ( std::list<Job*>::iterator i = jobs.begin(); i != jobs.end(); i++ ) {
        DEBUG("Registering additional job " << (*i)->get_config().getName());
        is->register_node( **i );
    }
    is.release()->detach();
    DEBUG("Launched command stream");
}

void CommandLine::register_node( Job& j ) { pimpl->register_node( j ); }
void CommandLine::erase_node( Job&  j ) { pimpl->erase_node( j ); }

}
