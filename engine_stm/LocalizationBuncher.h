#ifndef DSTORM_ENGINE_LOCALIZATIONBUNCHER_H
#define DSTORM_ENGINE_LOCALIZATIONBUNCHER_H

#include <dStorm/input/Traits.h>
#include <dStorm/Localization.h>
#include <map>
#include <memory>
#include <dStorm/output/Output.h>
#include <boost/utility.hpp>
#include <boost/ptr_container/ptr_map.hpp>
#include <boost/thread/mutex.hpp>
#include <dStorm/input/Source.h>

namespace dStorm {
namespace engine_stm {

class Visitor;

template <typename Input>
class Source;

template <typename Input>
class LocalizationBuncher 
: public boost::iterator_facade< 
    LocalizationBuncher<Input>, 
    output::LocalizedImage,
    std::input_iterator_tag>
{
    Source<Input>* master;

    boost::shared_ptr<output::LocalizedImage> output;
    frame_index outputImage;

    friend class boost::iterator_core_access;
    output::LocalizedImage& dereference() const;
    bool equal(const LocalizationBuncher& o) const;
    void increment();

  public:
    LocalizationBuncher( Source<Input>* master, frame_index image );
    LocalizationBuncher(const LocalizationBuncher&);
    ~LocalizationBuncher();
};

template <typename InputType>
class Source
: public input::Source<output::LocalizedImage>,
  boost::noncopyable
{
  public:
    typedef input::Source<InputType> Input;
    typedef typename Input::iterator InputIterator;
    typedef input::Source<output::LocalizedImage> Base;

  private:
    std::auto_ptr< Input > base;
    InputIterator current, base_end;
    frame_index first_image;
    boost::optional<frame_index> last_image;
    typedef boost::ptr_map<frame_index,output::LocalizedImage> Canned;
    Canned canned;
    bool in_sequence;

    void attach_ui_( simparm::NodeHandle n ) { base->attach_ui(n); }

  public:
    std::auto_ptr<output::LocalizedImage> read( frame_index );
    bool is_finished( frame_index current ) const;

  public:
    Source( std::auto_ptr<Input> base ) ;
    ~Source();

    void dispatch(Messages m);
    iterator begin() 
        { return iterator( LocalizationBuncher<InputType>(this, first_image) ); }
    iterator end()
        { return iterator( LocalizationBuncher<InputType>(NULL, first_image) ); }
    TraitsPtr get_traits( input::BaseSource::Wishes );
    BaseSource& upstream() { return *base; }
    Capabilities capabilities() const 
        { return base->capabilities().reset( ConcurrentIterators ); }
};

}
}

#endif
