#ifndef JABBER_APP_PARAMS
#define JABBER_APP_PARAMS

#include <cstdint>
#include <vector>
#include <string>
#include <variant>
#include <array>
#include <concepts>
#include <utility>

#include <jabber.hpp>

namespace jabber_app
{

// ----------------------------------------------------------------------------
// Define concepts and base templated types and variables

/**
 * @brief Concept for enums of type std::uint8_t with a \c T::Size enumerator
 * (note that \c T::Size should be the last enumerator).
 */
template<typename T>
concept OptionEnum = 
         std::is_enum_v<T> && 
         std::same_as<std::underlying_type_t<T>, std::uint8_t> &&
         requires { T::Size; };

/// Concept for OptionEnum enumerators.
template<auto V>
concept OptionEnumerator = OptionEnum<decltype(V)>;

/// Define alias for an option array type for readability.
template<typename T, OptionEnum E>
using OptionArray = std::array<T,static_cast<std::size_t>(E::Size)>;
     
/**
 * @brief Templated array variable for all option names.
 * 
 * @details This is an array of names for \p E. Names can be accessed
 * by \c OptionNames<E>[0] for example.
 *  
 * @tparam E      A \ref OptionEnum.
 */
template<OptionEnum E>
constexpr OptionArray<std::string_view, E> OptionNames;

/**
 * @brief Templated string variable for a single option name
 * 
 * @details A name can be retrieved by \c OptionName<V> for example.
 * 
 * @tparam V   An \ref OptionEnumerator.
 */
template<auto V>
   requires OptionEnumerator<V>
constexpr std::string_view OptionName = 
[](){
   return OptionNames<decltype(V)>[static_cast<std::uint8_t>(V)];
}();

/**
 * @brief Base templated type for a specific option parameters, 
 * associated with \ref OptionEnumerator \p V.
 * 
 * @tparam V     A \ref OptionEnumerator.
 */
template<auto V>
   requires OptionEnumerator<V>
struct OptionParams;

/**
 * @brief A \c std::variant of all option parameters of \p E. Note
 * that there must be an explicitly-defined \ref OptionParams for each one,
 * otherwise this may fail.
 * 
 * @details The type is defined using template-metaprogramming to return a
 * \c std::variant of correct types, and then a decltype is called on that.
 * 
 * @tparam E     An \ref OptionEnum.
 */
template<OptionEnum E>
using OptionParamsVar = 
   decltype([]<std::size_t... Is>(std::index_sequence<Is...>) 
            { 
               return std::variant<OptionParams<static_cast<E>(Is)>...>{};
            }
            (std::make_index_sequence<static_cast<std::size_t>(E::Size)>()));

// ----------------------------------------------------------------------------
// Define main settings:
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

/// Strings associated with jabber::AcousticField::Kernel enumerators.
template<>
inline constexpr OptionArray<std::string_view, jabber::AcousticField::Kernel> 
   OptionNames<jabber::AcousticField::Kernel> 
{
   "GridPoint",      // AcousticField::Kernel::GridPoint
};

/// Struct for computation parameters.
struct CompParams
{  
   /// Initial time.
   double t0;

   /// Kernel type.
   jabber::AcousticField::Kernel kernel;
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

// ----------------------------------------------------------------------------

// Now define all actual option enums, names, and parameters.

/// Input XY data options.
enum class InputXY : std::uint8_t
{
   /// Provide x,y data directly in config file.
   Here,

   /// Read in x,y data from CSV file.
   FromCSV,

   /// Number of InputXY enumerators.
   Size
};

/// Strings associated with InputXY enumerators.
template<>
inline constexpr OptionArray<std::string_view, InputXY> OptionNames<InputXY>
{
   "Here",       // InputXY::Here
   "FromCSV",    // InputXY::FromCSV
};

template<>
struct OptionParams<InputXY::Here>
{
   /// Input x's.
   std::vector<double> x;

   /// Input y's.
   std::vector<double> y;
};

template<>
struct OptionParams<InputXY::FromCSV>
{
   /// CSV file address. First column are x's, second are y's. No header.
   std::string file;
};

// ----------------------------------------------------------------------------
/// Input R->R function options.
enum class FunctionType : std::uint8_t
{
   /// Piecewise linear fit.
   PiecewiseLinear,

   /// Piecewise lo10-log10 fit (linear on log10-log10 scale).
   PiecewiseLogLog,

   /// Number of FunctionType enumerators.
   Size
};

/// Strings associated with FunctionType enumerators.
template<>
inline constexpr OptionArray<std::string_view, FunctionType> 
   OptionNames<FunctionType> 
{
   "PiecewiseLinear",    // FunctionType::PiecewiseLinear
   "PiecewiseLogLog",    // FunctionType::PiecewiseLogLog
};

/// Parameters for FunctionType::PiecewiseLinear.
template<>
struct OptionParams<FunctionType::PiecewiseLinear>
{
   /// Input x,y data params.
   OptionParamsVar<InputXY> input_xy;
};

template<>
struct OptionParams<FunctionType::PiecewiseLogLog>
{
   /// Input x,y data params.
   OptionParamsVar<InputXY> input_xy;
};

/// Array indicating if FunctionType has associated/implemented BasePSD type.
static constexpr OptionArray<bool, FunctionType> FunctionTypeHasPSDType = 
{
   true,    // FunctionType::PiecewiseLinear
   true,    // FunctionType::PiecewiseLogLog
};

// ----------------------------------------------------------------------------

/// Strings associated with jabber::Interval::Method enumerators.
template<>
inline constexpr OptionArray<std::string_view, jabber::Interval::Method> 
   OptionNames<jabber::Interval::Method>
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

