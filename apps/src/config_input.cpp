#include "config_input.hpp"

#include <toml.hpp>

#include <iomanip>
#include <format>
#include <algorithm>

// Helper type for the std::visit
// (https://en.cppreference.com/w/cpp/utility/variant/visit)
template<class... Ts>
struct overloads : Ts... { using Ts::operator()...; };

#if __cpp_deduction_guides < 201907L
template<class... Ts>
overloads(Ts...) -> overloads<Ts...>;
#endif

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
                        
         constexpr int label_width = 11;

         out << WriteParam("Type", SourceNames[name_i], label_width);
         out << WriteParam("Amplitude", OutReal(wave.amp), label_width);
         out << WriteParam("Frequency", OutReal(wave.freq), label_width);
         out << WriteParam("Direction", OutRealVec(wave.direction), 
                                                            label_width);
         out << WriteParam("Phase", OutReal(wave.phase), label_width);
         out << WriteParam("Speed", std::string(&wave.speed), label_width);
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
            speeds_str += waves.speeds[i] + 
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
      },
      [&out](const SourceParams<SourceOption::DigitalPSD> &waves)
      {
         constexpr int label_width = 13;

         /// @todo: Finish this
         out << "Unimplemented verbose output for source type" << std::endl;
      },
      [&out](const SourceParams<SourceOption::WaveCSV> &waves)
      {
         constexpr int label_width = 7;

         out << WriteParam("File", waves.file, label_width);
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

/// Internal helper function here for getting enumerator from string.
template<typename T>
requires std::is_enum_v<T> && 
         std::same_as<std::underlying_type_t<T>, std::uint8_t>
void GetEnumerator(const std::string_view input_str, 
                std::span<const std::string_view> option_names,
                T &val)
{
   const auto it = std::find(option_names.begin(), 
                              option_names.end(), 
                              input_str);
   if (it == option_names.end())
   {
      throw std::invalid_argument(std::format("Invalid input argument: {}",
                                              input_str));
   }
   else
   {
      val = static_cast<T>(std::distance(option_names.begin(), it));
   }
}

void TOMLConfigInput::ParseSource(std::string source_serialized)
{
   toml::value in_source = toml::parse_str(source_serialized);
   std::string mode_type = in_source.at("Type").as_string();

   SourceOption source_op;
   GetEnumerator(mode_type, SourceNames, source_op);

   if (source_op == SourceOption::SingleWave)
   {
      SourceParams<SourceOption::SingleWave> meta;

      meta.amp = in_source.at("Amplitude").as_floating();
      meta.freq = in_source.at("Frequency").as_floating();
      meta.direction = toml::get<std::vector<double>>(
                                       in_source.at("DirVector"));
      meta.phase = in_source.at("Phase").as_floating();
      meta.speed = *(in_source.at("Speed").as_string().data());

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
                                          in_source.at("DirVectors"));
      meta.phases = toml::get<std::vector<double>>(
                                             in_source.at("Phases"));
      std::vector<std::string> speed_strs = 
                  toml::get<std::vector<std::string>>(in_source.at("Speeds"));
      meta.speeds.resize(speed_strs.size());
      std::transform(speed_strs.begin(), speed_strs.end(), meta.speeds.begin(),
                     [](std::string &s) -> char { return *(s.data()); });
      
      sources_.emplace_back(meta);
   }
   else if (source_op == SourceOption::DigitalPSD)
   {
      SourceParams<SourceOption::DigitalPSD> meta;

      toml::value in_input = in_source.at("InputData");
      PSDInputOption input_option;
      GetEnumerator(in_input.at("Type").as_string(), PSDInputNames, 
                        input_option);
      if (input_option == PSDInputOption::Here)
      {
         PSDInputParams<PSDInputOption::Here> input_params;
         input_params.freqs = toml::get<std::vector<double>>(
                                             in_input.at("Frequencies"));
         input_params.psds = toml::get<std::vector<double>>(
                                             in_input.at("PSDs"));
         meta.input_params = input_params;
      }
      else if (input_option == PSDInputOption::FromCSV)
      {
         PSDInputParams<PSDInputOption::FromCSV> input_params;
         input_params.file = in_input.at("File").as_string();
         meta.input_params = input_params;
      }
      
      meta.dim_fac = in_source.at("DimFactor").as_floating();
      GetEnumerator(in_source.at("Interpolation").as_string(), 
                     InterpolationNames,
                     meta.interp);
      
      toml::value in_disc = in_source.at("Discretization");
      meta.min_disc_freq = in_disc.at("Min").as_floating();
      meta.max_disc_freq = in_disc.at("Max").as_floating();
      meta.num_waves = in_disc.at("N").as_integer();
      GetEnumerator(in_disc.at("Interval").as_string(), IntervalNames,
                      meta.int_method);
      
      toml::value in_disc_method = in_disc.at("Method");
      DiscMethodOption disc_option;
      GetEnumerator(in_disc_method.at("Type").as_string(), DiscMethodNames,
                     disc_option);
      if (disc_option == DiscMethodOption::Uniform)
      {
         meta.disc_params = DiscMethodParams<DiscMethodOption::Uniform>{};
      }
      else if (disc_option == DiscMethodOption::UniformLog)
      {
         meta.disc_params = DiscMethodParams<DiscMethodOption::UniformLog>{};
      }
      else if (disc_option == DiscMethodOption::Random)
      {
         DiscMethodParams<DiscMethodOption::Random> disc_params;
         disc_params.seed = in_disc_method.at("Seed").as_integer();
         meta.disc_params = disc_params;
      }
      else if (disc_option == DiscMethodOption::RandomLog)
      {
         DiscMethodParams<DiscMethodOption::RandomLog> disc_params;
         disc_params.seed = in_disc_method.at("Seed").as_integer();
         meta.disc_params = disc_params;
      }

      toml::value in_dir = in_source.at("Direction");
      DirectionOption dir_option;
      GetEnumerator(in_dir.at("Type").as_string(), DirectionNames, dir_option);
      if (dir_option == DirectionOption::Constant)
      {
         DirectionParams<DirectionOption::Constant> dir_params;
         dir_params.direction = toml::get<std::vector<double>>(
                                                in_dir.at("Vector"));
         meta.dir_params = dir_params;
      }
      else if (dir_option == DirectionOption::RandomXYAngle)
      {
         DirectionParams<DirectionOption::RandomXYAngle> dir_params;
         dir_params.min_angle = in_dir.at("MinAngle").as_floating();
         dir_params.max_angle = in_dir.at("MaxAngle").as_floating();
         dir_params.seed = in_dir.at("Seed").as_integer();
         meta.dir_params = dir_params;
      }

      meta.phase_seed = in_source.at("PhaseSeed").as_integer();
      meta.speed = *(in_source.at("Speed").as_string().data());

      sources_.emplace_back(meta);
   }
   else if (source_op == SourceOption::WaveCSV)
   {
      SourceParams<SourceOption::WaveCSV> meta;
      meta.file = in_source.at("File").as_string();
      sources_.emplace_back(meta);
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
