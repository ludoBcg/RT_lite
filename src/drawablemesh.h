/*********************************************************************************************************************
 *
 * drawablemesh.h
 *
 * Buffer manager for mesh rendering
 * 
 * RT_lite
 * Ludovic Blache
 *
 *********************************************************************************************************************/

#ifndef DRAWABLEMESH_H
#define DRAWABLEMESH_H

#define QT_NO_OPENGL_ES_2
#include <GL/glew.h>

#include <lodepng.h>

#include "trimesh.h"

// The attribute locations we will use in the vertex shader
enum AttributeLocation 
{
    POSITION = 0,
    NORMAL = 1,
    COLOR = 2,
    UV = 3,
    TANGENT = 4,
    BITANGENT = 5
};



/*!
* \class DrawableMesh
* \brief Drawable mesh
* Render a TriMesh using a Blinn-Phong shading model and texture mapping
*/
class DrawableMesh
{
    public:

        /*------------------------------------------------------------------------------------------------------------+
        |                                        CONSTRUCTORS / DESTRUCTORS                                           |
        +------------------------------------------------------------------------------------------------------------*/

        /*!
        * \fn DrawableMesh
        * \brief Default constructor of DrawableMesh
        */
        DrawableMesh();


        /*!
        * \fn ~DrawableMesh
        * \brief Destructor of DrawableMesh
        */
        ~DrawableMesh();


        /*------------------------------------------------------------------------------------------------------------+
        |                                              GETTERS/SETTERS                                                |
        +-------------------------------------------------------------------------------------------------------------*/


        /*! \fn setSpeculatPower */
        inline void setSpeculatPower(float _specPow) { m_specPow = _specPow; }

        /*! \fn setAmbientColor */
        inline void setAmbientColor(int _r, int _g, int _b) { m_ambientColor = glm::vec3( (float)_r/255.0f, (float)_g/255.0f, (float)_b/255.0f ); }
        /*! \fn setAmbientColor */
        inline void setDiffuseColor(int _r, int _g, int _b) { m_diffuseColor = glm::vec3( (float)_r/255.0f, (float)_g/255.0f, (float)_b/255.0f ); }
        /*! \fn setAmbientColor */
        inline void setSpecularColor(int _r, int _g, int _b) { m_specularColor = glm::vec3( (float)_r/255.0f, (float)_g/255.0f, (float)_b/255.0f ); }


        /*! \fn setShadowMap */
        inline void setShadowMap(GLuint _shadowMap) { m_shadowMap = _shadowMap; }
        /*! \fn setSSAOKernel */
        inline void setSSAOKernel(std::vector<glm::vec3> _ssaoKernel) { m_ssaoKernel = _ssaoKernel; }
        /*! \fn setNoiseTex */
        inline void setNoiseTex(GLuint _noiseTex) { m_noiseTex = _noiseTex; }



