#ifndef DSTORM_CONFIG_H
#define DSTORM_CONFIG_H

#include <simparm/Set.hh>
#include <simparm/Entry.hh>
#include <simparm/OptionalEntry.hh>
#include <dStorm/UnitEntries.h>
#include <simparm/NumericEntry.hh>
#include <simparm/ChoiceEntry.hh>
#include <boost/units/cmath.hpp>
#include <dStorm/output/Basename_decl.h>

namespace dStorm {
namespace engine {
   using namespace simparm;

   class SpotFinderFactory;
   class SpotFitterFactory;

   /** Config entry collection class for the dSTORM engine. */
   class _Config : public Set
   {
      protected:
        void registerNamedEntries();
      public:
        _Config();
        ~_Config();

        /** The standard deviation for the exponential spot model. */
        FloatPixelEntry sigma_x, sigma_y;
        DoubleEntry sigma_xy;
        /** The uncertainty allowed in sigma estimation. */
        DoubleEntry delta_sigma;

        /** The proportionality factor for the smoothing & NMS mask size */
        DoubleEntry maskSizeFactor;
        /** The proportionality factor for the fitting mask size (the fitting
        *  mask determines which pixel are fitted with the exponential PSF
        *  model). */
        DoubleEntry fitSizeFactor;

        /** The method to use for spot detection. */
        simparm::NodeChoiceEntry<SpotFinderFactory> spotFindingMethod;
        /** The method to use for spot fitting. */
        simparm::NodeChoiceEntry< SpotFitterFactory > spotFittingMethod;

        /** If this option is set, the sigma estimation code is disabled. */
        BoolEntry fixSigma;

        /** Continue fitting until this number of bad fits occured. */
        UnsignedLongEntry motivation;
        /** Amplitude threshold to judge localizations by. */
        simparm::OptionalEntry< boost::units::quantity<
            cs_units::camera::intensity, float> > amplitude_threshold;

        typedef boost::units::quantity<cs_units::camera::length,unsigned long>
            Length;
        /** The smoothing/NMS mask size derived from the sigma value 
         *  in X direction */
        Length x_maskSize() const 
        { return Length(round( float(maskSizeFactor()) * sigma_x() ));}
        /** The smoothing/NMS mask size derived from the sigma value
         *  in Y direction */
        Length y_maskSize() const 
        { return Length(round( float(maskSizeFactor()) * sigma_y() ));}
        /** The fit window size derived from the sigma value in X direction */
        Length fitWidth() const 
        { return Length(round( float(fitSizeFactor()) * sigma_x() ));}
        /** The fit window size derived from the sigma value in Y direction */
        Length fitHeight() const 
        { return Length(round( float(fitSizeFactor()) * sigma_y() ));}

        /** Number of parallel computation threads to run. */
        UnsignedLongEntry pistonCount;

        void addSpotFinder( std::auto_ptr<SpotFinderFactory> factory );
        void addSpotFinder( SpotFinderFactory* factory ) 
            { addSpotFinder(std::auto_ptr<SpotFinderFactory>(factory)); }
        void addSpotFitter( std::auto_ptr<SpotFitterFactory> factory );
        void addSpotFitter( SpotFitterFactory* factory ) 
            { addSpotFitter(std::auto_ptr<SpotFitterFactory>(factory)); }

        void set_variables( output::Basename& ) const;
   };

   class Config : public _Config {
     private:
        /** Class changes the user level of sigma entries based on
         *  the setting of fixSigma. */
        class SigmaUserLevel : public Node::Callback {
            _Config& config;
            void adjust();
          public:
            SigmaUserLevel(_Config &config);
            void operator()(const Event&);
        };
        SigmaUserLevel user_level_watcher;
     public:
        Config();
        Config(const Config& c);
        virtual Config* clone() const { return new Config(*this); }
   };

}
}

#endif