/*********************************************************************************************************************
 *
 * main.cpp
 * 
 * RT_lite
 * Ludovic Blache
 *
 *********************************************************************************************************************/


// Standard includes 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <cstdlib>
#include <algorithm>

// OpenGL includes
#include <GL/glew.h>
#include <GLFW/glfw3.h>


#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "tools.h"
#include "drawablemesh.h"


// Window
GLFWwindow *m_window;           /*!<  GLFW window */
int m_winWidth = 1024;          /*!<  window width (XGA) */
int m_winHeight = 720;          /*!<  window height (XGA) */
const unsigned int TEX_WIDTH = 2048, TEX_HEIGHT = 2048; /*!< textures dimensions  */

Trackball m_trackball;          /*!<  model trackball */

// Scene
glm::vec3 m_centerCoords;       /*!<  coords of the center of the scene */
float m_radScene;               /*!< radius of the scene (i.e., diagonal of the BBox) */

// Light and cameras 
Camera m_camera;                /*!<  camera */
Camera m_cameraLight;           /*!<  light camera for shadow map */
float m_zoomFactor;             /*!<  zoom factor */
glm::vec3 m_camPos;             /*!<  camera position */
glm::vec3 m_lightSpherePosInit; /*!<  light source initial position in spherical coords (zenith, azimuth, radius) */
glm::vec3 m_lightSpherePos;     /*!<  light source position in spherical coords (zenith, azimuth, radius) */
glm::vec3 m_lightCol;           /*!<  light color */
glm::mat4 m_cstProjMatrix;      /*!< view matrix without zoom (for skybox display) */
float m_maxDistLight;           /*!< maximum distance of the (point) light source to the center of scene */
float m_minDistLight;           /*!< minimum distance of the (point) light source to the center of scene */
float m_lightCamNearRad;        /*!< distance between scene center and light camera near plane (NOT light camera near distance ! ) */
float m_lightCamFarRad;         /*!< distance between scene center and light camera far plane (NOT light camera far distance ! ) */

// 3D objects
TriMesh* m_triMesh;             /*!<  triangle mesh */
DrawableMesh* m_drawMesh;       /*!<  drawable object: mesh object */
DrawableMesh* m_drawQuad;       /*!<  drawable object: screen quad */
DrawableMesh* m_drawFloor;      /*!<  drawable object: floor quad */
DrawableMesh* m_drawSkybox;     /*!<  drawable object: skybox */

glm::mat4 m_modelMatrix;        /*!<  model matrix of the mesh */
    
GLuint m_defaultVAO;            /*!<  default VAO */

// FBOs
GLuint m_shadowFBO;             /*!< FBO for shadow map calculation: renders depthbuffer into m_shadowMapTex */
GLuint m_shadowMapTex;          /*!< Texture for shadow map rendering (i.e., depthbuffer from light cam) */
GLuint m_tsdFBO;                /*!< FBO for texture space diffusion: renders color component into m_tsdTex */
GLuint m_tsdTex;                /*!< Destination texture for texture space diffusion (stores result of lighting to texture space) */
GLuint m_gFBO;                  /*!< FBO for G-buffer: renders fragment position and normals coords as rgb colors into m_gPosition and m_gNormal */
GLuint m_gPosition;             /*!< G-buffer position screen-texture */
GLuint m_gNormal;               /*!< G-buffer normal screen-texture */
GLuint m_gColor;                /*!< G-buffer color screen-textures */
GLuint m_screenFBO;             /*!< FBO for screen-space processing: renders color component into m_screenTex*/
GLuint m_screenTex;             /*!< Destination texture for screen-space processing (stores final lighting result) */
GLuint m_SSAOFBO;               /*!< FBO for SSAO processing: renders SSAO (as color component) into m_SSAOTex */
GLuint m_SSAOTex;               /*!< Screen-texture to store SSAO result */
GLuint m_BlurFBO;               /*!< FBO for SSAO blurring: renders blured SSAO (as color component) into m_BlurTex */   
GLuint m_BlurFBO2;              /*!< FBO for SSLR blurring: renders blured SSLR (as color component) into m_BlurTex2 */    
GLuint m_BlurTex;               /*!< Screen-texture to store blurred SSAO */
GLuint m_BlurTex2;              /*!< Screen-texture to store blurred SSLR */
GLuint m_SSLRFBO;               /*!< FBO for SSLR renders SSLR (as color component) into m_SSLRTex */    
GLuint m_SSLRTex;               /*!< Screen-texture to store SSLR */

// shader programs
GLuint m_programLighting;       /*!< handle of the program object (i.e. shaders) for shaded surface rendering */
GLuint m_programShadow;         /*!< handle of the program object (i.e. shaders) for shadow map rendering */
GLuint m_programQuad;           /*!< handle of the program object (i.e. shaders) for screen quad rendering */
GLuint m_programSkybox;         /*!< handle of the program object (i.e. shaders) for skybox rendering */
GLuint m_programTex;            /*!< handle of the program object (i.e. shaders) for texture space diffusion rendering */
GLuint m_programGbuffer;        /*!< handle of the program object (i.e. shaders) for G-buffer calculation */
GLuint m_programSSAO;           /*!< handle of the program object (i.e. shaders) for SSAO calculation */
GLuint m_programQuadFinal;      /*!< handle of the program object (i.e. shaders) for Final color + SSAO rendering */
GLuint m_programSSLR;           /*!< handle of the program object (i.e. shaders) for SSLR calculation */


/* 2 types of quads: floor (horizontal), or screen quad */
enum quadType { FLOOR = 0, SCREEN = 1 };

