#ifndef JABBER_APP_PARAMS
#define JABBER_APP_PARAMS

#include <cstdint>
#include <vector>
#include <string>
#include <variant>
#include <array>
#include <optional>

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
struct InputXY
{
   /// Input XY data options.
   enum class Option : std::uint8_t
   {
      /// Provide x,y data directly in config file.
      Here,

      /// Read in x,y data from CSV file.
      FromCSV,

      /// Number of InputXY options.
      Size
   };

   using enum Option;

   /// Strings associated with InputXY enumerators.
   static constexpr std::array<std::string_view, 
                     static_cast<std::size_t>(Size)>
   kNames = 
   {
      "Here",       // Here
      "FromCSV",    // FromCSV
   };


   template<Option V>
   struct Params;

   template<>
   struct Params<Here>
   {
      /// Input x's.
      std::vector<double> x;

      /// Input y's.
      std::vector<double> y;
   };

   template<>
   struct Params<FromCSV>
   {
      /// CSV file address. First column are x's, second are y's. No header.
      std::string file;
   };

   /// Parameter variant type for InputXY.
   using ParamsVariant = std::variant<Params<Here>,Params<FromCSV>>;
};

// ----------------------------------------------------------------------------
struct FunctionType
{
   /// Input R->R function options.
   enum class Option : std::uint8_t
   {
      /// Piecewise linear fit.
      PiecewiseLinear,

      /// Piecewise lo10-log10 fit (linear on log10-log10 scale).
      PiecewiseLogLog,

      /// Number of FunctionType options.
      Size
   };

   using enum Option;

   /// Strings associated with FunctionType enumerators.
   static constexpr std::array<std::string_view, 
                        static_cast<std::size_t>(Size)>
   kNames = 
   {
      "PiecewiseLinear",    // PiecewiseLinear
      "PiecewiseLogLog",    // PiecewiseLogLog
   };


   template<Option V>
   struct Params;

   template<>
   struct Params<PiecewiseLinear>
   {
      /// Input x,y data params.
      InputXY::ParamsVariant input_xy;
   };

   template<>
   struct Params<PiecewiseLogLog>
   {
      /// Input x,y data params.
      InputXY::ParamsVariant input_xy;
   };

   using ParamsVariant = std::variant<Params<PiecewiseLinear>,
                                       Params<PiecewiseLogLog>>;

   /// Array indicating if FunctionType option has associated BasePSD type.
   static constexpr std::array<bool, 
                        static_cast<std::size_t>(Size)>
   kFunctionHasPSDType = 
   {
      true,    // PiecewiseLinear
      true,    // PiecewiseLogLog
   };

};
// ----------------------------------------------------------------------------
struct IntervalType
{
   using Option = jabber::Interval::Method;

   using enum Option;

   /// Strings associated with jabber::Interval::Method enumerators.
   static constexpr std::array<std::string_view, 
                        static_cast<std::size_t>(Size)>
   kNames = 
   {
      "Midpoint",      // jabber::Interval::Method::Midpoint
      "MidpointLog",   // jabber::Interval::Method::MidpointLog10
   };

};

// ----------------------------------------------------------------------------
struct DiscMethod
{
   /**
    * @brief Discretization method options for discretization of a continuous
    * function (frequency selection of continuous PSD).
    * 
    */
   enum class Option : std::uint8_t
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

      /// Number of DiscMethod options
      Size
   };
   
   using enum Option;

   /// Strings associated with DiscMethod enumerators.
   static constexpr std::array<std::string_view, 
                        static_cast<std::size_t>(Size)>
   kNames = 
   {
      "Uniform",      // Uniform
      "UniformLog",   // UniformLog
      "Random",       // Random
      "RandomLog",    // RandomLog
   };


   template<Option V>
   struct Params
   {
      // Default struct has zero params.
   };

   template<>
   struct Params<Random>
   {
      /// Seed to use in randomization.
      int seed;
   };

   template<>
   struct Params<RandomLog>
   {
      /// Seed to use in randomization.
      int seed;
   };

   using ParamsVariant = std::variant<Params<Uniform>,Params<UniformLog>,
                                       Params<Random>,Params<RandomLog>>;
};

