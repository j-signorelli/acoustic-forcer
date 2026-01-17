#ifndef JABBER_CONFIG
#define JABBER_CONFIG

#include <vector>
#include <string>
#include <variant>
#include <iostream>
#include <cstdint>
#include <array>
#include <format>

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

/// Acoustic wave speed options.
enum class SpeedOption : std::uint8_t
{
   /**
    * @brief "Slow" acoustic wave. Wavenumber vector should be computed
    *  assuming wave travels *against* freestream (wave speed < freestream).
    */
   Slow,

   /**
    * @brief "Fast" acoustic wave. Wavenumber vector should be computed
    *  assuming wave travels *with* freestream (wave speed > freestream).
    */
   Fast,

   /// Number of speed options. **This should always be exactly 2.**
   Size
};

/// Strings associated with SpeedOption enumerators.
static constexpr std::array<std::string_view, 
                           static_cast<std::size_t>(SpeedOption::Size)>
SpeedNames = 
{
   "Slow",      // SpeedOption::Slow
   "Fast",      // SpeedOption::Fast
};

/// Acoustic source options.
enum class SourceOption : std::uint8_t
{
   /// Single acoustic wave.
   SingleWave,

   /// Spectrum of N acoustic waves.
   WaveSpectrum,

   /// Number of SourceOptions
   Size
};

/// Strings associated with SourceOption enumerators.
static constexpr std::array<std::string_view, 
                           static_cast<std::size_t>(SourceOption::Size)>
SourceNames = 
{
   "SingleWave",      // SourceOption::SingleWave
   "WaveSpectrum",    // SourceOption::WaveSpectrum
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

   /// Wave speed.
   SpeedOption speed;
};

/// Struct for source parameters of spectrum of acoustic waves.
template<>
struct SourceParams<SourceOption::WaveSpectrum>
{
   /// Wave amplitudes.
   std::vector<double> amps;

   /// Wave frequencies (not angular).
   std::vector<double> freqs;

   /// Phases, in deg.
   std::vector<double> phases;

   /// Planar wave angles w.r.t. x-axis in xy-plane.
   std::vector<double> angles;

   /// Wave speeds.
   std::vector<SpeedOption> speeds;
};

/// All souce parameter options.
using SourceParamsVariant 
   = std::variant<SourceParams<SourceOption::SingleWave>,
                  SourceParams<SourceOption::WaveSpectrum>>;

// ----------------------------------------------------------------------------

/// Data options to compute *and* write to preCICE.
enum class DataOption : std::uint8_t
{
   /// Pressure perturbation, p'. 
   PressurePerturbation,

   /// Density, ρ.
   Density,

   /// Momentum, ρu.
   Momentum,

   /// Energy, ρE.
   Energy,

   /// Number of DataOptions.
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

/// Nondimensionalization options to apply before sending to preCICE.
enum class NondimensionalOption : std::uint8_t
{
   /// No nondimensionalization.
   None,

   /// Nondimensionalize using same method as in PlasCom2.
   PlasCom2,

   /// Number of NondimensionalOptions.
   Size
};

/// Strings associated with Nondimensionalization enumerators.
static constexpr std::array<std::string_view, 
                        static_cast<std::size_t>(NondimensionalOption::Size)>
NondimensionalNames = 
{
   "None",      // NondimensionalOption::None
   "PlasCom2"   // NondimensionalOption::PlasCom2
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
private:
   /// Precision used when printing double/float data in \ref OutReal().
   static constexpr int kPrecision = 12;

   /// Get string of double \p f with precision \ref kPrecision.
   static std::string OutReal(double f)
   {
      return std::format("{:.{}f}", f, kPrecision);
   }

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
