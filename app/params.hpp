#ifndef JABBER_APP_PARAMS
#define JABBER_APP_PARAMS

#include <cstdint>
#include <vector>
#include <string>
#include <variant>
#include <array>

namespace jabber_app
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

} // namespace jabber_app

#endif // JABBER_APP_PARAMS
