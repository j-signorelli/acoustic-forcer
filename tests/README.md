# Unit Tests
The following is detailed documentation on all of the unit tests performed - particularly any analytical solutions compared against.
## Flowfield Equations:

Density:
$$
\rho=\bar{\rho}+\dfrac{1}{\bar{c}^2}\sum_{j=1}^Np'_je^{i(\vec{k}\cdot\vec{x}+ \phi_j - \omega_jt)}
$$

Momentum ($+$ if fast, $-$ if slow!):
$$
\rho \vec{u}=\rho\left(\vec{\bar{U}} + \dfrac{1}{\bar{\rho} \bar{c}}\sum_{j=1}^N(\pm1)\hat{k}_jp'_je^{i(\vec{k}_j\cdot\vec{x}+\phi_j-\omega_jt)}\right)
$$

Energy:
$$
\rho E=\dfrac{\bar{p}}{\gamma-1} + \dfrac{1}{\gamma-1}\sum_{j=1}^Np'_je^{i(\vec{k}\cdot\vec{x}+ \phi_j - \omega_jt)} + \dfrac{1}{2}\rho||\vec{\bar{U}} + \dfrac{1}{\bar{\rho} \bar{c}}\sum_{j=1}^N(\pm1)\hat{k}_jp'_je^{i(\vec{k}_j\cdot\vec{x}+\phi_j-\omega_jt)}||^2
$$
## 1D Computation
### Base Flow
Consider the following base flow:
$$
\bar{U}=1000,
$$
$$
\bar{p}=2000,
$$
$$
\bar{\rho}=0.1792,\text{and}
$$
$$
\gamma=1.4,
$$
where it can be shown that $\bar{c}= \sqrt{\gamma\bar{p}/\bar{\rho}}=\sqrt{15625}=125$.

### Single Wave
Consider a single slow wave with the following properties:
$$
p'_1=10,
$$
$$
f_1=1000,\ \omega_1=2000\pi,
$$
$$
\hat{k}_1=1,
$$
$$
k_1=\dfrac{\omega_1}{\bar{U}-\bar{c}}=\dfrac{2000\pi}{875}=\dfrac{16}{7}\pi, \text{and}
$$
$$
\phi_1=\pi/3.
$$
The flow solution for this is exactly:
$$
\rho(x,t)=0.1729+\dfrac{1}{15625}10\cos\left(\frac{16\pi}{7} x + \frac{\pi}{3} - 2000\pi t\right)
$$
$$
\rho u=\rho(x,t)\left(1000 + \dfrac{-1}{(0.1792)(125)}10\cos\left(\frac{16\pi}{7} x + \frac{\pi}{3} - 2000\pi t\right)\right)
$$
$$
\rho E=\dfrac{2000}{1.4-1} + \dfrac{1}{1.4-1}10\cos\left(\frac{16\pi}{7} x + \frac{\pi}{3} - 2000\pi t\right) + \dfrac{1}{2}\rho u^2
$$
### Multi-Wave
Consider the above wave, along with an additional one with the following properties:

## 2D Computation
### Single Wave
### Multi-Wave

## 3D Computation
### Single Wave
### Multi-Wave