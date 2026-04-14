#ifndef SCENEBASIC_UNIFORM_H
#define SCENEBASIC_UNIFORM_H

#include "helper/scene.h"

#include <glm/glm.hpp>
#include <glad/glad.h>
#include "helper/glslprogram.h"
#include "helper/torus.h"
#include "helper/teapot.h"
#include "helper/plane.h"
#include "helper/objmesh.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <array>
#include "helper/cube.h"
#include "helper/skybox.h"
#include "helper/particleutils.h"
#include "helper/random.h"


class SceneBasic_Uniform : public Scene
{
private:
    glm::mat4 rotationMatrix;
    SkyBox sky;
    Plane plane;
    Cube cube;
    std::unique_ptr<ObjMesh> XWingMesh;
    std::unique_ptr<ObjMesh> TieMesh;
    std::unique_ptr<ObjMesh> BladeMEsh;
    glm::mat4 rotateModel;
    GLuint XWingDiffuseTexture, XWingNormalMap, LSmixingTexture;
	GLuint TieDiffuseTexture, TieNormalMap;
    GLuint WorkbenchDiffuseMap;
    GLuint cubeTex;

    float waveAngle;
    float waveTime;
    Plane wavePlane = Plane(13.0f, 10.0f, 200, 2);
    GLuint waveDiffuseTexture;
    void setMatricesWave();


    float tPrev = 0.0f;
    float angle = 0.0f;
    float rotSpeed;
    bool bladeOn = false;
    bool fogEnabled = true;
    float fogScale = 1.0f;
    bool rusty = false;

    // Input keys
    bool Q_Pressed = false;
    bool E_Pressed = false;
    bool F_Pressed = false;
    bool R_Pressed = false;
    bool C_Pressed = false;

    float shipRollDeg = 0.0f;
    float shipRollSpeed = 90.0f;
    float maxShipRollDeg = 25.0f;

    glm::vec3 bladeColour;
    std::array<glm::vec3, 6> bladeColours = {
        glm::vec3(0.0f, 0.5f, 1.0f),  // BLUE
        glm::vec3(0.0f, 1.0f, 0.0f),   // GREEN
        glm::vec3(1.0f, 1.0f, 0.0f),   // YELLOW
        glm::vec3(1.0f, 0.0f, 0.0f),   // RED
        glm::vec3(1.0f, 0.0f, 1.0f),   // MAGENTA
        glm::vec3(1.0f, 1.0f, 1.0f),    // WHITE
    };
    int bladeColourIndex = 0;
    glm::vec3 fogColour = glm::vec3(0.6f, 0.6f, 0.6f);
    const glm::vec3 fogGrey = glm::vec3(0.6f, 0.6f, 0.6f);
    float skyFogAmount = 0.35f;


    GLSLProgram prog, skyboxShader, bladeEmissive, waveProg, particleProg, particleFlatProg;
    void compile();
    void setMatrices();
    void updateFogColour();
    void updateCamera(float dt);
    void initParticleBuffers();
    float randFloat();
    Random rand;


    float shipYawDeg = 0.0f;
    float hiltYawSpeed = 90.0f;

    //particle stuff
    GLuint initVelBuf, birthTimeBuf;
    GLuint particleVAO;
    GLuint nParticles = 2000;
    GLuint particleTex;

    float particleLifetime = 0.3f;
    glm::vec3 emitterPos = glm::vec3(0.0f, 1.5f, 0.0f);

    glm::vec3 emitterDir = glm::vec3(0.0f, 0.0f, 1.0f);

    float particleTime = 0.0f;
    float particleAngle = 0.0f;

    std::array<glm::vec3, 4> engineOffsets = {
    glm::vec3(1.75f, 1.0f,  7.75f),
    glm::vec3(-1.75f, 1.0f,  7.75f),
    glm::vec3(1.75f, -1.0f,  7.75f),
    glm::vec3(-1.75f, -1.0f,  7.75f)
    };

    // Camera stuff (ORBIT)
    
 
    

    // Camera stuff (STATIC FOLLOW)
    glm::vec3 shipPos = glm::vec3(0.0f, 2.0f, 0.0f);

    glm::vec3 camTarget = glm::vec3(0.0f, 0.75f, 0.0f); // focus point (central model)
    glm::vec3 camOffset = glm::vec3(0.0f, 5.0f, 14.0f);

    glm::vec3 camPos = glm::vec3(5.0f, 7.5f, 7.5f);
    glm::vec3 camUp = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 camTargetOffset = glm::vec3(0.0f, 1.2f, 0.0f);


public:
    SceneBasic_Uniform();

    void initScene();
    void update(float t);
    void render();
    void resize(int, int);

};

#endif // SCENEBASIC_UNIFORM_H
