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

/*
Contents:
1. Constructor
2. Scene lifecycle
3. Core render / matrix helpers
4. Model transform helpers
5. General utility helpers
6. Particle helpers
7. Camera / shadow helpers
8. Scene draw passes
9. Lane movement helpers
10. Gameplay helpers
*/


// =========================================================
// 1. Constructor
// =========================================================

SceneBasic_Uniform::SceneBasic_Uniform()
	: tPrev(0),
	angle(0.0f),
	rotSpeed(glm::pi<float>() / 2.0f),
	plane(79.4f, 53.2f, 100, 100),
	sky(120.0f)
{
	XWingMesh = ObjMesh::load("media/X-Wing.obj", true);
	TieMesh = ObjMesh::load("media/imp_fly_tiefighter.obj", true);
}


// =========================================================
// 2. Scene lifecycle
// =========================================================

void SceneBasic_Uniform::initScene()
{
	compile();
	//glGenVertexArrays(1, &fsTriVao);
	setupFBO();

	prog.use();
	prog.setUniform("ShadowMap", 3);
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_STENCIL_TEST);
	glClearStencil(0);

	model = mat4(1.0f);

	updateCamera(0.0f);

	projection = mat4(1.0f);
	rotateModel = mat4(1.0f);
	rotateModel = glm::translate(rotateModel, vec3(4.0f, 0.26f, 5.0f));
	angle = 0.0f;

	// blinnPhongSpot stuff
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

	// wave
	waveDiffuseTexture = Texture::loadTexture("media/texture/water.jpg");

	// skybox
	cubeTex = Texture::loadCubeMap("media/texture/cube/skybox1/skybox");
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeTex);
	skyboxShader.use();
	skyboxShader.setUniform("FogColour", fogColour);

	// fog
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
	initExplosionParticleBuffers();
	initSmokeParticleBuffers();

	particleTex = Texture::loadTexture("media/texture/fire.png");
	smokeTex = Texture::loadTexture("media/texture/smoke.png");

	particleProg.use();
	particleProg.setUniform("ParticleTex", 0);
	particleProg.setUniform("ParticleLifetime", particleLifetime);
	particleProg.setUniform("ParticleSize", 0.4f);
	particleProg.setUniform("Gravity", glm::vec3(0.0f, -0.2f, 0.0f));

	smokeProg.use();
	smokeProg.setUniform("ParticleTex", 0);
	smokeProg.setUniform("ParticleLifetime", smokeLifetime);
	smokeProg.setUniform("Gravity", glm::vec3(0.0f, 0.15f, 0.0f));
	smokeProg.setUniform("MinParticleSize", smokeMinParticleSize);
	smokeProg.setUniform("MaxParticleSize", smokeMaxParticleSize);

	waveProg.use();
	waveProg.setUniform("ShadowMap", 3);

	prog.use();

	textRenderer.init(
		"shader/text.vert",
		"shader/text.frag",
		"media/text/spr_font_hellomyoldfriend_12x12_by_lotovik_sheet.png",
		"media/text/spr_font_hellomyoldfriend_12x12_by_lotovik_characterstring.txt",
		width,
		height,
		24,
		24,
		10,
		11
	);

	// lane runner defaults
	currentLane = 1;
	updateShipLanePosition();
	resetTieObstacle();
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

		smokeProg.compileShader("shader/particle_smoke.vert");
		smokeProg.compileShader("shader/particle_fountain.frag");
		smokeProg.link();

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

	if (waveAnimationEnabled)
	{
		waveTime = t;
	}

	particleTime = t;
	particleAngle = fmod(particleAngle + 0.01f, glm::two_pi<float>());

	if (explosionActive)
	{
		float explosionElapsed = particleTime - explosionStartTime;
		if (explosionElapsed > explosionLifetime)
		{
			explosionActive = false;
		}
	}

	GLFWwindow* win = glfwGetCurrentContext();
	if (win)
	{
		bool cNow = glfwGetKey(win, GLFW_KEY_C) == GLFW_PRESS;
		if (cNow && !C_Pressed)
		{
			waveAnimationEnabled = !waveAnimationEnabled;
		}
		C_Pressed = cNow;

		bool fNow = glfwGetKey(win, GLFW_KEY_F) == GLFW_PRESS;
		if (fNow && !F_Pressed)
		{
			if (runState == RunState::StartScreen || runState == RunState::GameOver)
			{
				restartGame();
			}
		}
		F_Pressed = fNow;

		if (runState == RunState::Playing)
		{
			if (forwardScrollEnabled)
			{
				forwardScrollOffset += forwardScrollSpeed * deltaT;

				difficultyTime += deltaT;
				tieMoveSpeed = glm::min(
					tieBaseMoveSpeed + difficultyTime * speedIncreasePerSecond,
					tieMaxMoveSpeed
				);
			}

			updateTieObstacle(deltaT);
			
			if (checkTieCollision())
			{
				triggerGameOver();
			}

			updateLaneInput(win);
			updateShipLanePosition();

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
	}

	updateCamera(deltaT);
	render();
}


