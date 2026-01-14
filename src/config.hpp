#ifndef JABBER_CONFIG
#define JABBER_CONFIG

#include <vector>
#include <string>
#include <variant>
#include <iostream>

namespace jabber
{

/// Parsed config file input
class Config
{
public:

   /// Struct for base flow parameters
   struct BaseFlowMeta
   {
      /// Density.
      double rho;

      /// Pressure.
      double p;

      /// Velocity (in x-direction).
      double U;

      /// Specific heat ratio.
      double gamma;
   };

   /// Struct for source metadata of single acoustic wave.
   struct SingleWaveMeta
   {
      /// Wave amplitude.
      double amp;

      /// Wave frequency (not angular).
      double freq;  // not angular
      double phase; // in deg.
      double angle; // wrt x-axis in xy-plane, deg.
   };

   /// All souce meta options.
   using SourceMeta = std::variant<SingleWaveMeta>;

   /// Struct for preCICE metadata.
   struct PreciceMeta
   {
      std::string participant_name;
      std::string config_file;
      std::string fluid_mesh_name;

      /**
       * @brief Mesh access region, defined according to 
       * precice::Participant::setMeshAccessRegion()
       */
      std::vector<double> mesh_access_region;
   };

private:
   BaseFlowMeta base_flow_;
   SourceMeta source_;
   PreciceMeta precice_;

public:

   /**
    * @brief Construct a new Config object.
    * 
    * @param config_file      TOML config file to parse.
    * @param out              [Optional] ostream to write parsed config file
    *                         to (verbose processing).
    */
   Config(std::string config_file, std::ostream *out=nullptr);

   BaseFlowMeta& BaseFlow() { return base_flow_; }
   const BaseFlowMeta& BaseFlow() const { return base_flow_; }

   SourceMeta& Source() { return source_; }
   const SourceMeta& Source() const { return source_; }

   PreciceMeta& Precice() { return precice_; }
   const PreciceMeta& Precice() const { return precice_; }

   void PrintBaseFlowMeta(std::ostream &out) const;

   void PrintSourceMeta(std::ostream &out) const;

   void PrintPreciceMeta(std::ostream &out) const;
};

} // namespace jabber

#endif // JABBER_CONFIG
