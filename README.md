# PBR Textured Implementation in C++

This repository focuses on the implementation of a Physically-Based Rendering (PBR) model, specifically within the `pbr_textured.cpp` file.

## Key Features

### PBR Model
The implementation adheres to the Physically-Based Rendering model, designed to simulate the realistic interaction between light and materials.

### Microfacets Theory
The code is grounded on Microfacets Theory, taking into account the microscopic surface details when rendering materials.

### Rendering Function
The core of this repository lies in the rendering function, which computes both lighting and material appearance.

$$
\text{Final Color} = \frac{\text{Albedo}}{\pi} + \text{Specular}
$$


To understand the full scope of our rendering process, consider the extended rendering equation:

$$
L_o(p, \omega_o) = L_e(p, \omega_o) + \int_{\Omega} f(p, \omega_o, \omega_i) \times L_i(p, \omega_i) \times (\omega_i \cdot n) \, d\omega_i
$$

### Explanations for BRDF Parameters

- **( $\alpha$ ) (roughness coefficient)**: This is the roughness parameter, affecting the spread of microfacets. A smaller value leads to a smoother surface, while a larger value results in a rougher surface.

- **( $F_0$ ) (Fresnel-Schlick Approximation)**: The value of reflectance at zero angle of incidence, which defines how reflective the material is.

- **( $N$ ) (Normal Vector)**: The surface normal vector at the point of interest.

- **( $V$ ) (View Vector)**: The vector pointing from the surface point to the camera.

- **( $L$ ) (Light Vector)**: The vector pointing from the surface point to the light source.

### BRDF Function
The Bidirectional Reflectance Distribution Function (BRDF) is implemented in detail to provide high-fidelity rendering.

- **Distribution (D) Function**

$$
D(h) = \frac{\alpha^2}{\pi \times ((\alpha^2 - 1) \times (1 + (\alpha^2 - 1) \times (h \cdot n)^2))}
$$

- **Fresnel (F) Function**

$$
F(\theta) = F_0 + (1 - F_0) \times (1 - \cos(\theta))^5
$$

- **Geometry (G) Function**

$$
G = G_1(N \cdot V) \times G_1(N \cdot L)
$$

The $( G_1 )$ term is calculated as follows:

$$
G_1(v) = \frac{2}{1 + \sqrt{1 + \alpha^2 \times (1 - (v \cdot n)^2) / (v \cdot n)^2}}
$$

### Explanations for Rendering Equation Parameters

- **( $L_o(p, \omega_o)$ )**: The outgoing radiance at point $( p )$ in direction $( \omega_o )$.

- **( $L_e(p, \omega_o)$ )**: The emitted radiance from point $( p )$ in direction $( \omega_o )$.

- **( $f(p, \omega_o, \omega_i)$ )**: The BRDF at point $( p )$.

- **( $L_i(p, \omega_i)$ )**: The incoming radiance at point $( p )$ in direction $( \omega_i )$.

- **( $\omega_i \cdot n$ )**: The dot product between the incoming light direction and the surface normal, affecting how much light is received.