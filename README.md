# PBR Textured Implementation in C++

This repository focuses on the implementation of a Physically-Based Rendering (PBR) model, specifically within the `pbr_textured.cpp` file.

## Key Features

### PBR Model
The implementation adheres to the Physically-Based Rendering model, designed to simulate the realistic interaction between light and materials.

### Microfacets Theory
The code is grounded on Microfacets Theory, taking into account the microscopic surface details when rendering materials.

\[
\text{Microfacet Distribution (D)} = \frac{\alpha^2}{\pi} (N \cdot H)^{\alpha^2 - 1} e^{-\frac{(N \cdot H)^2}{\alpha^2}}
\]

### Rendering Function
The core of this repository lies in the rendering function, which computes both lighting and material appearance.

\[
\text{Final Color} = \frac{\text{Albedo}}{\pi} + \text{Specular}
\]

### BRDF Function
The Bidirectional Reflectance Distribution Function (BRDF) is implemented in detail to provide high-fidelity rendering.

- **Distribution (D) Function**

\[
D(h) = \frac{\alpha^2}{\pi \times ((\alpha^2 - 1) \times (1 + (\alpha^2 - 1) \times (h \cdot n)^2))}
\]

- **Fresnel (F) Function**

\[
F(\theta) = F_0 + (1 - F_0) \times (1 - \cos(\theta))^5
\]

- **Geometry (G) Function**

\[
G = G_1(N \cdot V) \times G_1(N \cdot L)
\]

## Note
This implementation prioritizes accurate representation of physical properties and lighting phenomena through careful formulation of D, F, G functions within the BRDF.

