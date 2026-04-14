#include "scenebasic_uniform.h"

#include <cstdio>
#include <cstdlib>

#include <string>
using std::string;

#include <sstream>
#include <iostream>
using std::cerr;
using std::endl;

#include <GLFW/glfw3.h>
#include "helper/glutils.h"
#include "helper/texture.h"

using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;

SceneBasic_Uniform::SceneBasic_Uniform() : tPrev(0), angle(0.0f), rotSpeed(glm::pi<float>() / 2.0f), plane(79.4f, 53.2f, 100, 100), sky(100.0f) {
	XWingMesh = ObjMesh::load("media/X-Wing.obj", true);
	TieMesh = ObjMesh::load("media/imp_fly_tiefighter.obj", true);
	//LightsaberMesh = ObjMesh::load("media/Lightsaber_03.obj", true);
	BladeMEsh = ObjMesh::load("media/cylinder2.obj", true);
}

void SceneBasic_Uniform::initScene()
{
	compile();
	//glGenVertexArrays(1, &fsTriVao);

	prog.use();
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_STENCIL_TEST);
	glClearStencil(0);

	model = mat4(1.0f);

	updateCamera(0.0f);

	projection = mat4(1.0f);
	rotateModel = mat4(1.0f);
	rotateModel = glm::translate(rotateModel, vec3(4.0f, 0.26f, 5.0f));
	angle = 0.0f;


	//blinnPhongSpot stuff
	prog.setUniform("Spot.L", vec3(0.7f));
	prog.setUniform("Spot.La", vec3(0.5f));
	prog.setUniform("Spot.Exponent", 25.0f);
	prog.setUniform("Spot.Cutoff", glm::radians(20.0f));

	prog.setUniform("Light.L", vec3(0.5f));
	prog.setUniform("Light.La", vec3(0.25f));


	// model
	XWingDiffuseTexture = Texture::loadTexture("media/texture/xwing_main.png");
	XWingNormalMap = Texture::loadTexture("media/texture/xwing_main_n.png");
	TieDiffuseTexture = Texture::loadTexture("media/texture/imp_fly_tiefighter.png");
	LSmixingTexture = Texture::loadTexture("media/texture/rust.png");

	// plane
	WorkbenchDiffuseMap = Texture::loadTexture("media/texture/workbench.png");

	//wave
	waveDiffuseTexture = Texture::loadTexture("media/texture/water.jpg");

	// skybox
	cubeTex = Texture::loadCubeMap("media/texture/cube/space/space");
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeTex);
	skyboxShader.use();
	skyboxShader.setUniform("FogColour", fogColour);

	// fog
	fogColour = bladeColours[bladeColourIndex];
	prog.use();
	prog.setUniform("Fog.MinDistance", 10.0f);
	prog.setUniform("Fog.MaxDistance", 45.0f);
	prog.setUniform("Fog.Colour", fogColour);
	prog.setUniform("Fog.Colour", fogColour);

	updateFogColour();

	// --- PARTICLE FOUNTAIN ---
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	initParticleBuffers();

	particleTex = Texture::loadTexture("media/texture/fire.png");
	//emitterPos = shipPos + glm::vec3(1.75f, 1.0f, 7.75f);
	particleProg.use();
	particleProg.setUniform("ParticleTex", 0);
	particleProg.setUniform("ParticleLifetime", particleLifetime);
	particleProg.setUniform("ParticleSize", 0.4f);
	particleProg.setUniform("Gravity", glm::vec3(0.0f, -0.2f, 0.0f));
	//particleProg.setUniform("EmitterPos", emitterPos);

	prog.use();
}

void SceneBasic_Uniform::compile()
{
	try {
		prog.compileShader("shader/basic_uniform.vert");
		prog.compileShader("shader/basic_uniform.frag");
		prog.link();
		prog.use();

		skyboxShader.compileShader("shader/skybox.vert");
		skyboxShader.compileShader("shader/skybox.frag");
		skyboxShader.link();
		//skyboxShader.use();

		waveProg.compileShader("shader/wave.vert");
		waveProg.compileShader("shader/wave.frag");
		waveProg.link();
		//waveProg.use();

		particleProg.compileShader("shader/particle_fountain.vert");
		particleProg.compileShader("shader/particle_fountain.frag");
		particleProg.link();

		particleFlatProg.compileShader("shader/flat_vert.glsl");
		particleFlatProg.compileShader("shader/flat_frag.glsl");
		particleFlatProg.link();

	}
	catch (GLSLProgramException& e) {
		cerr << e.what() << endl;
		exit(EXIT_FAILURE);
	}
}

