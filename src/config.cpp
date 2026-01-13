#include "config.hpp"

#include <toml.hpp>

// Helper type for the std::visit (https://en.cppreference.com/w/cpp/utility/variant/visit)
template<class... Ts>
struct overloads : Ts... { using Ts::operator()...; };

namespace jabber
{

Config::Config(std::string config_file, std::ostream *out)
{
   toml::value file = toml::parse(config_file);

   // Parse the mode of run
   toml::value mode = file.at("Mode");
   std::string mode_type = mode.at("Type").as_string();
   if (mode_type == "SingleWave")
   {
      SingleWave meta;

      meta.amp = mode.at("Amplitude").as_floating();
      meta.freq = mode.at("Frequency").as_floating();
      meta.angle = mode.at("Angle").as_floating();
      meta.phase = mode.at("Phase").as_floating();

      mode_ = meta;
   }
   else
   {
      throw std::invalid_argument("Invalid mode type");
   }
   if (out)
   {
      PrintModeMeta(*out);
   }

   // Parse preCICE related fields
   toml::value precice = file.at("preCICE");
   precice_.participant_name = precice.at("ParticipantName").as_string();
   precice_.config_file = precice.at("ConfigFile").as_string();
   precice_.mesh_access_region = toml::get<std::vector<double>>(
                                          precice.at("MeshAccessRegion"));
   if (out)
   {
      PrintPreciceMeta(*out);
   }
}

void Config::PrintModeMeta(std::ostream &out)
{
   out << "Mode" << std::endl;
   std::visit(
   [&out](SingleWave &wave)
   {
      out << "\tType:      SingleWave" << std::endl;
      out << "\tAmplitude: " << wave.amp << std::endl;
      out << "\tFrequency: " << wave.freq << std::endl;
      out << "\tAngle:     " << wave.angle << std::endl;
      out << "\tPhase:     " << wave.phase << std::endl;
   }, mode_);
}

void Config::PrintPreciceMeta(std::ostream &out)
{
   out << "preCICE" << std::endl;
   out << "\tParticipant Name:   " << precice_.participant_name << std::endl;
   out << "\tConfiguration File: " << precice_.config_file << std::endl;
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
