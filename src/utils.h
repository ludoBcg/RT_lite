/*********************************************************************************************************************
 *
 * utils.h
 *
 * Helper functions
 * 
 * RT_lite
 * Ludovic Blache
 *
 *********************************************************************************************************************/


#ifndef UTILS_H
#define UTILS_H


#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <random>

#define GLM_FORCE_RADIANS

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "GLtools.h"



        /*------------------------------------------------------------------------------------------------------------+
        |                                            MISC. FUNCTIONS                                                  |
        +------------------------------------------------------------------------------------------------------------*/



/*!
* \fn sphericalToEuclidean
* \brief Spherical coordinates to Euclidean coordinates
* \param _spherical : spherical 3D coords
* \return 3D Euclidean coords
*/
glm::vec3 sphericalToEuclidean(glm::vec3 _spherical)
{
    return glm::vec3( sin(_spherical.x) * cos(_spherical.y),
                      sin(_spherical.y),
                      cos(_spherical.x) * cos(_spherical.y) ) * _spherical.z;
}



/*!
* \fn readShaderSource
* \brief read shader program and copy it in a string
* \param _filename : shader file name
* \return string containing shader program
*/
std::string readShaderSource(const std::string& _filename)
{
    std::ifstream file(_filename);
    std::stringstream stream;
    stream << file.rdbuf();

    return stream.str();
}



/*!
* \fn showShaderInfoLog
* \brief print out shader info log (i.e. compilation errors)
* \param _shader : shader
*/
void showShaderInfoLog(GLuint _shader)
{
    GLint logInfoLength = 0;
    glGetShaderiv(_shader, GL_INFO_LOG_LENGTH, &logInfoLength);
    std::vector<char> logInfo(logInfoLength);
    glGetShaderInfoLog(_shader, logInfoLength, &logInfoLength, &logInfo[0]);
    std::string logInfoStr(logInfo.begin(), logInfo.end());
    std::cerr << "[SHADER INFOLOG] " << logInfoStr << std::endl;
}



/*!
* \fn showProgramInfoLog
* \brief print out program info log (i.e. linking errors)
* \param _program : program
*/
void showProgramInfoLog(GLuint _program)
{
    GLint logInfoLength = 0;
    glGetProgramiv(_program, GL_INFO_LOG_LENGTH, &logInfoLength);
    std::vector<char> logInfo(logInfoLength);
    glGetProgramInfoLog(_program, logInfoLength, &logInfoLength, &logInfo[0]);
    std::string logInfoStr(logInfo.begin(), logInfo.end());
    std::cerr << "[PROGRAM INFOLOG] " << logInfoStr << std::endl;
}



/*!
* \fn loadShaderProgram
* \brief load shader program from shader files
* \param _vertShaderFilename : vertex shader filename
* \param _fragShaderFilename : fragment shader filename
*/
GLuint loadShaderProgram(const std::string& _vertShaderFilename, const std::string& _fragShaderFilename, const std::string& _vertHeader="", const std::string& _fragHeader="")
{
    // read headers
    std::string vertHeaderSource, fragHeaderSource;
    vertHeaderSource = readShaderSource(_vertHeader);
    fragHeaderSource = readShaderSource(_fragHeader);


    // Load and compile vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    std::string vertexShaderSource = readShaderSource(_vertShaderFilename);
    if(!_vertHeader.empty() )
    {
        // if headers are provided, add them to the shader
        const char *vertSources[2] = {vertHeaderSource.c_str(), vertexShaderSource.c_str()};
        glShaderSource(vertexShader, 2, vertSources, nullptr);
    }
    else
    {
        // if no header provided, the shader is contained in a single file
        const char *vertexShaderSourcePtr = vertexShaderSource.c_str();
        glShaderSource(vertexShader, 1, &vertexShaderSourcePtr, nullptr);
    }
    glCompileShader(vertexShader);
    GLint success = 0;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) 
    {
        errorLog() << "loadShaderProgram(): Vertex shader compilation failed:";
        showShaderInfoLog(vertexShader);
        glDeleteShader(vertexShader);
        return 0;
    }


    // Load and compile fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    std::string fragmentShaderSource = readShaderSource(_fragShaderFilename);
    if(!_fragHeader.empty() )
    {
        // if headers are provided, add them to the shader
        const char *fragSources[2] = {fragHeaderSource.c_str(), fragmentShaderSource.c_str()};
        glShaderSource(fragmentShader, 2, fragSources, nullptr);
    }
    else
    {
        // if no header provided, the shader is contained in a single file
        const char *fragmentShaderSourcePtr = fragmentShaderSource.c_str();
        glShaderSource(fragmentShader, 1, &fragmentShaderSourcePtr, nullptr);
    }
    glCompileShader(fragmentShader);
    success = 0;
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) 
    {
        errorLog() << "loadShaderProgram(): Fragment shader compilation failed:";
        showShaderInfoLog(fragmentShader);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return 0;
    }


    // Create program object
    GLuint program = glCreateProgram();

    // Attach shaders to the program
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);


    // Link program
    glLinkProgram(program);

    // Check linking status
    success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) 
    {
        errorLog() << "loadShaderProgram(): Linking failed:";
        showProgramInfoLog(program);
        glDeleteProgram(program);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return 0;
    }

    // Clean up
    glDetachShader(program, vertexShader);
    glDetachShader(program, fragmentShader);

    return program;
}



