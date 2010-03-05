#ifndef DSTORM_CIMGLIST_H
#define DSTORM_CIMGLIST_H

#include <dStorm/input/ImageTraits.h>
#include <dStorm/input/Source.h>
#include <dStorm/input/Config.h>
#include <dStorm/input/Method.h>
#include <dStorm/input/FileBasedMethod.h>
#include <memory>
#include <string>
#include <stdexcept>
#include <stdio.h>
#include <simparm/FileEntry.hh>
#include <CImg.h>
#include <stdint.h>

namespace dStorm {
  /** The CImgList namespace provides sources that operate on
   *  a standard CImgList structure. The provided filename is
   *  loaded by CImgList::load() and images in the resulting
   *  sequence are returned one by one. */
  namespace CImgList {
    using cimg_library::CImg;
    using cimg_library::CImgList;

    template <typename PixelType> std::string ident();

    template <typename PixelType>
    class Source : public simparm::Object,
         public input::Source< CImg<PixelType> >
                   
    {
      public:
         Source(const char *src );
         virtual ~Source() {}

         virtual int dimx() const 
            { return sourceImages.front().width; }
         virtual int dimy() const 
            { return sourceImages.front().height; }
         virtual int quantity() const 
            { return sourceImages.size; }

         Object& getConfig() { return *this; }

      private:
         CImgList<PixelType> sourceImages;

      protected:
        virtual CImg<PixelType>*
            fetch(int image_index)
        {
            return new CImg<PixelType>(sourceImages[image_index]);
        }

    };

    template <typename PixelType>
    class Config : public input::FileBasedMethod< CImg<PixelType> > {
      public:
        typedef CImg<PixelType> Yield;
        typedef input::Config MasterConfig;

        Config(MasterConfig& src)
            : simparm::Object("CImgList" + ident<PixelType>(),
                              "List of CImg objects") {}
        Config(const Config &c, MasterConfig& src) 
            : simparm::Object(c), input::Method< CImg<PixelType> >(c) {}

        simparm::FileEntry &inputFile;

        Config* clone(MasterConfig& newMaster) const
            { return new Config<PixelType>(*this, newMaster); }

      protected:
        input::Source<Yield>* impl_makeSource()
            { return new Source<PixelType>( inputFile().c_str() ); }
    };

  }
}

#endif