#ifndef DSTORM_ANDORDIRECT_CAMERACONNECTION_H

#include "AndorDirect_decl.h"
#include <boost/asio/ip/tcp.hpp>
#include <dStorm/ImageTraits.h>
#include <dStorm/engine/Image.h>
#include <boost/variant/variant.hpp>
#include <simparm/Entry.hh>

namespace dStorm {
namespace AndorCamera {

struct CameraConnection {
    typedef quantity<camera::length,int> pixel;
    CameraConnection(const std::string& hostname, int camera, const std::string& port);
    ~CameraConnection();
    void set_traits( input::Traits<engine::Image>& );
    void send( const std::string& );
    void start_acquisition( CamTraits&, simparm::StringEntry& status );
    void stop_acquisition();

    struct EndOfAcquisition {};
    struct FetchImage { quantity<camera::time,int> frame_number; };
    struct ImageError { quantity<camera::time,int> frame_number; };
    struct Simparm { std::string message; };
    struct StatusChange { 
        StatusChange(std::string a) : status(a) {}
        std::string status; 
    };
    typedef boost::variant<EndOfAcquisition,FetchImage,ImageError,Simparm,StatusChange> FrameFetch;
    FrameFetch next_frame();

    void read_data( CamImage& );
  private:
    boost::asio::ip::tcp::iostream stream;
    Simparm parse_simparm(std::string line);
};

}
}

#endif
