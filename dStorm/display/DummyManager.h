#ifndef DSTORM_DISPLAY_DUMMYMANAGER_H
#define DSTORM_DISPLAY_DUMMYMANAGER_H

#include "Manager.h"
#include <list>
#include <boost/optional/optional.hpp>

namespace dStorm {
namespace display {

class DummyManager : public Manager
{
    struct Handle;
    std::list< Handle* > open_windows;
    boost::optional<Change> image;

    std::auto_ptr<WindowHandle> register_data_source_impl(
        const WindowProperties& properties,
        DataSource& handler);

    void store_image_impl( const StorableImage& );
    simparm::Node* getConfig() { return NULL; }
  public:
    ~DummyManager();
    Change& get_stored_image() { return *image; }
};

}
}

#endif