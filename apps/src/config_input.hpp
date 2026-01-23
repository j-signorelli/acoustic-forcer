#ifndef JABBER_APP_CONFIG
#define JABBER_APP_CONFIG

#include "params.hpp"

#include <iostream>
#include <format>
#include <sstream>

namespace jabber_app
{

/// Parsed config file input
class ConfigInput
{
private:
   /// Precision used when printing double/float data in \ref OutReal().
   static constexpr int kPrecision = 12;

   /// Get string of double \p f with precision \ref kPrecision.
   static std::string OutReal(double f)
   {
      return std::format("{:.{}f}", f, kPrecision);
   }

   /// Get string of double vector \p vec with precision \ref kPrecision.
   static std::string OutRealVec(const std::vector<double> &vec)
   {
      std::stringstream ss;
      ss << "[";
      for (int i = 0; i < vec.size(); i++)
      {
         ss << OutReal(vec[i]) 
               << ((i+1 == vec.size()) ? "]" : ", ");
      }
      return ss.str();
   }

protected:

   /// Input base flow parameters.
   BaseFlowParams base_flow_;

   /// Input source parameters.
   std::vector<SourceParamsVariant> sources_;

   /// Input computation parameters.
   CompParams comp_;

   /// Input preCICE parameters.
   std::optional<PreciceParams> precice_;

public:

   /// Get reference to base flow parameters.
   BaseFlowParams& BaseFlow() { return base_flow_; }

   /// Get const reference to base flow parameters.
   const BaseFlowParams& BaseFlow() const { return base_flow_; }

   /// Get reference to source parameters.
   std::vector<SourceParamsVariant>& Sources() { return sources_; }

   /// Get const reference to source parameters.
   const std::vector<SourceParamsVariant>& Sources() const { return sources_; }

   /// Get reference to computation parameters.
   CompParams& Comp() { return comp_; }

   /// Get const reference to computation parameters.
   const CompParams& Comp() const { return comp_; }

   /// Get reference to preCICE parameters.
   std::optional<PreciceParams>& Precice() { return precice_; }

   /// Get const reference to preCICE parameters.
   const std::optional<PreciceParams>& Precice() const { return precice_; }

   /// Print the configured base flow parameters.
   void PrintBaseFlowParams(std::ostream &out) const;

   /// Print the configured source parameters.
   void PrintSourceParams(std::ostream &out) const;

   /// Print the configured computation parameters.
   void PrintCompParams(std::ostream &out) const;

   /// Print the configured preCICE parameters.
   void PrintPreciceParams(std::ostream &out) const;
};

/// Parsed TOML config file input.
class TOMLConfigInput : public ConfigInput
{
public:
   /**
    * @brief Construct a new TOMLConfigInput object.
    * 
    * @param config_file      TOML config file to parse.
    * @param out              [Optional] ostream to write parsed config file
    *                         to (verbose processing).
    */
   TOMLConfigInput(std::string config_file, std::ostream *out=nullptr);
};

} // namespace jabber_app

#endif // JABBER_APP_CONFIG
