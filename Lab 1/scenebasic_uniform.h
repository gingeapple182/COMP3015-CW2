#ifndef SCENEBASIC_UNIFORM_H
#define SCENEBASIC_UNIFORM_H

#include "helper/scene.h"

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <irrKlang.h>

#include "helper/glslprogram.h"
#include "helper/torus.h"
#include "helper/teapot.h"
#include "helper/plane.h"
#include "helper/objmesh.h"
#include <glm/gtc/matrix_transform.hpp>
#include <array>
#include "helper/cube.h"
#include "helper/skybox.h"
#include "helper/particleutils.h"
#include "helper/random.h"
#include "helper/text_renderer.h"

using namespace irrklang;

class SceneBasic_Uniform : public Scene
{
private:
    // =========================================================
    // Core scene geometry / meshes
    // =========================================================
    SkyBox sky;
    Plane plane;
    Plane wavePlane = Plane(250.0f, 250.0f, 400, 400);
    Cube cube;

    std::unique_ptr<ObjMesh> XWingMesh;
    std::unique_ptr<ObjMesh> TieMesh;

    // =========================================================
    // Shared transform matrices
    // =========================================================
    glm::mat4 rotationMatrix;
    glm::mat4 rotateModel;

    // =========================================================
    // Textures
    // =========================================================
    GLuint XWingDiffuseTexture, XWingNormalMap, LSmixingTexture;
    GLuint TieDiffuseTexture, TieNormalMap;
    GLuint WorkbenchDiffuseMap;
    GLuint waveDiffuseTexture;
    GLuint cubeTex;

    // =========================================================
    // Gameplay State
    // =========================================================
    enum class RunState
    {
        StartScreen,
        Playing,
        GameOver
    };

    RunState runState = RunState::StartScreen;

    int score = 0;

    // =========================================================
    // General scene timing / animation state
    // =========================================================
    float tPrev = 0.0f;
    float angle = 0.0f;
    float rotSpeed;

    float waveAngle = 0.0f;
    float waveTime = 0.0f;

    bool waveAnimationEnabled = true;
    bool forwardScrollEnabled = false;
    float forwardScrollOffset = 0.0f;
    float forwardScrollSpeed = -4.0f;

    // =========================================================
    // Feature toggles / render state
    // =========================================================
    bool fogEnabled = false;
    float fogScale = 1.0f;
    bool rusty = false;

    // =========================================================
    // Input key state
    // =========================================================
    bool Q_Pressed = false;
    bool E_Pressed = false;
    bool F_Pressed = false;
    bool R_Pressed = false;
    bool C_Pressed = false;

    // =========================================================
    // Ship rotation / movement feel
    // =========================================================
    float shipYawDeg = 0.0f;

    float shipRollDeg = 0.0f;
    float shipRollSpeed = 90.0f;
    float maxShipRollDeg = 25.0f;

    // =========================================================
    // Fog colour settings
    // =========================================================
    
    glm::vec3 fogColour = glm::vec3(0.6f, 0.6f, 0.6f);
    float skyFogAmount = 0.35f;

    // =========================================================
    // Shader programs
    // =========================================================
    GLSLProgram prog, skyboxShader, waveProg, particleProg, smokeProg, particleFlatProg;
    BitmapTextRenderer textRenderer;

    // =========================================================
    // Random helper
    // =========================================================
    Random rand;

    // =========================================================
    // Particle system
    // =========================================================
    GLuint initVelBuf, birthTimeBuf;
    GLuint particleVAO;
    GLuint nParticles = 2000;
    GLuint particleTex;
    GLuint smokeTex; 

    float particleLifetime = 0.3f;
    float particleTime = 0.0f;
    float particleAngle = 0.0f;

    glm::vec3 emitterPos = glm::vec3(0.0f, 1.5f, 0.0f);
    glm::vec3 emitterDir = glm::vec3(0.0f, 0.0f, 1.0f);

    /*std::array<glm::vec3, 4> engineOffsets = {
    glm::vec3(1.75f, 1.0f,  7.75f),
    glm::vec3(-1.75f, 1.0f,  7.75f),
    glm::vec3(1.75f, -1.0f,  7.75f),
    glm::vec3(-1.75f, -1.0f,  7.75f)
    };*/

    std::array<glm::vec3, 4> engineOffsets = {
        glm::vec3(0.85f, 0.5f,  3.875f),
        glm::vec3(-0.875f, 0.5f,  3.875f),
        glm::vec3(0.875f, -0.5f,  3.875f),
        glm::vec3(-0.875f, -0.5f,  3.875f)
    };

    // =========================================================
    // Explosion particle system
    // =========================================================
    GLuint explosionInitVelBuf, explosionBirthTimeBuf;
    GLuint explosionVAO;
    GLuint explosionParticles = 1000;

