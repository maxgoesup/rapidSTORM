#ifndef TESTPLUGIN_VERBOSE_H
#define TESTPLUGIN_VERBOSE_H

#include <simparm/Entry.hh>
#include <simparm/NumericEntry.hh>
#include <dStorm/output/Output.h>
#include <dStorm/output/OutputBuilder.h>
#include <iostream>
#include <stdexcept>
#include <boost/units/io.hpp>
#include <dStorm/log.h>

struct Verbose
: public dStorm::output::OutputObject
{
    struct _Config;
    typedef simparm::Structure<_Config> Config;
    typedef dStorm::output::OutputBuilder<Verbose> Source;

    Verbose(const Config& config) ;
    ~Verbose();
    Verbose* clone() const;

    AdditionalData announceStormSize(const Announcement& a) { 
        if (  a.traits.resolution.is_set() ) {
            LOG( "Verbose plugin got announcement with "
                    << a.traits.size.transpose() << " and size "
                    << *a.traits.resolution );
            if ( a.traits.speed.is_set() ) {
                LOG("Announced speed is " << *a.traits.speed );
            }
        } else {
            LOG( "Verbose plugin got announcement with "
                    << a.traits.size.transpose() );
        }
        return AdditionalData(); 
    }
    Result receiveLocalizations(const EngineResult& er) {
        LOG( "Verbose plugin got results for " << er.forImage);
        return KeepRunning;
    }
    void propagate_signal(ProgressSignal s) {
        LOG("Verbose plugin got signal " << s);
    }

};

struct Verbose::_Config
 : public simparm::Object 
{
    _Config();
    void registerNamedEntries() {}
    bool can_work_with(const dStorm::output::Capabilities&)
        {return true;}
};

Verbose::_Config::_Config()
 : simparm::Object("Verbose", "Verbose")
{
}

Verbose* Verbose::clone() const { 
    LOG( "Cloning verbose plugin" );
    return new Verbose(*this); 
}

Verbose::Verbose( const Config& config )
        : OutputObject("SegFault", "SegFault")
{
    LOG( "Constructed verbose plugin" );
}

Verbose::~Verbose()
{
    LOG( "Destroyed verbose plugin" );
}

#endif