// UI flags
bool m_isBackgroundWhite = false;   /*!< background color flag */
bool m_isFloorOn = true;            /*!< draw floor flag */
static int m_lightType = 0;         /*!< Type of light source: Point = 0, Directional = 1 */
bool m_isShadowOn = false;          /*!< Shadow mapping flag */
bool m_isEnvReflecOn = true;        /*!< Environment mapping reflection on  */
bool m_isEnvRefracOn = false;       /*!< Environment mapping refraction on  */
static int m_modelType = 0;         /*!< Type of model : basic mesh (no UV) = 0, UV (unwrapped) mesh = 1, PBR (provided with textures) mesh = 2 */
bool m_isAlbedoTexOn = false;       /*!< Texture (albedo) mapping on  */
bool m_isNormalMapOn = false;       /*!< normal mapping on  */
bool m_isAmbientOccMapOn = false;   /*!< ambient occlusion mapping on  */
bool m_isPBRMapOn = false;          /*!< gloss and/or metal map mapping on  */
bool m_isEnvMapOn = false;          /*!< environment mapping on  */
int m_envMapType = 0;               /*!< environment mapping type: reflection = 0 , refraction = 1  */
bool m_isAOMapOn = false;           /*!< ambient occlusion mapping on  */
bool m_isSimTransmitOn = false;     /*!< simulate transmission on  */
bool m_isTSDOn = false;             /*!< texture space diffusion on  */
bool m_isSSAOOn = false;            /*!< screen-space ambient occlusion on  */
bool m_isSSLROn = false;            /*!< screen-space light reflection on  */


int m_filterWidth = 2;

float m_ssaoRadius = 1.0;

std::vector<glm::vec3> m_ssaoKernel;
GLuint m_noiseTex;          

glm::vec3 bBoxMin;
static int m_fileMesh = 0;
const char *m_fileBasicMeshList[] = { "armadillo",
                                      "gargoyle",
                                      "dragon",
                                      "sphinx",
                                      "teapot"  };
const char *m_fileUVMeshList[] =    { "bunny_UV",
                                      "head_UV",
                                      "suzanne_UV" };
const char *m_filePBRMeshList[] =   { "grenade_PBR",
                                      "cerberus_PBR",
                                      "matball_PBR" };

static int m_fileTex = 0;
const char *m_fileTexList[] =       { "UV_template",
                                      "chessboard",
                                      "sand",
                                      "fabric" };
static int m_fileCubeMap = 1;
const char *m_fileCubeMapList[] =   { "Church",
                                      "Water",
                                      "River",
                                      "GoldenGate",
                                      "Forrest",
                                      "Vasa" };


std::string shaderDir = "../../src/shaders/";   /*!< relative path to shaders folder  */
std::string modelDir = "../../models/";   /*!< relative path to meshes and textures files folder  */


// Functions definitions

void initialize();
void initScene(TriMesh *_triMesh);
void setupImgui(GLFWwindow *window);
void update();
void displayShadowMap();
void displayGBuffering();
void displayLighting();
void displayTSD();
void displaySSAO();
void resizeCallback(GLFWwindow* window, int width, int height);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void charCallback(GLFWwindow* window, unsigned int codepoint);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void cursorPosCallback(GLFWwindow* window, double x, double y);
void runGUI();
int main(int argc, char** argv);




    /*------------------------------------------------------------------------------------------------------------+
    |                                                      INIT                                                   |
    +-------------------------------------------------------------------------------------------------------------*/


void initialize()
{   
    // init scene parameters
    m_lightCol = glm::vec3(1.0f, 1.0f, 1.0f);
    m_zoomFactor = 1.0f;

    // Setup background color
    glClearColor(0.0f, 0.0f, 0.0f, 0.0);

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);    
        
    // init model matrix
    m_modelMatrix = glm::mat4(1.0f);

    // init mesh
    m_triMesh = new TriMesh(true, false, false);
    m_triMesh->readFile(modelDir + "teapot.obj");

    initScene(m_triMesh);

    // setup mesh rendering
    m_drawMesh = new DrawableMesh;
    m_drawMesh->createMeshVAO(m_triMesh);

    // setup screen quad rendering
    m_drawQuad = new DrawableMesh;
    m_drawQuad->createQuadVAO(SCREEN);

    // setup floor quad rendering
    m_drawFloor = new DrawableMesh;
    m_drawFloor->createQuadVAO(FLOOR, bBoxMin.y, m_centerCoords, m_radScene);

    m_drawSkybox = new DrawableMesh;
    m_drawSkybox->createCubeVAO( m_centerCoords, m_radScene);

    // shader headers: function and declarations shared by several shader programs
    std::string vertHeader = shaderDir + "header.vert";
    std::string fragHeader = shaderDir + "header.frag";
    // init shaders
    m_programLighting = loadShaderProgram(shaderDir + "lighting.vert", shaderDir + "lighting.frag", vertHeader, fragHeader);  // compute 3D lighting (writes to UV coords if TSD on) 
    m_programShadow = loadShaderProgram(shaderDir + "shadowMap.vert", shaderDir + "shadowMap.frag");    // renders 3D scene and writes depthbuffer to shadowmap
    m_programQuad = loadShaderProgram(shaderDir + "quadTex.vert", shaderDir + "quadTex.frag");          // renders screenQuad with texture one (blurs texture if blurring on)
    m_programSkybox = loadShaderProgram(shaderDir + "skyBox.vert", shaderDir + "skyBox.frag");          // renders sky box with environment map
    m_programTex = loadShaderProgram(shaderDir + "meshTex.vert", shaderDir + "meshTex.frag");           // renders mesh with texture, without any lighting
    m_programGbuffer = loadShaderProgram(shaderDir + "gBuffer.vert", shaderDir + "gBuffer.frag");       // renders 3D scene and writes G-buffers to positionTex, normalTex, and colorTex
    m_programSSAO = loadShaderProgram(shaderDir + "ssao.vert", shaderDir + "ssao.frag");                // renders scene from G-buffer and writes SSAO map
    m_programQuadFinal = loadShaderProgram(shaderDir + "final.vert", shaderDir + "final.frag");         // renders scenes from screenTex and SSAmap
    m_programSSLR = loadShaderProgram(shaderDir + "sslr.vert", shaderDir + "sslr.frag");                // renders scene from G-buffer and writes SSLR map

    // build FBO and depth texture output for shadow map generation
    buildShadowFBOandTex(&m_shadowFBO, &m_shadowMapTex, TEX_WIDTH, TEX_HEIGHT);

    // build FBO and texture output for texture space diffusion
    buildScreenFBOandTex(&m_screenFBO, &m_screenTex, TEX_WIDTH, TEX_HEIGHT, true, false);

    // build G-buffer FBO and textures
    buildGbuffFBOandTex(&m_gFBO, &m_gPosition, &m_gNormal, &m_gColor, TEX_WIDTH, TEX_HEIGHT);

    // build FBO and texture output for texture space diffusion
    buildScreenFBOandTex(&m_tsdFBO, &m_tsdTex, TEX_WIDTH, TEX_HEIGHT, false, true);

    // build FBO and texture output for texture space diffusion
    buildScreenFBOandTex(&m_SSAOFBO, &m_SSAOTex, TEX_WIDTH, TEX_HEIGHT, false, false);

    // build FBO and texture output for blurring of SSAO texture
    buildScreenFBOandTex(&m_BlurFBO, &m_BlurTex, TEX_WIDTH, TEX_HEIGHT, false, false);

    // build FBO and texture output for blurring of SSAO texture
    buildScreenFBOandTex(&m_BlurFBO2, &m_BlurTex2, TEX_WIDTH, TEX_HEIGHT, false, false);

    // build FBO and texture output for SSLR
    buildScreenFBOandTex(&m_SSLRFBO, &m_SSLRTex, TEX_WIDTH, TEX_HEIGHT, true, false);

    m_ssaoKernel = buildRandKernel();
    buildKernelRot(&m_noiseTex); 

    m_drawQuad->setSSAOKernel(m_ssaoKernel);
    m_drawQuad->setNoiseTex(m_noiseTex);
}


