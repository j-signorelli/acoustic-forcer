#ifndef JABBER_ACOUSTIC_FIELD
#define JABBER_ACOUSTIC_FIELD

#include <vector>

namespace jabber
{

/// Intermediary helper-struct for initializing waves in AcousticField.
struct Wave
{
   double amplitude;
   double frequency;
   double phase;
   std::vector<double> wave_number_vec;
};

/**
 * @brief Class for specifying and computing a broadband-spectrum acoustic
 * field onto a provided grid.
 * 
 * @details All acoustic wave data is stored in a Struct-of-Arrays style in
 * this class.
 * 
 */
class AcousticField
{
private:
   /// Spatial dimension.
   const int dim_;

   /// SoA coordinates to compute waves on, [dim][node].
   std::vector<std::vector<double>> coords_;

   /// Number of waves.
   int num_waves_;

   /// Array of all wave amplitudes.
   std::vector<double> amplitude_;

   /// Array of all wave frequencies.
   std::vector<double> frequency_;

   /// Array of all wave phases.
   std::vector<double> phase_;

   /// SoA of all wave wave-number vectors, [dim][wave].
   std::vector<std::vector<double>> wave_number_;

   /**
    * @brief k·x+φ term computed once for each wave in \ref Finalize().
    * Pre-computing this before calls to \ref Compute() reduces inner-loop
    * FLOPs.
    */
   std::vector<double> factor_;
   
public:
   /**
    * @brief Construct a new AcousticField object.
    * 
    * @param dim        Spatial dimension of mesh.
    * @param coords     Mesh coordinates to compute acoustic forcing on, in
    *                   XYZ XYZ ordering (output from precice::Participant
    *                   ::getMeshVertexIDsAndCoordinates).
    */
   AcousticField(int dim, const std::vector<double> coords);

   /// Get the spatial dimesion.
   int Dim() const { return dim_; }

   /// Get the number of waves.
   int NumWaves() const { return num_waves_; }

   /// Set the number of waves.
   void SetNumWaves(int num_waves);

   /**
    * @brief Reserve a number of waves to be included in the field.
    * 
    * @details Calling this prior to any \ref AddWave() can improve
    * initialization efficiency.
    */
   void ReserveNumWaves(int res_waves);

   /// Add a Wave to the acoustic field.
   void AddWave(const Wave &w);
   
   double& Amplitude(int i) { return amplitude_[i]; }
   const double& Amplitude(int i) const { return amplitude_[i]; }

   double& Frequency(int i) { return frequency_[i]; }
   const double& Frequency(int i) const { return frequency_[i]; }

   double& Phase(int i) { return phase_[i]; }
   const double& Phase(int i) const { return phase_[i]; }

   double& WaveNumber(int d, int i) { return wave_number_[d][i]; }
   const double& WaveNumber(int d, int i) const { return wave_number_[d][i]; }

   void Finalize();

   void Evaluate(double t, std::vector<double> &p_prime) const;
};

} // namespace jabber

#endif // JABBER_ACOUSTIC_FIELD
