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

   /// **Normalized** wave directional vector.
   std::vector<double> k_hat;

   /// Wave speed flag. "true" if slow, "false" if fast.
   bool speed_flag;
};

/**
 * @brief Class for specifying and computing a broadband-spectrum acoustic
 * field onto a provided grid and base flow.
 * 
 * @details All acoustic wave data is stored in a Struct-of-Arrays style in
 * this class.
 * 
 */
class AcousticField
{
private:

   /**
    * @brief Generalized kernel function for evaluating perturbed base flow
    * using CPU vectorization.
    * 
    * 
    * @details This function was designed following Intel guidelines for 
    * vectorizing code - including that the innermost loop is the vectorized
    * one. As such, this function is templated with \p TDim for loop unrolling
    * on the momentum terms. Data is stored in a SoA-format for contiguous 
    * memory accesses across hardware threads.
    * 
    * @tparam TDim            Physical dimension.
    * 
    * @param num_pts          Number of physical points to evaluate at.
    * @param rho_bar          Base flow density.
    * @param p_bar            Base flow pressure.
    * @param U_bar            Base flow velocity.
    * @param gamma            Specific heat ratio.
    * @param num_waves        Number of acoustic waves to compute.
    * @param wave_amps        Acoustic wave amplitudes, sized \p num_waves.
    * @param wave_omegas      Acoustic wave angular frequencies, sized 
    *                         \p num_waves.
    * @param wave_dirs        Acoustic wave normalized direction vectors
    *                         (k_hat), dimensioned as \p num_waves x \p TDim.
    * @param k_dot_x_p_phi    k·x+φ term computed for all waves at all points,
    *                         dimensioned as \p num_waves x \p num_pts but
    *                         flattened.
    * @param t                Time.
    * @param rho              Output flow density to compute, sized \p num_pts.
    * @param rhoV             Output flow momentum vector to compute,
    *                         dimensioned as \p TDim x \p num_pts but flattened.
    * @param rhoE             Output flow energy to compute, sized \p num_pts.
    */
   template<int TDim>
   static void ComputeKernel(const std::size_t num_pts, const double rho_bar,
                             const double p_bar, const double *U_bar, 
                             const double gamma, const int num_waves, 
                             const double *wave_amps, 
                             const double *wave_omegas,
                             const double *wave_dirs,
                             const double *__restrict__ k_dot_x_p_phi,
                             const double t,
                             double *__restrict__ rho,
                             double *__restrict__ rhoV,
                             double *__restrict__ rhoE);

   /// Spatial dimension.
   const int dim_;

   /// Number of points/coordinates of field.
   const std::size_t num_pts_;

   /// Base flow pressure.
   const double p_bar_;

   /// Base flow density.
   const double rho_bar_;

   /// Base flow velocity vector, of size \ref Dim().
   const std::vector<double> U_bar_;

   /// Base flow specific heat ratio, γ.
   const double gamma_;

   /// Base flow speed of sound
   const double c_infty_;

   /// SoA coordinates to compute waves on, [dim][node].
   std::vector<std::vector<double>> coords_;

   /// Array of all wave data (AoS).
   std::vector<Wave> waves_;

   /**
    * @brief Array of all wave amplitudes p', assembled in \ref Finalize(),
    * for contiguous memory storage.
    * 
    * @details Pre-assembling this before calls to \ref Compute() is required.
    * Size is \ref NumWaves().
    */
   std::vector<double> amplitude_;

   /**
    * @brief Array of all wave direction vectors assembled in \ref Finalize() 
    * for contiguous memory storage.
    * 
    * @details Pre-assembling this before calls to \ref Compute() is required.
    * Size is \ref Dim() * \ref NumWaves(). This is a flattened array ordered
    * as [dim][wave].
    */
   std::vector<double> k_hat_;