void initScene(TriMesh *_triMesh)
{
    m_triMesh->computeAABB();
    bBoxMin = m_triMesh->getBBoxMin();
    glm::vec3 bBoxMax = m_triMesh->getBBoxMax();
    if(bBoxMin != bBoxMax)
    {
        // set the center of the scene to the center of the bBox
        m_centerCoords = glm::vec3( (bBoxMin.x + bBoxMax.x) * 0.5f, (bBoxMin.y + bBoxMax.y) * 0.5f, (bBoxMin.z + bBoxMax.z) * 0.5f );
    }
    m_radScene = glm::length(bBoxMax - bBoxMin) * 0.5f;
    m_minDistLight = m_radScene * 3.5f;
    m_maxDistLight = m_radScene * 8.0f;
    m_lightCamNearRad = m_radScene * 3.0f;
    m_lightCamFarRad = m_radScene * 3.5f;

    // init camera and lightsource position
    m_camPos = glm::vec3(0.0f, m_radScene*0.6f, m_radScene*3.0f);
    m_lightSpherePos = m_lightSpherePosInit = glm::vec3(M_PI*0.5f, 0.0f, m_radScene*6.0f);
    // init camera
    m_camera.init(/*m_radScene*/ 0.01f, m_radScene*8.0f, 45.0f, 1.0f, m_winWidth, m_winHeight, m_camPos, glm::vec3(0.0f, 0.0f, 0.0f), 0); 
    m_cstProjMatrix = m_camera.getProjectionMatrix();
    // init light camera
    m_cameraLight.init(m_lightSpherePos.z - m_lightCamNearRad, m_lightSpherePos.z + m_lightCamFarRad, 45.0f, 1.0f, m_winWidth, m_winHeight, sphericalToEuclidean(m_lightSpherePos) + m_centerCoords, m_centerCoords, m_lightType, m_radScene);
    // init trackball
    m_trackball.init(m_winWidth, m_winHeight);
}


void setupImgui(GLFWwindow *window)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    // enable keyboard controls?
    ImGui::StyleColorsDark();
    // platform and renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");
}


    /*------------------------------------------------------------------------------------------------------------+
    |                                                     UPDATE                                                  |
    +-------------------------------------------------------------------------------------------------------------*/

void update()
{
    // update model matrix with trackball rotation
    m_modelMatrix = glm::translate( m_trackball.getRotationMatrix(), -m_centerCoords);
}



    /*------------------------------------------------------------------------------------------------------------+
    |                                                     DISPLAY                                                 |
    +-------------------------------------------------------------------------------------------------------------*/

void displayShadowMap() 
{
    if(m_isShadowOn || m_isSimTransmitOn)
    {
        // bind dedicated FBO
        glBindFramebuffer(GL_FRAMEBUFFER, m_shadowFBO);

        // resize viewport to output texture dimension
        glViewport(0, 0, TEX_WIDTH, TEX_HEIGHT);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//        glEnable(GL_CULL_FACE);   // !! @ TODO ? !!
//        glCullFace(GL_FRONT);

        // build LightView matrix
        glm::mat4 lv = m_cameraLight.getViewMatrix();
        // build LightViewProjection matrix
        glm::mat4 projection = m_cameraLight.getProjectionMatrix();
        glm::mat4 lvp = projection * lv;

        // draw scene
        m_drawMesh->drawShadow(m_programShadow, lvp);
        if(m_isFloorOn)
            m_drawFloor->drawShadow(m_programShadow, lvp);

//        glCullFace(GL_BACK);
//        glDisable(GL_CULL_FACE);


        // Bind default framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        // resize viewport to window dimensions
        glViewport(0, 0, m_winWidth, m_winHeight);
    }

}


void displayGBuffering()
{
    // generate G-buffer only if SSAO is activated
    if( m_isSSAOOn || m_isSSLROn )
    {
        // bind dedicated FBO
        glBindFramebuffer(GL_FRAMEBUFFER, m_gFBO);

        // resize viewport to output texture dimension
        glViewport(0, 0, TEX_WIDTH, TEX_HEIGHT);

        // switch background to black to make sure empty fragments are not processed
        glClearColor(0.0f, 0.0f, 0.0f, 0.0);    

        // Clear window with background color
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        glm::mat4 modelMat = m_modelMatrix;
        glm::mat4 viewMat = m_camera.getViewMatrix();
        glm::mat4 projMat = m_camera.getProjectionMatrix();

        // draw objects
        m_drawMesh->drawGbuffer(m_programGbuffer, modelMat, viewMat, projMat, false);

//        if(m_isFloorOn)
//            m_drawFloor->drawGbuffer(m_programGbuffer, modelMat, viewMat, projMat, true);


        // Bind default framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        // resize viewport to window dimensions
        glViewport(0, 0, m_winWidth, m_winHeight);


        if(m_isBackgroundWhite)
            glClearColor(1.0f, 1.0f, 1.0f, 0.0);
        else
            glClearColor(0.0f, 0.0f, 0.0f, 0.0);

    }
}


