#ifndef JABBER_CONFIG
#define JABBER_CONFIG

#include <vector>
#include <string>
#include <variant>
#include <iostream>
#include <cstdint>
#include <array>
namespace jabber
{

namespace config
{

// ----------------------------------------------------------------------------
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

// ----------------------------------------------------------------------------

/// Source options.
enum class SourceOption : std::uint8_t
{
   SingleWave,
   Size
};

/// Strings associated with SourceOption enumerators.
static constexpr std::array<std::string_view, 
                           static_cast<std::size_t>(SourceOption::Size)>
SourceNames = 
{
   "SingleWave"      // Source::SingleWave
};

template<SourceOption s>
struct SourceParams;

/// Struct for source parameters of single acoustic wave.
template<>
struct SourceParams<SourceOption::SingleWave>
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
using SourceParamsVariant 
   = std::variant<SourceParams<SourceOption::SingleWave>>;

// ----------------------------------------------------------------------------

/// Data options to compute + write.
enum class DataOption : std::uint8_t
{
   PressurePerturbation,
   Density,
   Momentum,
   Energy,
   Size
};

/// Strings associated with Data enumerators.
static constexpr std::array<std::string_view, 
                              static_cast<std::size_t>(DataOption::Size)>
DataNames = 
{
   "p'",    // Data::PressurePerturbation
   "rho",   // Data::Density
   "rhoV",  // Data::Momentum
   "rhoE"   // Data::Energy
};

// ----------------------------------------------------------------------------

/// Nondimensionalization options.
enum class NondimensionalOption : std::uint8_t
{
   None,
   PlasCom2,
   Size
};

/// Strings associated with Nondimensionalization enumerators.
static constexpr std::array<std::string_view, 
                        static_cast<std::size_t>(NondimensionalOption::Size)>
NondimensionalNames = 
{
   "None",      // Nondimensionalization::None
   "PlasCom2"   // Nondimensionalization::PlasCom2
};

// ----------------------------------------------------------------------------

/// Struct for computation parameters.
struct CompParams
{  

   /// Initial time.
   double t0;

   /// Data to compute + write.
   std::vector<DataOption> data;

   /// Data nondimensionalization.
   NondimensionalOption non_dim;
};
// ----------------------------------------------------------------------------

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

// ----------------------------------------------------------------------------

/// Parsed config file input
class ConfigInput
{
protected:

   /// Input base flow parameters.
   BaseFlowParams base_flow_;

   /// Input source parameters.
   SourceParamsVariant source_;

   /// Input computation parameters.
   CompParams comp_;

   /// Input preCICE parameters.
   PreciceParams precice_;

public:

   /// Get reference to base flow parameters.
   BaseFlowParams& BaseFlow() { return base_flow_; }

   /// Get const reference to base flow parameters.
   const BaseFlowParams& BaseFlow() const { return base_flow_; }

   /// Get reference to source parameters.
   SourceParamsVariant& Source() { return source_; }

   /// Get const reference to source parameters.
   const SourceParamsVariant& Source() const { return source_; }

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

/// Parsed TOML config file input.
class TOMLConfigInput : public ConfigInput
{
public:
   /**
    * @brief Construct a new TOMLConfigInput object.
    * 
    * @param config_file      TOML config file to parse.
    * @param out              [Optional] ostream to write parsed config file
    *                         to (verbose processing).
    */
   TOMLConfigInput(std::string config_file, std::ostream *out=nullptr);
};

} // namespace config

} // namespace jabber

#endif // JABBER_CONFIG
