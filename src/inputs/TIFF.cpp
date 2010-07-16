#define DSTORM_TIFFLOADER_CPP

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_TIFFIO_H

#include "debug.h"

#include <stdexcept>
#include <cassert>
#include <errno.h>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <functional>
#include <stdint.h>

#include <simparm/ChoiceEntry_Impl.hh>

#include <dStorm/Image.h>
#include <tiffio.h>

#include "TIFF.h"
#include <dStorm/input/Source.h>
#include <dStorm/input/Source_impl.h>
#include <dStorm/ImageTraits.h>
#include <dStorm/input/FileBasedMethod_impl.h>

#include <boost/iterator/iterator_facade.hpp>
#include <boost/units/base_units/us/inch.hpp>

#include "TIFFOperation.h"

using namespace std;

namespace dStorm {
namespace TIFF {

template<typename Pixel>
Source<Pixel>::Source(const char *src, bool ignore_warnings)
: simparm::Set("TIFF", "TIFF image reader"),
  BaseSource( static_cast<simparm::Node&>(*this),    
      Flags() ),
  ignore_warnings(ignore_warnings)
{
    TIFFOperation op( "in opening TIFF file",
                      *this, ignore_warnings );
    tiff = TIFFOpen( src, "rm" );
    if ( tiff == NULL ) op.throw_exception_for_errors();

    TIFFGetField( tiff, TIFFTAG_IMAGEWIDTH, &_width );
    TIFFGetField( tiff, TIFFTAG_IMAGELENGTH, &_height );

    float xres, yres;
    int xgiven = TIFFGetField( tiff, TIFFTAG_XRESOLUTION, &xres );
    int ygiven = TIFFGetField( tiff, TIFFTAG_YRESOLUTION, &yres );
    if ( xgiven == 1 || ygiven == 1 ) {
        float res;
        if ( xgiven == 1 && ygiven == 1 )
            res = (xres+yres)/2;
        else if ( xgiven == 1 )
            res = xres;
        else
            res = yres;

        int unit = RESUNIT_INCH;
        TIFFGetField( tiff, TIFFTAG_RESOLUTIONUNIT, &unit );
        if ( unit == RESUNIT_INCH )
            resolution = boost::units::quantity<cs_units::camera::resolution,float>(
                res * cs_units::camera::pixel / (0.0254 * boost::units::si::meters));
        else if ( unit == RESUNIT_CENTIMETER )
            resolution = boost::units::quantity<cs_units::camera::resolution,float>(
                res * cs_units::camera::pixel / (0.01 * boost::units::si::meters));
    }

#if 0
    _no_images = 1;
    while ( TIFFReadDirectory(tiff) != 0 )
        _no_images += 1;
#endif

    current_directory = 0;
}

template<typename Pixel>
class Source<Pixel>::iterator 
: public boost::iterator_facade<iterator,Image,std::random_access_iterator_tag>
{
    mutable Source* src;
    int directory;
    mutable Image img;

    void go_to_position() const {
        TIFFOperation op( "in reading TIFF file",
                          *src, src->ignore_warnings );
        int rv = TIFFSetDirectory(src->tiff, directory);
        if ( rv == 1 ) {
            src->current_directory = directory;
        } else {
            op.throw_exception_for_errors();
        }
    }
    void check_position() const {
        if ( src->current_directory != directory )
            go_to_position();
    }
    void check_params() const;

  public:
    iterator() : src(NULL) {}
    iterator(Source &s) : src(&s), directory(0) {}

    Image& dereference() const; 
    bool equal(const iterator& i) const {
        DEBUG( "Comparing " << src << " " << i.src << " " << directory << " " << i.directory );
        return (src == i.src) && (src == NULL || directory == i.directory);
    }
    void increment() { 
        TIFFOperation op( "in reading TIFF file",
                          *src, src->ignore_warnings );
        check_position();
        img.invalidate(); 
        if ( TIFFReadDirectory(src->tiff) != 1 ) {
            op.throw_exception_for_errors();
            /* Code from here only executed when no error
             * was encountered */
            DEBUG( "Setting iterator to NULL" );
            src = NULL;
        } else {
            directory++; 
            src->current_directory = directory;
        }
    }
    void decrement() { 
        img.invalidate(); 
        if ( directory == 0 ) 
            src = NULL; 
        else {
            --directory;
            go_to_position();
        }
    }
    void advance(int n) { 
        if (n) {
            img.invalidate(); 
            directory += n;
            go_to_position();
        }
    }
    int distance_to(const iterator& i) {
        return i.directory - directory;
    }
};

template<typename Pixel>
void
Source<Pixel>::iterator::check_params() const
{
    ::TIFF *tiff = src->tiff;
    uint32_t width, height;
    uint16_t bitspersample;
    TIFFGetField( tiff, TIFFTAG_IMAGEWIDTH, &width );
    TIFFGetField( tiff, TIFFTAG_IMAGELENGTH, &height );
    TIFFGetField( tiff, TIFFTAG_BITSPERSAMPLE, &bitspersample );

    if ( int(width) != src->_width || int(height) != src->_height ) {
        std::stringstream error;
        error << "TIFF image no. " << TIFFCurrentDirectory(tiff)
            << " has dimensions (" << width << ", " << height
            << ") different from first image (" << src->_width
            << ", " << src->_height << "). Aborting.";
        throw std::runtime_error( error.str() );
    }
    if ( bitspersample != sizeof(Pixel)*8 ) {
        std::stringstream error;
        error << "TIFF image no. " << TIFFCurrentDirectory(tiff) << " has "
                << bitspersample << " bits per pixel, but "
                << sizeof(Pixel)*8 << " bits are necessary.";
        throw std::runtime_error( error.str() );
    }
}

template<typename Pixel>
typename Source<Pixel>::Image&
Source<Pixel>::iterator::dereference() const
{ 
    if ( img.is_invalid() ) {
        TIFFOperation op( "in reading TIFF file",
                          *src, src->ignore_warnings );
        check_position();
        check_params();

        typename Image::Size sz;
        ::TIFF *tiff = src->tiff;
        tsize_t strip_size = TIFFStripSize( tiff );
        tstrip_t strip_count = TIFFNumberOfStrips( tiff );
        sz.x() = src->_width * cs_units::camera::pixel;
        sz.y() = src->_height * cs_units::camera::pixel;
        Image i( sz, directory * cs_units::camera::frame);

        DEBUG("Reading image " << directory << " " << i.size());
        assert( i.size() >= (strip_size * strip_count / sizeof(Pixel)) * cs_units::camera::pixel * cs_units::camera::pixel );

        for (tstrip_t strip = 0; strip < strip_count; strip++) {
            TIFFReadEncodedStrip( tiff, strip, 
                i.ptr() + (strip * strip_size / sizeof(Pixel)),
                strip_size );
        }
        img = i;

        op.throw_exception_for_errors();
    }

    return img;
}

template<typename Pixel>
Source<Pixel>::~Source() {
    TIFFClose( tiff );
}

template<typename Pixel>
typename Source<Pixel>::base_iterator
Source<Pixel>::begin() {
    return base_iterator( iterator(*this) );
}

template<typename Pixel>
typename Source<Pixel>::base_iterator
Source<Pixel>::end() {
    return base_iterator( iterator() );
}

template<typename Pixel>                                   
typename Source<Pixel>::TraitsPtr
Source<Pixel>::get_traits() 
{
    TraitsPtr rv( new typename TraitsPtr::element_type());
    rv->size.x() = _width * cs_units::camera::pixel;
    rv->size.y() = _height * cs_units::camera::pixel;
    rv->dim = 1; /* TODO: Read from file */
    rv->resolution = resolution;

    return rv;
}

template<typename Pixel>
Source< Pixel >*
Config<Pixel>::impl_makeSource()
{
    Source<Pixel>* ptr =
        new Source<Pixel>(this->inputFile().c_str(), 
                          ignore_warnings());
    ptr->push_back( this->inputFile );
    this->inputFile.editable = false;
    return ptr;
}

template<typename Pixel>
Config<Pixel>::Config( input::Config& src) 
: FileBasedMethod< Image<Pixel,2> >(
        src, "TIFF", "TIFF file", 
        "extension_tif", ".tif" ),
  tiff_extension("extension_tiff", ".tiff"),
  ignore_warnings("IgnoreLibtiffWarnings",
    "Ignore libtiff warnings", false)
{
    ignore_warnings.userLevel 
        = simparm::Object::Intermediate;
    this->inputFile.push_back(tiff_extension);
    this->push_back( src.pixel_size_in_nm );
    this->push_back( ignore_warnings );
}

template<typename Pixel>
Config<Pixel>::Config(
    const Config<Pixel>::Config &c,
    input::Config& src
) 
: FileBasedMethod< Image<Pixel,2> >(c, src),
  tiff_extension(c.tiff_extension),
  ignore_warnings(c.ignore_warnings)
{
    this->inputFile.push_back(tiff_extension);
    this->push_back( src.pixel_size_in_nm );
    this->push_back( ignore_warnings );
}

//template class Config<unsigned char>;
template class Config<unsigned short>;
//template class Config<unsigned int>;
//template class Config<float>;
//template class Config<double>;

}
}

#endif