void displayLighting()
{
    
    if( m_isTSDOn )
    {
        // Bind dedicated FBO if the results must be saved in mesh texture for TSD

        if (m_modelType != 1 && m_modelType != 2) 
            std::cerr << "[ERROR] Main::Display(): Cannot apply texture space diffusion without UV map" << std::endl;

        // bind dedicated FBO
        glBindFramebuffer(GL_FRAMEBUFFER, m_tsdFBO);

        // resize viewport to output texture dimension
        glViewport(0, 0, TEX_WIDTH, TEX_HEIGHT);
    }
    else if( m_isSSAOOn || m_isSSLROn )
    {
        // Bind dedicated FBO if the results must be saved in screen-space texture for future rendering steps

        // bind dedicated FBO
        glBindFramebuffer(GL_FRAMEBUFFER, m_screenFBO);

        // resize viewport to output texture dimension
        glViewport(0, 0, TEX_WIDTH, TEX_HEIGHT);
    }


    m_drawMesh->setShadowMap(m_shadowMapTex);
    m_drawFloor->setShadowMap(m_shadowMapTex);

    // Clear window with background color
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



    // build ModelView matrix
    glm::mat4 mv = m_camera.getViewMatrix() * m_modelMatrix;
    // build ModelViewProjection matrix
    glm::mat4 projection = m_camera.getProjectionMatrix();
    glm::mat4 mvp = projection * mv;

     
    glm::mat4 modelMat = m_modelMatrix;

    glm::mat4 viewMat = m_camera.getViewMatrix();
    glm::mat4 projMat = m_camera.getProjectionMatrix();

    glm::mat4 lightSpaceMat =  m_cameraLight.getProjectionMatrix() * m_cameraLight.getViewMatrix();


    // draw objects
    m_drawMesh->draw(m_programLighting, modelMat, viewMat, projMat, sphericalToEuclidean(m_lightSpherePos), m_camPos, m_lightCol, lightSpaceMat, m_maxDistLight);

    if(m_isFloorOn && !m_isTSDOn)
        m_drawFloor->draw(m_programLighting, modelMat, viewMat, projMat, sphericalToEuclidean(m_lightSpherePos), m_camPos, m_lightCol, lightSpaceMat, m_maxDistLight);


    // draw sky box
    if(m_isEnvMapOn && !m_isTSDOn)
    {
        glm::mat4 v = glm::mat4(glm::mat3( m_camera.getViewMatrix() )) * m_modelMatrix;
        //glm::mat4 v = m_camera.getViewMatrix() ;
        glm::mat4 p = m_cstProjMatrix;// m_camera.getProjectionMatrix();

        m_drawSkybox->drawSkyBox(m_programSkybox, v, p);
    }


    if( m_isTSDOn || m_isSSAOOn || m_isSSLROn)
    {
        // Bind default framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        // resize viewport to window dimensions
        glViewport(0, 0, m_winWidth, m_winHeight);
    }
}


void displayTSD()
{
    if( m_isTSDOn )
    {
        // Clear window with background color
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

     
        glm::mat4 modelMat = m_modelMatrix;
        glm::mat4 viewMat = m_camera.getViewMatrix();
        glm::mat4 projMat = m_camera.getProjectionMatrix();


        // re-use FBO to overwrite tsdTex
        glBindFramebuffer(GL_FRAMEBUFFER, m_tsdFBO);
        glViewport(0, 0, TEX_WIDTH, TEX_HEIGHT);
        m_drawQuad->drawScreenQuad(m_programQuad, m_tsdTex, true, true, m_filterWidth);
        m_drawQuad->drawScreenQuad(m_programQuad, m_tsdTex, true, false, m_filterWidth);


        if( m_isSSAOOn || m_isSSLROn )
        {
            // Bind dedicated FBO if the results must be saved in screen-space texture for future rendering steps

            // bind dedicated FBO
            glBindFramebuffer(GL_FRAMEBUFFER, m_screenFBO);

            // resize viewport to output texture dimension
            glViewport(0, 0, TEX_WIDTH, TEX_HEIGHT);
        }
        else
        {
            // Bind default framebuffer if there is no future rendering steps

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glViewport(0, 0, m_winWidth, m_winHeight);
        }
        // Clear window with background color
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //m_drawQuad->drawScreenQuad(m_programQuad, modelMat, viewMat, projMat, m_tsdTex, false);
        m_drawMesh->drawTex(m_programTex, modelMat, viewMat, projMat, m_tsdTex);


        glm::mat4 lightSpaceMat =  m_cameraLight.getProjectionMatrix() * m_cameraLight.getViewMatrix();

        // draw floor
        if(m_isFloorOn)
            m_drawFloor->draw(m_programLighting, modelMat, viewMat, projMat, sphericalToEuclidean(m_lightSpherePos), m_camPos, m_lightCol, lightSpaceMat, m_maxDistLight);


        // draw skybox
        if(m_isEnvMapOn)
        {
            glm::mat4 v = glm::mat4(glm::mat3( m_camera.getViewMatrix() )) * m_modelMatrix;
            glm::mat4 p = m_cstProjMatrix;

            m_drawSkybox->drawSkyBox(m_programSkybox, v, p);
        }

        // Bind default framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        // resize viewport to window dimensions
        glViewport(0, 0, m_winWidth, m_winHeight);
    }

}



