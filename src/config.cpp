#include "config.hpp"

#include <toml.hpp>

#include <iomanip>

// Helper type for the std::visit
// (https://en.cppreference.com/w/cpp/utility/variant/visit)
template<class... Ts>
struct overloads : Ts... { using Ts::operator()...; };

namespace jabber
{
namespace config
{

static constexpr std::string_view TRUE = "true";
static constexpr std::string_view FALSE = "false";

void ConfigInput::PrintBaseFlowParams(std::ostream &out) const
{
   out << "Base Flow" << std::endl;
   out << "\trho:   " << std::setprecision(14) << base_flow_.rho << std::endl;
   out << "\tp:     " << std::setprecision(14) << base_flow_.p << std::endl;
   out << "\tU:     " << std::setprecision(14) << base_flow_.U << std::endl;
   out << "\tgamma: " << std::setprecision(14) << base_flow_.gamma
                                                               << std::endl;
}

void ConfigInput::PrintSourceParams(std::ostream &out) const
{
   out << "Source" << std::endl;
   std::visit(
   [&out](const SourceParams<SourceOption::SingleWave> &wave)
   {
      out << "\tType:      SingleWave" << std::endl;
      out << "\tAmplitude: " << wave.amp << std::endl;
      out << "\tFrequency: " << wave.freq << std::endl;
      out << "\tAngle:     " << wave.angle << std::endl;
      out << "\tPhase:     " << wave.phase << std::endl;
      out << "\tSlow:      " << (wave.slow ? TRUE : FALSE) << std::endl;
   }, source_);
}

void ConfigInput::PrintCompParams(std::ostream &out) const
{
   out << "Computation" << std::endl;
   out << "\tt0:   " << std::setprecision(14) << comp_.t0 << std::endl;
   out << "\tData: [";
   for (int i = 0; i < comp_.data.size(); i++)
   {  
      const DataOption &dat = comp_.data[i];
      out << DataNames[static_cast<std::size_t>(dat)]
          << (i+1 == comp_.data.size() ? "]" : ",");
   }
   out << std::endl;
}

void ConfigInput::PrintPreciceParams(std::ostream &out) const
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

TOMLConfigInput::TOMLConfigInput(std::string config_file, std::ostream *out)
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
      PrintBaseFlowParams(*out);
   }

   // Parse the acoustic source of run
   toml::value in_source = file.at("Source");
   std::string mode_type = in_source.at("Type").as_string();
   const std::string_view *it = std::find(SourceNames.begin(), 
                                          SourceNames.end(), mode_type);
   if (it != SourceNames.end())
   {
      SourceOption source_op = static_cast<SourceOption>(it-SourceNames.begin());

      if (source_op == SourceOption::SingleWave)
      {
         SourceParams<SourceOption::SingleWave> meta;

         meta.amp = in_source.at("Amplitude").as_floating();
         meta.freq = in_source.at("Frequency").as_floating();
         meta.angle = in_source.at("Angle").as_floating();
         meta.phase = in_source.at("Phase").as_floating();
         meta.slow = in_source.at("Slow").as_boolean();

         source_ = meta;
      }
      else
      {
         throw std::logic_error("Error reading Source.Type");
      }
   }
   else
   {
      throw std::invalid_argument("Invalid Source.Type");
   }
   if (out)
   {
      PrintSourceParams(*out);
   }

   // Parse compute fields of run
   toml::value in_comp = file.at("Computation");
   comp_.t0 = in_comp.at("t0").as_floating();
   std::vector<std::string> data_string_vec = 
                     toml::get<std::vector<std::string>>(in_comp.at("Data"));
   for (const std::string &data_string : data_string_vec)
   {
      const std::string_view *it = std::find(DataNames.begin(),
                                             DataNames.end(),
                                             data_string);
      if (it != DataNames.end())
      {
         DataOption data = static_cast<DataOption>(it - DataNames.begin());
         comp_.data.push_back(data);
      }
      else
      {
         throw std::invalid_argument("Invalid Computation.Type(s)");
      }
   }
   if (out)
   {
      PrintCompParams(*out);
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
      PrintPreciceParams(*out);
   }

   if (out)
   {
      *out << std::endl;
   }
}
} // namespace config
} // namespace jabber
