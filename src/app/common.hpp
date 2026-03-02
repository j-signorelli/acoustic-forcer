#ifndef JABBER_APP_COMMON
#define JABBER_APP_COMMON

#include "config_input.hpp"

#include <jabber.hpp>
#include <iostream>
#include <concepts>
#include <memory>
namespace jabber_app
{

constexpr static std::string_view LINE = 
"-----------------------------------------------------------------------";

/// Print Jabber banner.
void PrintBanner(std::ostream &out);

/// Normalize the provided vector data
void Normalize(std::span<const double> vec, std::span<double> norm_vec);

/**
 * @brief All visitor options for each InputXY::Params, for initializing 
 * \ref x and \ref y vectors.
 * 
 */
struct InputXYVisitor
{  
   using enum InputXY::Option;

   /// Reference to x-data vector to set.
   std::vector<double> &x;

   /// Reference to y-data vector to set.
   std::vector<double> &y;

   void operator() (const InputXY::Params<Here> &op);
   void operator() (const InputXY::Params<FromCSV> &op);
};

/**
 * @brief All visitor options for each FunctionType::Params, for initializing a
 * \ref jabber::Function or \ref jabber::BasePSD type.
 * 
 */
struct FunctionTypeVisitor
{
   using enum FunctionType::Option;

   /// Function to initialize
   std::variant<std::unique_ptr<jabber::Function>*,
                std::unique_ptr<jabber::BasePSD>*> T_ptr_ptr_var;

   void operator() (const FunctionType::Params<PiecewiseLinear> &op);
   void operator() (const FunctionType::Params<PiecewiseLogLog> &op);
};

/**
 * @brief All visitor options for each DiscMethod::Params, for initializing
 * a discretized frequency range, \p freqs.
 * 
 */
struct DiscMethodVisitor
{  
   using enum DiscMethod::Option;

   /// Minimum frequency bound.
   const double &min_freq;

   /// Maximum frequency bound.
   const double &max_freq;

   /// **Sized** frequency vector to initialize.
   std::vector<double> &freqs;

   void operator() (const DiscMethod::Params<Uniform> &op);
   void operator() (const DiscMethod::Params<UniformLog> &op);
   void operator() (const DiscMethod::Params<Random> &op);
   void operator() (const DiscMethod::Params<RandomLog> &op);
};

/**
 * @brief All visitor options for each Direction::Params, for initializing
 * wave directional vectors in \p k_hats.
 * 
 */
struct DirectionVisitor
{
   using enum Direction::Option;

   /// **Sized** vector of direction vectors for each wave.
   std::vector<std::vector<double>> &k_hats;

   void operator() (const Direction::Params<Constant> &op);
   void operator() (const Direction::Params<RandomXYAngle> &op);
};


/**
 * @brief All visitor options for each Source::Params, for initializing 
 * \ref jabber::Wave's for each type and appending to \p waves.
 * 
 */
struct SourceVisitor
{
   using enum Source::Option;

   /// Reference of wave vector to append jabber::Wave structs to.
   std::vector<jabber::Wave> &waves;
   void operator() (const Source::Params<SingleWave> &op);
   void operator() (const Source::Params<WaveSpectrum> &op);
   void operator() (const Source::Params<PSD> &op);
   void operator() (const Source::Params<WaveCSV> &op);
};

/**
 * @brief Initialize a \ref jabber::AcousticField object from user input and 
 * grid.
 * 
 * @param conf          Input config object.
 * @param coords        Mesh coordinates vector, in XYZ XYZ ordering.
 * @param dim           Spatial dimension of mesh.
 * @return jabber::AcousticField    Finalized acoustic field.
 */
jabber::AcousticField InitializeAcousticField(const ConfigInput &conf, 
                                                std::span<const double> coords,
                                                int dim);

} // namespace jabber_app

#endif // JABBER_APP_COMMON
