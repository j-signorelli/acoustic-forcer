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
   struct BaseFlowParams
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

   /// Struct for source parameters of single acoustic wave.
   struct SingleWaveParams
   {
      /// Wave amplitude.
      double amp;

      /// Wave frequency (not angular).
      double freq;

      /// Phase, in deg.
      double phase;

      /// Planar wave angle, w.r.t. x-axis in xy-plane.
      double angle;

      /// True if wave is "slow", false if "fast".
      bool slow;
   };

   /// All souce parameter options.
   using SourceParams = std::variant<SingleWaveParams>;


   /// Struct for computation parameters.
   struct CompParams
   {
      /// Initial time.
      double t0;
   };

   /// Struct for preCICE parameters.
   struct PreciceParams
   {

      /// Jabber participant name.
      std::string participant_name;

      /// Address to preCICE config file.
      std::string config_file;

      /// Name of mesh to get coordinates from for computation onto.
      std::string fluid_mesh_name;

      /**
       * @brief Mesh access region, defined according to 
       * precice::Participant::setMeshAccessRegion()
       */
      std::vector<double> mesh_access_region;
   };

private:

   /// Input base flow parameters.
   BaseFlowParams base_flow_;

   /// Input source parameters.
   SourceParams source_;

   /// Input computation parameters.
   CompParams comp_;

   /// Input preCICE parameters.
   PreciceParams precice_;

public:

   /**
    * @brief Construct a new Config object.
    * 
    * @param config_file      TOML config file to parse.
    * @param out              [Optional] ostream to write parsed config file
    *                         to (verbose processing).
    */
   Config(std::string config_file, std::ostream *out=nullptr);

   /// Get reference to base flow parameters.
   BaseFlowParams& BaseFlow() { return base_flow_; }

   /// Get const reference to base flow parameters.
   const BaseFlowParams& BaseFlow() const { return base_flow_; }

   /// Get reference to source parameters.
   SourceParams& Source() { return source_; }

   /// Get const reference to source parameters.
   const SourceParams& Source() const { return source_; }

   /// Get reference to computation parameters.
   CompParams& Comp() { return comp_; }

   /// Get const reference to computation parameters.
   const CompParams& Comp() const { return comp_; }

   /// Get reference to preCICE parameters.
   PreciceParams& Precice() { return precice_; }

   /// Get const reference to preCICE parameters.
   const PreciceParams& Precice() const { return precice_; }

   /// Print the configured base flow parameters.
   void PrintBaseFlowParams(std::ostream &out) const;

   /// Print the configured source parameters.
   void PrintSourceParams(std::ostream &out) const;

   /// Print the configured computation parameters.
   void PrintCompParams(std::ostream &out) const;

   /// Print the configured preCICE parameters.
   void PrintPreciceParams(std::ostream &out) const;
};

} // namespace jabber

#endif // JABBER_CONFIG
