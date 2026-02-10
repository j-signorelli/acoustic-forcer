#ifndef JABBER_APP_COMMON
#define JABBER_APP_COMMON

#include "config_input.hpp"

#include <jabber.hpp>
#include <iostream>

namespace jabber_app
{

constexpr static std::string_view LINE = 
"-----------------------------------------------------------------------";

/// Print Jabber banner.
void PrintBanner(std::ostream &out);

/// Normalize the provided vector data
void Normalize(std::span<const double> vec, std::span<double> norm_vec);

/**
 * @brief All visitor options for each SourceParams, for initializing 
 * \ref jabber::Wave's for each type and appending to \p waves.
 * 
 */
struct SourceParamsInitializer
{
   std::vector<jabber::Wave> &waves;
   void operator() (const SourceParams<SourceOption::SingleWave> &sp);
   void operator() (const SourceParams<SourceOption::WaveSpectrum> &sp);
   void operator() (const SourceParams<SourceOption::DigitalPSD> &sp);
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