   /// Number of DiscMethod enumerators.
   Size
};

/// Strings associated with DiscMethod enumerators.
template<>
inline constexpr OptionArray<std::string_view, DiscMethod> OptionNames<DiscMethod>
{
   "Uniform",      // DiscMethod::Uniform
   "UniformLog",   // DiscMethod::UniformLog
   "Random",       // DiscMethod::Random
   "RandomLog",    // DiscMethod::RandomLog
};

/// Parameters for discretization method DiscMethod::Uniform.
template<>
struct OptionParams<DiscMethod::Uniform>
{
   // No parameters.
};

/// Parameters for discretization method DiscMethod::UniformLog.
template<>
struct OptionParams<DiscMethod::UniformLog>
{
   // No parameters.
};

/// Parameters for discretization method DiscMethod::Random.
template<>
struct OptionParams<DiscMethod::Random>
{
   /// Seed to use in randomization.
   int seed;
};

/// Parameters for discretization method DiscMethod::RandomLog.
template<>
struct OptionParams<DiscMethod::RandomLog>
{
   /// Seed to use in randomization.
   int seed;
};

// ----------------------------------------------------------------------------

/// Wave direction options.
enum class Direction : std::uint8_t
{
   /// Constant direction.
   Constant,

   /// Random angle in XY-plane from x-axis for each wave/frequency.
   RandomXYAngle,

   /// Number of Direction enumerators.
   Size
};

/// Strings associated with Direction enumerators.
template<>
inline constexpr OptionArray<std::string_view, Direction> OptionNames<Direction>
{
   "Constant",         // Direction::Constant
   "RandomXYAngle",    // Direction::RandomXYAngle
};

/// Parameters for direction option Direction::Constant.
template<>
struct OptionParams<Direction::Constant>
{
   /// Planar wave directional vector, can be non-normalized.
   std::vector<double> direction;
};

/// Parameters for direction option Direction::RandomXYAngle.
template<>
struct OptionParams<Direction::RandomXYAngle>
{
   /// Min angle of wave from x-axis, in XY-plane (CCW+, CW-) (in degrees).
   double min_angle;

   /// Max angle of wave from x-axis, in XY-plane (CCW+, CW-) (in degrees).
   double max_angle;

   /// Seed to use in randomization.
   int seed;
};

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

   // Number of TransferFunction enumerators.
   Size
};

/// Strings associated with TransferFunction enumerators.
template<>
inline constexpr OptionArray<std::string_view, TransferFunction> 
   OptionNames<TransferFunction>
{
   "None",               // TransferFunction::None
   "LowFrequencyLimit",  // TransferFunction::LowFrequencyLimit
   "Input",              // TransferFunction::Input
   "FlowNormalFit"       // TransferFunction::FlowNormalFit
};

template<>
struct OptionParams<TransferFunction::None>
{
   // No parameters.
};

template<>
struct OptionParams<TransferFunction::LowFrequencyLimit>
{
   // No parameters.
};

template<>
struct OptionParams<TransferFunction::Input>
{
   /// Transfer function representation, (f, V^2).
   OptionParamsVar<FunctionType> input_tf;
};

template<>
struct OptionParams<TransferFunction::FlowNormalFit>
{
   // TODO
};

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

   /// Number of Source enumerators.
   Size
};

/// Strings associated with Source enumerators.
template<>
inline constexpr OptionArray<std::string_view, Source> OptionNames<Source>
{
   "SingleWave",      // Source::SingleWave
   "WaveSpectrum",    // Source::WaveSpectrum
   "PSD",             // Source::PSD
   "WaveCSV"          // Source::WaveCSV
};


/// Struct for source parameters of single acoustic wave.
template<>
struct OptionParams<Source::SingleWave>
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
struct OptionParams<Source::WaveSpectrum>
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
struct OptionParams<Source::PSD>
{

   /// PSD function representation (f, PSD).
   OptionParamsVar<FunctionType> input_psd;

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
   OptionParamsVar<DiscMethod> disc_params;

   /// Direction method parameters.
   OptionParamsVar<Direction> dir_params;

   /// Seed to use for wave phase randomization.
   int phase_seed;

   /// Wave speeds to use.
   char speed;

   /// Transfer function parameters.
   OptionParamsVar<TransferFunction> tf_params;
};

template<>
struct OptionParams<Source::WaveCSV>
{
   /// Wave CSV file (output from \ref jabber::WriteWaves()).
   std::string file;
};

// ----------------------------------------------------------------------------
} // namespace jabber_app

#endif // JABBER_APP_PARAMS
