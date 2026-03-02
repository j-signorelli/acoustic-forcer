#include "config_input.hpp"

#include <toml.hpp>

#include <iomanip>
#include <format>
#include <algorithm>

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

   using enum Source::Option;

   out << "Sources" << std::endl;

   for (const Source::ParamsVariant &source : sources_)
   {
      std::visit(
      overloads
      {
      [&out](const Source::Params<SingleWave> &wave)
      {
         std::size_t name_i 
                        = static_cast<std::size_t>(SingleWave);
                        
         constexpr int label_width = 11;

         out << WriteParam("Type", Source::kNames[name_i], label_width);
         out << WriteParam("Amplitude", OutReal(wave.amp), label_width);
         out << WriteParam("Frequency", OutReal(wave.freq), label_width);
         out << WriteParam("Direction", OutRealVec(wave.direction), 
                                                            label_width);
         out << WriteParam("Phase", OutReal(wave.phase), label_width);
         out << WriteParam("Speed", std::string(&wave.speed), label_width);
         out << std::endl;
      },
      [&out](const Source::Params<WaveSpectrum> &waves)
      {
         std::size_t name_i = 
                        static_cast<std::size_t>(WaveSpectrum);
         
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
         out << WriteParam("Type", Source::kNames[name_i], label_width);
         out << WriteParam("Amplitudes", amplitudes_str, label_width);
         out << WriteParam("Frequencies", freqs_str, label_width);
         out << WriteParam("Directions", dirs_str, label_width);
         out << WriteParam("Phases", phases_str, label_width);
         out << WriteParam("Speeds", speeds_str, label_width);
         out << std::endl;
      },
      [&out](const Source::Params<PSD> &waves)
      {
         constexpr int label_width = 13;

         /// @todo: Finish this
         out << "Unimplemented verbose output for source type" << std::endl;
      },
      [&out](const Source::Params<WaveCSV> &waves)
      {
         constexpr int label_width = 7;

         std::size_t name_i = 
                        static_cast<std::size_t>(WaveSpectrum);
         out << WriteParam("Type", Source::kNames[name_i], label_width);
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
                  KernelType::kNames[static_cast<std::size_t>(comp_.kernel)],
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

void TOMLConfigInput::ParseBaseFlow
   (std::string toml_string, BaseFlowParams &op)
{
   toml::value in_val = toml::parse_str(toml_string);

   op.rho = in_val.at("rho").as_floating();
   op.p = in_val.at("p").as_floating();
   op.U = toml::get<std::vector<double>>(in_val.at("U"));
   op.gamma = in_val.at("gamma").as_floating();
}

/// Internal helper function here for getting enumerator from string.
template<typename T>
T::Option GetOption(const std::string &name)
{
   using Option = typename T::Option;

   const auto it = std::find(T::kNames.begin(), T::kNames.end(), name);

   Option option;

   if (it == T::kNames.end())
   {
      throw std::invalid_argument(std::format("Invalid input argument: {}",
                                              name));
   }
   else
   {
      option = static_cast<T::Option>(std::distance(T::kNames.begin(), it));
   }
   return option;
}

void TOMLConfigInput::ParseInputXY
   (std::string toml_string, InputXY::ParamsVariant &opv)
{
   using enum InputXY::Option;

   toml::table in_val = toml::parse_str(toml_string).as_table();
   InputXY::Option option = GetOption<InputXY>(in_val.at("Type").as_string());

   if (option == Here)
   {
      InputXY::Params<Here> op;
      op.x = toml::get<std::vector<double>>(in_val.at("X"));
      op.y = toml::get<std::vector<double>>(in_val.at("Y"));
      opv = op;
   }
   else if (option == FromCSV)
   {
      InputXY::Params<FromCSV> op;
      op.file = in_val.at("File").as_string();
      opv = op;
   }
}

void TOMLConfigInput::ParseFunctionType
   (std::string toml_string, FunctionType::ParamsVariant &opv)
{
   using enum FunctionType::Option;

   toml::value in_val = toml::parse_str(toml_string);
   FunctionType::Option option = 
            GetOption<FunctionType>(in_val.at("Type").as_string());

   if (option == PiecewiseLinear)
   {
      FunctionType::Params<PiecewiseLinear> op;
      in_val.at("Data").as_table_fmt().fmt = toml::table_format::multiline;
      ParseInputXY(toml::format(in_val.at("Data")), op.input_xy);
      opv = op;
   }
   else if (option == PiecewiseLogLog)
   {
      FunctionType::Params<PiecewiseLogLog> op;
      in_val.at("Data").as_table_fmt().fmt = toml::table_format::multiline;
      ParseInputXY(toml::format(in_val.at("Data")), op.input_xy);
      opv = op;
   }
}

void TOMLConfigInput::ParseDiscMethod
   (std::string toml_string, DiscMethod::ParamsVariant &opv)
{
   using enum DiscMethod::Option;

   toml::value in_val = toml::parse_str(toml_string);
   DiscMethod::Option option = 
               GetOption<DiscMethod>(in_val.at("Type").as_string());

   if (option == Uniform)
   {
      opv = DiscMethod::Params<Uniform>{};
   }
   else if (option == UniformLog)
   {
      opv = DiscMethod::Params<UniformLog>{};
   }
   else if (option == Random)
   {
      DiscMethod::Params<Random> op;
      op.seed = in_val.at("Seed").as_integer();
      opv = op;
   }
   else if (option == RandomLog)
   {
      DiscMethod::Params<RandomLog> op;
      op.seed = in_val.at("Seed").as_integer();
      opv = op;
   }
}

void TOMLConfigInput::ParseDirection
   (std::string toml_string, Direction::ParamsVariant &opv)
{
   using enum Direction::Option;

   toml::value in_val = toml::parse_str(toml_string);
   Direction::Option option = 
                     GetOption<Direction>(in_val.at("Type").as_string());

   if (option == Constant)
   {
      Direction::Params<Constant> op;
      op.direction = toml::get<std::vector<double>>(in_val.at("Vector"));
      opv = op;
   }
   else if (option == RandomXYAngle)
   {
      Direction::Params<RandomXYAngle> op;
      op.min_angle = in_val.at("MinAngle").as_floating();
      op.max_angle = in_val.at("MaxAngle").as_floating();
      op.seed = in_val.at("Seed").as_integer();
      opv = op;
   }
}

void TOMLConfigInput::ParseTransferFunction
   (std::string toml_string, TransferFunction::ParamsVariant &opv)
{
   using enum TransferFunction::Option;

   toml::value in_val = toml::parse_str(toml_string);

   TransferFunction::Option option = 
            GetOption<TransferFunction>(in_val.at("Type").as_string());

   if (option == LowFrequencyLimit)
   {
      opv = TransferFunction::Params<LowFrequencyLimit>{};
   }
   else if (option == Input)
   {
      TransferFunction::Params<Input> op;
      in_val.at("InputTF").as_table_fmt().fmt = toml::table_format::multiline;
      ParseFunctionType(toml::format(in_val.at("InputTF")), op.input_tf);
      opv = op;
   }
   else if (option == FlowNormalFit)
   {
      TransferFunction::Params<FlowNormalFit> op;
      op.shock_standoff_dist = in_val.at("ShockStandoffDist").as_floating();
      opv = op;
   }
}

void TOMLConfigInput::ParseSource
   (std::string toml_string, Source::ParamsVariant &opv)
{
   using enum Source::Option;

   toml::value in_val = toml::parse_str(toml_string);

   Source::Option option = GetOption<Source>(in_val.at("Type").as_string());

   if (option == SingleWave)
   {
      Source::Params<SingleWave> op;

      op.amp = in_val.at("Amplitude").as_floating();
      op.freq = in_val.at("Frequency").as_floating();
      op.direction = toml::get<std::vector<double>>(
                                       in_val.at("DirVector"));
      op.phase = in_val.at("Phase").as_floating();
      op.speed = *(in_val.at("Speed").as_string().data());

      opv = op;
   }
   else if (option == WaveSpectrum)
   {
      Source::Params<WaveSpectrum> op;
      
      op.amps = toml::get<std::vector<double>>(
                                          in_val.at("Amplitudes"));
      op.freqs = toml::get<std::vector<double>>(
                                          in_val.at("Frequencies"));
      op.directions = toml::get<std::vector<std::vector<double>>>(
                                          in_val.at("DirVectors"));
      op.phases = toml::get<std::vector<double>>(
                                             in_val.at("Phases"));
      std::vector<std::string> speed_strs = 
                  toml::get<std::vector<std::string>>(in_val.at("Speeds"));
      op.speeds.resize(speed_strs.size());
      std::transform(speed_strs.begin(), speed_strs.end(), op.speeds.begin(),
                     [](std::string &s) -> char { return *(s.data()); });
      
      opv = op;
   }
   else if (option == PSD)
   {
      Source::Params<PSD> op;

      op.dim_fac = in_val.at("ScaleFactor").as_floating();
      op.phase_seed = in_val.at("PhaseSeed").as_integer();
      op.speed = *(in_val.at("Speed").as_string().data());
      
      in_val.at("InputPSD").as_table_fmt().fmt = toml::table_format::multiline;
      ParseFunctionType(toml::format(in_val.at("InputPSD")), op.input_psd);
      
      toml::value in_disc_val = in_val.at("Discretization");
      op.min_disc_freq = in_disc_val.at("Min").as_floating();
      op.max_disc_freq = in_disc_val.at("Max").as_floating();
      op.num_waves = in_disc_val.at("N").as_integer();
      op.int_method = 
         GetOption<IntervalType>(in_disc_val.at("Interval").as_string());
      
      in_disc_val.at("Method").as_table_fmt().fmt = 
                                                toml::table_format::multiline;
      ParseDiscMethod(toml::format(in_disc_val.at("Method")), op.disc_params);

      in_val.at("Direction").as_table_fmt().fmt = 
                                                toml::table_format::multiline;
      ParseDirection(toml::format(in_val.at("Direction")), op.dir_params);

      if (in_val.contains("TransferFunction"))
      {
         op.tf_params = TransferFunction::ParamsVariant{};
         in_val.at("TransferFunction").as_table_fmt().fmt = 
                                                toml::table_format::multiline;
         ParseTransferFunction(toml::format(in_val.at("TransferFunction")), 
                                 *op.tf_params);
      }
      opv = op;
   }
   else if (option == WaveCSV)
   {
      Source::Params<WaveCSV> op;
      op.file = in_val.at("File").as_string();
      opv = op;
   }
}

void TOMLConfigInput::ParseComputation
   (std::string toml_string, CompParams &op)
{
   toml::value in_val = toml::parse_str(toml_string);
   op.t0 = in_val.at("t0").as_floating();
   op.kernel = GetOption<KernelType>(in_val.at("Kernel").as_string());
}

void TOMLConfigInput::ParsePrecice
   (std::string toml_string, PreciceParams &op)
{
   toml::value in_val = toml::parse_str(toml_string);
   
   op.participant_name = in_val.at("ParticipantName")
                                             .as_string();
   op.config_file = in_val.at("ConfigFile").as_string();
   op.fluid_mesh_name = in_val.at("FluidMeshName").as_string();
   op.mesh_access_region = 
         toml::get<std::vector<double>>(in_val.at("MeshAccessRegion"));
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
      ParseSource(toml::format(in_source), sources_.emplace_back());
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
      precice_ = PreciceParams{};
      ParsePrecice(toml::format(in_precice), *precice_);
      if (out)
      {
         PrintPreciceParams(*out);
      }
   }
}

} // namespace jabber