/*!
* \fn buildShadowFBOandTex
* \brief Generate a FBO and attach a texture to its depth output (used for shadow maps generation)
* \param _shadowFBO : pointer to id of FBO to generate
* \param _shadowMapTex : pointer to id of texture to generate 
* \param _texWidth : texture width
* \param _texHeight : texture height
*/
void buildShadowFBOandTex(GLuint *_shadowFBO, GLuint *_shadowMapTex, unsigned int _texWidth, unsigned int _texHeight)
{
    // NOTE: this function takes POINTERS  *_shadowFBO and *_shadowMapTex as input
    // If we had _shadowFBO and _shadowMapTex as parameters, we would get a COPY of these values and 
    // therefore the generated FBO and texture would not be accessible in Main.cpp 

    // generate FBO 
    glGenFramebuffers(1, _shadowFBO);       // we use _shadowFBO instead of &_shadowFBO because _shadowFBO is already a pointer

    // generate texture
    glGenTextures(1, _shadowMapTex);        // we use _shadowMapTex instead of &_shadowMapTex because _shadowMapTex is ALREADY a pointer
    glBindTexture(GL_TEXTURE_2D, *_shadowMapTex);       // we use *_shadowMapTex instead of _shadowMapTex because _shadowMapTex is a pointer and here we want the value of the variable (1)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, _texWidth, _texHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };   // border color set to white to avoid fake shadow outside shadowmap
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor); 

    // bind FBO
    glBindFramebuffer(GL_FRAMEBUFFER, *_shadowFBO);     // see (1)

    // attach texture to the depth output of FBO
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, *_shadowMapTex, 0);       // see (1)

    // we are not going to render any color
    glDrawBuffer(GL_NONE);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        errorLog() << "buildShadowFBOandTex(): Shadow FBO incomplete";
    }

    // Bind default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);  
}



/*!
* \fn buildScreenFBOandTex
* \brief Generate a FBO and attach a texture to its color output (used for variousscreen texture generation)
*        - can use a renderbuffer to handle depth buffering (for 3D geometry rendering) 
         - can initialize texture with null alpha value (for TSD texture generation)
* \param _screenFBO : pointer to id of FBO to generate
* \param _screenTex : pointer to id of texture to generate 
* \param _texWidth : texture width
* \param _texHeight : texture height
* \param _useRenderBuffer : use a renderbuffer or not
* \param _nullAlpha : initialize texture with null alpha or not
*/
void buildScreenFBOandTex(GLuint *_screenFBO, GLuint *_screenTex, unsigned int _texWidth, unsigned int _texHeight, bool _useRenderBuffer, bool _nullAlpha )
{

    // generate FBO 
    glGenFramebuffers(1, _screenFBO);      

    // generate texture
    glGenTextures(1, _screenTex);
    glBindTexture(GL_TEXTURE_2D, *_screenTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F,  _texWidth, _texHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    // bind FBO
    glBindFramebuffer(GL_FRAMEBUFFER, *_screenFBO);

    // attach textures to color output of the FBO
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, *_screenTex, 0);

    if(_nullAlpha)
    {   
        glDrawBuffer(GL_COLOR_ATTACHMENT0); //Only need to do this once.
        // make sure that alphae channel is set to zero everywhere (for gaussian blur mask in TSD)
        GLuint clearColor[4] = {0, 0, 0, 0};
        glClearBufferuiv(GL_COLOR, 0, clearColor);
    }

    if(_useRenderBuffer)
    {
        // create and attach depth buffer (renderbuffer) to handle polygon occlusion properly (for 3D geometry rendering)
        unsigned int rboScreen;
        glGenRenderbuffers(1, &rboScreen);
        glBindRenderbuffer(GL_RENDERBUFFER, rboScreen);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, _texWidth, _texHeight);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboScreen);
    }

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        errorLog() << "buildScreenFBOandTex(): screen FBO incomplete";
    }

    // Bind default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);  
}



