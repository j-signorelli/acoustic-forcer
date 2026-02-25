#include "config_input.hpp"

#include <toml.hpp>

#include <iomanip>
#include <format>
#include <algorithm>

// Helper type for the std::visit
// (https://en.cppreference.com/w/cpp/utility/variant/visit)
template<class... Ts>
struct overloads : Ts... { using Ts::operator()...; };

using namespace jabber;

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

   for (const OptionParamsVar<Source> &source : sources_)
   {
      std::visit(
      overloads
      {
      [&out](const OptionParams<Source::SingleWave> &wave)
      {
         std::size_t name_i 
                        = static_cast<std::size_t>(Source::SingleWave);
                        
         constexpr int label_width = 11;

         out << WriteParam("Type", OptionNames<Source>[name_i], label_width);
         out << WriteParam("Amplitude", OutReal(wave.amp), label_width);
         out << WriteParam("Frequency", OutReal(wave.freq), label_width);
         out << WriteParam("Direction", OutRealVec(wave.direction), 
                                                            label_width);
         out << WriteParam("Phase", OutReal(wave.phase), label_width);
         out << WriteParam("Speed", std::string(&wave.speed), label_width);
         out << std::endl;
      },
      [&out](const OptionParams<Source::WaveSpectrum> &waves)
      {
         std::size_t name_i = 
                        static_cast<std::size_t>(Source::WaveSpectrum);
         
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
         out << WriteParam("Type", OptionNames<Source>[name_i], label_width);
         out << WriteParam("Amplitudes", amplitudes_str, label_width);
         out << WriteParam("Frequencies", freqs_str, label_width);
         out << WriteParam("Directions", dirs_str, label_width);
         out << WriteParam("Phases", phases_str, label_width);
         out << WriteParam("Speeds", speeds_str, label_width);
         out << std::endl;
      },
      [&out](const OptionParams<Source::PSD> &waves)
      {
         constexpr int label_width = 13;

         /// @todo: Finish this
         out << "Unimplemented verbose output for source type" << std::endl;
      },
      [&out](const OptionParams<Source::WaveCSV> &waves)
      {
         constexpr int label_width = 7;
         std::size_t name_i = 
                        static_cast<std::size_t>(Source::WaveCSV);
         out << WriteParam("Type", OptionNames<Source>[name_i], label_width);
         out << WriteParam("File", waves.file, label_width);
         out << std::endl;
      }
      }, source);
   }
}