void SceneBasic_Uniform::render()
{
	// Directional light used for both lighting and shadow mapping
	glm::vec3 lightDirWorld = glm::normalize(glm::vec3(-0.6f, -1.0f, -0.4f));

	// Centre the shadow area around the gameplay area / ship
	glm::vec3 shadowCenter = glm::vec3(shipPos.x, 0.0f, shipPos.z);

	// Place the shadow camera back along the light direction
	glm::vec3 lightPosWorld = shadowCenter - lightDirWorld * 30.0f;

	lightView = glm::lookAt(lightPosWorld, shadowCenter, glm::vec3(0.0f, 1.0f, 0.0f));
	lightProj = glm::ortho(-30.0f, 30.0f, -30.0f, 30.0f, 1.0f, 80.0f);

	// ---------- PASS 1: render shadow map ----------
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glViewport(0, 0, shadowMapWidth, shadowMapHeight);
	glClear(GL_DEPTH_BUFFER_BIT);

	projection = lightProj;
	view = lightView;

	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);

	drawShadowCasters();

	glCullFace(GL_BACK);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// ---------- PASS 2: render scene normally ----------
	glViewport(0, 0, width, height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	projection = glm::perspective(glm::radians(70.0f), (float)width / height, 0.3f, 300.0f);
	updateCamera(0.0f);

	// Convert directional light into view space for normal scene shading
	glm::vec3 lightDirViewSpace = glm::mat3(view) * -lightDirWorld;

	prog.use();

	// Keep these if your main shader still expects them
	prog.setUniform("Spot.Position", glm::vec3(view * glm::vec4(lightPosWorld, 1.0f)));
	prog.setUniform("Spot.Direction", glm::mat3(view) * lightDirWorld);

	// Main directional light for shading
	prog.setUniform("Light.Position", glm::vec4(lightDirViewSpace, 0.0f));

	prog.setUniform("FogEnabled", fogEnabled ? 1 : 0);
	prog.setUniform("FogScale", fogScale);

	// Skybox
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

	// Main lit scene objects
	prog.use();
	drawSceneMain();

	// Wave surface
	waveProg.use();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, waveDiffuseTexture);
	waveProg.setUniform("DiffuseTex", 0);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, shadowTex);
	waveProg.setUniform("ShadowMap", 3);

	waveProg.setUniform("Time", waveTime);
	waveProg.setUniform("ForwardOffset", forwardScrollOffset);

	waveProg.setUniform("LightPosition", glm::vec4(lightDirViewSpace, 0.0f));
	waveProg.setUniform("LightIntensity", glm::vec3(0.5f));
	waveProg.setUniform("AmbientLight", glm::vec3(0.25f));

	waveProg.setUniform("MaterialKd", 0.2f, 0.5f, 0.9f);
	waveProg.setUniform("MaterialKs", 0.8f, 0.8f, 0.8f);
	waveProg.setUniform("MaterialKa", 0.2f, 0.5f, 0.9f);
	waveProg.setUniform("MaterialShininess", 100.0f);

	setWaveModelMatrix();
	setMatricesWave();
	wavePlane.render();

	// Particle pass
	//if (runState != RunState::GameOver || runState != RunState::StartScreen)
	if (runState == RunState::Playing)
	{
		prog.use();
		glDepthMask(GL_FALSE);

		particleProg.use();
		particleProg.setUniform("Time", particleTime);

		// Restore normal engine particle settings every frame
		particleProg.setUniform("ParticleLifetime", particleLifetime);
		particleProg.setUniform("ParticleSize", 0.4f);
		particleProg.setUniform("Gravity", glm::vec3(0.0f, -0.2f, 0.0f));

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

		prog.use();
	}

	if (explosionActive)
	{
		glDepthMask(GL_FALSE);

		particleProg.use();
		particleProg.setUniform("Time", particleTime - explosionStartTime);
		particleProg.setUniform("ParticleTex", 0);
		particleProg.setUniform("ParticleLifetime", explosionLifetime);
		particleProg.setUniform("ParticleSize", 1.2f);
		particleProg.setUniform("Gravity", glm::vec3(0.0f, -1.2f, 0.0f));

		glm::mat4 particleModel = glm::mat4(1.0f);
		glm::mat4 particleMV = view * particleModel;

		particleProg.setUniform("MV", particleMV);
		particleProg.setUniform("Proj", projection);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, particleTex);

		glBindVertexArray(explosionVAO);
		particleProg.setUniform("EmitterPos", explosionPos);
		glDrawArraysInstanced(GL_TRIANGLES, 0, 6, explosionParticles);
		glBindVertexArray(0);

		glDepthMask(GL_TRUE);

		prog.use();
	}

	if (smokeActive)
	{
		glDepthMask(GL_FALSE);

		smokeProg.use();
		smokeProg.setUniform("Time", particleTime - smokeStartTime);
		smokeProg.setUniform("ParticleTex", 0);
		smokeProg.setUniform("ParticleLifetime", smokeLifetime);
		smokeProg.setUniform("Gravity", glm::vec3(0.0f, 0.15f, 0.0f));
		smokeProg.setUniform("MinParticleSize", smokeMinParticleSize);
		smokeProg.setUniform("MaxParticleSize", smokeMaxParticleSize);

		glm::mat4 particleModel = glm::mat4(1.0f);
		glm::mat4 particleMV = view * particleModel;

		smokeProg.setUniform("MV", particleMV);
		smokeProg.setUniform("Proj", projection);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, smokeTex);

		glBindVertexArray(smokeVAO);
		smokeProg.setUniform("EmitterPos", smokePos);
		glDrawArraysInstanced(GL_TRIANGLES, 0, 6, smokeParticles);
		glBindVertexArray(0);

		glDepthMask(GL_TRUE);

		prog.use();
	}
	renderUI();
}


