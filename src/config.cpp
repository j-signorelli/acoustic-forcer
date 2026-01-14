#include "config.hpp"

#include <toml.hpp>

#include <iomanip>

// Helper type for the std::visit
// (https://en.cppreference.com/w/cpp/utility/variant/visit)
template<class... Ts>
struct overloads : Ts... { using Ts::operator()...; };

namespace jabber
{

Config::Config(std::string config_file, std::ostream *out)
{
   toml::value file = toml::parse(config_file);

   // Parse base flow parameters
   toml::value in_base_flow = file.at("BaseFlow");
   base_flow_.rho = in_base_flow.at("rho").as_floating();
   base_flow_.p = in_base_flow.at("p").as_floating();
   base_flow_.U = in_base_flow.at("U").as_floating();
   base_flow_.gamma = in_base_flow.at("gamma").as_floating();
   if (out)
   {
      PrintBaseFlowMeta(*out);
   }

   // Parse the acoustic source of run
   toml::value in_source = file.at("Source");
   std::string mode_type = in_source.at("Type").as_string();
   if (mode_type == "SingleWave")
   {
      SingleWaveMeta meta;

      meta.amp = in_source.at("Amplitude").as_floating();
      meta.freq = in_source.at("Frequency").as_floating();
      meta.angle = in_source.at("Angle").as_floating();
      meta.phase = in_source.at("Phase").as_floating();

      source_ = meta;
   }
   else
   {
      throw std::invalid_argument("Invalid mode type");
   }
   if (out)
   {
      PrintSourceMeta(*out);
   }

   // Parse preCICE related fields
   toml::value in_precice = file.at("preCICE");
   precice_.participant_name = in_precice.at("ParticipantName").as_string();
   precice_.config_file = in_precice.at("ConfigFile").as_string();
   precice_.fluid_mesh_name = in_precice.at("FluidMeshName").as_string();
   precice_.mesh_access_region = toml::get<std::vector<double>>(
                                          in_precice.at("MeshAccessRegion"));
   if (out)
   {
      PrintPreciceMeta(*out);
   }

   if (out)
   {
      *out << std::endl;
   }
}

void Config::PrintBaseFlowMeta(std::ostream &out) const
{
   out << "Base Flow" << std::endl;
   out << "\trho:   " << std::setprecision(14) << base_flow_.rho << std::endl;
   out << "\tp:     " << std::setprecision(14) << base_flow_.p << std::endl;
   out << "\tU:     " << std::setprecision(14) << base_flow_.U << std::endl;
   out << "\tgamma: " << std::setprecision(14) << base_flow_.gamma
                                                               << std::endl;
}

void Config::PrintSourceMeta(std::ostream &out) const
{
   out << "Source" << std::endl;
   std::visit(
   [&out](const SingleWaveMeta &wave)
   {
      out << "\tType:      SingleWave" << std::endl;
      out << "\tAmplitude: " << wave.amp << std::endl;
      out << "\tFrequency: " << wave.freq << std::endl;
      out << "\tAngle:     " << wave.angle << std::endl;
      out << "\tPhase:     " << wave.phase << std::endl;
   }, source_);
}

void Config::PrintPreciceMeta(std::ostream &out) const
{
   out << "preCICE" << std::endl;
   out << "\tParticipant Name:   " << precice_.participant_name << std::endl;
   out << "\tConfiguration File: " << precice_.config_file << std::endl;
   out << "\tFluid Mesh Name:    " << precice_.fluid_mesh_name << std::endl;
   out << "\tMesh Access Region: {";
   for (int i = 0; i < precice_.mesh_access_region.size(); i+=2)
   {
      out << "[" << precice_.mesh_access_region[i] << ","
                 << precice_.mesh_access_region[i+1] << "]";
      out << ((i+2 < precice_.mesh_access_region.size()) ? "," : "}");
   }
   out << std::endl;
}

} // namespace jabber
