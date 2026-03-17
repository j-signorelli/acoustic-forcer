#include "transfer_functions.hpp"

#include <random>
#include <format>

namespace jabber
{

void LowFrequencyLimitTF(double mach_bar, double gamma, char speed,
                           std::span<double> powers)
{
   const double sign = speed == 'S' ? -1 : 1;
   const double num = (mach_bar*mach_bar + sign*2.0*mach_bar - sign/mach_bar);
   const double denom = (gamma*mach_bar*mach_bar - (gamma - 1.0)/2.0);
   const double frac = num/denom;

   const double chi = frac*frac;
   for (double &power : powers)
   {
      power /= chi;
   }
}

/// Point struct of (f,chi).
struct Point
{
   double f;
   double chi;
};

Point operator+(const Point &pt1, const Point &pt2)
{
   return Point{pt1.f + pt2.f, pt1.chi + pt2.chi};
}

Point operator-(const Point &pt1, const Point &pt2)
{
   return Point{pt1.f - pt2.f, pt1.chi - pt2.chi};
}

template<typename T>
   requires std::is_arithmetic_v<T>
Point operator*(const T &c, const Point &pt)
{
   return Point{pt.f * c, pt.chi * c};
}

template<typename T>
   requires std::is_arithmetic_v<T>
Point operator+(const T &c, const Point &pt)
{
   return Point{pt.f + c, pt.chi + c};
}

struct FlowNormalBezierFit
{
   /// Control points for the flow-normal TF Bezier curve fit.
   static constexpr std::array<Point, 9> kControlPts
   {{ {0.0     ,    1.0},
      {0.16204 ,    1.0},
      {0.36886 , 2.1021},
      {0.27954 , 4.6026},
      {0.47948 , 4.6111},
      {0.36266 , 6.9099},
      {0.50724 , 2.5099},
      {0.63124 , 1.6516},
      {0.80111 , 1.3674}  }};

   static constexpr double kfMin = kControlPts.front().f;
   static constexpr double kfMax = kControlPts.back().chi;
   static constexpr int kN = kControlPts.size();

   /**
    * @brief Compute the Bezier curve defined by the points within
    * [ \p IMin , \p IMax ], using de Casteljau's algorithm.
    */
   template<std::size_t IMin, std::size_t IMax>
   static Point B(double t)
   {
      if constexpr (IMin == IMax)
      {
         return kControlPts[IMin];
      }
      else
      {
         return (1-t)*B<IMin,IMax-1>(t) + t*B<IMin+1,IMax>(t);
      }
   }
   
   /// Evaluate the flow normal TF curve-fit at a given t.
   static Point Eval(double t)
   {
      return B<0,kN>(t);
   }

   /// Evaluate the derivative of the curve fit at a given t.
   static Point DEval(double t)
   {
      return kN*(B<1,kN>(t) - B<0, kN-1>(t));
   }
};

void FlowNormalFitTF(double mach_bar, double gamma, char speed, 
                        double delta, std::span<const double> freqs,
                        std::span<double> powers)
{
   const double chi_star = 
      [&]() 
      { 
         double c;
         LowFrequencyLimitTF(mach_bar, gamma, speed, {&c,1});
         return c;
      }();

   // Compute Bezier curve parameter t for each freq using Newton's method
   constexpr std::size_t kNumNewtonIt = 100;
   constexpr double kNewtonTol = 1e-14;
   constexpr std::size_t kMinNewtonIt = 10;

   for (std::size_t i = 0; i < freqs.size(); i++)
   {
      const double &f_norm = freqs[i]/delta;
      if (f_norm < FlowNormalBezierFit::kfMin || 
            f_norm > FlowNormalBezierFit::kfMax)
      {
         std::string err_string = 
            std::format("Frequency {} must be within range [{},{}].",
                           f, FlowNormalBezierFit::kfMin, 
                           FlowNormalBezierFit::kfMax);

         throw std::invalid_argument(err_string);
      }

      double tn = 0.5;
      double tnp1;
      for (std::size_t n = 0; n < kNumNewtonIt; n++)
      {
         tnp1 = tn - FlowNormalBezierFit::Eval(tn).f
                        / FlowNormalBezierFit::DEval(tn).f;
         if (tnp1 < 0)
         {
            tnp1 = 0.0;
         }
         else if (tnp1 > 1)
         {
            tnp1 = 1.0;
         }

         // If converged, apply transfer function
         if (n >= kMinNewtonIt && std::abs(tnp1-tn) < kNewtonTol)
         {
            n = kNumNewtonIt;
            powers[i] /= 
         }
         else if (n + 1 == kNumNewtonIt)
         {
            std::string err_string = 
            std::format("Unable to converge to Bezier parameter t for "
                        "frequency {}", f);
            throw std::logic_error(err_string);
         }
      }

   }
}

} // namespace jabber