void SceneBasic_Uniform::resize(int w, int h)
{
	glViewport(0, 0, w, h);
	width = w;
	height = h;
	projection = glm::perspective(glm::radians(70.0f), (float)w / h, 0.3f, 300.0f);

	textRenderer.resize(w, h);
}


// =========================================================
// 3. Core render / matrix helpers
// =========================================================

void SceneBasic_Uniform::setMatrices()
{
	mat4 mv = view * model;
	prog.setUniform("ModelViewMatrix", mv);
	prog.setUniform("NormalMatrix", glm::mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2])));
	prog.setUniform("MVP", projection * mv);

	glm::mat4 shadowMatrix = shadowBias * lightProj * lightView * model;
	prog.setUniform("ShadowMatrix", shadowMatrix);
}


void SceneBasic_Uniform::setMatricesWave()
{
	glm::mat4 mv = view * model;
	waveProg.setUniform("ModelViewMatrix", mv);
	waveProg.setUniform("NormalMatrix",
		glm::mat3(glm::vec3(mv[0]), glm::vec3(mv[1]), glm::vec3(mv[2])));
	waveProg.setUniform("MVP", projection * mv);
	waveProg.setUniform("ModelMatrix", model);

	glm::mat4 shadowMatrix = shadowBias * lightProj * lightView * model;
	waveProg.setUniform("ShadowMatrix", shadowMatrix);
}