void displaySSAO()
{
    if( m_isSSAOOn )
    {

        // bind dedicated FBO
        glBindFramebuffer(GL_FRAMEBUFFER, m_SSAOFBO);
        // resize viewport to output texture dimension
        glViewport(0, 0, TEX_WIDTH, TEX_HEIGHT);

        // switch background to white to make sure empty fragments do not disapear after applying AO factor to color
        glClearColor(1.0f, 1.0f, 1.0f, 1.0); 
        // Clear window with background color
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 
        glm::mat4 modelMat = m_modelMatrix;
        glm::mat4 viewMat = m_camera.getViewMatrix();
        glm::mat4 projMat = m_camera.getProjectionMatrix();

        // generate SSAO texture
        m_drawQuad->drawScreenQuadSSAO(m_programSSAO, projMat, m_gPosition, m_gNormal, m_ssaoRadius, (float)m_winWidth, (float)m_winHeight);


        // bind Appropriate FBO
        glBindFramebuffer(GL_FRAMEBUFFER, m_BlurFBO);
        glViewport(0, 0, TEX_WIDTH, TEX_HEIGHT); 
        // SSAO texture blurring
        m_drawQuad->drawScreenQuad(m_programQuad, m_SSAOTex, true, true, 4);
        m_drawQuad->drawScreenQuad(m_programQuad, m_BlurTex, true, false, 4);

        if( m_isSSLROn ) 
        {
            // Bind dedicated FBO if the results must be saved in screen-space texture for future rendering steps

            // bind dedicated FBO
            glBindFramebuffer(GL_FRAMEBUFFER, m_SSAOFBO);
            // resize viewport to output texture dimension
            glViewport(0, 0, TEX_WIDTH, TEX_HEIGHT);
        }
        else
        {
            // Bind default framebuffer if there is no future rendering steps

            // Bind default framebuffer
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            // resize viewport to window dimensions
            glViewport(0, 0, m_winWidth, m_winHeight); 
        }

        // Clear window with background color
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Draw screen Texture + SSAO
        m_drawQuad->drawScreenQuadFinal(m_programQuadFinal, modelMat, viewMat, projMat, m_BlurTex, m_screenTex, 1);// occ_type = ssao


        if(m_isBackgroundWhite)
            glClearColor(1.0f, 1.0f, 1.0f, 0.0);
        else
            glClearColor(0.0f, 0.0f, 0.0f, 0.0);


    }
}



void displaySSLR()
{
    if( m_isSSLROn )
    {

        // bind dedicated FBO
        glBindFramebuffer(GL_FRAMEBUFFER, m_SSLRFBO);
        // resize viewport to output texture dimension
        glViewport(0, 0, TEX_WIDTH, TEX_HEIGHT);

        // switch background to white to make sure empty fragments do not disapear after applying AO factor to color
        glClearColor(1.0f, 1.0f, 1.0f, 1.0); 
        // Clear window with background color
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 
        glm::mat4 modelMat = m_modelMatrix;
        glm::mat4 viewMat = m_camera.getViewMatrix();
        glm::mat4 projMat = m_camera.getProjectionMatrix();

        // generate SSLR texture
        m_drawQuad->drawScreenQuadSSLR(m_programSSLR, modelMat, viewMat, projMat, m_gPosition, m_gNormal, m_screenTex, m_ssaoRadius, (float)m_winWidth, (float)m_winHeight);


        // bind Appropriate FBO
        glBindFramebuffer(GL_FRAMEBUFFER, m_BlurFBO2);
        glViewport(0, 0, TEX_WIDTH, TEX_HEIGHT); 
        // SSLR texture blurring
        m_drawQuad->drawScreenQuad(m_programQuad, m_SSLRTex, true, true, 4);
        m_drawQuad->drawScreenQuad(m_programQuad, m_BlurTex2, true, false, 4);

        // Bind default framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        // resize viewport to window dimensions
        glViewport(0, 0, m_winWidth, m_winHeight);

        // Clear window with background color
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if( m_isSSAOOn )
        {
            // Draw SSLR + Screen texture (which contains SSAO already)
            m_drawQuad->drawScreenQuadFinal(m_programQuadFinal, modelMat, viewMat, projMat, m_BlurTex2, m_SSAOTex, 2); // occ_type = SSLR
        }
        else
        {
            // Draw SSLR + Screen texture (without SSAO)
            m_drawQuad->drawScreenQuadFinal(m_programQuadFinal, modelMat, viewMat, projMat, m_BlurTex2, m_screenTex, 2); // occ_type = SSLR
        }

        if(m_isBackgroundWhite)
            glClearColor(1.0f, 1.0f, 1.0f, 0.0);
        else
            glClearColor(0.0f, 0.0f, 0.0f, 0.0);

        // debug
        //m_drawQuad->drawScreenQuad(m_programQuad, /*m_SSLRTex*/ m_SSAOTex, false); 
    }

}


    /*------------------------------------------------------------------------------------------------------------+
    |                                                CALLBACK METHODS                                             |
    +-------------------------------------------------------------------------------------------------------------*/


void resizeCallback(GLFWwindow* window, int width, int height)
{
    m_winWidth = width;
    m_winHeight = height;
    glViewport(0, 0, width, height);

    // re-init trackball and camera
    m_camera.initProjectionMatrix(m_winWidth, m_winHeight, 1.0f, 0);
    m_cstProjMatrix = m_camera.getProjectionMatrix();
    m_camera.initProjectionMatrix(m_winWidth, m_winHeight, m_zoomFactor, 0);
    m_trackball.init(m_winWidth, m_winHeight);

    // keep drawing while resize
    update();
    displayLighting();
    displayTSD();

    // Swap between front and back buffer
    glfwSwapBuffers(m_window);
}


