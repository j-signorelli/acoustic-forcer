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
 * @brief Read in a digitzed power spectral density from a CSF file, where
 * first column are frequencies and second column are PSDs.
 * 
 * @todo unit test.
 * 
 * @param file_name                                CSV file name.
 * @return std::vector<std::pair<double,double>>   Vector of CSV file rows,
 *                                                 where the pair's first
 *                                                 value is the frequency
 *                                                 and second is PSD.
 */
std::vector<std::pair<double,double>> ReadPSDFromCSV(std::string file_name);

std::vector<jabber::Wave>
InitializeFromPSD(const SourceParams<SourceOption::DigitalPSD> &source_params);



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