// =========================================================
// 4. Model transform helpers
// =========================================================

void SceneBasic_Uniform::setWaveModelMatrix()
{
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, shipPos.z) + wavePlaneOffset);
	model = glm::scale(model, wavePlaneScale);
}


void SceneBasic_Uniform::setXWingModelMatrix()
{
	model = glm::mat4(1.0f);
	model = glm::translate(model, shipPos);
	model = glm::scale(model, xwingScale);
	model = glm::rotate(model, glm::radians(shipYawDeg), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, glm::radians(shipRollDeg), glm::vec3(0.0f, 0.0f, 1.0f));
}


void SceneBasic_Uniform::setTieModelMatrix()
{
	model = glm::mat4(1.0f);
	model = glm::translate(model, tiePos);
	model = glm::scale(model, tieScale);
}


// =========================================================
// 5. General utility helpers
// =========================================================

void SceneBasic_Uniform::updateFogColour()
{
	// update shaders for skybox + normal
	prog.use();
	prog.setUniform("Fog.Colour", fogColour);

	skyboxShader.use();
	skyboxShader.setUniform("FogColour", fogColour);
}


static float wrapDeg(float a)
{
	while (a > 180.0f) a -= 360.0f;
	while (a < -180.0f) a += 360.0f;
	return a;
}


float SceneBasic_Uniform::randFloat()
{
	return rand.nextFloat();
}


// =========================================================
// 6. Particle helpers
// =========================================================

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

