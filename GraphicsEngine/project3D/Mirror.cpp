#include "Mirror.h"

#include <gl_core_4_4.h>
#include <iostream>
#include <glm/ext.hpp>

Mirror::Mirror(vec2 size)
	: m_size(size)
{
}


Mirror::~Mirror()
{
	glDeleteBuffers(1, &m_VBO);
	glDeleteVertexArrays(1, &m_VAO);
}


Camera& Mirror::reflect(Transform* mirror, Transform* camera) {
	
	static bool runOnce = true;

	mat4 transform = mirror->getMatrix();

	vec3 worldNormal = vec3(transform * vec4(Transform::vOut,0));

	if (runOnce) std::cout << "Mirror WorldNormal : " << glm::to_string(worldNormal) << std::endl;
	
	// Position and orientation
	vec3 view = camera->getPosition() - mirror->getPosition();	    // Vector from centre point to camera
	vec3 nView = glm::normalize(view);								// Normalised vector

	if (runOnce) std::cout << "Mirror View Vector : " << glm::to_string(view) << std::endl;
	if (runOnce) std::cout << "Mirror Normalised View : " << glm::to_string(nView) << std::endl;

	vec3 axis = glm::normalize(glm::cross(worldNormal, nView));		// Find axis of rotation from view to normal
	float halfAngle = acos(glm::dot(nView, worldNormal));			// Find angle between view and normal
	float inverseAngle = (glm::pi<float>() / 2.0f) - halfAngle;		// Find angle between view and reflection plane
	
	if (runOnce) std::cout << "Mirror Reflection Axis : " << glm::to_string(axis) << std::endl;
	if (runOnce) std::cout << "Mirror Ref half-angle : " << halfAngle << std::endl;
	if (runOnce) std::cout << "Mirror Ref Inverse Angle : " << inverseAngle << std::endl;

	glm::quat rotation = glm::angleAxis(2 * inverseAngle, axis);			// Construct quaternion

	vec3 rView = vec3(glm::mat4_cast(rotation) * glm::vec4(view, 0)); // vector from "reflection camera" to centre point

	if (runOnce) std::cout << "Mirror Reflected View : " << glm::to_string(rView) << std::endl;

	// Construct Camera
	Camera& result = m_camera;
	result.setOrientation(mirror->getOrientation() * quat(vec3(0,glm::pi<float>(),0)));
	result.setPosition(mirror->getPosition() + rView);

	if (runOnce) std::cout << "Mirror Reflection Camera Posn : " << glm::to_string(result.getPosition()) << std::endl;
	if (runOnce) std::cout << "Mirror Reflection Camera Direction : " << glm::to_string(result.getView().getMatrix() * vec4(-Transform::vOut,0)) << std::endl;
	if (runOnce) std::cout << "Mirror Reflection Camera Orientation : " << glm::to_string(glm::eulerAngles(result.getOrientation())) << std::endl;

	// Configure Frustum
	glm::vec4 topRight = glm::vec4((Transform::vUp + Transform::vRight) * vec3(m_size, 0) / 2.0f, 1); // Model Space
	glm::vec4 bottomLeft = glm::vec4((Transform::vUp + Transform::vRight) * vec3(m_size, 0) / -2.0f, 1);

	topRight = transform * topRight; // World Space
	bottomLeft = transform * bottomLeft;

	topRight = result.getView().getMatrix() * topRight;	// Camera Space
	bottomLeft = result.getView().getMatrix() * bottomLeft;

	vec3 mins = glm::min(vec3(topRight), vec3(bottomLeft)); // Find mins and maxes
	vec3 maxs = glm::max(vec3(topRight), vec3(bottomLeft));

	if (runOnce) std::cout << "Mirror Frustum Mins : " << glm::to_string(mins) << std::endl;
	if (runOnce) std::cout << "Mirror Frustum Maxs : " << glm::to_string(maxs) << std::endl;

	result.setProjection(glm::frustum(mins.x, maxs.x, mins.y, maxs.y, -mins.z, 1000.0f));

	runOnce = false;

	// Return
	return m_camera;
}

void Mirror::Init() {
	m_buffer.init(1024,1024);
	//m_buffer.setViewport(2048,1024);
	buildQuad();
}

void Mirror::begin() {
	m_buffer.begin();
}

void Mirror::end() {
	m_buffer.end();
}

void Mirror::draw() {
	// draw out full-screen quad
	GLenum err;
	err = glGetError();
	if (err != GL_NO_ERROR)
		std::cerr << "Mirror::Draw() - Pre Draw : " << err << std::endl;

	glBindVertexArray(m_VAO);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	err = glGetError();
	if (err != GL_NO_ERROR)
		std::cerr << "Mirror::Draw() - Post Draw : " << err << std::endl;
}

void Mirror::buildQuad() {
	float halfTexel[] = { 0.0f, 0.0f };// { -0.5f / m_buffer.m_viewport[2], -0.5f / m_buffer.m_viewport[3] };
	float vertices[] = // Texture must be flipped on X (around Y)
	{
		-m_size.x / 2.0f, -m_size.y / 2.0f, 0, 1, 1 - halfTexel[0], halfTexel[1],
		m_size.x / 2.0f, -m_size.y / 2.0f, 0, 1, halfTexel[0], halfTexel[1],
		m_size.x / 2.0f, m_size.y / 2.0f, 0, 1, halfTexel[0], 1 - halfTexel[1],
		-m_size.x / 2.0f, m_size.y / 2.0f, 0, 1, 1 - halfTexel[0], 1 - halfTexel[1]
	};

	glGenVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);

	glGenBuffers(1, &m_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, vertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0); // position
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 6, 0);

	glEnableVertexAttribArray(2); // UVs
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 6, ((char*)0) + 16);

	// Detach
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


