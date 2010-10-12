#ifndef DSTORM_ANDORCAMERA_INPUTCHAINLINK_H
#define DSTORM_ANDORCAMERA_INPUTCHAINLINK_H

#include <dStorm/input/ChainLink.h>
#include <simparm/Object.hh>
#include <simparm/Entry.hh>
#include "Context.h"

namespace AndorCamera {

/** The Method class provides the configuration items for Andor camera
    *  acquisition that are acquisition-specific - control elements and
    *  acquisition area borders. All camera specific parameters are in
    *  AndorCamera::Config. */
class Method 
: public dStorm::input::ChainForwarder, public simparm::Object,
  public simparm::Listener
{
  private:
    class CameraSwitcher;
    std::auto_ptr<CameraSwitcher> switcher;
    Context::Ptr last_context;

    void operator()( const simparm::Event& );
  public:
    simparm::BoolEntry show_live_by_default;

  private:
    void registerNamedEntries();
    void context_changed( ContextRef context );

  public:
    Method();
    Method(const Method &c);
    virtual ~Method();

    simparm::Node* getNode() { return this; }

    Method* clone() const { return new Method(*this); }
    bool uses_input_file() const { return false; }
};


}

#endif