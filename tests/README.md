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
Consider a single **slow** wave with the following properties:
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
\rho=0.1792+\dfrac{1}{15625}10\cos\left(\frac{16\pi}{7} x + \frac{\pi}{3} - 2000\pi t\right)
$$
$$
\rho u=\rho\left(1000 + \dfrac{-1}{(0.1792)(125)}10\cos\left(\frac{16\pi}{7} x + \frac{\pi}{3} - 2000\pi t\right)\right)
$$
$$
\rho E=\dfrac{2000}{1.4-1} + \dfrac{1}{1.4-1}10\cos\left(\frac{16\pi}{7} x + \frac{\pi}{3} - 2000\pi t\right) + \dfrac{1}{2}\rho u^2
$$
### Multi-Wave
Consider the above wave, along with an additional **fast** one with the following properties:
$$
p'_2=5,
$$
$$
f_2=1250,\ \omega_2=2500\pi,
$$
$$
\hat{k}_2=1,
$$
$$
k_2=\dfrac{\omega_2}{\bar{U}+\bar{c}}=\dfrac{2500\pi}{1125}=\dfrac{20}{9}\pi, \text{and}
$$
$$
\phi_2=\pi.
$$
The flow solution for this is exactly:
$$
\rho=0.1792+\dfrac{1}{15625}\left[10\cos\left(\frac{16\pi}{7} x + \frac{\pi}{3} - 2000\pi t\right)+5\cos\left(\frac{20\pi}{9} x + \pi - 2500\pi t\right)\right]
$$
$$
\rho u=\rho\left(1000 + \dfrac{1}{(0.1792)(125)}\left[-10\cos\left(\frac{16\pi}{7} x + \frac{\pi}{3} - 2000\pi t\right)+5\cos\left(\frac{20\pi}{9} x + \pi - 2500\pi t\right)\right]\right)
$$
$$
\rho E=\dfrac{2000}{1.4-1} + \dfrac{1}{1.4-1}\left[10\cos\left(\frac{16\pi}{7} x + \frac{\pi}{3} - 2000\pi t\right)+5\cos\left(\frac{20\pi}{9} x + \pi - 2500\pi t\right)\right]+ \dfrac{1}{2}\rho u^2
$$
## 2D Computation
### Base Flow
Consider the following base flow:
$$
\vec{\bar{U}}=[600,800],
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
Consider a single **slow** wave with the following properties:
$$
p'_1=10,
$$
$$
f_1=1000,\ \omega_1=2000\pi,
$$
$$
\hat{k}_1=[1.0,0.0],
$$
$$
k_1=\dfrac{\omega_1}{\vec{\bar{U}}\cdot\hat{k}_1-\bar{c}}=\dfrac{2000\pi}{600-125}=\dfrac{2000\pi}{600-125}=\dfrac{2000\pi}{475}=\dfrac{80\pi}{19}, \text{and}
$$
$$
\phi_1=\pi/3.
$$
The flow solution for this is exactly:

$$
\rho=0.1792+\dfrac{1}{15625}10\cos\left(\frac{80\pi}{19} x + \frac{\pi}{3} - 2000\pi t\right)
$$
$$
\rho u=\rho\left(600 + \dfrac{-1}{(0.1792)(125)}10\cos\left(\frac{80\pi}{19}x + \frac{\pi}{3} - 2000\pi t\right)\right)
$$
$$
\rho v=\rho\left(800\right)
$$
$$
\rho E=\dfrac{2000}{1.4-1} + \dfrac{1}{1.4-1}10\cos\left(\frac{80\pi}{19}x + \frac{\pi}{3} - 2000\pi t\right) + \dfrac{1}{2}\rho(u^2+v^2)
$$

### Multi-Wave
Consider the above wave, along with an additional **fast** one with the following properties:
$$
p'_2=5,
$$
$$
f_2=1250,\ \omega_2=2500\pi,
$$
$$
\hat{k}_2=\left[\frac{6}{10},\frac{8}{10}\right]
$$
$$
k_2=\dfrac{\omega_2}{\vec{\bar{U}}\cdot\hat{k}_2+\bar{c}}=\dfrac{2500\pi}{1000+125}=\dfrac{2500\pi}{1125}=\dfrac{20}{9}\pi, \text{and}
$$
$$
\phi_2=\pi.
$$
The flow solution for this is exactly:

