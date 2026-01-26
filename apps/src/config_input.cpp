#include "config_input.hpp"

#include <toml.hpp>

#include <iomanip>
#include <format>

// Helper type for the std::visit
// (https://en.cppreference.com/w/cpp/utility/variant/visit)
template<class... Ts>
struct overloads : Ts... { using Ts::operator()...; };

namespace jabber_app
{

void ConfigInput::PrintBaseFlowParams(std::ostream &out) const
{
   out << "Base Flow" << std::endl;
   constexpr int label_width = 7;
   out << WriteParam("rho", OutReal(base_flow_.rho), label_width);
   out << WriteParam("p", OutReal(base_flow_.p), label_width);
   out << WriteParam("U", OutRealVec(base_flow_.U), label_width);
   out << WriteParam("gamma", OutReal(base_flow_.gamma), label_width);
   out << std::endl;
}

void ConfigInput::PrintSourceParams(std::ostream &out) const
{

   out << "Sources" << std::endl;

   for (const SourceParamsVariant &source : sources_)
   {
      std::visit(
      overloads
      {
      [&out](const SourceParams<SourceOption::SingleWave> &wave)
      {
         std::size_t name_i 
                        = static_cast<std::size_t>(SourceOption::SingleWave);
         std::size_t speed_i = static_cast<std::size_t>(wave.speed);

         constexpr int label_width = 11;

         out << WriteParam("Type", SourceNames[name_i], label_width);
         out << WriteParam("Amplitude", OutReal(wave.amp), label_width);
         out << WriteParam("Frequency", OutReal(wave.freq), label_width);
         out << WriteParam("Direction", OutRealVec(wave.direction), 
                                                            label_width);
         out << WriteParam("Phase", OutReal(wave.phase), label_width);
         out << WriteParam("Speed", SpeedNames[speed_i], label_width);
         out << std::endl;
      },
      [&out](const SourceParams<SourceOption::WaveSpectrum> &waves)
      {
         std::size_t name_i = 
                        static_cast<std::size_t>(SourceOption::WaveSpectrum);
         
         constexpr int label_width = 13;

         // Assemble data such that each wave is on newline
         std::string amplitudes_str = OutRealVec(waves.amps, 
                                             std::format(",\n\t{:<{}}", 
                                                         "", 
                                                         label_width+3));

         std::string freqs_str = OutRealVec(waves.freqs, 
                                             std::format(",\n\t{:<{}}", 
                                                         "", 
                                                         label_width+3));

         std::string phases_str = OutRealVec(waves.phases, 
                                             std::format(",\n\t{:<{}}", 
                                                         "", 
                                                         label_width+3));

         std::string dirs_str = "[";
         for (int i = 0; i < waves.directions.size(); i++)
         {
            dirs_str += OutRealVec(waves.directions[i]) + 
                        (i+1 == waves.directions.size() ? "]" :
                           std::format("\n\t{:<{}}", "", label_width+3));

         }
         std::string speeds_str = "[";
         for (int i = 0; i < waves.speeds.size(); i++)
         {
            std::size_t speed_i = static_cast<std::size_t>(waves.speeds[i]);
            speeds_str += std::string(SpeedNames[speed_i]) + 
                           ((i+1 == waves.speeds.size()) ? "]\n" :
                              std::format(",\n\t{:<{}}", "", label_width+3));
         }
         out << WriteParam("Type", SourceNames[name_i], label_width);
         out << WriteParam("Amplitudes", amplitudes_str, label_width);
         out << WriteParam("Frequencies", freqs_str, label_width);
         out << WriteParam("Directions", dirs_str, label_width);
         out << WriteParam("Phases", phases_str, label_width);
         out << WriteParam("Speeds", speeds_str, label_width);
         out << std::endl;
      }
      }, source);
   }
}

void ConfigInput::PrintCompParams(std::ostream &out) const
{
   out << "Computation" << std::endl;
   out << WriteParam("t0", OutReal(comp_.t0), 3);
   out << std::endl;
}

void ConfigInput::PrintPreciceParams(std::ostream &out) const
{
   if (precice_.has_value())
   {
      constexpr int label_width = 20;
      out << "preCICE" << std::endl;
      out << WriteParam("Participant Name", precice_->participant_name, 
                        label_width);
      out << WriteParam("Configuration File", precice_->config_file,
                        label_width);
      out << WriteParam("Fluid Mesh Name", precice_->fluid_mesh_name, 
                        label_width);
      out << WriteParam("Mesh Access Region", 
                        OutRealVec(precice_->mesh_access_region),
                        label_width);
      out << std::endl;
   }
}

TOMLConfigInput::TOMLConfigInput(std::string config_file, std::ostream *out)
{
   toml::value file = toml::parse(config_file);

   // Parse base flow parameters
   toml::value in_base_flow = file.at("BaseFlow");
   ParseBaseFlow(toml::format(in_base_flow));
   if (out)
   {
      PrintBaseFlowParams(*out);
   }

   // Parse the acoustic sources of run
   toml::array in_sources = file.at("Sources").as_array();
   for (const toml::value &in_source : in_sources)
   {
      ParseSource(toml::format(in_source));
   }
   if (out)
   {
      PrintSourceParams(*out);
   }

   // Parse compute fields of run
   toml::value in_comp = file.at("Computation");
   ParseComputation(toml::format(in_comp));
   if (out)
   {
      PrintCompParams(*out);
   }

   // Parse preCICE related fields - if exists
   if (file.contains("preCICE"))
   {
      toml::value in_precice = file.at("preCICE");
      ParsePrecice(toml::format(in_precice));
      if (out)
      {
         PrintPreciceParams(*out);
      }
   }
}

void TOMLConfigInput::ParseBaseFlow(std::string base_flow_serialized)
{
   toml::value in_base_flow = toml::parse_str(base_flow_serialized);

   base_flow_.rho = in_base_flow.at("rho").as_floating();
   base_flow_.p = in_base_flow.at("p").as_floating();
   base_flow_.U = toml::get<std::vector<double>>(in_base_flow.at("U"));
   base_flow_.gamma = in_base_flow.at("gamma").as_floating();
}

void TOMLConfigInput::ParseSource(std::string source_serialized)
{
   toml::value in_source = toml::parse_str(source_serialized);
   std::string mode_type = in_source.at("Type").as_string();
   const std::string_view *it = std::find(SourceNames.begin(), 
                                          SourceNames.end(), mode_type);
   if (it != SourceNames.end())
   {
      SourceOption source_op = static_cast<SourceOption>(
                                             it-SourceNames.begin());

      if (source_op == SourceOption::SingleWave)
      {
         SourceParams<SourceOption::SingleWave> meta;

         meta.amp = in_source.at("Amplitude").as_floating();
         meta.freq = in_source.at("Frequency").as_floating();
         meta.direction = toml::get<std::vector<double>>(
                                          in_source.at("Direction"));
         meta.phase = in_source.at("Phase").as_floating();
         std::string speed_type =  in_source.at("Speed").as_string();

         const std::string_view *speed_it = std::find(SpeedNames.begin(),
                                                      SpeedNames.end(), 
                                                      speed_type);
         SpeedOption speed_op = static_cast<SpeedOption>(
                                             speed_it-SpeedNames.begin());
         meta.speed = speed_op;

         sources_.emplace_back(meta);
      }
      else if (source_op == SourceOption::WaveSpectrum)
      {
         SourceParams<SourceOption::WaveSpectrum> meta;
         
         meta.amps = toml::get<std::vector<double>>(
                                             in_source.at("Amplitudes"));
         meta.freqs = toml::get<std::vector<double>>(
                                             in_source.at("Frequencies"));
         meta.directions = toml::get<std::vector<std::vector<double>>>(
                                             in_source.at("Directions"));
         meta.phases = toml::get<std::vector<double>>(
                                                in_source.at("Phases"));
         std::vector<std::string> speed_string_vec = 
               toml::get<std::vector<std::string>>(in_source.at("Speeds"));
         for (const std::string &speed_string : speed_string_vec)
         {
            const std::string_view *speed_it = std::find(SpeedNames.begin(),
                                                         SpeedNames.end(),
                                                         speed_string);
            SpeedOption speed_op = static_cast<SpeedOption>(speed_it-
                                                      SpeedNames.begin());
            meta.speeds.push_back(speed_op);
         }
         
         sources_.emplace_back(meta);
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
}

void TOMLConfigInput::ParseComputation(std::string comp_serialized)
{
   toml::value in_comp = toml::parse_str(comp_serialized);
   comp_.t0 = in_comp.at("t0").as_floating();
}

void TOMLConfigInput::ParsePrecice(std::string precice_serialized)
{
   precice_ = PreciceParams();
   toml::value in_precice = toml::parse_str(precice_serialized);
   precice_->participant_name = in_precice.at("ParticipantName")
                                             .as_string();
   precice_->config_file = in_precice.at("ConfigFile").as_string();
   precice_->fluid_mesh_name = in_precice.at("FluidMeshName").as_string();
   precice_->mesh_access_region = 
         toml::get<std::vector<double>>(in_precice.at("MeshAccessRegion"));
}

} // namespace jabber
