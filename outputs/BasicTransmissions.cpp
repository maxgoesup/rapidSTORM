#define CImgBuffer_TIFFLOADER_CPP
#include "BasicTransmissions.h"

#include "LocalizationCounter.h"
#include "ProgressMeter.h"
#include "AverageImage.h"
#include "TraceFilter.h"
#include "expression/Source_decl.h"
#include "Slicer.h"
#include "MemoryCache.h"
#include "LocalizationFile.h"
#include "SigmaDiff3D.h"
#include "LinearAlignment.h"
#include "DriftRemover.h"
#include "RegionOfInterest.h"
#include "RegionSegmenter.h"
#include "SpotMeter.h"
#include "PrecisionEstimator.h"
#include "VarianceEstimator.h"

using namespace std;
using namespace dStorm::outputs;

namespace dStorm {
namespace output {

void basic_outputs( dStorm::Config* o ) {
    o->add_output( localization_file::writer::create() );
    o->add_output( make_progress_meter_source().release() );
    o->add_output( make_localization_counter_source().release() );
    o->add_output( make_average_image_source().release() );
    o->add_output( memory_cache::make_output_source().release() );
    o->add_output( make_trace_count_source().release() );
    o->add_output( slicer::make_output_source() );
    o->add_output( expression::make_output_source().release() );
    o->add_output( make_sigma_diff_3d().release() );
    o->add_output( make_linear_alignment().release() );
    o->add_output( drift_remover::make().release() );
    o->add_output( make_roi_filter_source().release() );
    o->add_output( make_segmenter_source().release() );
    o->add_output( make_spot_meter_source().release() );
    o->add_output( make_precision_estimator_source().release() );
    o->add_output( make_variance_estimator_source().release() );
}

}
}
