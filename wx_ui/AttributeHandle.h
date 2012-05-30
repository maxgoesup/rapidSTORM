#ifndef SIMPARM_WX_ATTRIBUTE_HANDLE_H
#define SIMPARM_WX_ATTRIBUTE_HANDLE_H

#include <simparm/Attribute.h>
#include <boost/thread/mutex.hpp>

namespace simparm {
namespace wx_ui {

class BaseAttributeHandle {
    mutable boost::mutex mutex;
    BaseAttribute *a;
public:
    BaseAttributeHandle( BaseAttribute& a ) : a(&a) {}
    boost::optional< std::string > get_value() const { 
        boost::lock_guard< boost::mutex > lock( mutex );
        return (a) ? a->get_value() : boost::optional< std::string >();
    }

    void set_value( std::string value ) {
        boost::lock_guard< boost::mutex > lock( mutex );
        if (a) a->set_value(value);
    }

    void detach() {
        boost::lock_guard< boost::mutex > lock( mutex );
        a = NULL;
    }
};

}
}

#endif
