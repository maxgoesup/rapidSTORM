#ifndef DSTORM_INPUT_MANAGER_H
#define DSTORM_INPUT_MANAGER_H

#include "Link_decl.h"

#include "../Source_decl.h"
#include <boost/shared_ptr.hpp>
#include <string>
#include <list>
#include <utility>
#include <memory>
#include <simparm/Node_decl.hh>
#include "MetaInfo_decl.h"
#include <dStorm/InsertionPlace.h>

namespace dStorm {
namespace input {
namespace chain {

class Link {
    Link *less_specialized;
  protected:
    typedef boost::shared_ptr<const MetaInfo> TraitsRef;
  private:
    TraitsRef meta_info;
  public:
    class AtEnd {};

  public:
    Link();
    Link(const Link&);
    virtual ~Link();

    /** Method which is called by the downstream element (e.g. input) 
     *  to notify the upstream element (e.g. engine) of changed capabilities. */
    virtual AtEnd traits_changed( TraitsRef, Link* ) = 0;

    virtual BaseSource* makeSource() = 0;
    virtual Link* clone() const = 0;
    virtual void registerNamedEntries( simparm::Node& ) = 0;

    TraitsRef current_traits() const { return meta_info; }

    typedef dStorm::InsertionPlace Place;
    virtual void insert_new_node( std::auto_ptr<Link>, Place ) = 0;
    virtual std::string name() const = 0;
    virtual std::string description() const = 0;
 
  protected:
    enum SetType { Add, Remove };
    virtual void set_upstream_element( Link& element, SetType );
    static void set_upstream_element( Link& on, Link& to, SetType type )
        { on.set_upstream_element(to, type); }
    AtEnd notify_of_trait_change( TraitsRef new_traits );
};

/** Specialized case of Link for terminal nodes, that is,
 *  those that need no further elements up the chain. */
struct Terminus : public Link {
    /** This method makes no sense for a terminus, since it should be
     *  called by predecessors, which are nonexistent. It is only defined
     *  for its virtual table entry and will simply throw an error when
     *  called. */
    virtual AtEnd traits_changed( TraitsRef, Link* );

    virtual Terminus* clone() const = 0;
    virtual void insert_new_node( std::auto_ptr<Link> l, Place ); 
};

}
}
}


#endif
