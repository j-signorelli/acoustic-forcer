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
   /// Get string of double \p f.
   static std::string OutReal(double f)
   {
      return std::format("{}", f);
   }

   /// Get string of double vector \p vec with precision \ref kPrecision.
   static std::string OutRealVec(const std::vector<double> &vec, 
                                    const std::string_view delim=", ")
   {
      std::stringstream ss;
      ss << "[";
      for (int i = 0; i < vec.size(); i++)
      {
         ss << OutReal(vec[i]) 
               << ((i+1 == vec.size()) ? "]" : delim);
      }
      return ss.str();
   }

   /**
    * @brief Convenient string generator for writing parameter information to 
    * console.
    */
   static std::string WriteParam(const std::string_view param_name, 
                                    const std::string_view value,
                                    int param_width, int left_margin=8)
   {
      return std::format("{:<{}}{:<{}}= {}\n", "", left_margin, param_name,
                                                   param_width, value);
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
    * @brief Parse base flow parameters from a serialized TOML string of that
    * section.
    */
   static void ParseBaseFlow(std::string base_flow_serialized, 
                              BaseFlowParams &params);

   /**
    * @brief Parse source parameters from a serialized TOML string of that
    * section, and append it to \ref sources_.
    */
   static void ParseSource(std::string source_serialized,
                           SourceParamsVariant &params_var);

   /**
    * @brief Parse computation parameters from a serialized TOML string of that
    * section.
    */
   static void ParseComputation(std::string comp_serialized,
                                 CompParams &params);

   /**
    * @brief Parse preCICE parameters from a serialized TOML string of that
    * section.
    */
   static void ParsePrecice(std::string precice_serialized,
                              PreciceParams &params);


   /// Construct an uninitialized TOMLConfigInput object.
   TOMLConfigInput() {};
   /**
    * @brief Construct a new TOMLConfigInput object.
    * 
    * @param config_file      TOML config file address to parse.
    * @param out              [Optional] ostream to write parsed config file
    *                         to (verbose processing).
    */
   TOMLConfigInput(std::string config_file, std::ostream *out=nullptr);

};

} // namespace jabber_app

#endif // JABBER_APP_CONFIG
