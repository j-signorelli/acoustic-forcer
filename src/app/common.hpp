#ifndef JABBER_APP_COMMON
#define JABBER_APP_COMMON

#include "config_input.hpp"

#include <jabber.hpp>
#include <iostream>
#include <memory>

namespace jabber_app
{

constexpr static std::string_view LINE = 
"-----------------------------------------------------------------------";

/// Print Jabber banner.
void PrintBanner(std::ostream &out);

/// Normalize the provided vector data
void Normalize(std::span<const double> vec, std::span<double> norm_vec);


/// Base option visitor templated type.
template<OptionEnum E>
struct OptionVisitor;

/**
 * @brief All visitor options for each InputXY params, for initializing 
 * \ref x and \ref y vectors.
 * 
 */
template<>
struct OptionVisitor<InputXY>
{  

   /// Reference to x-data vector to set.
   std::vector<double> &x;

   /// Reference to y-data vector to set.
   std::vector<double> &y;

   void operator() (const OptionParams<InputXY::Here> &op);
   void operator() (const OptionParams<InputXY::FromCSV> &op);
};

/**
 * @brief All visitor options for each FunctionType params, for initializing a
 * \ref jabber::Function or \ref jabber::BasePSD type.
 */
template<>
struct OptionVisitor<FunctionType>
{
   /// Pointers to pointer to initialize.
   std::variant<std::unique_ptr<jabber::Function>*,
                std::unique_ptr<jabber::BasePSD>*> T_ptr_ptr_var;

   void operator() (const OptionParams<FunctionType::PiecewiseLinear> &op);
   void operator() (const OptionParams<FunctionType::PiecewiseLogLog> &op);
};

/**
 * @brief All visitor options for each DiscMethod params, for initializing
 * a discretized frequency range, \p freqs.
 */
template<>
struct OptionVisitor<DiscMethod>
{  
   /// Minimum frequency bound.
   const double &min_freq;

   /// Maximum frequency bound.
   const double &max_freq;

   /// **Sized** frequency vector to initialize.
   std::vector<double> &freqs;

   void operator() (const OptionParams<DiscMethod::Uniform> &op);
   void operator() (const OptionParams<DiscMethod::UniformLog> &op);
   void operator() (const OptionParams<DiscMethod::Random> &op);
   void operator() (const OptionParams<DiscMethod::RandomLog> &op);
};

/**
 * @brief All visitor options for each Direction params, for initializing
 * wave directional vectors in \p k_hats.
 */
template<>
struct OptionVisitor<Direction>
{
   /// **Sized** vector of direction vectors for each wave.
   std::vector<std::vector<double>> &k_hats;

   void operator() (const OptionParams<Direction::Constant> &op);
   void operator() (const OptionParams<Direction::RandomXYAngle> &op);
};


/**
 * @brief All visitor options for each Source params, for initializing 
 * \ref jabber::Wave's for each type and appending to \p waves.
 */
template<>
struct OptionVisitor<Source>
{
   /// Reference of wave vector to append jabber::Wave structs to.
   std::vector<jabber::Wave> &waves;
   void operator() (const OptionParams<Source::SingleWave> &op);
   void operator() (const OptionParams<Source::WaveSpectrum> &op);
   void operator() (const OptionParams<Source::PSD> &op);
   void operator() (const OptionParams<Source::WaveCSV> &op);
};

/**
 * @brief Initialize a \ref jabber::AcousticField object from user input and 
 * grid.
 * 
 * @param conf                      Input config object.
 * @param coords                    Mesh coordinates vector, in XYZ XYZ
 *                                  ordering.
 * @param dim                       Spatial dimension of mesh.
 * @return jabber::AcousticField    Finalized acoustic field.
 */
jabber::AcousticField InitializeAcousticField(const ConfigInput &conf, 
                                                std::span<const double> coords,
                                                int dim);

} // namespace jabber_app

#endif // JABBER_APP_COMMON