        /*! \fn setAmbientFlag */
        inline void setAmbientFlag(bool _useAmbient) { m_useAmbient = _useAmbient; }
        /*! \fn setDiffuseFlag */
        inline void setDiffuseFlag(bool _useDiffuse) { m_useDiffuse = _useDiffuse; }
        /*! \fn setSpecularFlag */
        inline void setSpecularFlag(bool _useSpecular) { m_useSpecular = _useSpecular; }
        /*! \fn setAlbedoTexFlag */
        inline void setAlbedoTexFlag(bool _useAlbedoTex) 
        { 
            if(!m_uvProvided && _useAlbedoTex)
            if(!m_uvProvided && _useAlbedoTex)
                std::cerr << "[WARNING] DrawableMesh::setAlbedoTexFlag(): No UV coords available" << std::endl;
            m_useAlbedoTex = _useAlbedoTex; 
        }
        /*! \fn setEnvMapReflecFlag */
        inline void setEnvMapReflecFlag(bool _useEnvMapReflec) { m_useEnvMapReflec = _useEnvMapReflec; }
        /*! \fn setEnvMapFlag */
        inline void setEnvMapRefracFlag(bool _useEnvMapRefrac) { m_useEnvMapRefrac = _useEnvMapRefrac; }
        /*! \fn setNormalMapFlag */
        inline void setNormalMapFlag(bool _useNormalMap) 
        {
            if(_useNormalMap)
            {
                if(!m_uvProvided)
                    std::cerr << "[WARNING] DrawableMesh::setNormalMapFlag(): No UV coords available" << std::endl;
                if(!m_tangentProvided)
                    std::cerr << "[WARNING] DrawableMesh::setNormalMapFlag(): No tangent coords available" << std::endl;
                if(!m_bitangentProvided)
                    std::cerr << "[WARNING] DrawableMesh::setNormalMapFlag(): No bitangent coords available" << std::endl;
            }
            m_useNormalMap = _useNormalMap; 
        }
        /*! \fn setPBRFlag */
        inline void setPBRFlag(bool _usePBR) 
        { 
            if(!m_uvProvided && _usePBR)
                std::cerr << "[WARNING] DrawableMesh::setPBRFlag(): No UV coords available" << std::endl;
            m_usePBR = _usePBR; 
        }
        /*! \fn setAmbMapFlag */
        inline void setAmbMapFlag(bool _useAmbMap) 
        { 
            if(!m_uvProvided && _useAmbMap)
                std::cerr << "[WARNING] DrawableMesh::setAmbMapFlag(): No UV coords available" << std::endl;
            m_useAmbMap = _useAmbMap; 
        }
        /*! \fn setShadowMapFlag */
        inline void setShadowMapFlag(bool _useShadowMap) { m_useShadowMap = _useShadowMap; }
        /*! \fn setUseGammaCorrecFlag */
        inline void setUseGammaCorrecFlag(bool _useGammaCorrec) { m_useGammaCorrec = _useGammaCorrec; }
        /*! \fn setLightDirFlag */
        inline void setLightDirFlag(bool _isLightDir) { m_isLightDir = _isLightDir; }
        /*! \fn setSimTransmitFlag */
        inline void setSimTransmitFlag(bool _useSimTransmit) { m_useSimTransmit = _useSimTransmit; }
        /*! \fn setTSDFlag */
        inline void setTSDFlag(bool _useTSD) { m_useTSD = _useTSD; }

        /*! \fn getAmbientFlag */
        inline bool getAmbientFlag() { return m_useAmbient; }
        /*! \fn getDiffuseFlag */
        inline bool getDiffuseFlag() { return m_useDiffuse; }
        /*! \fn getSpecularFlag */
        inline bool getSpecularFlag() { return m_useSpecular; }
        /*! \fn getAlbedoTexFlag */
        inline bool getAlbedoTexFlag() { return m_useAlbedoTex; }
        /*! \fn getEnvMapReflecFlag */
        inline bool getEnvMapReflecFlag() { return m_useEnvMapReflec; }
        /*! \fn getEnvMapRefraccFlag */
        inline bool getEnvMapRefracFlag() { return m_useEnvMapRefrac; }
        /*! \fn getNormalMapFlag */
        inline bool getNormalMapFlag() { return m_useNormalMap; }
        /*! \fn getPBRFlag */
        inline bool getPBRFlag() { return m_usePBR; }
        /*! \fn getAmbMapFlag */
        inline bool getAmbMapFlag() { return m_useAmbMap; }
        /*! \fn getShadowMapFlag */
        inline bool getShadowMapFlag() { return m_useShadowMap; }
        /*! \fn getUseGammaCorrecFlag */
        inline bool getUseGammaCorrecFlag() { return m_useGammaCorrec; }
        /*! \fn getLightDirFlag */
        inline bool getLightDirFlag() { return m_isLightDir; }
        /*! \fn getSimTransmitFlag */
        inline bool getSimTransmitFlag() { return m_useSimTransmit; }
        /*! \fn getTSDFlag */
        inline bool getTSDFlag() { return m_useTSD; }


        /*------------------------------------------------------------------------------------------------------------+
        |                                               OTHER METHODS                                                 |
        +-------------------------------------------------------------------------------------------------------------*/

        /*!
        * \fn createMeshVAO
        * \brief Create mesh VAO and VBOs, calling fillVAO().
        * \param _triMesh : Mesh to update mesh VAO and VBOs from
        */
        void createMeshVAO(TriMesh* _triMesh);