   /**
    * @brief k·x+φ term computed in \ref Finalize() once for each wave and
    * point.
    * 
    * @details Pre-computing this before calls to \ref Compute() is required, 
    * as it reduces redundant inner-loop FLOPs. Size is \ref NumWaves() * 
    * \ref NumPoints(). This is a flattened array is ordered as [wave][point].
    */
   std::vector<double> k_dot_x_p_phi_;

   /**
    * @brief ω=2πf coefficient computed in \ref Finalize() once for each wave
    * and point.
    * 
    * @details Pre-computing this before calls to \ref Compute() is required, 
    * as it reduces redundant inner-loop FLOPs. Size is \ref NumWaves().
    */
   std::vector<double> omega_;

   /**
    * @brief Fluid density ρ, computed in \ref Compute().
    * 
    * @details Size is \ref NumPoints().
    */
   std::vector<double> rho_;

   /**
    * @brief Fluid momentum ρu, computed in \ref Compute().
    * 
    * @details Size is \ref NumPoints() * \ref Dim(), with data in XXX YYY 
    * ordering.
    */
   std::vector<double> rhoV_;

   /**
    * @brief Fluid energy ρE, computed in \ref Compute().
    * 
    * @details Size is \ref NumPoints().
    */
   std::vector<double> rhoE_;

public:
   /**
    * @brief Construct a new AcousticField object.
    * 
    * @param dim        Spatial dimension of mesh.
    * @param coords     Mesh coordinates to compute acoustic forcing on, in
    *                   XYZ XYZ ordering (output from precice::Participant
    *                   ::getMeshVertexIDsAndCoordinates).
    * @param p_bar      Base flow pressure.
    * @param rho_bar    Base flow density.
    * @param U_bar      Base flow velocity vector, of size \p dim.
    * @param gamma      Base flow specific heat ratio, γ.
    */
   AcousticField(int dim, const std::vector<double> coords,
                  double p_bar, double rho_bar,
                  const std::vector<double> U_bar, double gamma);

   /// Get the spatial dimension.
   int Dim() const { return dim_; }

   /// Get the number of points/coordinates associated with this field.
   std::size_t NumPoints() const { return num_pts_; }

   /// Get the base flow velocity vector.
   const std::vector<double>& BaseVelocity() const { return U_bar_; }
   
   /// Get the base flow pressure
   double BasePressure() const { return p_bar_; }

   /// Get the base flow density
   double BaseDensity() const { return rho_bar_; }

   /// Get the base flow specific heat ratio, γ.
   double Gamma() const { return gamma_; }

   /// Get the number of waves.
   int NumWaves() const { return waves_.size(); }

   /// Set the number of waves.
   void SetNumWaves(int num_waves) { waves_.resize(num_waves); }

   /**
    * @brief Reserve a number of waves to be included in the field.
    * 
    * @details Calling this prior to any \ref AddWave() can improve
    * initialization efficiency.
    */
   void ReserveNumWaves(int res_waves) { waves_.reserve(res_waves); }

   /// Add a Wave to the acoustic field.
   void AddWave(const Wave &w);
   
   /// Get reference to Wave \p i from acoustic field.
   Wave& GetWave(int i) { return waves_[i]; }
   
   /// Get const reference to Wave \p i from acoustic field.
   const Wave& GetWave(int i) const { return waves_[i]; }

   /**
    * @brief Finalize the acoustic field, to be called after specifying all
    * waves before \ref Compute().
    * 
    * @details This function:
    *    1. Evaluates factors that are constant in time to reduce inner-loop
    *       FLOPS in \ref Compute() (In particular, \ref k_dot_x_p_phi_ 
    *       and \ref omega_).
    *    2. Assembles pressure wave amplitudes in \ref amplitude_ such that
    *       they are contiguous in memory.
    *    3. Allocates the flowfield solution memory \ref rho, \ref rhoV, 
    *       and \ref rhoE vectors.
    * 
    * It should be noted that the actual performance impact of these
    * precautions has not been profiled.
    */
   void Finalize();

   /// Compute the perturbed flowfield at time \p t.
   void Compute(double t);



};

} // namespace jabber

#endif // JABBER_ACOUSTIC_FIELD
