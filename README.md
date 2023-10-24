# PBR Textured Implementation in C++

This repository focuses on the implementation of a Physically-Based Rendering (PBR) model, specifically within the `pbr_textured.cpp` file.


## PBR Model
The implementation adheres to the Physically-Based Rendering model, designed to simulate the realistic interaction between light and materials.
Note that at this stage `lighting.textured.cpp` we don't have IBL light yet.

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

### Explanations for Rendering Equation Parameters

- **( $L_o(p, \omega_o)$ )**: The outgoing radiance at point $( p )$ in direction $( \omega_o )$.

- **( $L_e(p, \omega_o)$ )**: The emitted radiance from point $( p )$ in direction $( \omega_o )$.

- **( $f(p, \omega_o, \omega_i)$ )**: The BRDF at point $( p )$.

- **( $L_i(p, \omega_i)$ )**: The incoming radiance at point $( p )$ in direction $( \omega_i )$.

- **( $\omega_i \cdot n$ )**: The dot product between the incoming light direction and the surface normal, affecting how much light is received.

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

### Explanations for BRDF Parameters

- **( $\alpha$ ) (roughness coefficient)**: This is the roughness parameter, affecting the spread of microfacets. A smaller value leads to a smoother surface, while a larger value results in a rougher surface.

- **( $F_0$ ) (Fresnel-Schlick Approximation)**: The value of reflectance at zero angle of incidence, which defines how reflective the material is.

- **( $N$ ) (Normal Vector)**: The surface normal vector at the point of interest.

- **( $V$ ) (View Vector)**: The vector pointing from the surface point to the camera.

- **( $L$ ) (Light Vector)**: The vector pointing from the surface point to the light source.
- 

## PBR with IBL in C++
This repository focuses on Physically-Based Rendering (PBR) using Image-Based Lighting (IBL), specifically for diffuse IBL. The main file of interest is `ibr_irradiance_conversion.cpp`, accompanied by several fragment shaders, including `equirectangular_to_cubemap.fs` and `irradiance_convolution.fs`.

### Loading HDR Texture
An HDR image located at `"res/textures/hdr/newport_loft.hdr"` is loaded using the `stbi_loadf` function from the `stb.image.h` library. This image serves as the environmental light source; however, it first needs to be converted into a cubemap.

### Environment Cubemap Configuration

During this stage, a framebuffer and a `GL_CUBE_MAP` named `environmentCubemap` are set up to capture the scene. The viewport is set to 512x512 pixels. We populate a `captureViews` array to send different view matrices to the GPU for each of the 6 necessary renderings.

The conversion from the HDR equirectangular texture to the environment cubemap is a key process here. This is performed by the fragment shader `equirectangular_to_cubemap.fs`. 

The shader uses spherical coordinates to map each point on the cubemap to a corresponding point in the equirectangular map. The formula to convert a 3D direction vector \( v \) to a 2D point \( (u, v) \) on the equirectangular map is as follows:

$$
(u, v) = \left( \frac{ \arctan(v.z, v.x) }{ 2\pi } + 0.5, \frac{ \arcsin(v.y) }{ \pi } + 0.5 \right)
$$

This formula ensures that the HDR equirectangular texture is correctly projected onto the 3D cubemap, capturing the environmental lighting accurately.


### Creating the Irradiance Map
Here, another cubemap is configured with a viewport size of 32x32. The `irradiance_convolution.fs` shader converts the initial cubemap into an `irradianceMap`.
// TODO

### PBR and Diffuse IBL 
The `pbr_ibl_diffuse_textured.fs` shader is used to render spheres utilizing PBR. Instead of traditional ambient lighting, we sample from the `irradianceMap` using the normal vector. 
(i.e., `vec3 irradiance = texture(irradianceMap, N).rgb;`)

### Skybox Rendering
A skybox is rendered, setting its depth value explicitly to 1.0f. This prevents overdraw and is implemented in `background.vs` by setting `gl_Position` to `clipPos.xyww`.