        /*!
        * \fn createMeshVAO
        * \brief Create quad VAO and VBOs (for floor or screen quad).
        * \param _type : defines if the quad to build is a screen quad (=1) of a floor quad (=2)
        * \param _y : Y coords of quad to build (for floor quad only) 
        * \param _centerCoords : center of the scene (for floor quad only) 
        * \param _radScene : Radius of the scene (for floor quad only) 
        */
        void createQuadVAO(int _type, float _y = 0.0f, glm::vec3 _centerCoords = glm::vec3(0.0f), float _radScene = 0.0f);

        /*!
        * \fn createCubeVAO
        * \brief Create cube VAO and VBOs (for skybox).
        * \param _centerCoords : center of the scene (for floor quad only) 
        * \param _radScene : Radius of the scene (for floor quad only) 
        */
        void createCubeVAO(glm::vec3 _centerCoords, float _radScene);

        /*!
        * \fn updateMeshVAO
        * \brief Update mesh VAO and VBOs, calling fillVAO().
        * \param _triMesh : Mesh to update mesh VAO and VBOs from
        */
        void updateMeshVAO(TriMesh* _triMesh);

        /*!
        * \fn createVAO
        * \brief Fill mesh VAO and VBOs
        * \param _triMesh : Mesh to fill mesh VAO and VBOs from
        * \param _create : true to init the VBOs and VAO, false to update them 
        */
        void fillVAO(TriMesh* _triMesh, bool _create);

        /*!
        * \fn draw
        * \brief Draw the content of the mesh VAO
        * \param _program : shader program
        * \param _modelMat : model matrix
        * \param _viewMat :camera view matrix
        * \param _projMat :camera projection matrix
        * \param _lightPos : 3D coords of light position
        * \param _lightCol : RGB color of the light
        * \param _lightMat : Projection-View matric of the light camera
        */
        void draw(GLuint _program, glm::mat4 _modelMat, glm::mat4 _viewMat, glm::mat4 _projMat, glm::vec3 _lightPos, glm::vec3 _camPos, glm::vec3 _lightCol, glm::mat4 _lightMat, float _distLightMax);

        /*!
        * \fn draw
        * \brief Draw a sky box with cube map for environment mapping
        * \param _program : shader program
        * \param _modelMat : model matrix
        * \param _viewMat :camera view matrix
        * \param _projMat :camera projection matrix
        */
        void drawSkyBox(GLuint _program, glm::mat4 _viewMat, glm::mat4 _projMat);

        /*!
        * \fn drawScreenQuad
        * \brief Draw the screen quad, mapped with a given texture
        * \param _program : shader program
        * \param _tex : texture to map on the screen quad 
        * \param _isBlurOn : true to activate Gaussian blur
        * \param _isGaussH : true for horizontal blur, false for vertical blur
        * \param _filterWidth : Guassian fildter width
        */
        void drawScreenQuad(GLuint _program, GLuint _tex, bool _isBlurOn, bool _isGaussH = true, int _filterWidth = 0 );

        /*!
        * \fn drawScreenQuadSSAO
        * \brief Draw the screen quad, mapped with G-buffer (position and normal textures), and compute screen-space ambient occlusion
        * \param _program : shader program
        * \param _projMat :camera projection matrix
        * \param _posTex : G-buffer position texture
        * \param _normalTex : G-buffer normal texture
        * \param _radius : neighborhood radius for SSAO computation
        * \param _screenWidth, _screenHeight : window dimensions
        */
        void drawScreenQuadSSAO(GLuint _program, glm::mat4 _projMat, GLuint _posTex, GLuint _normalTex, float _radius, float _screenWidth, float _screenHeight);

        /*!
        * \fn drawScreenQuadSSLR
        * \brief Draw the screen quad, mapped with G-buffer (position and normal textures), and compute screen-space light reflection
        * \param _program : shader program
        * \param _modelMat : model matrix
        * \param _viewMat :camera view matrix
        * \param _projMat :camera projection matrix
        * \param _posTex : G-buffer position texture
        * \param _normalTex : G-buffer normal texture
        * \param _screenTex : screen-space scene rendering texture
        * \param _radius : neighborhood radius for SSLR computation
        * \param _screenWidth, _screenHeight : window dimensions
        */
        void drawScreenQuadSSLR(GLuint _program, glm::mat4 _modelMat, glm::mat4 _viewMat, glm::mat4 _projMat, GLuint _posTex, GLuint _normalTex, GLuint _screenTex, float _radius, float _screenWidth, float _screenHeight);

