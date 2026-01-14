#ifndef JABBER_ACOUSTIC_FIELD
#define JABBER_ACOUSTIC_FIELD

#include <vector>

namespace jabber
{

/// Intermediary helper-struct for initializing waves in AcousticField.
struct Wave
{
   /// Wave amplitude, p'.
   double amplitude;

   /// Wave frequency, f.
   double frequency;

   /// Wave phase, φ.
   double phase;

   /// Wavenumber vector, k.
   std::vector<double> k;
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

   /// Number of points/coordinates of field.
   const std::size_t num_pts;

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
   std::vector<std::vector<double>> k_;

   /**
    * @brief k·x+φ term computed once for each wave in \ref Finalize().
    * Pre-computing this before calls to \ref Compute() is required, as it
    * reduces inner-loop FLOPs.
    */
   std::vector<double> k_dot_x_p_phi_;

   /**
    * @brief ω=2πf coefficient computed once for each wave in \ref Finalize().
    * Pre-computing this before calls to \ref Compute() is required, as it
    * reduces inner-loop FLOPs.
    */
   std::vector<double> omega_;
   
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

   /// Get the number of points/coordinates associated with this field.
   int NumPoints() const { return num_pts; }

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
   
   /// Get copy of Wave data from acoustic field.
   void GetWave(int i, Wave &w) const;

   /// Get reference to amplitude of wave \p i.
   double& Amplitude(int i) { return amplitude_[i]; }

   /// Get const reference to amplitude of wave \p i.
   const double& Amplitude(int i) const { return amplitude_[i]; }

   /// Get reference to frequency of wave \p i.
   double& Frequency(int i) { return frequency_[i]; }

   /// Get const reference to frequency of wave \p i.
   const double& Frequency(int i) const { return frequency_[i]; }

   /// Get reference to phase of wave \p i.
   double& Phase(int i) { return phase_[i]; }

   /// Get const reference to phase of wave \p i.
   const double& Phase(int i) const { return phase_[i]; }

   /// Get reference to wavenumber vector component \p d of wave \p i.
   double& WaveNumber(int d, int i) { return k_[d][i]; }

   /// Get const reference to wavenumber vector component \p d of wave \p i.
   const double& WaveNumber(int d, int i) const { return k_[d][i]; }

   /**
    * @brief Finalize the acoustic field, to be called after specifying all
    * waves before \ref Compute().
    * 
    * @details This function evaluates any and all factors that remain
    * constant across all waves, to reduce inner-loop FLOPs in \ref Compute().
    * In particular, \ref k_dot_x_p_phi_ and \ref omega_.
    */
   void Finalize();

   /**
    * @brief Compute the acoustic field at time \p t.
    * 
    * @param t          Time.
    * @param p_prime    Output perturbation pressures, at coordinates specified
    *                   at construction.
    */
   void Compute(double t, std::vector<double> &p_prime) const;
};

} // namespace jabber

#endif // JABBER_ACOUSTIC_FIELD