void SceneBasic_Uniform::initExplosionParticleBuffers()
{
	glGenBuffers(1, &explosionInitVelBuf);
	glGenBuffers(1, &explosionBirthTimeBuf);

	// -----------------------------
	// Initial velocity buffer
	// Explosion should spread in all directions
	// -----------------------------
	std::vector<GLfloat> velData(explosionParticles * 3);

	for (uint32_t i = 0; i < explosionParticles; i++)
	{
		float z = glm::mix(-1.0f, 1.0f, randFloat());
		float phi = glm::mix(0.0f, glm::two_pi<float>(), randFloat());
		float r = sqrt(glm::max(0.0f, 1.0f - z * z));

		glm::vec3 dir(
			r * cos(phi),
			z,
			r * sin(phi)
		);

		float speed = glm::mix(3.0f, 8.0f, randFloat());
		glm::vec3 v = glm::normalize(dir) * speed;

		velData[i * 3 + 0] = v.x;
		velData[i * 3 + 1] = v.y;
		velData[i * 3 + 2] = v.z;
	}

	glBindBuffer(GL_ARRAY_BUFFER, explosionInitVelBuf);
	glBufferData(GL_ARRAY_BUFFER, velData.size() * sizeof(GLfloat), velData.data(), GL_STATIC_DRAW);

	// -----------------------------
	// Birth time buffer
	// Small spread so the burst is not perfectly uniform
	// -----------------------------
	std::vector<GLfloat> birthData(explosionParticles);

	for (uint32_t i = 0; i < explosionParticles; i++)
	{
		birthData[i] = glm::mix(0.0f, 0.12f, randFloat());
	}

	glBindBuffer(GL_ARRAY_BUFFER, explosionBirthTimeBuf);
	glBufferData(GL_ARRAY_BUFFER, birthData.size() * sizeof(GLfloat), birthData.data(), GL_STATIC_DRAW);

	// -----------------------------
	// VAO
	// -----------------------------
	glGenVertexArrays(1, &explosionVAO);
	glBindVertexArray(explosionVAO);

	GLuint quadVBO;
	glGenBuffers(1, &quadVBO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);

	static const float dummy[6] = { 0, 1, 2, 3, 4, 5 };
	glBufferData(GL_ARRAY_BUFFER, sizeof(dummy), dummy, GL_STATIC_DRAW);

	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
	glEnableVertexAttribArray(2);
	glVertexAttribDivisor(2, 0);

	// Attribute 0 = initial velocity
	glBindBuffer(GL_ARRAY_BUFFER, explosionInitVelBuf);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribDivisor(0, 1);

	// Attribute 1 = birth time
	glBindBuffer(GL_ARRAY_BUFFER, explosionBirthTimeBuf);
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribDivisor(1, 1);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void SceneBasic_Uniform::initSmokeParticleBuffers()
{
	glGenBuffers(1, &smokeInitVelBuf);
	glGenBuffers(1, &smokeBirthTimeBuf);

	// -----------------------------
	// Initial velocity buffer
	// Smoke rises upward with soft sideways spread
	// -----------------------------
	std::vector<GLfloat> velData(smokeParticles * 3);

	for (uint32_t i = 0; i < smokeParticles; i++)
	{
		float x = glm::mix(-0.5f, 0.5f, randFloat());
		float y = glm::mix(0.9f, 1.4f, randFloat());
		float z = glm::mix(-0.5f, 0.5f, randFloat());

		glm::vec3 dir = glm::normalize(glm::vec3(x, y, z));
		float speed = glm::mix(0.45f, 1.1f, randFloat());
		glm::vec3 v = dir * speed;

		velData[i * 3 + 0] = v.x;
		velData[i * 3 + 1] = v.y;
		velData[i * 3 + 2] = v.z;
	}

	glBindBuffer(GL_ARRAY_BUFFER, smokeInitVelBuf);
	glBufferData(GL_ARRAY_BUFFER, velData.size() * sizeof(GLfloat), velData.data(), GL_STATIC_DRAW);

	// -----------------------------
	// Birth time buffer
	// Spread births across one lifetime so emission loops continuously
	// -----------------------------
	std::vector<GLfloat> birthData(smokeParticles);
	float rate = smokeLifetime / static_cast<float>(smokeParticles);

	for (uint32_t i = 0; i < smokeParticles; i++)
	{
		birthData[i] = rate * i;
	}

	glBindBuffer(GL_ARRAY_BUFFER, smokeBirthTimeBuf);
	glBufferData(GL_ARRAY_BUFFER, birthData.size() * sizeof(GLfloat), birthData.data(), GL_STATIC_DRAW);

	// -----------------------------
	// VAO
	// -----------------------------
	glGenVertexArrays(1, &smokeVAO);
	glBindVertexArray(smokeVAO);

	GLuint quadVBO;
	glGenBuffers(1, &quadVBO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);

	static const float dummy[6] = { 0, 1, 2, 3, 4, 5 };
	glBufferData(GL_ARRAY_BUFFER, sizeof(dummy), dummy, GL_STATIC_DRAW);

	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
	glEnableVertexAttribArray(2);
	glVertexAttribDivisor(2, 0);

	glBindBuffer(GL_ARRAY_BUFFER, smokeInitVelBuf);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribDivisor(0, 1);

	glBindBuffer(GL_ARRAY_BUFFER, smokeBirthTimeBuf);
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribDivisor(1, 1);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}



// =========================================================
// 7. Camera / shadow helpers
// =========================================================

void SceneBasic_Uniform::updateCamera(float dt)
{
	camPos = glm::vec3(0.0f, shipPos.y, shipPos.z) + camOffset;
	glm::vec3 camTarget = glm::vec3(shipPos.x, shipPos.y, shipPos.z) + camTargetOffset;
	view = glm::lookAt(camPos, camTarget, camUp);
}


void SceneBasic_Uniform::setupFBO()
{
	glGenTextures(1, &shadowTex);
	glBindTexture(GL_TEXTURE_2D, shadowTex);

	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_DEPTH_COMPONENT24,
		shadowMapWidth,
		shadowMapHeight,
		0,
		GL_DEPTH_COMPONENT,
		GL_FLOAT,
		nullptr
	);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	float border[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LESS);

	glGenFramebuffers(1, &shadowFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowTex, 0);

	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		std::cerr << "Shadow framebuffer is not complete!" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


// =========================================================
// 8. Scene draw passes
// =========================================================

void SceneBasic_Uniform::drawShadowCasters()
{
	prog.use();

	if (runState != RunState::GameOver)
	{
		// X-Wing
		setXWingModelMatrix();
		setMatrices();
		XWingMesh->render();

		// TIE
		setTieModelMatrix();
		setMatrices();
		TieMesh->render();
	}

	// Flat receiver plane for testing
	model = mat4(1.0f);
	model = glm::translate(model, vec3(0.0f, -1.5f, 0.0f));
	model = glm::scale(model, vec3(3.0f));
	setMatrices();
	plane.render();

	// Wave surface also casts / receives shadow information
	waveProg.use();
	waveProg.setUniform("Time", waveTime);

	setWaveModelMatrix();
	setMatricesWave();
	wavePlane.render();

	prog.use();
}


void SceneBasic_Uniform::drawSceneMain()
{
	prog.use();

	// Test receiver plane
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, WorkbenchDiffuseMap);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, XWingNormalMap); // temporary just so slot 1 has something valid

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, WorkbenchDiffuseMap);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, shadowTex);

	prog.setUniform("UseFlatColour", 1);
	prog.setUniform("FlatColour", glm::vec3(0.7f, 0.7f, 0.7f));

	prog.setUniform("Material.Ks", vec3(0.0f));
	prog.setUniform("Material.Ka", vec3(0.1f));
	prog.setUniform("Material.Shininess", 180.0f);
	prog.setUniform("MixAmount", 0.0f);

	model = mat4(1.0f);
	model = glm::translate(model, vec3(0.0f, -1.5f, 0.0f));
	model = glm::scale(model, vec3(3.0f));
	setMatrices();
	plane.render();

	prog.setUniform("UseFlatColour", 0);

	if (runState != RunState::GameOver)
	{
		// X-Wing
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, XWingDiffuseTexture);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, XWingNormalMap);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, rusty ? LSmixingTexture : XWingDiffuseTexture);

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, shadowTex);

		prog.setUniform("Material.Ks", vec3(0.25f));
		prog.setUniform("Material.Ka", vec3(0.15f));
		prog.setUniform("Material.Shininess", 80.0f);
		prog.setUniform("MixAmount", 0.8f);

		setXWingModelMatrix();
		setMatrices();
		XWingMesh->render();

		// TIE
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TieDiffuseTexture);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, XWingNormalMap);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, TieDiffuseTexture);

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, shadowTex);

		prog.setUniform("Material.Ks", vec3(0.25f));
		prog.setUniform("Material.Ka", vec3(0.15f));
		prog.setUniform("Material.Shininess", 80.0f);
		prog.setUniform("MixAmount", 0.0f);

		setTieModelMatrix();
		setMatrices();
		TieMesh->render();
	}
}


