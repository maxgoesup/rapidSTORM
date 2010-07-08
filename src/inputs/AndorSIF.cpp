#include "config.h"
#ifdef HAVE_LIBREADSIF

#include "debug.h"

#define CImgBuffer_SIFLOADER_CPP

#include <read_sif.h>
#include <stdexcept>
#include <cassert>
#include <errno.h>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <functional>

#include <simparm/ChoiceEntry_Impl.hh>
#include <simparm/Message.hh>

#include "AndorSIF.h"
#include <dStorm/input/Source.h>
#include <dStorm/input/Source_impl.h>
#include <dStorm/ImageTraits.h>
#include <AndorCamera/Config.h>
#include <dStorm/input/BasenameWatcher.h>
#include <dStorm/input/FileBasedMethod_impl.h>
#include <dStorm/helpers/exception.h>

#include <boost/iterator/iterator_facade.hpp>

using namespace std;

namespace dStorm {
namespace input {
namespace AndorSIF {

template<typename Pixel>
std::auto_ptr< typename Source<Pixel>::Image >
Source<Pixel>::load(int count)
{
    DEBUG("Loading next image");
    std::auto_ptr< Image > result;
    if ( had_errors )
        return result;
    const int sz = readsif_imageSize(dataSet);
    float buffer[sz];
    for (int i = 0; i < sz; i++) buffer[i] = 5;
    typename Image::Size dim;
    dim.x() = readsif_imageWidth(dataSet, 0) 
        * cs_units::camera::pixel;
    dim.y() = readsif_imageHeight(dataSet, 0)
        * cs_units::camera::pixel;

    DEBUG("Calling GetNextImage");
    int rv_of_readsif_getImage = 
            readsif_getImage( dataSet, count, buffer );
    if ( rv_of_readsif_getImage == -1 ) {
        simparm::Message m("Error in SIF file",
            "Error while reading SIF file: " + std::string(readsif_error)
               + ". Will skip remaining images.", 
               simparm::Message::Warning);
        send(m);
        had_errors = true;
        return result;
    } else if ( rv_of_readsif_getImage == -2 ) {
        simparm::Message m("Error in SIF file",
            "Error while reading SIF file: " + std::string(readsif_error)
               + ". Will skip one image.", 
               simparm::Message::Warning);
        send(m);
        return result;
    } else if ( rv_of_readsif_getImage == 1 ) {
        throw std::logic_error("Too many images read from SIF source");
    }

    DEBUG("Creating new image");
    result.reset( new Image(dim) );
    DEBUG("Created new image");
    /* The pixel might need casting. This is done here. */
    for (int p = 0; p < sz; p++) {
        (*result)[p] = (Pixel)buffer[p];
    }
    DEBUG("Loaded next image");
    return result;
}

template<typename Pixel>
void Source<Pixel>::init(FILE *src)
{
    /* Open the SIF file and find a readable data set. */
    dataSet = NULL;
    stream = src;
    file = readsif_open_File(src);
    if (file == NULL) 
        throw std::runtime_error(file_ident + "is no valid SIF file");

    dataSet = readsif_read_DataSet(file);
    if (dataSet == NULL)  {
        string sif_error_if_any = (readsif_error[0] == 0) ? "" :
            (" " + string(readsif_error));
        throw std::runtime_error(file_ident + " contained no data or could not be read." + sif_error_if_any);
    }
    else if ( readsif_numberOfSubimages(dataSet) > 1 )
        clog << "Warning: SIF file contains multiple subimages. This "
                "feature is not supported and only the first subimage "
                "will be used." << endl;

    im_count = readsif_numberOfImages(dataSet);

    /* Read the additional information file from the SIF file
     * and store it in SIF info structure. */
    AndorCamera::Config *sifInfo = new AndorCamera::Config;
    this->sifInfo.reset(sifInfo);
    sifInfo->targetTemperature = 
        int(dataSet->instaImage.temperature) * boost::units::celsius::degrees;
    sifInfo->targetTemperature.editable = false;
    sifInfo->outputAmp = 
        (AndorCamera::OutputAmp)dataSet->instaImage.OutputAmp;
    sifInfo->outputAmp.editable = false;

    {
        stringstream ss;
        ss << dataSet->instaImage.data_v_shift_speed*1E6 << " �s";
        sifInfo->VS_Speed.addChoice(-1, "SIFSpeed", ss.str());
        sifInfo->VS_Speed.editable = false;
        sifInfo->VS_Speed.userLevel = simparm::Object::Expert;
    }
    {
        stringstream ss;
        ss << 1E-6/dataSet->instaImage.pixel_readout_time << " MHz";
        sifInfo->HS_Speed.addChoice(-1, "SIFSpeed", ss.str());
        sifInfo->HS_Speed.editable = false;
        sifInfo->HS_Speed.userLevel = simparm::Object::Expert;
    }
    sifInfo->emccdGain = dataSet->instaImage.PreAmpGain;
    sifInfo->emccdGain.editable = false;
    sifInfo->realExposureTime 
        = float(dataSet->instaImage.exposure_time) * boost::units::si::second;
    sifInfo->realExposureTime.editable = false;
    sifInfo->cycleTime 
        = float(dataSet->instaImage.kinetic_cycle_time) * boost::units::si::second;
    sifInfo->cycleTime.editable = false;

    simparm::BasicEntry* whn[] = {
        new simparm::UnsignedLongEntry("ImageWidth", "Image width", readsif_imageWidth(dataSet, 0) ),
        new simparm::UnsignedLongEntry("ImageHeight", "Image height", readsif_imageHeight(dataSet, 0) ),
        new simparm::UnsignedLongEntry("ImageNumber", "Number of images",
            readsif_numberOfImages(dataSet) ) 
    };

    int n = sizeof(whn) / sizeof(simparm::BasicEntry*);
    for (int i = 0; i < n; i++) {
        whn[i]->editable = false;
        sifInfo->push_back( std::auto_ptr<simparm::Node>( whn[i] ));
    }

    receive_changes_from( showDetails.value );
    receive_changes_from( hideDetails.value );

    hideDetails.trigger();
}

template<typename Pixel>
Source<Pixel>::Source(FILE *src, const string &i)
: Object("AndorSIF", "SIF file"),
  BaseSource(static_cast<simparm::Node&>(*this), Flags()),
  Set("AndorSIF", "SIF file"),
  has_been_iterated(false),
  stream(NULL), file(NULL), dataSet(NULL), had_errors(false), file_ident(i),
  showDetails("ShowDetails", "Show SIF file information"),
  hideDetails("HideDetails", "Hide SIF file information")
{
   init(src);
}

template<typename Pixel>
Source<Pixel>::Source(const char *filename) 
: Set("AndorSIF", "SIF file"),
  BaseSource
    ( static_cast<simparm::Node&>(*this), Flags() ),
  has_been_iterated(false),
  stream(NULL), file(NULL), dataSet(NULL), had_errors(false), file_ident(filename),
  showDetails("ShowDetails", "Show SIF file information"),
  hideDetails("HideDetails", "Hide SIF file information")
{
   FILE *result = fopen(filename, "rb");
   if (result != NULL)
      init(result);
   else {
      throw std::runtime_error((("Could not open " + file_ident) + ": ") +
                         strerror(errno));
    }
}

template<typename Pixel>
Source<Pixel>::~Source() {
    if ( dataSet != NULL )
        readsif_destroy_DataSet( dataSet );
    if ( file != NULL )
        readsif_destroy_File( file );
    if ( stream != NULL ) {
        fclose(stream);
    }
}

template<typename Pixel>
void Source<Pixel>::operator()(const simparm::Event& e) {
    if ( &e.source == &showDetails.value && showDetails.triggered() ) {
        showDetails.untrigger();
        if ( sifInfo.get() ) sifInfo->viewable = true;
        showDetails.viewable = false;
        hideDetails.viewable = true;
    } else if ( &e.source == &hideDetails.value && hideDetails.triggered() ) {
        hideDetails.untrigger();
        if ( sifInfo.get() ) sifInfo->viewable = false;
        showDetails.viewable = true;
        hideDetails.viewable = false;
    }
    
}

template<typename Pixel>
typename Source<Pixel>::TraitsPtr 
Source<Pixel>::get_traits()
{
   TraitsPtr rv( new typename TraitsPtr::element_type() );
   rv->size.x() = 
        readsif_imageWidth( dataSet, 0 ) * cs_units::camera::pixel;
   rv->size.y() = readsif_imageHeight( dataSet, 0 )
        * cs_units::camera::pixel;

    rv->speed = 1.0 * cs_units::camera::frame / ( dataSet->instaImage.kinetic_cycle_time
        * boost::units::si::second );

   rv->last_frame =
    (readsif_numberOfImages(dataSet) - 1) * cs_units::camera::frame;
    return rv;
}

template<typename Pixel>
Source< Pixel >*
Config<Pixel>::impl_makeSource()
{
    Source<Pixel>* ptr =
        new Source<Pixel>(this->inputFile().c_str());
    ptr->push_back( this->inputFile );
    ptr->push_back_SIF_info();
    this->inputFile.editable = false;
    return ptr;
}

template<typename Pixel>
Config<Pixel>::Config( input::Config& src) 
: FileBasedMethod< dStorm::Image<Pixel,2> >(src,
    "AndorSIF", "Andor SIF file",
    "extension_sif", ".sif")
{
    this->push_back(src.firstImage);
    this->push_back(src.lastImage);
    this->push_back(src.pixel_size_in_nm);
}

template<typename Pixel>
Config<Pixel>::Config(
    const Config<Pixel>::Config &c,
    input::Config& src
) 
: FileBasedMethod< dStorm::Image<Pixel,2> >(c, src)
{
    this->push_back(src.firstImage);
    this->push_back(src.lastImage);
    this->push_back(src.pixel_size_in_nm);
}

template <typename Pixel>
class Source<Pixel>::iterator
: public boost::iterator_facade<iterator,Image,std::input_iterator_tag>
{
    mutable dStorm::Image<Pixel,2> img;
    Source* src;
    int count;
    mutable bool did_load;

    friend class boost::iterator_core_access;

    Image& dereference() const { 
        if ( ! did_load ) {
            DEBUG("Loading at " << count);
            std::auto_ptr<dStorm::Image<Pixel,2> > i = src->load(count);
            if ( i.get() == NULL && src->had_errors ) throw dStorm::abort();
            if ( i.get() != NULL )
                img = *i;
            else
                img.invalidate();
            img.frame_number() = count * cs_units::camera::frame;
            did_load = true;
        }
        return img; 
    }
    bool equal(const iterator& i) const { 
        DEBUG( "Comparing " << count << " with " << i.count ); 
        return count == i.count || (src && src->had_errors); 
    }

    void increment() { DEBUG("Incrementing iterator from " << count); ++count; did_load = false; img.invalidate(); }
  public:
    iterator() : src(NULL), count(0), did_load(false) {}
    iterator(Source& s, int c = 0) : src(&s), count(c), did_load(false)
    {}
};

template <typename PixelType>
typename Source<PixelType>::base_iterator 
Source<PixelType>::begin() {
    return base_iterator( iterator(*this, 0) );
}
template <typename PixelType>
typename Source<PixelType>::base_iterator 
Source<PixelType>::end() {
    return base_iterator( iterator(*this, im_count) );
}

template class Config<unsigned short>;
//template class Config<unsigned int>;
//template class Config<float>;

}
}
}

#endif