void SceneBasic_Uniform::update(float t)
{
	float deltaT = t - tPrev;
	if (tPrev == 0.0f)
		deltaT = 0.0f;
	tPrev = t;
	angle += 0.25f * deltaT;
	if (angle > glm::two_pi<float>())
		angle -= glm::two_pi<float>();

	waveTime = t;

	//particleTime = fmod(t, particleLifetime);
	particleTime = t;
	particleAngle = fmod(particleAngle + 0.01f, glm::two_pi<float>());

	GLFWwindow* win = glfwGetCurrentContext();
	if (win)
	{
		float rollInput = 0.0f;

		if (glfwGetKey(win, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(win, GLFW_KEY_LEFT) == GLFW_PRESS)
			rollInput = 1.0f;

		if (glfwGetKey(win, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(win, GLFW_KEY_RIGHT) == GLFW_PRESS)
			rollInput = -1.0f;

		if (rollInput != 0.0f)
		{
			shipRollDeg += rollInput * shipRollSpeed * deltaT;
			shipRollDeg = glm::clamp(shipRollDeg, -maxShipRollDeg, maxShipRollDeg);
		}
		else
		{
			if (shipRollDeg > 0.0f)
			{
				shipRollDeg -= shipRollSpeed * deltaT;
				if (shipRollDeg < 0.0f) shipRollDeg = 0.0f;
			}
			else if (shipRollDeg < 0.0f)
			{
				shipRollDeg += shipRollSpeed * deltaT;
				if (shipRollDeg > 0.0f) shipRollDeg = 0.0f;
			}
		}
	}

	updateCamera(deltaT);
	render();
}

void SceneBasic_Uniform::render()
{
	prog.use();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	// BlinnPhong spotlight
	glm::vec4 lightPosWorld = glm::vec4(10.0f * cos(angle), 10.0f, 10.0f * sin(angle), 1.0f);
	prog.setUniform("Spot.Position", glm::vec3(view * lightPosWorld));

	glm::vec3 targetWorld = glm::vec3(0.0f, 2.0f, 3.0f);
	glm::vec3 lightDirWorldSpot = glm::normalize(targetWorld - glm::vec3(lightPosWorld));

	glm::vec3 lightDirView = glm::mat3(view) * lightDirWorldSpot;
	prog.setUniform("Spot.Direction", lightDirView);

	glm::vec3 lightDirWorld = glm::normalize(glm::vec3(-0.6f, -1.0f, -0.4f));
	glm::vec3 lightDirViewSpace = glm::mat3(view) * -lightDirWorld;
	prog.setUniform("Light.Position", glm::vec4(lightDirViewSpace, 0.0f));


	// fog
	prog.setUniform("FogEnabled", fogEnabled ? 1 : 0);
	prog.setUniform("FogScale", fogScale);

	

	// Skybox stuff
	skyboxShader.use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeTex);
	skyboxShader.setUniform("SkyboxTex", 0);

	glm::mat4 skyView = glm::mat4(glm::mat3(view));
	glm::mat4 skyMVP = projection * skyView;
	skyboxShader.setUniform("MVP", skyMVP);

	skyboxShader.setUniform("FogEnabled", fogEnabled ? 1 : 0);
	skyboxShader.setUniform("FogColour", fogColour);
	skyboxShader.setUniform("SkyFogAmount", skyFogAmount);

	sky.render();


	prog.use();

	// Workbench
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, WorkbenchDiffuseMap);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, WorkbenchDiffuseMap);

	prog.setUniform("Material.Ks", vec3(0.0f));
	prog.setUniform("Material.Ka", vec3(0.1f));
	prog.setUniform("Material.Shininess", 180.0f);

	model = mat4(1.0f);
	model = glm::translate(model, vec3(0.0f, -1.0f, 0.0f));
	//model = glm::rotate(model, glm::radians(25.0f), vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, vec3(0.5f));
	setMatrices();
	//plane.render();

	// --- WAVE PLANE RENDERING ---
	waveProg.use();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, waveDiffuseTexture);
	waveProg.setUniform("DiffuseTex", 0);

	waveProg.setUniform("Time", waveTime);

	waveProg.setUniform("MaterialKd", 0.2f, 0.5f, 0.9f);
	waveProg.setUniform("MaterialKs", 0.8f, 0.8f, 0.8f);
	waveProg.setUniform("MaterialKa", 0.2f, 0.5f, 0.9f);
	waveProg.setUniform("MaterialShininess", 100.0f);

	model = glm::mat4(1.0f);
	//model = glm::rotate(model, glm::radians(-10.0f), glm::vec3(0, 0, 1));
	//model = glm::rotate(model, glm::radians(50.0f), glm::vec3(1, 0, 0));

	setMatricesWave();
	wavePlane.render();


	prog.use();

	// X-Wing hilt
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, XWingDiffuseTexture);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, XWingNormalMap);

	if (rusty) {
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, LSmixingTexture);
	}
	else {
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, XWingDiffuseTexture);
	}

	prog.setUniform("Material.Ks", vec3(0.25f));
	prog.setUniform("Material.Ka", vec3(0.15f));
	prog.setUniform("Material.Shininess", 80.0f);
	prog.setUniform("MixAmount", 0.8f);

	model = mat4(1.0f);
	//model = glm::translate(model, vec3(0.0f, 2.0f, 0.0f));
	model = glm::translate(model, shipPos);
	model = glm::scale(model, vec3(1.0f));
	//model = glm::rotate(model, glm::radians(270.0f), vec3(1.0f, 0.0f, 0.0f));
	//model = glm::rotate(model, glm::radians(300.0f), vec3(0.0f, 0.0f, 1.0f));
	model = glm::rotate(model, glm::radians(shipYawDeg), vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, glm::radians(shipRollDeg), vec3(0.0f, 0.0f, 1.0f));

	setMatrices();
	XWingMesh->render();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, TieDiffuseTexture);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, TieDiffuseTexture);

	prog.setUniform("Material.Ks", vec3(0.25f));
	prog.setUniform("Material.Ka", vec3(0.15f));
	prog.setUniform("Material.Shininess", 80.0f);
	prog.setUniform("MixAmount", 0.8f);

	model = mat4(1.0f);
	model = glm::translate(model, vec3(5.0f, 3.0f, 5.0f));
	model = glm::scale(model, vec3(0.5f));
	setMatrices();
	TieMesh->render();

	// --- PARTICLE FOUNTAIN ---
	glDepthMask(GL_FALSE);

	particleProg.use();
	particleProg.setUniform("Time", particleTime);

	glm::mat4 particleModel = glm::mat4(1.0f);
	glm::mat4 particleMV = view * particleModel;

	particleProg.setUniform("MV", particleMV);
	particleProg.setUniform("Proj", projection);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, particleTex);
	particleProg.setUniform("ParticleTex", 0);

	glBindVertexArray(particleVAO);

	glm::mat4 shipRotation = glm::mat4(1.0f);
	shipRotation = glm::rotate(shipRotation, glm::radians(shipYawDeg), glm::vec3(0.0f, 1.0f, 0.0f));
	shipRotation = glm::rotate(shipRotation, glm::radians(shipRollDeg), glm::vec3(0.0f, 0.0f, 1.0f));

	for (const auto& offset : engineOffsets)
	{
		glm::vec3 rotatedOffset = glm::vec3(shipRotation * glm::vec4(offset, 1.0f));
		glm::vec3 worldEmitterPos = shipPos + rotatedOffset;

		particleProg.setUniform("EmitterPos", worldEmitterPos);
		glDrawArraysInstanced(GL_TRIANGLES, 0, 6, nParticles);
	}

	glBindVertexArray(0);
	glDepthMask(GL_TRUE);

}