// =========================================================
// 9. Lane movement helpers
// =========================================================

void SceneBasic_Uniform::updateLaneInput(GLFWwindow* win)
{
	bool leftNow = glfwGetKey(win, GLFW_KEY_A) == GLFW_PRESS ||
		glfwGetKey(win, GLFW_KEY_LEFT) == GLFW_PRESS;

	bool rightNow = glfwGetKey(win, GLFW_KEY_D) == GLFW_PRESS ||
		glfwGetKey(win, GLFW_KEY_RIGHT) == GLFW_PRESS;

	// Only move lanes on a fresh press
	if (leftNow && !leftLanePressed)
	{
		currentLane--;
		if (currentLane < 0)
			currentLane = 0;
	}

	if (rightNow && !rightLanePressed)
	{
		currentLane++;
		if (currentLane > laneCount - 1)
			currentLane = laneCount - 1;
	}

	leftLanePressed = leftNow;
	rightLanePressed = rightNow;
}


void SceneBasic_Uniform::updateShipLanePosition()
{
	// lane 0 = -laneSpacing, lane 1 = 0, lane 2 = +laneSpacing
	shipPos.x = (currentLane - 1) * laneSpacing;
}

void SceneBasic_Uniform::randomiseTieLane()
{
	tieLane = static_cast<int>(randFloat() * laneCount);

	if (tieLane < 0) tieLane = 0;
	if (tieLane >= laneCount) tieLane = laneCount - 1;
}

