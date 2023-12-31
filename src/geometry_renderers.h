// This header file provides a collection of functions for easily rendering specific geometric shapes in OpenGL.
// 
// Note: Each function includes vertex attributes by position, normal, and texture coordinates, which means:
// You have to specify layout(location = x) in glsl code by this order as well! 
//
// 
// Author: Zhenhuan Yu
// Date: 2023/09/17

#pragma once

#include <vector>
#include <cmath>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

namespace yzh {

	// Base class for all shapes with pure virual functions
	class GeometryShape
	{
	public:
		GeometryShape() {  }
		virtual ~GeometryShape() {}
		virtual void Render() = 0;

		virtual float SurfaceArea() const { return 0.0f; }
		virtual float Volume() const { return 0.0f; }
	};

	// This class provides a cube using OpenGL with dimensions of 2 * 2 * 2 units
	// The cube's vertex attributes include position, normal, and texture coordinates.
	// Note: This function does not use an Index Buffer Object (IBO).
	class Cube: public GeometryShape
	{
	public:
		Cube() 
		{
			if (this->VAO == 0) {
				float vertices[] = {
					// Position           // Normal           // TexCoords
					-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, 
					 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, 
					 1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f,      
					 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, 
					-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, 
					-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, 

					-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, 
					 1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, 
					 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, 
					 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, 
					-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, 
					-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, 

					-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, 
					-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, 
					-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, 
					-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, 
					-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, 
					-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, 

					 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, 
					 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, 
					 1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f,     
					 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, 
					 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, 
					 1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f,   

					 -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, 
					  1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, 
					  1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, 
					  1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, 
					 -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, 
					 -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, 

					 -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, 
					  1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f,
					  1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f,   
					  1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, 
					 -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, 
					 -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f      
				};

				glGenVertexArrays(1, &this->VAO);
				glGenBuffers(1, &this->VBO);
				// fill buffer
				glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
				glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
				// link vertex attributes
				glBindVertexArray(this->VAO);
				glEnableVertexAttribArray(0);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
				glEnableVertexAttribArray(1);
				glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
				glEnableVertexAttribArray(2);
				glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
				glBindBuffer(GL_ARRAY_BUFFER, 0);
				glBindVertexArray(0);
			}
		}

		Cube(const Cube& others) = delete;
		Cube& operator=(const Cube& others) = delete;

		~Cube() override
		{
			if (this->VAO != 0) {
				glDeleteVertexArrays(1, &this->VAO);
				glDeleteBuffers(1, &this->VBO);
				this->VAO = 0;
			}
		}

		void Render() override
		{
			if (this->VAO != 0) {
				glBindVertexArray(this->VAO);
				glDrawArrays(GL_TRIANGLES, 0, 36);
				glBindVertexArray(0);
			}
		}

	private:
		unsigned int VAO = 0, VBO = 0;
	};

	// This class provides a sphere in OpenGL with a radius of 2.0 units.
	// Vertex attributes include position, normal, and texture coordinates.
	// The sphere is detailed with 64 segments along both the X and Y axes, resulting in a highly detailed mesh.
	// he function allows for customization of the sphere's detail level through x_segments and y_segments.
	class Sphere: public GeometryShape
	{
	public:
		Sphere(const unsigned int x_segments = 64,
			const unsigned int y_segments = 64)
		{
			// Ensure the process will only be excuted once
			if (this->VAO == 0) {
				glGenVertexArrays(1, &this->VAO);
				glGenBuffers(1, &this->VBO);
				glGenBuffers(1, &this->IBO);

				std::vector<float> vertices;
				std::vector<unsigned int> indices;

				const unsigned int X_SEGMENTS = x_segments;
				const unsigned int Y_SEGMENTS = y_segments;
				const float PI = 3.14159265359f;
				float radius = 2.0f;
				for (unsigned int y = 0; y <= Y_SEGMENTS; ++y) {
					for (unsigned int x = 0; x <= X_SEGMENTS; ++x) {
						float xSegment = (float)x / (float)X_SEGMENTS;
						float ySegment = (float)y / (float)Y_SEGMENTS;

						float xPos = radius * std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
						float yPos = radius * std::cos(ySegment * PI);
						float zPos = radius * std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

						// Normalizing the normal
						float norm = std::sqrt(xPos * xPos + yPos * yPos + zPos * zPos);

						vertices.push_back(xPos); // Position
						vertices.push_back(yPos);
						vertices.push_back(zPos);
						vertices.push_back(xPos / norm); // Normal
						vertices.push_back(yPos / norm);
						vertices.push_back(zPos / norm);
						vertices.push_back(xSegment); // UV coords
						vertices.push_back(ySegment);
					}
				}

				bool oddRow = false;
				for (unsigned int y = 0; y < Y_SEGMENTS; ++y) {
					if (!oddRow) {
						// even rows: y == 0, y == 2; and so on 
						for (unsigned int x = 0; x <= X_SEGMENTS; ++x) {
							indices.push_back(y * (X_SEGMENTS + 1) + x);
							indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
						}
					}
					else {
						for (int x = X_SEGMENTS; x >= 0; --x) {
							indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
							indices.push_back(y * (X_SEGMENTS + 1) + x);
						}
					}
					oddRow = !oddRow;
				}

				glBindVertexArray(this->VAO);
				glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
				glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->IBO);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
				glEnableVertexAttribArray(0);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
				glEnableVertexAttribArray(1);
				glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
				glEnableVertexAttribArray(2);
				glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
				glBindVertexArray(0);
			}
		}

