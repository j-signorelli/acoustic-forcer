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
/// Input XY data options.
enum class InputXY : std::uint8_t
{
   /// Provide x,y data directly in config file.
   Here,

   /// Read in x,y data from CSV file.
   FromCSV,

   /// Number of InputXYOptions.
   Size
};

/// Strings associated with InputXY enumerators.
static constexpr std::array<std::string_view, 
                     static_cast<std::size_t>(InputXY::Size)>
InputXYNames = 
{
   "Here",       // InputXY::Here
   "FromCSV",    // InputXY::FromCSV
};

template<InputXY i>
struct InputXYParams;

template<>
struct InputXYParams<InputXY::Here>
{
   /// Input x's.
   std::vector<double> x;

   /// Input y's.
   std::vector<double> y;
};

template<>
struct InputXYParams<InputXY::FromCSV>
{
   /// CSV file address. First column are x's, second are y's. No header.
   std::string file;
};

/// All input xy parameter options.
using InputXYParamsVariant 
   = std::variant<InputXYParams<InputXY::Here>,
                  InputXYParams<InputXY::FromCSV>>;
static_assert(std::variant_size_v<InputXYParamsVariant> == 
                    static_cast<std::size_t>(InputXY::Size),
             "Missing InputXYParams in InputXYParamsVariant.");

// ----------------------------------------------------------------------------
/// Input R->R function options.
enum class FunctionType : std::uint8_t
{
   /// Piecewise linear fit.
   PiecewiseLinear,

   /// Piecewise lo10-log10 fit (linear on log10-log10 scale).
   PiecewiseLogLog,

   /// Number of FunctionOptions.
   Size
};

/// Strings associated with FunctionType enumerators.
static constexpr std::array<std::string_view, 
                     static_cast<std::size_t>(FunctionType::Size)>
FunctionNames = 
{
   "PiecewiseLinear",    // FunctionType::PiecewiseLinear
   "PiecewiseLogLog",    // FunctionType::PiecewiseLogLog
};

template<FunctionType d>
struct FunctionTypeParams;

/// Parameters for FunctionType::PiecewiseLinear.
template<>
struct FunctionTypeParams<FunctionType::PiecewiseLinear>
{
   /// Input x,y data params.
   InputXYParamsVariant input_xy;
};

template<>
struct FunctionTypeParams<FunctionType::PiecewiseLogLog>
{
   /// Input x,y data params.
   InputXYParamsVariant input_xy;
};

/// All function parameter options.
using FunctionParamsVariant 
   = std::variant<FunctionTypeParams<FunctionType::PiecewiseLinear>,
                  FunctionTypeParams<FunctionType::PiecewiseLogLog>>;
static_assert(std::variant_size_v<FunctionParamsVariant> == 
                    static_cast<std::size_t>(FunctionType::Size),
             "Missing FunctionTypeParams in FunctionParamsVariant.");

// Array indicating if FunctionType has associated/implemented BasePSD type.
static constexpr std::array<bool, 
                     static_cast<std::size_t>(FunctionType::Size)>
FunctionHasPSDType = 
{
   true,    // FunctionType::PiecewiseLinear
   true,    // FunctionType::PiecewiseLogLog
};

// ----------------------------------------------------------------------------
/// Strings associated with jabber::Interval::Method enumerators.
static constexpr std::array<std::string_view, 
                     static_cast<std::size_t>(jabber::Interval::Method::Size)>
IntervalNames = 
{
   "Midpoint",      // Interval::Method::Midpoint
   "MidpointLog",   // Interval::Method::MidpointLog10
};

// ----------------------------------------------------------------------------
/**
 * @brief Discretization method options for discretization of a continuous
 * function (frequency selection of continuous PSD).
 * 
 */
enum class DiscMethod : std::uint8_t
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

/// Strings associated with DiscMethod enumerators.
static constexpr std::array<std::string_view, 
                     static_cast<std::size_t>(DiscMethod::Size)>
DiscMethodNames = 
{
   "Uniform",      // DiscMethod::Uniform
   "UniformLog",   // DiscMethod::UniformLog
   "Random",       // DiscMethod::Random
   "RandomLog",    // DiscMethod::RandomLog
};

template<DiscMethod d>
struct DiscMethodParams
{
   // Default struct has zero params.
};

/// Parameters for discretization method DiscMethod::Random.
template<>
struct DiscMethodParams<DiscMethod::Random>
{
   /// Seed to use in randomization.
   int seed;
};

/// Parameters for discretization method DiscMethod::RandomLog.
template<>
struct DiscMethodParams<DiscMethod::RandomLog>
{
   /// Seed to use in randomization.
   int seed;
};

/// All discretization method parameter options.
using DiscMethodParamsVariant 
   = std::variant<DiscMethodParams<DiscMethod::Uniform>,
                  DiscMethodParams<DiscMethod::UniformLog>,
                  DiscMethodParams<DiscMethod::Random>,
                  DiscMethodParams<DiscMethod::RandomLog>>;
static_assert(std::variant_size_v<DiscMethodParamsVariant> == 
                    static_cast<std::size_t>(DiscMethod::Size),
             "Missing DiscMethodParams in DiscMethodParamsVariant.");

// ----------------------------------------------------------------------------

/// Wave direction options.
enum class Direction : std::uint8_t
{
   /// Constant direction.
   Constant,