void SceneBasic_Uniform::resetTieObstacle()
{
	randomiseTieLane();
	
	tiePos.x = (tieLane - 1) * laneSpacing;
	tiePos.y = tieY;
	tiePos.z = tieStartZ;
}

void SceneBasic_Uniform::updateTieObstacle(float deltaT)
{
	if (!forwardScrollEnabled || runState == RunState::GameOver)
		return;

	tiePos.z += tieMoveSpeed * deltaT;

	if (tiePos.z > tieDespawnZ)
	{
		score += 100;
		resetTieObstacle();
	}
}


// =========================================================
// 10. Gameplay helpers
// =========================================================

bool SceneBasic_Uniform::checkTieCollision() const
{
	if (currentLane != tieLane)
		return false;

	float zDistance = glm::abs(tiePos.z - shipPos.z);
	return zDistance < collisionZThreshold;
}

void SceneBasic_Uniform::triggerGameOver()
{
	runState = RunState::GameOver;
	forwardScrollEnabled = false;

	explosionActive = true;
	explosionStartTime = particleTime;
	explosionPos = shipPos;

	smokeActive = true;
	smokeStartTime = particleTime;
	smokePos = glm::vec3(shipPos.x, 0.2f, shipPos.z);

	std::cout << "GAME OVER\n";
}

void SceneBasic_Uniform::restartGame()
{
	runState = RunState::Playing;
	forwardScrollEnabled = true;

	score = 0;
	difficultyTime = 0.0f;
	tieMoveSpeed = tieBaseMoveSpeed;
	forwardScrollOffset = 0.0f;

	currentLane = 1;
	updateShipLanePosition();
	shipRollDeg = 0.0f;

	resetTieObstacle();

	explosionActive = false;
	smokeActive = false;
}

// =========================================================
// UI
// =========================================================

void SceneBasic_Uniform::renderUI()
{
	if (!textRenderer.isInitialised())
		return;

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (runState == RunState::StartScreen)
	{
		textRenderer.renderTextShadowed("X-WING RUNNER", 40.0f, height - 70.0f, 3.0f);
		textRenderer.renderTextShadowed("PRESS F TO START", 40.0f, height - 130.0f, 1.25f);
		textRenderer.renderTextShadowed("A / D OR ARROWS TO MOVE", 40.0f, height - 165.0f, 0.9f);
		textRenderer.renderTextShadowed("AVOID THE TIE FIGHTERS", 40.0f, height - 195.0f, 0.9f);
	}
	else if (runState == RunState::Playing)
	{
		std::string scoreText = "SCORE: " + std::to_string(score);
		textRenderer.renderTextShadowed(scoreText, 20.0f, height - 40.0f, 1.0f);
	}
	else if (runState == RunState::GameOver)
	{
		textRenderer.renderTextShadowed("GAME OVER", 40.0f, height - 100.0f, 2.0f);

		std::string scoreText = "FINAL SCORE: " + std::to_string(score);
		textRenderer.renderTextShadowed(scoreText, 40.0f, height - 150.0f, 1.0f);

		textRenderer.renderTextShadowed("PRESS F TO RESTART", 40.0f, height - 190.0f, 1.0f);
	}

	glEnable(GL_DEPTH_TEST);
}