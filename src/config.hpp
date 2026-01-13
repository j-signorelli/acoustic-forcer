#ifndef JABBER_CONFIG
#define JABBER_CONFIG

#include <vector>
#include <string>
#include <variant>
#include <iostream>

namespace jabber
{

/// Parsed config file information
class Config
{
public:

   /// Base flow parameters
   struct BaseFlowMeta
   {
      double rho;
      double p;
      double U;
      double gamma;
   };

   /// Single acoustic wave
   struct SingleWaveMeta
   {
      double amp;
      double freq;
      double phase;
      double angle; // wrt x-axis in xy-plane
   };
   using ModeMeta = std::variant<SingleWaveMeta>;

   /// Relevant preCICE information
   struct PreciceMeta
   {
      std::string participant_name;
      std::string config_file;
      std::string fluid_mesh_name;
      std::vector<double> mesh_access_region;
   };

private:
   BaseFlowMeta base_flow_;
   ModeMeta mode_;
   PreciceMeta precice_;

public:
   Config(std::string config_file, std::ostream *out=nullptr);

   ModeMeta& Mode() { return mode_; }
   const ModeMeta& Mode() const { return mode_; }

   PreciceMeta& Precice() { return precice_; }
   const PreciceMeta& Precice() const { return precice_; }

   void PrintBaseFlowMeta(std::ostream &out) const;

   void PrintModeMeta(std::ostream &out) const;

   void PrintPreciceMeta(std::ostream &out) const;
};

} // namespace jabber

#endif // JABBER_CONFIG
