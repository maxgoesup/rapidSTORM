#ifndef LOCPREC_PRECISIONESTIMATOR_H
#define LOCPREC_PRECISIONESTIMATOR_H

#include <memory>
#include <dStorm/Output.h>
#include <dStorm/OutputBuilder.h>
#include <dStorm/FileOutputBuilder.h>
#include "LocalizationList.h"
#include <simparm/NumericEntry.hh>
#include <simparm/FileEntry.hh>
#include <Eigen/Core>

namespace locprec {
    namespace Precision {
        struct FitSigmas { double x, y, xy; int n; double a; };
        FitSigmas fitWithGauss
            ( double res_enh, const dStorm::Localization* f, int n )
;
    };

    class SinglePrecisionEstimator 
    : public simparm::Object, public dStorm::Output 
    {
      private:
        simparm::FileEntry printTo;
        Eigen::Vector3d pixel_dim_in_nm;
        double res_enh;

        ost::Mutex mutex;

        class _Config;

      public:
        typedef simparm::Structure<_Config> Config;
        typedef dStorm::FileOutputBuilder<SinglePrecisionEstimator> Source;

        SinglePrecisionEstimator ( const Config& config );
        SinglePrecisionEstimator *clone() const;

        AdditionalData announceStormSize(const Announcement&); 
        Result receiveLocalizations(const EngineResult&);
        void propagate_signal(ProgressSignal);

        EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    };

    class MultiPrecisionEstimator 
    : public simparm::Object, public dStorm::Output 
    {
        simparm::UnsignedLongEntry usedSpots;
        simparm::DoubleEntry x_sd, y_sd, corr;
        Eigen::Vector3d pixel_dim_in_nm;
        double res_enh;

        dStorm::LocalizationList localizations;

        ost::Mutex mutex;

        void registerNamedEntries();
        void estimatePrecision();

        class _Config;
        
      public:
        typedef simparm::VirtualStructure<_Config> Config;
        typedef dStorm::OutputBuilder<MultiPrecisionEstimator> Source;

        MultiPrecisionEstimator( const Config& config );
        MultiPrecisionEstimator(const MultiPrecisionEstimator& c);

        MultiPrecisionEstimator *clone() const;

        AdditionalData announceStormSize(const Announcement& a);
        Result receiveLocalizations(const EngineResult& e)
            { return localizations.receiveLocalizations(e); }
        void propagate_signal(ProgressSignal s) 
        {
            localizations.propagate_signal(s);
            if ( s == Engine_run_succeeded )
                estimatePrecision();
        }

        EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    };

    class PrecisionEstimatorConfig : public virtual simparm::Node
    {
      protected:

        void registerNamedEntries() {
            push_back(resEnh);
        }

      public:
        simparm::DoubleEntry resEnh;

        PrecisionEstimatorConfig()
        : resEnh("FitBinSize", "Bin size for precision Gauss fit", 0.1)
            {}
        PrecisionEstimatorConfig* clone() const = 0;
    };

    struct MultiPrecisionEstimator::_Config 
        : public PrecisionEstimatorConfig, public simparm::Object
    { 
        _Config(); 
        _Config* clone() const { return new _Config(*this); }
    };

    class SinglePrecisionEstimator::_Config :
        public PrecisionEstimatorConfig, public simparm::Object
    {
      protected:
        void registerNamedEntries() {
            PrecisionEstimatorConfig::registerNamedEntries();
            push_back( outputFile );
        }
      public:
        simparm::FileEntry outputFile;

        _Config();
        _Config* clone() const { return new _Config(*this); }
    };
}
#endif