// ----------------------------------------------------------------------------
struct Direction
{
   /// Wave direction options.
   enum class Option : std::uint8_t
   {
      /// Constant direction.
      Constant,

      /// Random angle in XY-plane from x-axis for each wave/frequency.
      RandomXYAngle,

      /// Number of Direction options.
      Size
   };

   using enum Option;

   /// Strings associated with DiscMethod enumerators.
   static constexpr std::array<std::string_view, 
                        static_cast<std::size_t>(Size)>
   kNames = 
   {
      "Constant",         // Constant
      "RandomXYAngle",    // RandomXYAngle
   };

   template<Option V>
   struct Params;

   template<>
   struct Params<Constant>
   {
      /// Planar wave directional vector, can be non-normalized.
      std::vector<double> direction;
   };

   template<>
   struct Params<RandomXYAngle>
   {
      /// Min angle of wave from x-axis, in XY-plane (CCW+, CW-) (in degrees).
      double min_angle;

      /// Max angle of wave from x-axis, in XY-plane (CCW+, CW-) (in degrees).
      double max_angle;

      /// Seed to use in randomization.
      int seed;
   };

   using ParamsVariant = std::variant<Params<Constant>, Params<RandomXYAngle>>;

};

// ----------------------------------------------------------------------------
struct TransferFunction
{
      /// Transfer function options.
   enum class Option : std::uint8_t
   {
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

      /// Number of TransferFunction options.
      Size
   };

   using enum Option;

   /// Strings associated with TransferFunction enumerators.
   static constexpr std::array<std::string_view, 
                              static_cast<std::size_t>(Size)>
   kNames = 
   {
      "LowFrequencyLimit",  // LowFrequencyLimit
      "Input",              // Input
      "FlowNormalFit"       // FlowNormalFit
   };

   template<Option V>
   struct Params
   {
      // Default struct has zero params
   };

   template<>
   struct Params<Input>
   {
      /// Transfer function representation, (f, V^2).
      FunctionType::ParamsVariant input_tf;
   };

   template<>
   struct Params<FlowNormalFit>
   {
      /// Shock standoff distance from pitot probe.
      double shock_standoff_dist;
   };

   using ParamsVariant = std::variant<Params<LowFrequencyLimit>, Params<Input>,
                                       Params<FlowNormalFit>>;
};

// ----------------------------------------------------------------------------
struct Source
{
   /// Acoustic source options.
   enum class Option : std::uint8_t
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
   
   using enum Option;

   /// Strings associated with Source enumerators.
   static constexpr std::array<std::string_view, 
                              static_cast<std::size_t>(Size)>
   kNames = 
   {
      "SingleWave",      // SingleWave
      "WaveSpectrum",    // WaveSpectrum
      "PSD",             // PSD
      "WaveCSV"          // WaveCSV
   };

   template<Option V>
   struct Params;

   template<>
   struct Params<SingleWave>
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

   template<>
   struct Params<WaveSpectrum>
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

   template<>
   struct Params<PSD>
   {

      /// PSD function representation (f, PSD).
      FunctionType::ParamsVariant input_psd;

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
      DiscMethod::ParamsVariant disc_params;

      /// Direction method parameters.
      Direction::ParamsVariant dir_params;

      /// Seed to use for wave phase randomization.
      int phase_seed;

      /// Wave speeds to use.
      char speed;

      /// Transfer function parameters. None if not set.
      std::optional<TransferFunction::ParamsVariant> tf_params;
   };

   template<>
   struct Params<WaveCSV>
   {
      /// Wave CSV file (output from \ref jabber::WriteWaves()).
      std::string file;
   };

   /// All source parameter options.
   using ParamsVariant = std::variant<Params<SingleWave>,Params<WaveSpectrum>,
                                       Params<PSD>,Params<WaveCSV>>;
};


// ----------------------------------------------------------------------------
struct KernelType
{
   using Option = jabber::AcousticField::Kernel;

   using enum Option;

   /// Strings associated with jabber::AcousticField::Kernel enumerators.
   static constexpr std::array<std::string_view, 
                  static_cast<std::size_t>(Size)>
   kNames = 
   {
      "GridPoint",      // jabber::AcousticField::Kernel::GridPoint
   };

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