void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (ImGui::GetIO().WantCaptureKeyboard) { return; }   // Skip other handling when ImGUI is used   

    // return to init positon when "R" pressed
    if (key == GLFW_KEY_R && action == GLFW_PRESS) 
    {
        // restart trackball
        m_trackball.reStart();
        // re-init zoom
        m_zoomFactor = 1.0f;
        m_ssaoRadius = m_zoomFactor;
        // update camera
        m_camera.initProjectionMatrix(m_winWidth, m_winHeight, m_zoomFactor, 0);

        // re-init light source position
        m_lightSpherePos = m_lightSpherePosInit;
        m_cameraLight.init(m_lightSpherePos.z - m_lightCamNearRad, m_lightSpherePos.z + m_lightCamFarRad, 45.0f, 1.0f, m_winWidth, m_winHeight, sphericalToEuclidean(m_lightSpherePos) + m_centerCoords, m_centerCoords, m_lightType, m_radScene);
    }

    // Arrow keys for light source position control
    if (key == GLFW_KEY_RIGHT && ( action == GLFW_PRESS ||  action == GLFW_REPEAT    ) ) 
    {
        // increase azimuth angle of the light pos
        m_lightSpherePos.x += 0.1f;
    }
    else if (key == GLFW_KEY_LEFT && ( action == GLFW_PRESS ||  action == GLFW_REPEAT    ) ) 
    {
        // decrease azimuth angle of the light pos
        m_lightSpherePos.x -= 0.1f;
    }
    else if (key == GLFW_KEY_UP && ( action == GLFW_PRESS ||  action == GLFW_REPEAT    ) ) 
    {
        if(m_lightSpherePos.y < glm::radians(90.0f) )
        {
            // increase zenith angle of the light pos
            m_lightSpherePos.y += 0.1f;
        }
    }
    else if (key == GLFW_KEY_DOWN && ( action == GLFW_PRESS ||  action == GLFW_REPEAT    ) ) 
    {
        if(m_lightSpherePos.y > glm::radians(0.0f) )
        {
            // decrease zenith angle of the light pos
            m_lightSpherePos.y -= 0.1f;
        }
    }
    else if (key == GLFW_KEY_KP_ADD && ( action == GLFW_PRESS ||  action == GLFW_REPEAT    ) ) 
    {
        if(m_lightSpherePos.z < m_maxDistLight && m_lightType == 0)
        {
            // increase radius of the light pos
            m_lightSpherePos.z += 0.1f;
            // update light camera
            m_cameraLight.init(m_lightSpherePos.z - m_lightCamNearRad, m_lightSpherePos.z + m_lightCamFarRad, 45.0f, 1.0f, m_winWidth, m_winHeight, sphericalToEuclidean(m_lightSpherePos)+m_centerCoords, m_centerCoords, m_lightType, m_radScene);
        }
    }
    else if (key == GLFW_KEY_KP_SUBTRACT && ( action == GLFW_PRESS ||  action == GLFW_REPEAT    ) ) 
    {
        if(m_lightSpherePos.z > m_minDistLight && m_lightType == 0)
        {
            // decreas radius of the light pos
            m_lightSpherePos.z -= 0.1f;
            // update light camera
            m_cameraLight.init(m_lightSpherePos.z - m_lightCamNearRad, m_lightSpherePos.z + m_lightCamFarRad, 45.0f, 1.0f, m_winWidth, m_winHeight, sphericalToEuclidean(m_lightSpherePos)+m_centerCoords, m_centerCoords, m_lightType, m_radScene);
        }
    }

    // update light camera view matrix
    if(m_lightType == 1)
        m_cameraLight.initViewMatrix(sphericalToEuclidean(glm::vec3(m_lightSpherePos.x, m_lightSpherePos.y, m_lightSpherePosInit.z))+m_centerCoords, m_centerCoords);
    else
        m_cameraLight.initViewMatrix(sphericalToEuclidean(m_lightSpherePos)+m_centerCoords, m_centerCoords);

}


void charCallback(GLFWwindow* window, unsigned int codepoint)
{}


void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (ImGui::GetIO().WantCaptureMouse) { return; }  // Skip other handling when ImGUI is used   
    
    // get mouse cursor position
    double x, y;
    glfwGetCursorPos(window, &x, &y);

    // activate/de-activate trackball with mouse button
    if (action == GLFW_PRESS) 
    {
        if (button == GLFW_MOUSE_BUTTON_LEFT)
            m_trackball.startTracking( glm::vec2(x, y) );
    }
    else 
    {
        if (button == GLFW_MOUSE_BUTTON_LEFT)
            m_trackball.stopTracking();
    }
    
}


void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (ImGui::GetIO().WantCaptureMouse) { return; }  // Skip other handling when ImGUI is used

    // update zoom factor
    double newZoom = m_zoomFactor - yoffset / 10.0f;
    if(newZoom > 0.0f && newZoom < 2.0f)
    {
        m_zoomFactor -= (float)yoffset/10.0f;
        m_ssaoRadius = m_zoomFactor;
    }
    // update camera
    m_camera.initProjectionMatrix(m_winWidth, m_winHeight, m_zoomFactor, 0);

}


void cursorPosCallback(GLFWwindow* window, double x, double y)
{
    // rotate trackball according to mouse cursor movement
    if ( m_trackball.isTracking()) 
        m_trackball.move( glm::vec2(x, y) );

}




    /*------------------------------------------------------------------------------------------------------------+
    |                                                      MAIN                                                   |
    +-------------------------------------------------------------------------------------------------------------*/

