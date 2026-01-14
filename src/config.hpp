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
      double freq;

      /// Phase, in deg.
      double phase;

      /// Planar wave angle, w.r.t. x-axis in xy-plane.
      double angle;
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

   /// Input base flow metadata.
   BaseFlowMeta base_flow_;

   /// Input source metadata.
   SourceMeta source_;

   /// Input preCICE metadata.
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

   /// Get reference to base flow metadata.
   BaseFlowMeta& BaseFlow() { return base_flow_; }

   /// Get const reference to base flow metadata.
   const BaseFlowMeta& BaseFlow() const { return base_flow_; }

   /// Get reference to source metadata.
   SourceMeta& Source() { return source_; }

   /// Get const reference to source metadata.
   const SourceMeta& Source() const { return source_; }

   /// Get reference to preCICE metadata.
   PreciceMeta& Precice() { return precice_; }

   /// Get const reference to preCICE metadata.
   const PreciceMeta& Precice() const { return precice_; }

   /// Print the configured base flow metadata.
   void PrintBaseFlowMeta(std::ostream &out) const;

   /// Print the configured source metadata.
   void PrintSourceMeta(std::ostream &out) const;

   /// Print the configured preCICE metadata.
   void PrintPreciceMeta(std::ostream &out) const;
};

} // namespace jabber

#endif // JABBER_CONFIG
