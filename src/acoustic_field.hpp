#ifndef JABBER_ACOUSTIC_FIELD
#define JABBER_ACOUSTIC_FIELD

#include <vector>
#include <span>

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

   /// Wave speed flag. "true" if slow, "false" if fast.
   bool speed_flag;

   /// **Normalized** wave directional vector.
   std::vector<double> k_hat;
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
    * @brief Array of all **modified** (speed-encoded) wave direction vectors 
    * assembled in \ref Finalize() for contiguous memory storage. For fast 
    * acoustic waves, this holds just the wave direction vector. For slow
    * acoustic waves, this holds the inverted/negated wave direction vector.
    * 
    * @details Pre-assembling this before calls to \ref Compute() is required.
    * Size is \ref Dim() * \ref NumWaves(). This is a flattened array ordered
    * as [dim][wave].
    */
   std::vector<double> mod_k_hat_;

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
    *                   XYZ XYZ ordering.
    * @param p_bar      Base flow pressure.
    * @param rho_bar    Base flow density.
    * @param U_bar      Base flow velocity vector, of size \p dim.
    * @param gamma      Base flow specific heat ratio, γ.
    */
   AcousticField(int dim, std::span<const double> coords,
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
   void AddWave(const Wave &w) { waves_.push_back(w); }
   
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
    *       FLOPS in \ref Compute() (In particular, \ref k_dot_x_p_phi_, 
    *       \ref omega_, and \ref mod_k_hat_).
    *    2. Assembles pressure wave amplitudes in \ref amplitude_ such that
    *       they are contiguous in memory.
    *    3. Allocates the flowfield solution memory \ref rho, \ref rhoV, 
    *       and \ref rhoE vectors.
    *
    */
   void Finalize();

   /// Compute the perturbed flowfield at time \p t.
   void Compute(double t);
   
   /**
    * @brief Get span of computed flow densities.
    * 
    * @warning This should only be called after \ref Compute().
    */
   std::span<double> Density() { return rho_; }
   
   /**
    * @brief Get const span of computed flow densities.
    * 
    * @warning This should only be called after \ref Compute().
    */
   std::span<const double> Density() const { return rho_; }

   /**
    * @brief Get span of flow momentum across all components.
    * 
    * @warning This should only be called after \ref Compute().
    */
   std::span<double> Momentum()
   { 
      return std::span<double>(rhoV_);
   }
   
   /**
    * @brief Get const span of flow momentum across all components.
    * 
    * @warning This should only be called after \ref Compute().
    */
   std::span<const double> Momentum() const
   { 
      return std::span<const double>(rhoV_);
   }

   /**
    * @brief Get span of computed flow momentum for component \p comp.
    * 
    * @warning This should only be called after \ref Compute().
    */
   std::span<double> Momentum(int comp)
   { 
      return std::span<double>(rhoV_).subspan(num_pts_*comp, num_pts_);
   }
   
   /**
    * @brief Get const span of computed flow momentum for component \p comp.
    * 
    * @warning This should only be called after \ref Compute().
    */
   std::span<const double> Momentum(int comp) const
   { 
      return std::span<const double>(rhoV_).subspan(num_pts_*comp, num_pts_);
   }

   /**
    * @brief Get span of computed flow energy.
    * 
    * @warning This should only be called after \ref Compute().
    */
   std::span<double> Energy() { return rhoE_; }
   
   /**
    * @brief Get const span of computed flow energy.
    * 
    * @warning This should only be called after \ref Compute().
    */
   std::span<const double> Energy() const { return rhoE_; }


};

} // namespace jabber

#endif // JABBER_ACOUSTIC_FIELD
