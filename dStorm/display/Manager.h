#ifndef DSTORM_DISPLAY_MANAGER_H
#define DSTORM_DISPLAY_MANAGER_H

#include <memory>
#include <bitset>
#include "DataSource.h"
#include <boost/utility.hpp>
#include <boost/function.hpp>
#include <simparm/Node_decl.hh>

namespace dStorm {
namespace display {

class DataSource;
class ResizeChange;

struct SaveRequest {
    typedef boost::function<void (Change&)> ImageManipulator;
    std::string filename;
    boost::units::quantity< boost::units::si::length > scale_bar;
    ImageManipulator manipulator;

    SaveRequest();
};

struct StorableImage {
    const Change& image;
    std::string filename;
    boost::units::quantity< boost::units::si::length > scale_bar;

    StorableImage( const std::string& filename, const Change& image );
};

/** The Manager class provides is the primary public
 *  interface to the dStorm::Display module. Modules
 *  wishing to show data in image windows should acquire
 *  an WindowHandle using the register_data_source call,
 *  providing a callback through the DataSource interface.
 */
class Manager : boost::noncopyable {
  public:
    static void setSingleton(Manager&);
    static Manager& getSingleton();

    struct WindowFlags;
    struct WindowProperties;
    struct WindowHandle;

  private:
    /** \copydoc register_data_source */
    virtual std::auto_ptr<WindowHandle>
        register_data_source_impl
        (const WindowProperties& properties,
         DataSource& handler) = 0;

    /** \copydoc store_image_impl */
    virtual void store_image_impl( const StorableImage& ) = 0;

  public:
    virtual ~Manager() {}
    /** Method should return pointer to config element for the display handler, or NULL if none are necessary. */
    virtual simparm::Node* getConfig() = 0;

    /** Open a window that displays the data in \c handler
     *  with the provided \c properties. The provided
     *  DataSource must be valid until the WindowHandle
     *  object returned by the function is deleted.
     **/
    std::auto_ptr<WindowHandle>
        register_data_source
        (const WindowProperties& properties,
         DataSource& handler);

    /** Store the given image on the hard disk. The Change structure given should have the resize and clear flags set. */
    void store_image( std::string filename, const Change& image );
    void store_image( const StorableImage& );
};

/** WindowFlags summarize boolean flags for window
    *  behaviour. */
struct Manager::WindowFlags : public std::bitset<3> {
    /** If this flag is set, the window created by
        *  register_data_source is closed as soon as
        *  the WindowHandle is destroyed. Unset with
        *  leave_window_open_on_unregister.
        *  @param set Set the flag if this variable is
        *             true, unset otherwise. */
    WindowFlags& close_window_on_unregister
        (bool set = true)
        { this->set(0, set); return *this; }
    /** This method unsets the
        *   close_window_on_unregister flag. */
    WindowFlags& leave_window_open_on_unregister()
        { reset(0); return *this; }
    bool get_close_window_on_unregister() const
        { return (*this)[0]; }
    /** If this flag is set, the image handle
        *  destructor delays execution until the
        *  data window is closed by the user or by the
        *  close_window_on_unregister flag. If this
        *  flag is not set (default), leave the window
        *  open. This flag is reset by the 
        *  detach_from_window method.
        *
        *  @param set Set the flag if this variable is
        *             true, unset otherwise. */
    WindowFlags& wait_for_user_closing_window(bool set = true)
        { this->set(1, set); return *this; }
    /** This method unsets the wait_for_user_closing_window flag. */
    WindowFlags& detach_from_window()
        { this->reset(1); return *this; }
    /** If this flag is set to true, the data source's
        *  notice_drawn_rectangle method will be called when a rectangle
        *  drawing is completed. When not set (the default), the 
        *  rectangle will be taken as region to zoom to. */
    WindowFlags& notice_drawn_rectangle(bool set = true)
        { this->set(2, set); return *this; }
    /** This method unsets the notice_drawn_rectangle flag. */
    WindowFlags& zoom_on_drawn_rectangle()
        { this->reset(2); return *this; }
    bool get_notice_drawn_rectangle() const { return (*this)[2]; }
};

/** A WindowHandle is the representation of a data
    *  window. As long as a WindowHandle object lives
    *  for the DataSource that was provided in the
    *  register_data_source call, the DataSource object
    *  is considered valid and callable. */
class Manager::WindowHandle {
  protected:
    WindowHandle() {}
    WindowHandle(const WindowHandle&);
    WindowHandle& operator=(const WindowHandle&);
  public:
    WindowFlags flags;
    virtual ~WindowHandle() {}

    virtual void store_current_display( SaveRequest ) = 0;
};

/** The WindowProperties class provides the data
    *  necessary for creation of a data window by the
    *  register_data_source method. */
struct Manager::WindowProperties {
    /** Displayed name of the window. */
    std::string name;
    /** Flags used to modify window behaviour. Will
        *  be copied into the WindowHandle object. */
    WindowFlags flags;
    /** Initial size of the image data area. */
    ResizeChange initial_size;
};

std::auto_ptr< Manager > make_dummy_manager();

}
}

#endif