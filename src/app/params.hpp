#ifndef JABBER_APP_PARAMS
#define JABBER_APP_PARAMS

#include <cstdint>
#include <vector>
#include <string>
#include <variant>
#include <array>

#include <jabber.hpp>

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

   /// Velocity vector.
   std::vector<double> U;

   /// Specific heat ratio.
   double gamma;
};

// ----------------------------------------------------------------------------
/// Input digitized power spectral density data options.
enum class PSDInputOption : std::uint8_t
{
   /// Read in frequencies and PSDs specified directly in the config file.
   Here,

   /// Read in frequencies and PSDs from CSV file.
   FromCSV,

   /// Number of PSDInputOptions.
   Size
};

/// Strings associated with PSDInputOption enumerators.
static constexpr std::array<std::string_view, 
                     static_cast<std::size_t>(PSDInputOption::Size)>
PSDInputNames = 
{
   "Here",       // PSDInputOption::Here
   "FromCSV",    // PSDInputOption::FromCSV
};

template<PSDInputOption d>
struct PSDInputParams;

/// Parameters for PSDInputOption::Here.
template<>
struct PSDInputParams<PSDInputOption::Here>
{
   /// Digital wave frequencies (not angular) to fit.
   std::vector<double> freqs;

   /// Digital wave power spectral densities to fit.
   std::vector<double> psds;
};

template<>
struct PSDInputParams<PSDInputOption::FromCSV>
{
   /// CSV file address. First column are frequencies, second are PSDs.
   std::string file;
};

/// All discretization method parameter options.
using PSDInputParamsVariant 
   = std::variant<PSDInputParams<PSDInputOption::Here>,
                  PSDInputParams<PSDInputOption::FromCSV>>;
static_assert(std::variant_size_v<PSDInputParamsVariant> == 
                    static_cast<std::size_t>(PSDInputOption::Size),
             "Missing PSDInputParams in PSDInputParamsVariant.");

// ----------------------------------------------------------------------------
/// Interpolation options.
enum class InterpolationOption : std::uint8_t
{

   /// Piecewise linear fit.
   PiecewiseLinear,

   /// Piecewise lo10-log10 fit (linear on log10-log10 scale).
   PiecewiseLogLog,

   /// Number of InterpolationOptions.
   Size
};

/// Strings associated with InterpolationOption enumerators.
static constexpr std::array<std::string_view, 
                     static_cast<std::size_t>(InterpolationOption::Size)>
InterpolationNames = 
{
   "PiecewiseLinear",    // InterpolationOption::PiecewiseLinear
   "PiecewiseLogLog",    // InterpolationOption::PiecewiseLogLog
};

// ----------------------------------------------------------------------------
/// Strings associated with jabber::Interval::Method enumerators.
static constexpr std::array<std::string_view, 
                     static_cast<std::size_t>(jabber::Interval::Method::Size)>
IntervalNames = 
{
   "Midpoint",      // Interval::Method::Midpoint
   "MidpointLog",    // Interval::Method::MidpointLog10
};

// ----------------------------------------------------------------------------
/**
 * @brief Discretization method options for discretization of a continuous
 * function (frequency selection of continuous PSD).
 * 
 */
enum class DiscMethodOption : std::uint8_t
{
   /**
    * @brief Uniformly sample across interval. **Harmonic interaction
    * may occur**.
    */
   Uniform,

   /**
    * @brief Uniformly sample across interval in log10 scaling. **Harmonic
    * interaction may occur**.
    */
   UniformLog,

   /// Random sampling of uniform distribution.
   Random,

   /// Random sampling of uniform distribution but on log10 scale.
   RandomLog,

   /// Number of DiscMethodOptions.
   Size
};

/// Strings associated with DiscMethodOption enumerators.
static constexpr std::array<std::string_view, 
                     static_cast<std::size_t>(DiscMethodOption::Size)>
DiscMethodNames = 
{
   "Uniform",      // DiscMethodOption::Uniform
   "UniformLog",   // DiscMethodOption::UniformLog
   "Random",       // DiscMethodOption::Random
   "RandomLog",    // DiscMethodOption::RandomLog
};

template<DiscMethodOption d>
struct DiscMethodParams
{
   // Default struct has zero params.
};

/// Parameters for discretization method DiscMethodOption::Random.
template<>
struct DiscMethodParams<DiscMethodOption::Random>
{
   /// Seed to use in randomization.
   int seed;
};

/// Parameters for discretization method DiscMethodOption::RandomLog.
template<>
struct DiscMethodParams<DiscMethodOption::RandomLog>
{
   /// Seed to use in randomization.
   int seed;
};

/// All discretization method parameter options.
using DiscMethodParamsVariant 
   = std::variant<DiscMethodParams<DiscMethodOption::Uniform>,
                  DiscMethodParams<DiscMethodOption::UniformLog>,
                  DiscMethodParams<DiscMethodOption::Random>,
                  DiscMethodParams<DiscMethodOption::RandomLog>>;
static_assert(std::variant_size_v<DiscMethodParamsVariant> == 
                    static_cast<std::size_t>(DiscMethodOption::Size),
             "Missing DiscMethodParams in DiscMethodParamsVariant.");

// ----------------------------------------------------------------------------

/// Wave direction options.
enum class DirectionOption : std::uint8_t
{

   /// Constant direction.
   Constant,

   /// Random angle in XY-plane from x-axis for each wave/frequency.
   RandomXYAngle,

   /// Number of DiscMethodOptions.
   Size
};

/// Strings associated with DiscMethodOption enumerators.
static constexpr std::array<std::string_view, 
                     static_cast<std::size_t>(DirectionOption::Size)>
DirectionNames = 
{
   "Constant",         // DirectionOption::Constant
   "RandomXYAngle",    // DirectionOption::RandomXYAngle
};