/*!
* \fn buildGbuffFBOandTex
* \brief Generate a FBO and attach textures to 2 color outputs (used for G-buffer textures generation)
*        Use a renderbuffer to handle depth buffering
* \param _gFBO : pointer to id of FBO to generate
* \param _gPosition : pointer to id of texture to generate for 1st color output
* \param _gNormal : pointer to id of texture to generate for 2nd color output
* \param _gColor : pointer to id of texture to generate for 3rd color output
* \param _texWidth : texture width
* \param _texHeight : texture height
*/
void buildGbuffFBOandTex(GLuint *_gFBO, GLuint * _gPosition, GLuint * _gNormal, GLuint * _gColor, unsigned int _texWidth, unsigned int _texHeight)
{
    // generate FBO 
    glGenFramebuffers(1, _gFBO);

    // 1st texture (position buffer)
    glGenTextures(1, _gPosition);
    glBindTexture(GL_TEXTURE_2D, *_gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F,  _texWidth, _texHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // 2nd texture (normal buffer)
    glGenTextures(1, _gNormal);
    glBindTexture(GL_TEXTURE_2D, *_gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, _texWidth, _texHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // 3rdd texture (color buffer)
    glGenTextures(1, _gColor);
    glBindTexture(GL_TEXTURE_2D, *_gColor);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, _texWidth, _texHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    // bind FBO
    glBindFramebuffer(GL_FRAMEBUFFER, *_gFBO);

    // attach textures to different color outputs of the FBO
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, *_gPosition, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, *_gNormal, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, *_gColor, 0);

    // handle multiple color attachments
    GLenum attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments);

    // create and attach depth buffer (renderbuffer)
    unsigned int rboGbuff;
    glGenRenderbuffers(1, &rboGbuff);
    glBindRenderbuffer(GL_RENDERBUFFER, rboGbuff);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, _texWidth, _texHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboGbuff);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        errorLog() << "buildGbuffFBOandTex(): G-buffer FBO incomplete";
    }

    // Bind default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);  
}


/*!
* \fn buildTsdFBOandTex
* \brief Generate a FBO and attach texture to its color outputs (TSD texture generation)
*        Initializes texture with
* \param _gFBO : pointer to id of FBO to generate
* \param _gPosition : pointer to id of texture to generate for 1st color output
* \param _gNormal : pointer to id of texture to generate for 2nd color output
* \param _texWidth : texture width
* \param _texHeight : texture height
*/
//void buildTsdFBOandTex(GLuint *_tsdFBO, GLuint *_tsdTex, unsigned int _texWidth, unsigned int _texHeight)
//{
//    // NOTE: this function takes POINTERS  *_shadowFBO and *_shadowMapTex as input
//    // If we had _shadowFBO and _shadowMapTex as parameters, we would get a COPY of these values and 
//    // therefore the generated FBO and texture would not be accessible in Main.cpp 
//
//    // generate shadow map FBO 
//    glGenFramebuffers(1, _tsdFBO);       // we use _shadowFBO instead of &_shadowFBO because _shadowFBO is already a pointer
//
//    // generate shadow map texture
//    glGenTextures(1, _tsdTex);        // we use _shadowMapTex instead of &_shadowMapTex because _shadowMapTex is ALREADY a pointer
//    glBindTexture(GL_TEXTURE_2D, *_tsdTex);       // we use *_shadowMapTex instead of _shadowMapTex because _shadowMapTex is a pointer and here we want the value of the variable (1)
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16, _texWidth, _texHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER); 
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
//
//
//
//    // bind shadow FBO
//    glBindFramebuffer(GL_FRAMEBUFFER, *_tsdFBO);     // see (1)
//
//    // attach shadow texture to the depth output of shadow FBO (depth output)
//    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, *_tsdTex, 0);       // see (1)
//    // Optionnal: make sure that alphae channel is set to zero everywhere (for gaussian blut in TSD)
//    GLuint clearColor[4] = {0, 0, 0, 0};
//    glClearBufferuiv(GL_COLOR, 0, clearColor);
//
//    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
//        errorLog() << "buildShadowFBOandTex(): Shadow FBO incomplete";
//    }
//
//    // Bind default framebuffer
//    glBindFramebuffer(GL_FRAMEBUFFER, 0);  
//}


