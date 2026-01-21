#include <jabber.hpp>
#include <jabber_app.hpp>
#include <cxxopts.hpp>
#include <precice/precice.hpp>

#include <iostream>
#include <regex>

using namespace jabber;
using namespace jabber_app;

int main(int argc, char *argv[])
{
   PrintBanner(std::cout);
   std::cout << "Jabber preCICE Participant" << std::endl << std::endl
               << LINE << std::endl;

   // Option parser:
   cxxopts::Options options("jabber_participant", 
      "preCICE participant for coupling acoustic forcing with flow "
      "simulations.");
   options.add_options()
      ("c,config", "Config file.", cxxopts::value<std::string>())
      ("h,help", "Print usage information.");
   cxxopts::ParseResult result = options.parse(argc, argv);

   std::string args_str = result.arguments_string();
   args_str = std::regex_replace(args_str, std::regex("\n"), "\n\t");
   std::cout << "Command Line Arguments\n\t" << args_str << std::endl
               << LINE << std::endl;
               
   if (result.count("help"))
   {
      std::cout << options.help() << std::endl;
      return 0;
   }
   if (result.count("config") == 0)
   {
      std::cout << "Error: no config file specified." << std::endl;
      return 1;
   }
   
   // Parse config file
   std::string config_file = result["config"].as<std::string>();
   TOMLConfigInput conf(config_file, &std::cout);
   std::cout << LINE << std::endl;

   // Get the preCICE input
   const PreciceParams &precice_conf = *(conf.Precice());

   // Initialize preCICE participant
   precice::Participant participant(precice_conf.participant_name,
                                    precice_conf.config_file, 0, 1);
   participant.setMeshAccessRegion(precice_conf.fluid_mesh_name,
                                    precice_conf.mesh_access_region);
   participant.initialize();

   // Get mesh information from fluid participant
   int dim = participant.getMeshDimensions(precice_conf.fluid_mesh_name);
   int vertex_size = participant.getMeshVertexSize(
                                          precice_conf.fluid_mesh_name);
   std::vector<double> coords(dim*vertex_size);
   std::vector<int> vertex_ids(vertex_size);
   participant.getMeshVertexIDsAndCoordinates(precice_conf.fluid_mesh_name,
                                              vertex_ids, coords);

   // Assemble AcousticField object
   std::cout << "Assembling acoustic field data... ";
   AcousticField field = InitializeAcousticField(conf, coords, dim);
   std::cout << "Done!" << std::endl;

   double time = conf.Comp().t0;
   double dt;

   // Compute acoustic forcing
   while (participant.isCouplingOngoing())
   {
      dt = participant.getMaxTimeStepSize();

      // Compute acoustic forcing
      field.Compute(time);

      // Send data
      participant.writeData(precice_conf.fluid_mesh_name, "rho",
                             vertex_ids, field.Density());
      for (int d = 0; d < dim; d++)
      {
         participant.writeData(precice_conf.fluid_mesh_name, 
                              "rhoV" + std::to_string(d+1),
                             vertex_ids, field.Momentum(d));
      }
      participant.writeData(precice_conf.fluid_mesh_name, "rhoE",
                             vertex_ids, field.Energy());       
      participant.advance(dt);
      time += dt;
   }
   
   participant.finalize();

   return 0;
}