void runGUI()
{

    if(ImGui::Begin("Settings"))
    {

        // ImGui frame rate measurement
        float frameRate = ImGui::GetIO().Framerate;
        ImGui::Text("FrameRate: %.3f ms/frame (%.1f FPS)", 1000.0f / frameRate, frameRate);

            ImGui::Separator();

        if (ImGui::BeginTabBar("tab bar"))
        {
            // first tab: scene parameters
            if (ImGui::BeginTabItem("Scene"))
            {
                // change backbround color
                if( ImGui::Checkbox("White Background ", &m_isBackgroundWhite) )
                {
                    glClear(GL_COLOR_BUFFER_BIT);
                    if(m_isBackgroundWhite)
                        glClearColor(1.0f, 1.0f, 1.0f, 0.0);
                    else
                        glClearColor(0.0f, 0.0f, 0.0f, 0.0);
                }

                // show floor check box
                ImGui::Checkbox("Show floor ", &m_isFloorOn);

                // Light source type radio button
                ImGui::Text("Light source "); ImGui::SameLine();
                if( ImGui::RadioButton("point", &m_lightType, 0) )
                {
                    // re-init light camera
                    m_cameraLight.init(m_lightSpherePos.z - m_lightCamNearRad, m_lightSpherePos.z + m_lightCamFarRad, 45.0f, 1.0f, m_winWidth, m_winHeight, sphericalToEuclidean(m_lightSpherePos)+m_centerCoords, m_centerCoords, m_lightType, m_radScene);
                    m_drawMesh->setLightDirFlag(false);
                    m_drawFloor->setLightDirFlag(false);
                }
                ImGui::SameLine();
                if( ImGui::RadioButton("directional", &m_lightType, 1) )
                {
                    // re-init light camera
                    m_cameraLight.init(m_lightCamNearRad, m_lightCamNearRad + m_lightCamFarRad, 45.0f, 1.0f, m_winWidth, m_winHeight, sphericalToEuclidean(glm::vec3(m_lightSpherePos.x, m_lightSpherePos.y, m_lightSpherePosInit.z))+m_centerCoords, m_centerCoords, m_lightType, m_radScene);
                    m_drawMesh->setLightDirFlag(true);
                    m_drawFloor->setLightDirFlag(true);
                }
                else
                {
                    // slider for light radius
                    //ImGui::SliderFloat("distance to scene", &m_lightSpherePos.z, m_minDistLight, m_maxDistLight, "%.2f");
                    //m_cameraLight.init(m_lightSpherePos.z - m_lightCamNearRad, m_lightSpherePos.z + m_lightCamFarRad, 45.0f, 1.0f, m_winWidth, m_winHeight, sphericalToEuclidean(m_lightSpherePos)+m_centerCoords, m_centerCoords, m_lightType, m_radScene);
                }

                ImGui::EndTabItem();
            }

            // second tab: load model
            if (ImGui::BeginTabItem("Model"))
            {
                ImGui::Text("model type ");
                // radio button for mesh type to load
                ImGui::RadioButton("no UV mesh", &m_modelType, 0);
                ImGui::SameLine();
                ImGui::RadioButton("UV mesh", &m_modelType, 1);
                ImGui::SameLine();
                ImGui::RadioButton("PBR mesh", &m_modelType, 2);

                if (m_modelType == 0 )
                {
                    // load a basic mesh with no UV coords (texture mapping and texture-space diffusion not possible)
                    ImGui::ListBox("", &m_fileMesh, m_fileBasicMeshList, IM_ARRAYSIZE(m_fileBasicMeshList));
                }
                else if (m_modelType == 1 )
                {
                    // load a mesh with unwrapped UV coords (texture mapping and texture-space diffusion possible), but no specific textures
                    ImGui::ListBox("", &m_fileMesh, m_fileUVMeshList, IM_ARRAYSIZE(m_fileUVMeshList));
                }
                else if (m_modelType == 2 )
                {
                    // load a mesh with unwrapped UV coords, provided with PBR textures
                    ImGui::ListBox("", &m_fileMesh, m_filePBRMeshList, IM_ARRAYSIZE(m_filePBRMeshList));
                }
                if (ImGui::Button("Load mesh"))
                {
                    if (m_modelType == 0 )
                    {
                        m_triMesh->readFile( modelDir + std::string(m_fileBasicMeshList[m_fileMesh]) + ".obj" );
                    }
                    if (m_modelType == 1 )
                    {
                        m_triMesh->readFile( modelDir + std::string(m_fileUVMeshList[m_fileMesh]) + ".obj" );
                    }
                    if (m_modelType == 2 )
                    {
                        m_triMesh->readFile( modelDir + std::string(m_filePBRMeshList[m_fileMesh]) + ".obj" );
                        m_triMesh->computeTB();
                    }
                    initScene(m_triMesh);

                    // setup mesh rendering
                    m_drawMesh = new DrawableMesh;
                    m_drawMesh->createMeshVAO(m_triMesh);
                    m_drawMesh->setAlbedoTexFlag(m_isAlbedoTexOn);
                    m_drawMesh->setEnvMapReflecFlag(m_isEnvReflecOn);
                    m_drawMesh->setEnvMapReflecFlag(m_isEnvRefracOn);
                    m_drawMesh->setShadowMapFlag(m_isShadowOn);
                    m_drawMesh->setSimTransmitFlag(m_isSimTransmitOn);
                    m_drawMesh->setTSDFlag(m_isTSDOn);
                    
                    if(m_modelType == 2 && m_fileMesh == 0)
                    {
                        // load corresponding PBR textures
                        m_drawMesh->loadAlbedoTex( modelDir + "tex_grenade/Grenade_A.png" );
                        m_drawMesh->loadGlossMap( modelDir + "tex_grenade/Grenade_R.png" );
                        m_drawMesh->loadMetalMap( modelDir + "tex_grenade/Grenade_M.png" );
                        m_drawMesh->loadNormalMap( modelDir + "tex_grenade/Grenade_N.png" );
                        m_drawMesh->loadAmbientMap( modelDir + "tex_grenade/Grenade_AO.png" );
                    }
                    else if (m_modelType == 2 && m_fileMesh == 1)
                    {
                        m_drawMesh->loadAlbedoTex( modelDir + "tex_cerberus/Cerberus_A.png" );
                        m_drawMesh->loadGlossMap( modelDir + "tex_cerberus/Cerberus_R.png" );
                        m_drawMesh->loadMetalMap( modelDir + "tex_cerberus/Cerberus_M.png" );
                        m_drawMesh->loadNormalMap( modelDir + "tex_cerberus/Cerberus_N.png" );
                    }
                    else if (m_modelType == 2 && m_fileMesh == 2)
                    {
                        m_drawMesh->loadAlbedoTex( modelDir + "tex_matball/Matball_A.png" );
                        m_drawMesh->loadGlossMap( modelDir + "tex_matball/Matball_R.png" );
                        m_drawMesh->loadMetalMap( modelDir + "tex_matball/Matball_M.png" );
                        m_drawMesh->loadNormalMap( modelDir + "tex_matball/Matball_N.png" );
                        m_drawMesh->loadAmbientMap( modelDir + "tex_matball/Matball_AO.png" );
                    }

                    // setup floor quad rendering
                    m_drawFloor = new DrawableMesh;
                    m_drawFloor->createQuadVAO(FLOOR, bBoxMin.y, m_centerCoords, m_radScene);
                    m_drawFloor->setShadowMapFlag(m_isShadowOn);
                }

                ImGui::EndTabItem();
            }

            // third tab: load textures
            if (ImGui::BeginTabItem("Textures"))
            {
                // load texture
                ImGui::ListBox("", &m_fileTex, m_fileTexList, IM_ARRAYSIZE(m_fileTexList));
                if (ImGui::Button("Load texture"))
                    m_drawMesh->loadAlbedoTex( modelDir + "textures/" + std::string(m_fileTexList[m_fileTex]) + ".png" );

                ImGui::EndTabItem();
            }

            // fourth tab: load cubemap
            if (ImGui::BeginTabItem("Cubemap"))
            {
                // choose cubemap
                ImGui::ListBox("", &m_fileCubeMap, m_fileCubeMapList, IM_ARRAYSIZE(m_fileCubeMapList));
                if (ImGui::Button("Load Cube Map"))
                {
                    // add cube map to mesh rendering
                    m_drawMesh->loadCubeMap( modelDir + "cubemaps/" + std::string(m_fileCubeMapList[m_fileCubeMap]) );
                    m_drawSkybox->loadCubeMap( modelDir + "cubemaps/" + std::string(m_fileCubeMapList[m_fileCubeMap]) );
                    m_drawQuad->loadCubeMap( modelDir + "cubemaps/" + std::string(m_fileCubeMapList[m_fileCubeMap]) );
                }

                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        } // end tab bar

        ImGui::Separator();

        // toggle texture mapping
        if (m_modelType == 1 )
        {
            if(ImGui::Checkbox("Texture mapping ", &m_isAlbedoTexOn) )
                m_drawMesh->setAlbedoTexFlag(m_isAlbedoTexOn);
        }
        else if (m_modelType == 2 )
        {
            if(ImGui::Checkbox("Albedo Map ", &m_isAlbedoTexOn) )
                m_drawMesh->setAlbedoTexFlag(m_isAlbedoTexOn);
            if(ImGui::Checkbox("Normal Map ", &m_isNormalMapOn) )
                m_drawMesh->setNormalMapFlag(m_isNormalMapOn);
            if(ImGui::Checkbox("Metal and/or gloss Map ", &m_isPBRMapOn) )
                m_drawMesh->setPBRFlag(m_isPBRMapOn);
            if(m_fileMesh == 0 || m_fileMesh == 2)
                if(ImGui::Checkbox("AO Map ", &m_isAOMapOn) )
                    m_drawMesh->setAmbMapFlag(m_isAOMapOn);

        }

        // Shadow mapping checkbox
        if( ImGui::Checkbox("Simulate transmission", &m_isSimTransmitOn) )
        {
            m_drawMesh->setSimTransmitFlag(m_isSimTransmitOn);
        }

        if ( ImGui::Checkbox("Environment mapping ", &m_isEnvMapOn) )
        {
            m_envMapType = 0;
            m_isEnvReflecOn = true;
            m_isEnvRefracOn = false; 
            m_drawMesh->setEnvMapReflecFlag(m_isEnvReflecOn);
            m_drawMesh->setEnvMapRefracFlag(m_isEnvRefracOn);
        }
        if( m_isEnvMapOn )
        {
            
            if( ImGui::RadioButton("reflection", &m_envMapType, 0) )
            {
                m_isEnvReflecOn = true;
                m_isEnvRefracOn = false;
            }
            ImGui::SameLine();
            if( ImGui::RadioButton("refraction", &m_envMapType, 1) )
            {
                m_isEnvReflecOn = false;
                m_isEnvRefracOn = true; 
            }
            m_drawMesh->setEnvMapReflecFlag(m_isEnvReflecOn); 
            m_drawMesh->setEnvMapRefracFlag(m_isEnvRefracOn);
        }
        else
        {
            m_isEnvReflecOn = false;
            m_isEnvRefracOn = false;
            m_drawMesh->setEnvMapReflecFlag(m_isEnvReflecOn); 
            m_drawMesh->setEnvMapRefracFlag(m_isEnvRefracOn);
        }

        // Shadow mapping checkbox
        if( ImGui::Checkbox("Shadow mapping", &m_isShadowOn) )
        {
            m_drawMesh->setShadowMapFlag(m_isShadowOn);
            m_drawFloor->setShadowMapFlag(m_isShadowOn);
        }

        // Shadow mapping checkbox
        if (m_modelType == 1 ||  m_modelType == 2)
        {
            if( ImGui::Checkbox("Texture space diffusion", &m_isTSDOn) )
            {
                m_drawMesh->setTSDFlag(m_isTSDOn);
            }

            if(m_isTSDOn)
            {
                ImGui::SliderInt("filter width", &m_filterWidth, 1, 10);
            }
        }

        // Shadow mapping checkbox
        ImGui::Checkbox("SSAO", &m_isSSAOOn);

        // Shadow mapping checkbox
        ImGui::Checkbox("SSLR", &m_isSSLROn);

    } // end "Settings"

    
    ImGui::End();

    // render
    ImGui::Render();
}

int main(int argc, char** argv)
{

    /* Initialize GLFW and create a window */
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // <-- activate this line on MacOS
    m_window = glfwCreateWindow(m_winWidth, m_winHeight, "RT_lite demo", nullptr, nullptr);
    glfwMakeContextCurrent(m_window);
    glfwSetFramebufferSizeCallback(m_window, resizeCallback);
    glfwSetKeyCallback(m_window, keyCallback);
    glfwSetCharCallback(m_window, charCallback);
    glfwSetMouseButtonCallback(m_window, mouseButtonCallback);
    glfwSetScrollCallback(m_window, scrollCallback);
    glfwSetCursorPosCallback(m_window, cursorPosCallback);

    // init ImGUI
    setupImgui(m_window);


    // init GL extension wrangler
    glewExperimental = true;
    GLenum res = glewInit();
    if (res != GLEW_OK) 
    {
        fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
        return 1;
    }
    std::cout << std::endl
              << "Welcome to RT_lite" << std::endl << std::endl 
              << "Commands:" << std::endl
              << " - Mouse left button: trackball" << std::endl
              << " - Mouse scroll: camera zoom" << std::endl
              << " - right/left arrows: azimuth rotation of the light source" << std::endl
              << " - up/down arrows: zenith rotation of the light source" << std::endl
              << " - +/- : increase/decrease distance of (point) light source" << std::endl
              << " - R: re-init trackball and light position" << std::endl << std::endl
              << "Log:" << std::endl
              << "OpenGL version: " << glGetString(GL_VERSION) << std::endl
              << "Vendor: " << glGetString(GL_VENDOR) << std::endl;

    glGenVertexArrays(1, &m_defaultVAO);
    glBindVertexArray(m_defaultVAO);

    // call init function
    initialize();

    // main rendering loop
    while (!glfwWindowShouldClose(m_window)) 
    {
        // process events
        glfwPollEvents();
        // start frame for ImGUI
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        // build GUI
        runGUI();

        // idle updates
        update();
        // render shadow map
        displayShadowMap();
        // render G-buffer
        displayGBuffering();
        // render lighting
        displayLighting();
        // render scene
        displayTSD();
        // apply screen-space AO
        displaySSAO(); 
        // apply screen-space reflections
        displaySSLR();

        // render GUI
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        // Swap between front and back buffer
        glfwSwapBuffers(m_window);
    }


    // delete shadow map FBO and texture
    glDeleteFramebuffers(1, &m_shadowFBO);
    glDeleteTextures(1, &m_shadowMapTex);

    // Cleanup imGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // Close window
    glfwDestroyWindow(m_window);
    glfwTerminate();

    std::cout << std::endl << "Bye!" << std::endl;

    return 0;
}
