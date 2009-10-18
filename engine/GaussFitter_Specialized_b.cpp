#include <fit++/Exponential2D_Uncorrelated_Derivatives.hh>
#include "GaussFitter_Specialized.h"
#include "GaussFitter_ResidueAnalysis.h"

namespace dStorm {

template <>
template <>
void GaussFitter<false,true,false>::
create_specializations<1>();

template <>
template <>
void GaussFitter<false,true,false>::
create_specializations<0>()
{
    this->fill_specialization_array<5,6>();
    create_specializations<1>();
    dynamic_fitter_factory.reset( 
        new TableEntryMaker<Eigen::Dynamic,Eigen::Dynamic>() );
}

template class GaussFitter<false,true,false>;

}