        /*!
        * \fn drawScreenQuadFinal
        * \brief Draw screen-quad, compositing of scene rendering with screen-space occlusion / reflection. 
        * \param _program : shader program
        * \param _modelMat : model matrix
        * \param _viewMat :camera view matrix
        * \param _projMat :camera projection matrix
        * \param _ssaotex : SSAO or SSLR texture
        * \param _screenTex : screen-space scene rendering texture
        * \param _occType : 1 for SSAO, 2 for SSLR
        */
        void drawScreenQuadFinal(GLuint _program, glm::mat4 _modelMat, glm::mat4 _viewMat, glm::mat4 _projMat, GLuint _ssaotex, GLuint _screenTex, int _occType );

        /*!
        * \fn drawTex
        * \brief Draw a mesh with a texture mapped on it. Used for TSD. 
        * \param _program : shader program
        * \param _modelMat : model matrix
        * \param _viewMat :camera view matrix
        * \param _projMat :camera projection matrix
        * \param _tex : texture to map
        */
        void drawTex(GLuint _program, glm::mat4 _modelMat, glm::mat4 _viewMat, glm::mat4 _projMat, GLuint _tex );

        /*!
        * \fn drawShadow
        * \brief Draw the content of the mesh VAO with a unifornm color (used for shadow map generation)
        * \param _program : shader program
        * \param _lvp : light view-projection matrix
        */
        void drawShadow(GLuint _program, glm::mat4 _lvp);

        /*!
        * \fn drawGbuffer
        * \brief Draw the content of the mesh VAO for G-buffer generation
        * \param _program : shader program
        * \param _modelMat : model matrix
        * \param _viewMat :camera view matrix
        * \param _projMat :camera projection matrix
        * \param _isFloor : true if the mesh to draw is the floor quad
        */
        void drawGbuffer(GLuint _program, glm::mat4 _modelMat, glm::mat4 _viewMat, glm::mat4 _projMat, bool _isFloor);

        /*!
        * \fn loadAlbedoTex
        * \brief load the albedo texture from a file
        * \param _filename : name of texture image
        */
        inline void loadAlbedoTex(const std::string& _filename) { m_albedoTex = load2DTexture(_filename, true); }
        /*!
        * \fn loadNormalMap
        * \brief load the normal map texture from a file
        * \param _filename : name of texture image
        */
        inline void loadNormalMap(const std::string& _filename) { m_normalMap = load2DTexture(_filename, true); }
        /*!
        * \fn loadMetalMap
        * \brief load the metal map (PBR) texture from a file
        * \param _filename : name of texture image
        */
        inline void loadMetalMap(const std::string& _filename) { m_metalMap = load2DTexture(_filename, true); }
        /*!
        * \fn loadGlossMap
        * \brief load the gloss mao (PBR) texture from a file
        * \param _filename : name of texture image
        */
        inline void loadGlossMap(const std::string& _filename) { m_glossMap = load2DTexture(_filename, true); }
        /*!
        * \fn loadAmbientMap
        * \brief load the ambient map texture from a file
        * \param _filename : name of texture image
        */
        inline void loadAmbientMap(const std::string& _filename) { m_ambientMap = load2DTexture(_filename, true); }
        /*!
        * \fn loadCubeMap
        * \brief load a set of cube maps (for environment mapping) from a directory
        * \param _dirname : directory of the cube maps
        */
        inline void loadCubeMap(const std::string& _dirname) { m_cubeMap = loadCubemap(_dirname); }

        /*! 
        * \fn toggleShadedRenderFlag 
        * \brief reverse shaded rendering flag
        */
        inline void toggleShadedRenderFlag() { m_shadedRenderOn = !m_shadedRenderOn; }


    protected:

        /*------------------------------------------------------------------------------------------------------------+
        |                                                ATTRIBUTES                                                   |
        +-------------------------------------------------------------------------------------------------------------*/