		~Sphere() override
		{
			// Prevent multiple de-allocation
			if (this->VAO != 0) {
				glDeleteVertexArrays(1, &this->VAO);
				glDeleteBuffers(1, &this->VBO);
				glDeleteBuffers(1, &this->IBO);
				this->VAO = 0;
			}
		}

		Sphere(const Sphere& other) = delete;
		Sphere& operator=(const Sphere& other) = delete;

		void Render() override
		{
			if (this->VAO != 0) {
				glBindVertexArray(this->VAO);
				glDrawElements(GL_TRIANGLE_STRIP, (64 + 1) * 64 * 2, GL_UNSIGNED_INT, 0);
				glBindVertexArray(0);
			}
		}

		const unsigned int GetVAO() const { return VAO; }

	private:
		unsigned int VAO = 0, VBO = 0, IBO = 0;
	};

	// This class provides a 2D quad in OpenGL with dimensions of 2 * 2 units.
    // Vertex attributes include position, normal, and texture coordinates.
    // The quad is rendered using GL_TRIANGLE_STRIP for optimized rendering.
	class Quad: public GeometryShape
	{
	public:
		Quad()
		{
			if (this->VAO == 0) {
				// positions, normals, texture Coords
				float quadVertices[] = {
					-1.0f,  1.0f, 0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f,
					-1.0f, -1.0f, 0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,
					 1.0f,  1.0f, 0.0f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f,
					 1.0f, -1.0f, 0.0f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f,
				};

				glGenVertexArrays(1, &this->VAO);
				glGenBuffers(1, &this->VBO);
				glBindVertexArray(this->VAO);
				glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
				glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

				// Position attribute
				glEnableVertexAttribArray(0);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);

				// Normal attribute
				glEnableVertexAttribArray(1);
				glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));

				// Texture coordinate attribute
				glEnableVertexAttribArray(2);
				glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

				glBindVertexArray(0);
			}
		}

		~Quad() override
		{
			if (VAO != 0) {
				glDeleteVertexArrays(1, &this->VAO);
				glDeleteBuffers(1, &this->VBO);
				this->VAO = 0;
			}
		}

		void Render() override
		{
			if (VAO != 0) {
				glBindVertexArray(this->VAO);
				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
				glBindVertexArray(0);
			}
		}
	    
		const unsigned int GetVAO() const { return this->VAO; }

	private:
		unsigned int VAO = 0, VBO = 0;
	};

	// Circle Class:
	// Creates a 2D circle with radius 1, centered at the origin (0,0,0) on the XY plane.
	// The circle's edge smoothness is controlled by the number of segments (numSegments).
	// This class handles vertex positions only; normals and texture coordinates are not included.
	class Circle : public GeometryShape
	{
	public:
		Circle(int nrSegments = 36)
			: nrSegments(nrSegments) {
			if (this->VAO == 0) {
				initCircle();
			}
		}

		~Circle() override
		{
			if (this->VAO != 0) {
				glDeleteVertexArrays(1, &this->VAO);
				glDeleteBuffers(1, &this->VBO);
			}
		}

		void Render() override
		{
			if (this->VAO != 0) {
				glBindVertexArray(this->VAO);
				// Render the circle
				glDrawArrays(GL_TRIANGLE_FAN, 0, nrSegments + 2); // +2 for the center and the duplicate first vertex at the end
				glBindVertexArray(0);
			}
		}

	private:
		void initCircle()
		{
			std::vector<float> vertices;
			// Center point of the circle
			vertices.insert(vertices.end(), { 0.0f, 0.0f, 0.0f }); // Position
			vertices.insert(vertices.end(), { 0.0f, 0.0f, 1.0f }); // Normal
			vertices.insert(vertices.end(), { 0.5f, 0.5f });       // Texture Coord

			for (int i = 0; i <= nrSegments; ++i) {
				float angle = 2.0f * 3.14159265359f * i / nrSegments;
				float x = cosf(angle);
				float y = sinf(angle);
				float u = (x + 1.0f) / 2.0f;  // Mapping x to texture coordinate
				float v = (y + 1.0f) / 2.0f;  // Mapping y to texture coordinate

				vertices.insert(vertices.end(), { x, y, 0.0f });    // Position
				vertices.insert(vertices.end(), { 0.0f, 0.0f, 1.0f }); // Normal
				vertices.insert(vertices.end(), { u, v });          // Texture Coord
			}

			// Set up the VAO and VBO
			glGenVertexArrays(1, &this->VAO);
			glGenBuffers(1, &this->VBO);
			glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
			glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

			glBindVertexArray(this->VAO);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);
		}

		unsigned int VAO = 0, VBO = 0;
		int nrSegments;
	};

	class Cylinder : public GeometryShape
	{
	public:
		Cylinder();
		~Cylinder();
		void Render();

	private:
		void initCylinder();

	private:
		unsigned int VAO = 0, VBO = 0;
	};

	// TODO:
	// ----
	class Cone : public GeometryShape {
	public:
		Cone();
		~Cone();
		void Render();

	private:
		void initCone();

	private:
		unsigned int VAO = 0, VBO = 0;
	};
};