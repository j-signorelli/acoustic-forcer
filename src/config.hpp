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

   /// Single acoustic wave
   struct SingleWave
   {
      double amp;
      double freq;
      double phase;
      double angle;
   };
   using ModeMeta = std::variant<SingleWave>;

   /// Relevant preCICE information
   struct PreciceMeta
   {
      std::string participant_name;
      std::string config_file;
      std::vector<double> mesh_access_region;
   };

private:
   ModeMeta mode_;
   PreciceMeta precice_;

public:
   Config(std::string config_file, std::ostream *out=nullptr);

   ModeMeta& Mode() { return mode_; }
   const ModeMeta& Mode() const { return mode_; }

   PreciceMeta& Precice() { return precice_; }
   const PreciceMeta& Precice() const { return precice_; }

   void PrintModeMeta(std::ostream &out);
   
   void PrintPreciceMeta(std::ostream &out);
};

} // namespace jabber

#endif // JABBER_CONFIG
