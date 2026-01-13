#ifdef JABBER_WAVE
#define JABBER_WAVE

namespace jabber
{

/// Define struct-of-arrays for all waves
struct Waves
{
    std::vector<double> amplitude;
    std::vector<double> frequency;
    std::vector<std::vector<double>> wave_number;
    std::vector<double> phase;
}

} // namespace jabber

#endif // JABBER_WAVE