    float explosionLifetime = 1.0f;
    bool explosionActive = false;
    float explosionStartTime = 0.0f;
    glm::vec3 explosionPos = glm::vec3(0.0f);

    // =========================================================
    // Smoke particle system
    // =========================================================
    GLuint smokeInitVelBuf, smokeBirthTimeBuf;
    GLuint smokeVAO;
    GLuint smokeParticles = 600;

    float smokeLifetime = 2.8f;
    bool smokeActive = false;
    glm::vec3 smokePos = glm::vec3(0.0f);
    float smokeStartTime = 0.0f;

    float smokeMinParticleSize = 0.4f;
    float smokeMaxParticleSize = 1.8f;

    // =========================================================
    // Object transform settings
    // =========================================================

    // Wave
    glm::vec3 wavePlaneScale = glm::vec3(2.0f, 1.0f, 2.0f);
    glm::vec3 wavePlaneOffset = glm::vec3(0.0f, 0.0f, 00.0f);

    // X-Wing
    glm::vec3 shipPos = glm::vec3(0.0f, 2.0f, 10.0f);
    glm::vec3 xwingScale = glm::vec3(0.5f);

    // TIE obstacle
    glm::vec3 tiePos = glm::vec3(0.0f, 3.0f, -30.0f);
    glm::vec3 tieScale = glm::vec3(0.6f);

    int tieLane = 1;
    float tieBaseMoveSpeed = 10.0f;
    float tieMoveSpeed = 10.0f;
    float tieMaxMoveSpeed = 60.0f;
    bool tieHasPassedPlayer = false;
    float difficultyTime = 0.0f;
    float speedIncreasePerSecond = 1.2f;
    float tieStartZ = -50.0f;
    float tieDespawnZ = 25.0f;
    float tieY = 3.0f;

    // Optional second TIE obstacle
    glm::vec3 tiePos2 = glm::vec3(0.0f, 3.0f, -30.0f);
    int tieLane2 = 1;
    bool secondTieActive = false;
    bool tie2HasPassedPlayer = false;
    float doubleTieSpawnChance = 0.35f;

    // =========================================================
    // Camera settings
    // =========================================================
    glm::vec3 camTarget = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 camOffset = glm::vec3(0.0f, 5.5f, 12.0f);

    glm::vec3 camPos = glm::vec3(0.0f, 6.0f, 14.0f);
    glm::vec3 camUp = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 camTargetOffset = glm::vec3(0.0f, 2.0f, -6.0f);

    // =========================================================
    // Lane runner movement
    // =========================================================
    static constexpr int laneCount = 3;
    int currentLane = 1;      // 0 = left, 1 = centre, 2 = right
    float laneSpacing = 6.0f; // adjust if needed for your model scale
    bool leftLanePressed = false;
    bool rightLanePressed = false;

    //bool gameOver = false;
    float collisionZThreshold = 4.0f;

    // =========================================================
    // Shadow mapping
    // =========================================================
    GLuint shadowFBO = 0;
    GLuint shadowTex = 0;

    int shadowMapWidth = 2048;
    int shadowMapHeight = 2048;

    glm::mat4 lightView = glm::mat4(1.0f);
    glm::mat4 lightProj = glm::mat4(1.0f);

    glm::mat4 shadowBias = glm::mat4(
        0.5f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.5f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.5f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f
    );

    bool shadowPass = false;

    // =========================================================
    // Audio
    // =========================================================
    ISoundEngine* soundEngine = nullptr;
    ISound* xwingEngineLoop = nullptr;

    // =========================================================
    // Private helper functions
    // =========================================================
    void compile();
    void setMatrices();
    void setMatricesWave();

    void updateFogColour();
    void updateCamera(float dt);

    void initParticleBuffers();
    void initExplosionParticleBuffers();
    void initSmokeParticleBuffers();
    float randFloat();

    void setWaveModelMatrix();
    void setXWingModelMatrix();
    void setTieModelMatrix();

    void setupFBO();
    void drawShadowCasters();
    void drawSceneMain();

    void updateLaneInput(GLFWwindow* win);
    void updateShipLanePosition();

    void randomiseTieLane();
    int randomDifferentLane(int excludedLane);
    void resetTieObstacle();
    void updateTieObstacle(float deltaT);

    bool checkTieCollision() const;
    void triggerGameOver();
    void restartGame();

    void startXWingEngineLoop();
    void stopXWingEngineLoop();

    void renderUI();

public:
    SceneBasic_Uniform();
    ~SceneBasic_Uniform();

    void initScene();
    void update(float t);
    void render();
    void resize(int, int);
};

#endif // SCENEBASIC_UNIFORM_H