        GLuint m_meshVAO;           /*!< mesh VAO */
        GLuint m_defaultVAO;        /*!< default VAO */

        GLuint m_vertexVBO;         /*!< name of vertex 3D coords VBO */
        GLuint m_normalVBO;         /*!< name of normal vector VBO */
        GLuint m_colorVBO;          /*!< name of rgb color VBO */
        GLuint m_tangentVBO;        /*!< name of tangent vector VBO */
        GLuint m_bitangentVBO;      /*!< name of bitangent vector VBO */
        GLuint m_uvVBO;             /*!< name of UV coords VBO */
        GLuint m_indexVBO;          /*!< name of index VBO */

        int m_numVertices;          /*!< number of vertices in the VBOs */
        int m_numIndices;           /*!< number of indices in the index VBO */

        GLuint m_albedoTex;         /*!< index of albedo map texture */
        GLuint m_normalMap;         /*!< index of normal map texture */
        GLuint m_metalMap;          /*!< index of metal map texture */
        GLuint m_glossMap;          /*!< index of gloss map texture */
        GLuint m_ambientMap;        /*!< index of ambient map texture */
        GLuint m_cubeMap;           /*!< index of cube map texture */
        GLuint m_shadowMap;         /*!< index of shadow  map texture */
        GLuint m_noiseTex;          /*!< index of noise texture */
        std::vector<glm::vec3> m_ssaoKernel;

        float m_specPow;            /*!< specular power */

        glm::vec3 m_ambientColor;   /*!< ambient color */
        glm::vec3 m_diffuseColor;   /*!< diffuse color */
        glm::vec3 m_specularColor;  /*!< specular color */


        bool m_useAmbient;          /*!< flag to use ambient shading or not */
        bool m_useDiffuse;          /*!< flag to use diffuse shading or not */
        bool m_useSpecular;         /*!< flag to use specular shading or not */
        bool m_useAlbedoTex;        /*!< flag to use texture mapping or not (i.e. albedo map) */
        bool m_useNormalMap;        /*!< flag to use normal mapping or not */
        bool m_usePBR;              /*!< flag to use PBR textures or not */
        bool m_useAmbMap;           /*!< flag to use ambient map or not */
        bool m_useEnvMapReflec;     /*!< flag to use environment mapping reflection or not */
        bool m_useEnvMapRefrac;     /*!< flag to use environment mapping refraction or not */
        bool m_useShadowMap;        /*!< flag to use shadow mapping or not */
        bool m_useGammaCorrec;      /*!< flag to apply gamma correction or not */

        bool m_vertexProvided;      /*!< flag to indicate if vertex coords are available or not */
        bool m_normalProvided;      /*!< flag to indicate if normals are available or not */
        bool m_colorProvided;       /*!< flag to indicate if colors are available or not */
        bool m_tangentProvided;     /*!< flag to indicate if tangents are available or not */
        bool m_bitangentProvided;   /*!< flag to indicate if bitangnets are available or not */
        bool m_uvProvided;          /*!< flag to indicate if uv coords are available or not */
        bool m_indexProvided;       /*!< flag to indicate if indices are available or not */

        bool m_shadedRenderOn;      /*!< flag to indicate if shaded rendering is on */

        bool m_useSimTransmit;      /*!< flag to indicate if simulate light transmission is on */
        bool m_useTSD;              /*!< flag to indicate if texture space diffusion is on */

        bool m_isLightDir;          /*!< flag to indicate if the light source is directional (true) or point (false) */

        /*------------------------------------------------------------------------------------------------------------+
        |                                               OTHER METHODS                                                 |
        +-------------------------------------------------------------------------------------------------------------*/

        /*!
        * \fn load2DTexture
        * \brief load a 2D image to be used as texture
        * \param _filename : name of texture image
        * \param _repeat : repeat (true) or clamptoedge (false)
        */
        GLuint load2DTexture(const std::string& _filename, bool _repeat = false);

        /*!
        * \fn loadCubemap
        * \brief Load cubemap texture and let OpenGL generate a mipmap chain
        * \param _dirname : directory of cube map images
        */
        GLuint loadCubemap(const std::string& _dirname);

};
#endif // DRAWABLEMESH_H