template<DirectionOption d>
struct DirectionParams;

/// Parameters for direction option DirectionOption::Constant.
template<>
struct DirectionParams<DirectionOption::Constant>
{
   /// Planar wave directional vector, can be non-normalized.
   std::vector<double> direction;
};

/// Parameters for direction option DirectionOption::RandomXYAngle.
template<>
struct DirectionParams<DirectionOption::RandomXYAngle>
{
   /// Min angle of wave from x-axis, in XY-plane (CCW+, CW-) (in degrees).
   double min_angle;

   /// Max angle of wave from x-axis, in XY-plane (CCW+, CW-) (in degrees).
   double max_angle;

   /// Seed to use in randomization.
   int seed;
};

/// All direction parameter options.
using DirectionParamsVariant 
   = std::variant<DirectionParams<DirectionOption::Constant>,
                  DirectionParams<DirectionOption::RandomXYAngle>>;
static_assert(std::variant_size_v<DirectionParamsVariant> == 
                    static_cast<std::size_t>(DirectionOption::Size),
             "Missing DirectionParams in DirectionParamsVariant.");

// ----------------------------------------------------------------------------

/// Transfer function options.
enum class TransferOption : std::uint8_t
{
   /// Do not use a transfer function.
   None,

   /**
    * @brief Analytical low-frequency limit transfer function, from Chaudhry
    * & Candler, 2017.
    * 
    */
   LowFrequencyLimit,

   /// Digitized transfer function.
   DigitalTF,

   /**
    * @brief Extrapolate from an approximate fit of the collapsed/normalized
    * flow-normal transfer function in Chaudhry & Candler, 2017.
    * 
    */
   FlowNormalExtrapolate,

   // Number of TransferOptions.
   Size
};

/// Strings associated with TransferOption enumerators.
static constexpr std::array<std::string_view, 
                           static_cast<std::size_t>(TransferOption::Size)>
TransferNames = 
{
   "None",                      // TransferOption::None
   "LowFrequencyLimit",         // TransferOption::LowFrequencyLimit
   "DigitalTF",                 // TransferOption::DigitalTF
   "FlowNormalExtrapolate"      // TransferOption::FlowNormalExtrapolate
};

// ----------------------------------------------------------------------------

/// Acoustic source options.
enum class SourceOption : std::uint8_t
{
   /// Single acoustic wave.
   SingleWave,

   /// Spectrum of N acoustic waves.
   WaveSpectrum,

   /// Digitized/discrete power spectral density (PSD).
   DigitalPSD,

   /// Read in CSV file of Wave data (output from \ref jabber::WriteWaves()).
   WaveCSV,

   /// Number of SourceOptions.
   Size
};

/// Strings associated with SourceOption enumerators.
static constexpr std::array<std::string_view, 
                           static_cast<std::size_t>(SourceOption::Size)>
SourceNames = 
{
   "SingleWave",      // SourceOption::SingleWave
   "WaveSpectrum",    // SourceOption::WaveSpectrum
   "DigitalPSD",      // SourceOption::DigitalPSD
   "WaveCSV"          // SourceOption::WaveCSV
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

   /// Planar wave directional vector, can be non-normalized.
   std::vector<double> direction;

   /// Wave speed ('S' or 'F').
   char speed;
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

   /// Planar wave directional vector, can be non-normalized.
   std::vector<std::vector<double>> directions;

   /// Wave speeds ('S' or 'F').
   std::vector<char> speeds;
};

/// Struct for source parameters of digital PSD.
template<>
struct SourceParams<SourceOption::DigitalPSD>
{

   /// Digitized data (frequencies and PSDs) input parameters.
   PSDInputParamsVariant input_params;

   /// (For PSD unit V^2/Hz) Factor to multiply V by for pressure amplitude.
   double dim_fac;

   /// Interpolation method for fitting continuous curve to discrete PSD data.
   InterpolationOption interp;

   /// Minimum wave frequency in discrete frequency selection range.
   double min_disc_freq;

   /// Maximum wave frequency in discrete frequency selection range.
   double max_disc_freq;

   /// Number of waves to discretize PSD to.
   std::size_t num_waves;

   /// Interval method to use for frequency bin width.
   jabber::Interval::Method int_method;

   /// Discretization method parameters.
   DiscMethodParamsVariant disc_params;

   /// Direction method parameters.
   DirectionParamsVariant dir_params;

   /// Seed to use for wave phase randomization.
   int phase_seed;

   /// Wave speeds to use.
   char speed;
};

template<>
struct SourceParams<SourceOption::WaveCSV>
{
   /// Wave CSV file (output from \ref jabber::WriteWaves()).
   std::string file;
};

/// All source parameter options.
using SourceParamsVariant 
   = std::variant<SourceParams<SourceOption::SingleWave>,
                  SourceParams<SourceOption::WaveSpectrum>,
                  SourceParams<SourceOption::DigitalPSD>,
                  SourceParams<SourceOption::WaveCSV>>;
static_assert(std::variant_size_v<SourceParamsVariant> == 
                    static_cast<std::size_t>(SourceOption::Size),
             "Missing SourceParams in SourceParamsVariant.");

// ----------------------------------------------------------------------------
/// Strings associated with jabber::AcousticField::Kernel enumerators.
static constexpr std::array<std::string_view, 
                     static_cast<std::size_t>(jabber::AcousticField::Kernel::Size)>
KernelNames = 
{
   "GridPoint",      // AcousticField::Kernel::GridPoint
   "Wave",               // AcousticField::Kernel::Wave
};

// ----------------------------------------------------------------------------
/// Struct for computation parameters.
struct CompParams
{  
   /// Initial time.
   double t0;

   /// Kernel type.
   jabber::AcousticField::Kernel kernel;
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
