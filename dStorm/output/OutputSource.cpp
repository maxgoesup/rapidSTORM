#include "Output.h"
#include "OutputSource.h"
#include "BasenameAdjustedFileEntry.h"
#include <cassert>
#include <sstream>
#include <list>
#include <simparm/FileEntry.hh>

namespace dStorm {
namespace output {

class OutputSource::AdjustedList 
: public std::list< BasenameAdjustedFileEntry* > {};

OutputSource::OutputSource(simparm::Node& node) 
: node(node),
  adjustedList( new AdjustedList() )
{
    assert( &node != NULL );
    simparm::TreeAttributes::registerNamedEntries( node );
}

OutputSource::OutputSource(simparm::Node& node, const OutputSource& o) 
: simparm::TreeAttributes(o),
  node(node),
  adjustedList( new AdjustedList() )
{
    assert( &node != NULL );
    simparm::TreeAttributes::registerNamedEntries( node );
}

OutputSource::~OutputSource() {
}

void OutputSource::adjust_to_basename(BasenameAdjustedFileEntry& e)
{
    adjustedList->push_back( &e );
}

void
OutputSource::set_output_file_basename(const Basename& new_basename)
{
    for ( AdjustedList::iterator i  = adjustedList->begin(); 
                                 i != adjustedList->end(); i++)
    {
        (*i)->set_output_file_basename(
            new_basename );
    }
}

std::ostream &operator<<(std::ostream &o, 
                         Capabilities data) {
    std::string rv = "", sep = ", ";
    if ( data.test( Capabilities::SourceImage ) )
        rv += sep + "SourceImage";
    if ( data.test( Capabilities::SmoothedImage ) )
        rv += sep + "SmoothedImage";
    if ( data.test( Capabilities::CandidateTree ) )
        rv += sep + "CandidateTree";
    if ( data.test( Capabilities::InputBuffer) )
        rv += sep + "InputBuffer";
    if ( data.test( Capabilities::ClustersWithSources) )
        rv += sep + "LocalizationSources";

    if ( rv != "" )
        rv = rv.substr( sep.size() );
    size_t pos = rv.rfind(',');
    if ( pos != std::string::npos )
        rv.replace( pos, 1, " and" );
    return (o << rv);
}

std::ostream &operator<<(std::ostream &o, 
                         Output::Result r) 
{
    switch( r ) {
      case Output::KeepRunning: 
        return o << "KeepRunning";
      case Output::RemoveThisOutput:
        return o << "RemoveThisOutput";
      default: return o << "unknown result";
    }
}

std::ostream &operator<<(std::ostream &o, 
                         Output::ProgressSignal p) 
{
    switch( p ) {
      case Output::Engine_run_is_starting:
        return o << "Engine_run_is_starting";
      case Output::Engine_is_restarted: 
        return o << "Engine_is_restarted";
      case Output::Engine_run_failed: 
        return o << "Engine_run_failed";
      case Output::Engine_run_succeeded: 
        return o << "Engine_run_succeeded";
      default: return o << "unknown signal";
    }
}

void Output::check_additional_data_with_provided
(std::string name, AdditionalData can_provide, AdditionalData are_desired)
{
    if ( ( (~can_provide) & are_desired ).any() ) {
        std::stringstream ss;
        ss << "The data " << are_desired << " cannot be provided by "
           << "the data source " << name << ", which can only provide "
           << can_provide;
        throw std::logic_error(ss.str());
    }
}

void Output::insert_filename_with_check(
    std::string file, std::set<std::string>& present_filenames )
{
    if ( file != "" && present_filenames.insert( file ).second == false )
        throw std::runtime_error(
            "The output file name '" + file + "' was used by multiple "
            "outputs. Refusing to start job to avoid conflict.");
}


}
}
