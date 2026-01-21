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
   out << "\trho:   " << OutReal(base_flow_.rho) << std::endl;
   out << "\tp:     " << OutReal(base_flow_.p) << std::endl;
   out << "\tU:     " << OutRealVec(base_flow_.U) << std::endl;
   out << "\tgamma: " << OutReal(base_flow_.gamma) << std::endl;
}

void ConfigInput::PrintSourceParams(std::ostream &out) const
{

   out << "Source" << std::endl;

   std::visit(
   overloads
   {
   [&out](const SourceParams<SourceOption::SingleWave> &wave)
   {
      std::size_t name_i = static_cast<std::size_t>(SourceOption::SingleWave);
      std::size_t speed_i = static_cast<std::size_t>(wave.speed);

      out << "\tType:      " << SourceNames[name_i] << std::endl;
      out << "\tAmplitude: " << OutReal(wave.amp) << std::endl;
      out << "\tFrequency: " << OutReal(wave.freq) << std::endl;
      out << "\tAngle:     " << OutReal(wave.angle) << std::endl;
      out << "\tPhase:     " << OutReal(wave.phase) << std::endl;
      out << "\tSpeed:     " << SpeedNames[speed_i] << std::endl;
   },
   [&out](const SourceParams<SourceOption::WaveSpectrum> &waves)
   {
      std::size_t name_i = static_cast<std::size_t>(SourceOption::WaveSpectrum);

      out << "\tType:        " << SourceNames[name_i] << std::endl;
      out << "\tAmplitudes:  " << OutRealVec(waves.amps) << std::endl;
      out << "\tFrequencies: " << OutRealVec(waves.freqs) << std::endl;
      out << "\tAngles:      " << OutRealVec(waves.angles) << std::endl;
      out << "\tPhases:      " << OutRealVec(waves.phases) << std::endl;
      out << "\tSpeeds:      [";
      for (int i = 0; i < waves.speeds.size(); i++)
      {
         std::size_t speed_i = static_cast<std::size_t>(waves.speeds[i]);
         out << SpeedNames[speed_i] 
               << ((i+1 == waves.speeds.size()) ? "]" : ", ");
      }
      out << std::endl;
   }
   }, source_);
}

void ConfigInput::PrintCompParams(std::ostream &out) const
{
   out << "Computation" << std::endl;
   out << "\tt0:   " << OutReal(comp_.t0) << std::endl;
   out << std::endl;
}

void ConfigInput::PrintPreciceParams(std::ostream &out) const
{
   if (precice_.has_value())
   {
      out << "preCICE" << std::endl;
      out << "\tParticipant Name:   " << precice_->participant_name 
                                             << std::endl;
      out << "\tConfiguration File: " << precice_->config_file << std::endl;
      out << "\tFluid Mesh Name:    " << precice_->fluid_mesh_name 
                                             << std::endl;
      out << "\tMesh Access Region: {";
      for (int i = 0; i < precice_->mesh_access_region.size(); i+=2)
      {
         out << "[" << precice_->mesh_access_region[i] << ","
                  << precice_->mesh_access_region[i+1] << "]";
         out << ((i+2 < precice_->mesh_access_region.size()) ? "," : "}");
      }
      out << std::endl;
   }
}

TOMLConfigInput::TOMLConfigInput(std::string config_file, std::ostream *out)
{
   toml::value file = toml::parse(config_file);

   // Parse base flow parameters
   toml::value in_base_flow = file.at("BaseFlow");
   base_flow_.rho = in_base_flow.at("rho").as_floating();
   base_flow_.p = in_base_flow.at("p").as_floating();
   base_flow_.U = toml::get<std::vector<double>>(in_base_flow.at("U"));
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
      SourceOption source_op = static_cast<SourceOption>(
                                             it-SourceNames.begin());

      if (source_op == SourceOption::SingleWave)
      {
         SourceParams<SourceOption::SingleWave> meta;

         meta.amp = in_source.at("Amplitude").as_floating();
         meta.freq = in_source.at("Frequency").as_floating();
         meta.angle = in_source.at("Angle").as_floating();
         meta.phase = in_source.at("Phase").as_floating();
         std::string speed_type =  in_source.at("Speed").as_string();

         const std::string_view *speed_it = std::find(SpeedNames.begin(), 
                                                      SpeedNames.end(), 
                                                      speed_type);
         SpeedOption speed_op = static_cast<SpeedOption>(speed_it-
                                                         SpeedNames.begin());                  
         meta.speed = speed_op;

         source_ = meta;
      }
      else if (source_op == SourceOption::WaveSpectrum)
      {
         SourceParams<SourceOption::WaveSpectrum> meta;
         
         meta.amps = toml::get<std::vector<double>>(
                                                in_source.at("Amplitudes"));
         meta.freqs = toml::get<std::vector<double>>(
                                                in_source.at("Frequencies"));
         meta.angles = toml::get<std::vector<double>>(in_source.at("Angles"));
         meta.phases = toml::get<std::vector<double>>(in_source.at("Phases"));
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
   if (out)
   {
      PrintCompParams(*out);
   }

   // Parse preCICE related fields - if exists
   if (file.contains("preCICE"))
   {
      precice_ = PreciceParams();
      toml::value in_precice = file.at("preCICE");
      precice_->participant_name = in_precice.at("ParticipantName").as_string();
      precice_->config_file = in_precice.at("ConfigFile").as_string();
      precice_->fluid_mesh_name = in_precice.at("FluidMeshName").as_string();
      precice_->mesh_access_region = toml::get<std::vector<double>>(
                                             in_precice.at("MeshAccessRegion"));
      if (out)
      {
         PrintPreciceParams(*out);
      }
   }
   if (out)
   {
      *out << std::endl;
   }
}

} // namespace jabber