   /// Random angle in XY-plane from x-axis for each wave/frequency.
   RandomXYAngle,

   /// Number of DiscMethodOptions.
   Size
};

/// Strings associated with DiscMethod enumerators.
static constexpr std::array<std::string_view, 
                     static_cast<std::size_t>(Direction::Size)>
DirectionNames = 
{
   "Constant",         // Direction::Constant
   "RandomXYAngle",    // Direction::RandomXYAngle
};

template<Direction d>
struct DirectionParams;

/// Parameters for direction option Direction::Constant.
template<>
struct DirectionParams<Direction::Constant>
{
   /// Planar wave directional vector, can be non-normalized.
   std::vector<double> direction;
};

/// Parameters for direction option Direction::RandomXYAngle.
template<>
struct DirectionParams<Direction::RandomXYAngle>
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
   = std::variant<DirectionParams<Direction::Constant>,
                  DirectionParams<Direction::RandomXYAngle>>;
static_assert(std::variant_size_v<DirectionParamsVariant> == 
                    static_cast<std::size_t>(Direction::Size),
             "Missing DirectionParams in DirectionParamsVariant.");

// ----------------------------------------------------------------------------

/// Transfer function options.
enum class TransferFunction : std::uint8_t
{
   /// Do not use a transfer function.
   None,

   /**
    * @brief Analytical low-frequency limit transfer function, from Chaudhry
    * & Candler, 2017.
    * 
    */
   LowFrequencyLimit,

   /// Provide a transfer function.
   Input,

   /**
    * @brief Extrapolate from an approximate fit of the collapsed/normalized
    * flow-normal transfer function in Chaudhry & Candler, 2017.
    * 
    */
   FlowNormalFit,

   // Number of TransferOptions.
   Size
};

/// Strings associated with TransferFunction enumerators.
static constexpr std::array<std::string_view, 
                           static_cast<std::size_t>(TransferFunction::Size)>
TransferNames = 
{
   "None",               // TransferFunction::None
   "LowFrequencyLimit",  // TransferFunction::LowFrequencyLimit
   "Input",              // TransferFunction::Input
   "FlowNormalFit"       // TransferFunction::FlowNormalFit
};

template<TransferFunction t>
struct TransferParams
{
   // Default struct has zero params
};

template<>
struct TransferParams<TransferFunction::Input>
{
   /// Transfer function representation, (f, V^2).
   FunctionParamsVariant input_tf;
};

template<>
struct TransferParams<TransferFunction::FlowNormalFit>
{
   // TODO
};

/// All transfer parameter options.
using TransferParamsVariant 
   = std::variant<TransferParams<TransferFunction::None>,
                  TransferParams<TransferFunction::LowFrequencyLimit>,
                  TransferParams<TransferFunction::Input>,
                  TransferParams<TransferFunction::FlowNormalFit>>;
static_assert(std::variant_size_v<TransferParamsVariant> == 
                    static_cast<std::size_t>(TransferFunction::Size),
             "Missing TransferParams in TransferParamsVariant.");
// ----------------------------------------------------------------------------

/// Acoustic source options.
enum class Source : std::uint8_t
{
   /// Single acoustic wave.
   SingleWave,

   /// Spectrum of N acoustic waves.
   WaveSpectrum,

   /// Power spectral density (PSD).
   PSD,

   /// Read in CSV file of Wave data (output from \ref jabber::WriteWaves()).
   WaveCSV,

   /// Number of SourceOptions.
   Size
};

/// Strings associated with Source enumerators.
static constexpr std::array<std::string_view, 
                           static_cast<std::size_t>(Source::Size)>
SourceNames = 
{
   "SingleWave",      // Source::SingleWave
   "WaveSpectrum",    // Source::WaveSpectrum
   "PSD",             // Source::PSD
   "WaveCSV"          // Source::WaveCSV
};

template<Source s>
struct SourceParams;

/// Struct for source parameters of single acoustic wave.
template<>
struct SourceParams<Source::SingleWave>
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
struct SourceParams<Source::WaveSpectrum>
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

/// Struct for source parameters of PSD.
template<>
struct SourceParams<Source::PSD>
{

   /// PSD function representation (f, PSD).
   FunctionParamsVariant input_psd;

   /// (For PSD unit V^2/Hz) Scaling factor to multiply V by.
   double dim_fac;

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

   /// Transfer function parameters.
   TransferParamsVariant tf_params;
};

template<>
struct SourceParams<Source::WaveCSV>
{
   /// Wave CSV file (output from \ref jabber::WriteWaves()).
   std::string file;
};

/// All source parameter options.
using SourceParamsVariant 
   = std::variant<SourceParams<Source::SingleWave>,
                  SourceParams<Source::WaveSpectrum>,
                  SourceParams<Source::PSD>,
                  SourceParams<Source::WaveCSV>>;
static_assert(std::variant_size_v<SourceParamsVariant> == 
                    static_cast<std::size_t>(Source::Size),
             "Missing SourceParams in SourceParamsVariant.");

// ----------------------------------------------------------------------------
/// Strings associated with jabber::AcousticField::Kernel enumerators.
static constexpr std::array<std::string_view, 
                     static_cast<std::size_t>(jabber::AcousticField::Kernel::Size)>
KernelNames = 
{
   "GridPoint",      // AcousticField::Kernel::GridPoint
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