$$
\rho=0.1792+\dfrac{1}{15625}\left[10\cos\left(\frac{80\pi}{19} x + \frac{\pi}{3} - 2000\pi t\right) + 5\cos\left(\frac{4\pi}{3}x + \frac{16\pi}{9} y + \pi - 2500\pi t\right)\right]
$$
$$
\rho u=\rho\left(600 + \dfrac{1}{(0.1792)(125)}\left[-10\cos\left(\frac{80\pi}{19} x + \frac{\pi}{3} - 2000\pi t\right) + \dfrac{6}{10}5\cos\left(\frac{4\pi}{3}x + \frac{16\pi}{9} y + \pi - 2500\pi t\right)\right]\right)
$$
$$
\rho v=\rho\left(800 + \dfrac{1}{(0.1792)(125)}\dfrac{8}{10}5\cos\left(\frac{4\pi}{3}x + \frac{16\pi}{9} y + \pi - 2500\pi t\right)\right)
$$
$$
\rho E=\dfrac{2000}{1.4-1} + \dfrac{1}{1.4-1}\left[10\cos\left(\frac{80\pi}{19} x + \frac{\pi}{3} - 2000\pi t\right) + 5\cos\left(\frac{4\pi}{3}x + \frac{16\pi}{9}y + \pi - 2500\pi t\right)\right] + \dfrac{1}{2}\rho(u^2 + v^2)
$$
## 3D Computation
### Base Flow
Consider the following base flow:
$$
\vec{\bar{U}}=[600, 0, 450],
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
Consider a single **slow** wave with the following properties:
$$
p'_1=10,
$$
$$
f_1=1000,\ \omega_1=2000\pi,
$$
$$
\hat{k}_1=[1.0,0.0,0.0],
$$
$$
k_1=\dfrac{\omega_1}{\vec{\bar{U}}\cdot\hat{k}_1-\bar{c}}=\dfrac{2000\pi}{600-125}=\dfrac{2000\pi}{600-125}=\dfrac{2000\pi}{475}=\dfrac{80\pi}{19}, \text{and}
$$
$$
\phi_1=\pi/3.
$$
The flow solution for this is exactly:

$$
\rho=0.1792+\dfrac{1}{15625}10\cos\left(\frac{80\pi}{19} x + \frac{\pi}{3} - 2000\pi t\right)
$$
$$
\rho u=\rho\left(600 + \dfrac{-1}{(0.1792)(125)}10\cos\left(\frac{80\pi}{19}x + \frac{\pi}{3} - 2000\pi t\right)\right)
$$
$$
\rho v=0
$$
$$
\rho w=\rho(450)
$$
$$
\rho E=\dfrac{2000}{1.4-1} + \dfrac{1}{1.4-1}10\cos\left(\frac{80\pi}{19}x + \frac{\pi}{3} - 2000\pi t\right) + \dfrac{1}{2}\rho(u^2+v^2+w^2)
$$
### Multi-Wave
Consider the above wave, along with an additional **fast** one with the following properties:
$$
p'_2=5,
$$
$$
f_2=1250,\ \omega_2=2500\pi,
$$
$$
\hat{k}_2=\left[\frac{1}{3},\frac{2}{3},\frac{2}{3}\right]
$$
$$
k_2=\dfrac{\omega_2}{\vec{\bar{U}}\cdot\hat{k}_2+\bar{c}}=\dfrac{2500\pi}{200+300+125}=\dfrac{2500\pi}{625}=4\pi, \text{and}
$$
$$
\phi_2=\pi.
$$
The flow solution for this is exactly:

$$
\rho=0.1792+\dfrac{1}{15625}\left[10\cos\left(\frac{80\pi}{19} x + \frac{\pi}{3} - 2000\pi t\right) + 5\cos\left(\frac{4\pi}{3}x + \frac{8\pi}{3} y +\frac{8\pi}{3}z + \pi - 2500\pi t\right)\right]
$$
$$
\rho u=\rho\left(600 + \dfrac{1}{(0.1792)(125)}\left[-10\cos\left(\frac{80\pi}{19} x + \frac{\pi}{3} - 2000\pi t\right) + \dfrac{1}{3}5\cos\left(\frac{4\pi}{3}x + \frac{8\pi}{3} y +\frac{8\pi}{3}z + \pi - 2500\pi t\right)\right]\right)
$$
$$
\rho v=\rho\left(\dfrac{1}{(0.1792)(125)}\dfrac{2}{3}5\cos\left(\frac{4\pi}{3}x + \frac{8\pi}{3} y +\frac{8\pi}{3}z + \pi - 2500\pi t\right)\right)
$$
$$
\rho w=\rho\left(450+\dfrac{1}{(0.1792)(125)}\dfrac{2}{3}5\cos\left(\frac{4\pi}{3}x + \frac{8\pi}{3} y +\frac{8\pi}{3}z + \pi - 2500\pi t\right)\right)
$$
$$
\rho E=\dfrac{2000}{1.4-1} + \dfrac{1}{1.4-1}\left[10\cos\left(\frac{80\pi}{19} x + \frac{\pi}{3} - 2000\pi t\right) + 5\cos\left(\frac{4\pi}{3}x + \frac{8\pi}{3} y +\frac{8\pi}{3}z + \pi - 2500\pi t\right)\right] + \dfrac{1}{2}\rho(u^2 + v^2+w^2)
$$