//void buildSSAOFBOandTex(GLuint *_SSAOFBO, GLuint *_SSAOTex, unsigned int _texWidth, unsigned int _texHeight)
//{
//
//    // generate shadow map FBO 
//    glGenFramebuffers(1, _SSAOFBO);       // we use _screenFBO instead of &_screenFBO because _screenFBO is already a pointer
//
//    // - position color buffer
//    glGenTextures(1, _SSAOTex);
//    glBindTexture(GL_TEXTURE_2D, *_SSAOTex);
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F,  _texWidth, _texHeight, 0, GL_RGBA, GL_FLOAT, NULL);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER); 
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
//    
//
//    // bind shadow FBO
//    glBindFramebuffer(GL_FRAMEBUFFER, *_SSAOFBO);
//    // attach textures to different color outputs of the FBO
//    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, *_SSAOTex, 0);
//
//
//    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
//        errorLog() << "buildScreenFBOandTex(): screen FBO incomplete";
//    }
//
//    // Bind default framebuffer
//    glBindFramebuffer(GL_FRAMEBUFFER, 0);  
//}



/*
 * cf. https://learnopengl.com/Advanced-Lighting/SSAO
 */

float lerp(float a, float b, float f)
{
    return a + f * (b - a);
}  

std::vector<glm::vec3> buildRandKernel()
{
    // generate sample kernel 
    std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between 0.0 and 1.0
    std::default_random_engine generator;
    std::vector<glm::vec3> ssaoKernel;
    //for (unsigned int i = 0; i < 64; ++i)
    //{
    //    glm::vec3 sample( randomFloats(generator) * 2.0 - 1.0, 
    //                      randomFloats(generator) * 2.0 - 1.0, 
    //                      randomFloats(generator) );
    //    sample  = glm::normalize(sample);
    //    sample *= randomFloats(generator);
    //    float scale = (float)i / 64.0; 

    //    // scale samples s.t. they're more aligned to center of kernel
    //    scale = lerp(0.1f, 1.0f, scale * scale);
    //    sample *= scale;
    //    ssaoKernel.push_back(sample);  
    //}

    int i = 0;
    while(i < 64)//for (unsigned int i = 0; i < 64; ++i)
    {
        glm::vec3 sample( randomFloats(generator) * 2.0 - 1.0, 
                          randomFloats(generator) * 2.0 - 1.0, 
                          randomFloats(generator) );
        if( glm::length(sample) <= 1.0)
        {
            sample  = glm::normalize(sample);
            sample *= randomFloats(generator);
            float scale = (float)i / 64.0f; 

            // scale samples s.t. they're more aligned to center of kernel
            scale = lerp(0.1f, 1.0f, scale * scale);
            //sample *= scale;                      // remove artifacts !!!
            ssaoKernel.push_back(sample);  
            i++;
        }
    }

    return ssaoKernel;
}

void buildKernelRot(GLuint *_noiseTex)
{
    std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between 0.0 and 1.0
    std::default_random_engine generator;

    //create a 4x4 array of random rotation vectors oriented around the tangent-space surface normal
    std::vector<glm::vec3> ssaoNoise;
    for (unsigned int i = 0; i < 16; i++)
    {
        glm::vec3 noise( randomFloats(generator) * 2.0 - 1.0, 
                         randomFloats(generator) * 2.0 - 1.0, 
                         0.0f ); 

        ssaoNoise.push_back(noise);
    } 

    glGenTextures(1, _noiseTex);
    glBindTexture(GL_TEXTURE_2D, *_noiseTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);  
}

#endif // UTILS_H