void SceneBasic_Uniform::setMatrices() {
	mat4 mv = view * model;
	prog.setUniform("ModelViewMatrix", mv);
	prog.setUniform("NormalMatrix", glm::mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2])));
	prog.setUniform("MVP", projection * mv);
}

void SceneBasic_Uniform::setMatricesWave()
{
	glm::mat4 mv = view * model;
	waveProg.setUniform("ModelViewMatrix", mv);
	waveProg.setUniform("NormalMatrix",
		glm::mat3(glm::vec3(mv[0]), glm::vec3(mv[1]), glm::vec3(mv[2])));
	waveProg.setUniform("MVP", projection * mv);
}

void SceneBasic_Uniform::updateFogColour()
{
	fogColour = bladeOn ? bladeColours[bladeColourIndex] : fogGrey;

	// update shaders for skybox + normal
	prog.use();
	prog.setUniform("Fog.Colour", fogColour);

	skyboxShader.use();
	skyboxShader.setUniform("FogColour", fogColour);
}

void SceneBasic_Uniform::resize(int w, int h) {
	glViewport(0, 0, w, h);
	width = w;
	height = h;
	projection = glm::perspective(glm::radians(70.0f), (float)w / h, 0.3f, 100.0f);
	//initHiltEdgeFbo(w, h);
}