void ConfigInput::PrintCompParams(std::ostream &out) const
{
   out << "Computation" << std::endl;

   constexpr int label_width = 7;
   out << WriteParam("t0", OutReal(comp_.t0), label_width);
   out << WriteParam("Kernel", 
                     OptionNames<AcousticField::Kernel>[
                        static_cast<std::size_t>(comp_.kernel)],
                     label_width);
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

void TOMLConfigInput::ParseBaseFlow(std::string base_flow_serialized, 
                                    BaseFlowParams &bf_params)
{
   toml::value in_base_flow = toml::parse_str(base_flow_serialized);

   bf_params.rho = in_base_flow.at("rho").as_floating();
   bf_params.p = in_base_flow.at("p").as_floating();
   bf_params.U = toml::get<std::vector<double>>(in_base_flow.at("U"));
   bf_params.gamma = in_base_flow.at("gamma").as_floating();
}

/// Internal helper function here for getting enumerator from string name.
template<OptionEnum E>
void GetEnumerator(const std::string_view input_str, E &val)
{
   const auto it = std::find(OptionNames<E>.begin(), 
                              OptionNames<E>.end(), 
                              input_str);
   if (it == OptionNames<E>.end())
   {
      throw std::invalid_argument(std::format("Invalid input argument: {}",
                                              input_str));
   }
   else
   {
      val = static_cast<E>(std::distance(OptionNames<E>.begin(), it));
   }
}

template<>
void TOMLConfigInput::ParseOption<InputXY>
   (std::string option_serialized, OptionParamsVar<InputXY> &params_var)
{
   toml::value in_xy = toml::parse_str(option_serialized);
   InputXY in_xy_option;
   GetEnumerator(in_xy.at("Type").as_string(), in_xy_option);

   if (in_xy_option == InputXY::Here)
   {
      OptionParams<InputXY::Here> xy_params;
      xy_params.x = toml::get<std::vector<double>>(in_xy.at("X"));
      xy_params.y = toml::get<std::vector<double>>(in_xy.at("Y"));
      params_var = xy_params;
   }
   else if (in_xy_option == InputXY::FromCSV)
   {
      OptionParams<InputXY::FromCSV> xy_params;
      xy_params.file = in_xy.at("File").as_string();
      params_var = xy_params;
   }
}

template<>
void TOMLConfigInput::ParseOption<FunctionType>
   (std::string option_serialized, OptionParamsVar<FunctionType> &params_var)
{
   toml::value in_func = toml::parse_str(option_serialized);
   FunctionType func_option;
   GetEnumerator(in_func.at("Type").as_string(), func_option);

   if (func_option == FunctionType::PiecewiseLinear)
   {
      OptionParams<FunctionType::PiecewiseLinear> fp;
      ParseOption<InputXY>(toml::format(in_func.at("Data")), fp.input_xy);
      params_var = fp;
   }
   else if (func_option == FunctionType::PiecewiseLogLog)
   {
      OptionParams<FunctionType::PiecewiseLogLog> fp;
      ParseOption<InputXY>(toml::format(in_func.at("Data")), fp.input_xy);
      params_var = fp;
   }
}

template<>
void TOMLConfigInput::ParseOption<DiscMethod>
   (std::string option_serialized,OptionParamsVar<DiscMethod> &params_var)
{
   toml::value in_disc_method = toml::parse_str(option_serialized);
   DiscMethod disc_option;
   GetEnumerator(in_disc_method.at("Type").as_string(), disc_option);

   if (disc_option == DiscMethod::Uniform)
   {
      params_var = OptionParams<DiscMethod::Uniform>{};
   }
   else if (disc_option == DiscMethod::UniformLog)
   {
      params_var = OptionParams<DiscMethod::UniformLog>{};
   }
   else if (disc_option == DiscMethod::Random)
   {
      OptionParams<DiscMethod::Random> disc_params;
      disc_params.seed = in_disc_method.at("Seed").as_integer();
      params_var = disc_params;
   }
   else if (disc_option == DiscMethod::RandomLog)
   {
      OptionParams<DiscMethod::RandomLog> disc_params;
      disc_params.seed = in_disc_method.at("Seed").as_integer();
      params_var = disc_params;
   }
}

template<>
void TOMLConfigInput::ParseOption<Direction>
   (std::string option_serialized, OptionParamsVar<Direction> &params_var)
{
   toml::value in_dir = toml::parse_str(option_serialized);
   Direction dir_option;
   GetEnumerator(in_dir.at("Type").as_string(), dir_option);

   if (dir_option == Direction::Constant)
   {
      OptionParams<Direction::Constant> dir_params;
      dir_params.direction = toml::get<std::vector<double>>(
                                             in_dir.at("Vector"));
      params_var = dir_params;
   }
   else if (dir_option == Direction::RandomXYAngle)
   {
      OptionParams<Direction::RandomXYAngle> dir_params;
      dir_params.min_angle = in_dir.at("MinAngle").as_floating();
      dir_params.max_angle = in_dir.at("MaxAngle").as_floating();
      dir_params.seed = in_dir.at("Seed").as_integer();
      params_var = dir_params;
   }
}                                       

template<>
void TOMLConfigInput::ParseOption<Source>
   (std::string option_serialized, OptionParamsVar<Source> &params_var)
{
   toml::value in_source = toml::parse_str(option_serialized);
   Source source_op;
   GetEnumerator(in_source.at("Type").as_string(), source_op);

   if (source_op == Source::SingleWave)
   {
      OptionParams<Source::SingleWave> meta;

      meta.amp = in_source.at("Amplitude").as_floating();
      meta.freq = in_source.at("Frequency").as_floating();
      meta.direction = toml::get<std::vector<double>>(
                                       in_source.at("DirVector"));
      meta.phase = in_source.at("Phase").as_floating();
      meta.speed = *(in_source.at("Speed").as_string().data());

      params_var = meta;
   }
   else if (source_op == Source::WaveSpectrum)
   {
      OptionParams<Source::WaveSpectrum> meta;
      
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
      
      params_var = meta;
   }
   else if (source_op == Source::PSD)
   {
      OptionParams<Source::PSD> meta;

      meta.dim_fac = in_source.at("ScaleFactor").as_floating();
      
      toml::value in_disc = in_source.at("Discretization");
      meta.min_disc_freq = in_disc.at("Min").as_floating();
      meta.max_disc_freq = in_disc.at("Max").as_floating();
      meta.num_waves = in_disc.at("N").as_integer();
      GetEnumerator(in_disc.at("Interval").as_string(), meta.int_method);
      meta.phase_seed = in_source.at("PhaseSeed").as_integer();
      meta.speed = *(in_source.at("Speed").as_string().data());

      toml::value in_input = in_source.at("InputPSD");
      ParseOption<FunctionType>(toml::format(in_source.at("InputPSD")), 
                              meta.input_psd);

      
      ParseOption<DiscMethod>(toml::format(in_disc.at("Method")),
                              meta.disc_params);

      ParseOption<Direction>(toml::format(in_source.at("Direction")), 
                              meta.dir_params);

      params_var = meta;
   }
   else if (source_op == Source::WaveCSV)
   {
      OptionParams<Source::WaveCSV> meta;
      meta.file = in_source.at("File").as_string();
      params_var = meta;
   }
}

void TOMLConfigInput::ParseComputation(std::string comp_serialized,
                                       CompParams &c_params)
{
   toml::value in_comp = toml::parse_str(comp_serialized);
   c_params.t0 = in_comp.at("t0").as_floating();
   GetEnumerator(in_comp.at("Kernel").as_string(), c_params.kernel);
}

void TOMLConfigInput::ParsePrecice(std::string precice_serialized, 
                                    PreciceParams &p_params)
{
   toml::value in_precice = toml::parse_str(precice_serialized);
   p_params.participant_name = in_precice.at("ParticipantName")
                                             .as_string();
   p_params.config_file = in_precice.at("ConfigFile").as_string();
   p_params.fluid_mesh_name = in_precice.at("FluidMeshName").as_string();
   p_params.mesh_access_region = 
         toml::get<std::vector<double>>(in_precice.at("MeshAccessRegion"));
}

TOMLConfigInput::TOMLConfigInput(std::string config_file, std::ostream *out)
{
   toml::value file = toml::parse(config_file);

   // Parse base flow parameters
   toml::value in_base_flow = file.at("BaseFlow");
   ParseBaseFlow(toml::format(in_base_flow), base_flow_);
   if (out)
   {
      PrintBaseFlowParams(*out);
   }

   // Parse the acoustic sources of run
   toml::array in_sources = file.at("Sources").as_array();
   for (const toml::value &in_source : in_sources)
   {
      sources_.emplace_back();
      ParseOption<Source>(toml::format(in_source), sources_.back());
   }
   if (out)
   {
      PrintSourceParams(*out);
   }

   // Parse compute fields of run
   toml::value in_comp = file.at("Computation");
   ParseComputation(toml::format(in_comp), comp_);
   if (out)
   {
      PrintCompParams(*out);
   }

   // Parse preCICE related fields - if exists
   if (file.contains("preCICE"))
   {
      toml::value in_precice = file.at("preCICE");
      precice_ = PreciceParams();
      ParsePrecice(toml::format(in_precice), *precice_);
      if (out)
      {
         PrintPreciceParams(*out);
      }
   }
}

} // namespace jabber
