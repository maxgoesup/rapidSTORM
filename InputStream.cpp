#include "InputStream.h"
#include "ModuleLoader.h"
#include "doc/help/rapidstorm_help_file.h"
#include "JobStarter.h"
#include "engine/Car.h"

#include <simparm/IO.hh>

#include "debug.h"
#include <dStorm/error_handler.h>
#include <dStorm/helpers/BlockingThreadRegistry.h>

namespace dStorm {

struct InputStream::Pimpl
: public dStorm::Thread,
  public simparm::IO,
  JobMaster
{
    InputStream& impl_for;
    ost::Mutex mutex;
    ost::Condition all_cars_finished;
    typedef std::set<Job*> Jobs;
    Jobs running_cars;

    ErrorHandler::CleanupTag rehandle_stream;

    bool exhausted_input;

    std::auto_ptr<engine::CarConfig> original;
    std::auto_ptr<engine::CarConfig> config;
    std::auto_ptr<JobStarter> starter;
    simparm::Attribute<std::string> help_file;

    Pimpl(InputStream& papa, const engine::CarConfig*, 
          std::istream*, std::ostream*);
    ~Pimpl();

    void register_node( Job& );
    void erase_node( Job& );

    void run();
    void abnormal_termination(std::string abnormal_term);

    void terminate_remaining_cars();

    void processCommand(const std::string& cmd, std::istream& rest);

    void reset_config();
};

InputStream::InputStream(
    const engine::CarConfig& c,
    std::istream& i, std::ostream& o)
: dStorm::Thread("InputWatcher"),
  pimpl( new Pimpl(*this, &c, &i, &o) )
{
}

InputStream::InputStream(std::istream* i, std::ostream* o)
: dStorm::Thread("InputWatcher"),
  pimpl( new Pimpl(*this, NULL, i, o) )
{
}

InputStream::~InputStream() {}

InputStream::Pimpl::Pimpl(
    InputStream& impl_for,
    const engine::CarConfig* c,
    std::istream* i, std::ostream* o)
: dStorm::Thread("InputProcessor"),
  simparm::IO(i,o),
  impl_for( impl_for ),
  all_cars_finished( mutex ),
  exhausted_input( i == NULL ),
  original( (c) ? new engine::CarConfig(*c) : NULL ),
  starter( (original.get()) ? new JobStarter(this) : NULL ),
  help_file("help_file", dStorm::HelpFileName)
{
    ErrorHandler::CleanupArgs args;
    args.push_back("--Twiddler");
    rehandle_stream = ErrorHandler::make_tag(args);

    this->showTabbed = true;
    setDesc( ModuleLoader::getSingleton().makeProgramDescription() );
    this->push_back( help_file );
    reset_config();
}

void InputStream::Pimpl::reset_config() {
    ost::MutexLock lock(mutex);
    if ( original.get() ) {
        config.reset( new engine::CarConfig(*original) );
        this->push_back( *config );
        config->push_back( *starter );
        starter->setConfig( *config );
    }
}

void InputStream::Pimpl::terminate_remaining_cars() {
    ost::MutexLock lock(mutex);
    for ( Jobs::iterator i = running_cars.begin();
          i != running_cars.end(); i++)
        (*i)->stop();
}

InputStream::Pimpl::~Pimpl() 
{
    terminate_remaining_cars();
    while ( ! running_cars.empty() )
        all_cars_finished.wait();
}

void InputStream::run() {
    while ( !pimpl->exhausted_input ) {
        pimpl->start();
        pimpl->join();
    }
}

void InputStream::abnormal_termination(std::string reason) {
    std::cerr << "The command stream managment thread had an unexpected "
                 "error: " << reason << " Command processing must be "
                 "abandoned; your interface will stop to react. Sorry."
              << std::endl;
}

void InputStream::Pimpl::abnormal_termination(std::string reason) {
    std::cerr << "Could not perform action due to serious unhandled error: "
                 << reason << " I will recover this error and continue "
                 "running the program, but something is seriously broken "
                 "and it is quite likely that the program will behave "
                 "incorrectly now. Please report this bug." << std::endl;
}

void InputStream::Pimpl::run() {
    DEBUG("Current error handler is " << &ErrorHandler::get_current_handler());
    BlockingThreadRegistry::Handle handle(
        *ErrorHandler::get_current_handler().
            blocking_thread_registry,
        *this );
    DEBUG("Running input processing loop");
    while ( !exhausted_input ) {
        try {
            DEBUG("Processing input? " << ErrorHandler::global_termination_flag);
            if ( !ErrorHandler::global_termination_flag )
                processInput();
            DEBUG("Processed input");
            exhausted_input = true;
        } catch (const std::bad_alloc& e) {
            std::cerr << "Could not perform action: "
                      << "Out of memory" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Could not perform action: "
                      << e.what() << std::endl;
        }
    }
}

void InputStream::register_node( Job& node ) {pimpl->register_node(node);}
void InputStream::Pimpl::register_node( Job& node ) {
    ost::MutexLock lock(mutex);
    simparm::Node::push_back( node.get_config() );
    running_cars.insert( &node );
}

void InputStream::erase_node( Job& node ) {pimpl->erase_node(node);}
void InputStream::Pimpl::erase_node( Job& node ) {
    ost::MutexLock lock(mutex);
    simparm::Node::erase( node.get_config() );
    running_cars.erase( &node );
    if ( running_cars.empty() )
        all_cars_finished.signal();
}

void InputStream::Pimpl::processCommand
    (const std::string& cmd, std::istream& rest)
{
    DEBUG("Processing command " << cmd);
    if ( cmd == "wait_for_jobs" ) {
        terminate_remaining_cars();
        ost::MutexLock lock(mutex);
        while ( ! running_cars.empty() )
            all_cars_finished.wait();
    } else if ( cmd == "reset" ) {
        reset_config();
    } else
        simparm::IO::processCommand(cmd, rest);
}

}