static float wrapDeg(float a)
{
	while (a > 180.0f) a -= 360.0f;
	while (a < -180.0f) a += 360.0f;
	return a;
}

float SceneBasic_Uniform::randFloat() {
	return rand.nextFloat();
}

void SceneBasic_Uniform::initParticleBuffers()
{
	glGenBuffers(1, &initVelBuf);
	glGenBuffers(1, &birthTimeBuf);

	// -----------------------------
	// Initial velocity buffer
	// -----------------------------
	std::vector<GLfloat> velData(nParticles * 3);
	glm::mat3 basis = ParticleUtils::makeArbitraryBasis(emitterDir);

	for (uint32_t i = 0; i < nParticles; i++)
	{
		//float theta = glm::mix(0.0f, glm::pi<float>() / 20.0f, randFloat());
		float theta = glm::mix(0.0f, glm::pi<float>() / 40.0f, randFloat());
		float phi = glm::mix(0.0f, glm::two_pi<float>(), randFloat());

		glm::vec3 v(
			sin(theta) * cos(phi),
			cos(theta),
			sin(theta) * sin(phi)
		);

		//float speed = glm::mix(1.25f, 1.5f, randFloat());
		float speed = glm::mix(4.0f, 6.0f, randFloat());
		v = glm::normalize(basis * v) * speed;

		velData[i * 3 + 0] = v.x;
		velData[i * 3 + 1] = v.y;
		velData[i * 3 + 2] = v.z;
	}

	glBindBuffer(GL_ARRAY_BUFFER, initVelBuf);
	glBufferData(GL_ARRAY_BUFFER, velData.size() * sizeof(GLfloat), velData.data(), GL_STATIC_DRAW);

	// -----------------------------
	// Birth time buffer
	// -----------------------------
	std::vector<GLfloat> birthData(nParticles);
	float rate = particleLifetime / static_cast<float>(nParticles);

	for (uint32_t i = 0; i < nParticles; i++)
	{
		birthData[i] = rate * i;
	}

	glBindBuffer(GL_ARRAY_BUFFER, birthTimeBuf);
	glBufferData(GL_ARRAY_BUFFER, birthData.size() * sizeof(GLfloat), birthData.data(), GL_STATIC_DRAW);

	// -----------------------------
	// VAO
	// -----------------------------
	glGenVertexArrays(1, &particleVAO);
	glBindVertexArray(particleVAO);

	// Dummy vertex buffer so gl_VertexID runs 0..5 for each instance
	GLuint quadVBO;
	glGenBuffers(1, &quadVBO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);

	static const float dummy[6] = { 0, 1, 2, 3, 4, 5 };
	glBufferData(GL_ARRAY_BUFFER, sizeof(dummy), dummy, GL_STATIC_DRAW);

	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
	glEnableVertexAttribArray(2);
	glVertexAttribDivisor(2, 0);

	// Attribute 0 = initial velocity
	glBindBuffer(GL_ARRAY_BUFFER, initVelBuf);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribDivisor(0, 1);

	// Attribute 1 = birth time
	glBindBuffer(GL_ARRAY_BUFFER, birthTimeBuf);
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribDivisor(1, 1);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void SceneBasic_Uniform::updateCamera(float dt)
{
	camPos = shipPos + camOffset;
	glm::vec3 camTarget = shipPos + camTargetOffset;
	view = glm::lookAt(camPos, camTarget, camUp);
	
}
