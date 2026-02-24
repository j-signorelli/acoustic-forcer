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
 * @brief All visitor options for each InputXYParams, for initializing 
 * \ref x and \ref y vectors.
 * 
 * @todo: Unit test?
 * 
 */
struct InputXYVisitor
{  

   /// Reference to x-data vector to set.
   std::vector<double> &x;

   /// Reference to y-data vector to set.
   std::vector<double> &y;

   void operator() (const InputXYParams<InputXYOption::Here> &ip);
   void operator() (const InputXYParams<InputXYOption::FromCSV> &ip);
};

/**
 * @brief All visitor options for each FunctionParams, for initializing a
 * \ref jabber::Function or \ref jabber::BasePSD type.
 * 
 * @tparam T      Either \ref jabber::Function or \ref jabber::BasePSD.
 * 
 * @todo: Unit test?
 */
template<typename T>
struct FunctionVisitor
{
   /// Function to initialize
   std::unique_ptr<T> &T_ptr;

   void operator() (const FunctionParams<FunctionOption::PiecewiseLinear> &fp);
   void operator() (const FunctionParams<FunctionOption::PiecewiseLogLog> &fp);
};

/**
 * @brief All visitor options for each DiscMethodParams, for initializing
 * a discretized frequency range, \p freqs.
 * 
 * @todo: Unit test?
 */
struct DiscMethodVisitor
{  
   /// Minimum frequency bound.
   const double &min_freq;

   /// Maximum frequency bound.
   const double &max_freq;

   /// **Sized** frequency vector to initialize.
   std::vector<double> &freqs;

   void operator() (const DiscMethodParams<DiscMethodOption::Uniform> &dp);
   void operator() (const DiscMethodParams<DiscMethodOption::UniformLog> &dp);
   void operator() (const DiscMethodParams<DiscMethodOption::Random> &dp);
   void operator() (const DiscMethodParams<DiscMethodOption::RandomLog> &dp);
};

/**
 * @brief All visitor options for each DirectionParams, for initializing
 * wave directional vectors in \p k_hats.
 * 
 * @todo: Unit test?
 */
struct DirectionVisitor
{
   /// **Sized** vector of direction vectors for each wave.
   std::vector<std::vector<double>> &k_hats;

   void operator() (const DirectionParams<DirectionOption::Constant> &dp);
   void operator() (const DirectionParams<DirectionOption::RandomXYAngle> &dp);
};


/**
 * @brief All visitor options for each SourceParams, for initializing 
 * \ref jabber::Wave's for each type and appending to \p waves.
 * 
 * @todo: Unit test?
 * 
 */
struct SourceVisitor
{
   /// Reference of wave vector to append jabber::Wave structs to.
   std::vector<jabber::Wave> &waves;
   void operator() (const SourceParams<SourceOption::SingleWave> &sp);
   void operator() (const SourceParams<SourceOption::WaveSpectrum> &sp);
   void operator() (const SourceParams<SourceOption::PSD> &sp);
   void operator() (const SourceParams<SourceOption::WaveCSV> &sp);
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
