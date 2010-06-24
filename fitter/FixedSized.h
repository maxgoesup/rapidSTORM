#ifndef DSTORM_FITTER_CONCRETE_SIZE_H
#define DSTORM_FITTER_CONCRETE_SIZE_H

#include "Sized.h"
#include <Eigen/Core>

namespace dStorm {
namespace fitter {

template <class BaseFitter, int Width, int Height>
class FixedSized : public Sized
{
  public:
    typedef typename BaseFitter::SizeInvariants Common;
    typedef typename BaseFitter::template Specialized<Width,Height>
        ::Deriver Deriver;

  protected:
    Deriver deriver;
    typename Deriver::Position a, b, *c;
    Common& common;

  public:
    FixedSized(Common& common) : common(common) {}

    inline void setSize( int width, int height ) {
        deriver.setSize( width, height );
        a.resize( width, height );
        b.resize( width, height );
    }
    
    int fit(
        const engine::Spot& spot, Localization* target,
        const engine::BaseImage &image, int xl, int yl );

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

}
}

#endif