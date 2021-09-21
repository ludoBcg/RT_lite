/*********************************************************************************************************************
 *
 * drawablemesh.cpp
 * 
 * RT_lite
 * Ludovic Blache
 *
 *********************************************************************************************************************/

#include "drawablemesh.h"


DrawableMesh::DrawableMesh()
{
    /* TODO */
    //m_defaultVAO = 0;
    m_ambientColor = glm::vec3(0.0f, 0.0f, 0.1f);
    m_diffuseColor = glm::vec3(0.95f, 0.5f, 0.25f);
    m_specularColor = glm::vec3(0.0f, 0.8f, 0.0f);

    m_specPow = 128.0f;

    setAmbientFlag(true);
    setDiffuseFlag(true);
    setSpecularFlag(true);
    setAlbedoTexFlag(false);
    setEnvMapReflecFlag(false);
    setEnvMapRefracFlag(false);
    setNormalMapFlag(false);
    setPBRFlag(false);
    setAmbMapFlag(false);
    setShadowMapFlag(false);
    setUseGammaCorrecFlag(true);
    setLightDirFlag(false);
    setSimTransmitFlag(false);
    setTSDFlag(false);

    m_vertexProvided = false;
    m_normalProvided = false;
    m_colorProvided = false;
    m_tangentProvided = false;
    m_bitangentProvided = false;
    m_uvProvided = false;
    m_indexProvided = false;

    m_shadedRenderOn = true;

}


DrawableMesh::~DrawableMesh()
{
    glDeleteBuffers(1, &(m_vertexVBO));
    glDeleteBuffers(1, &(m_normalVBO));
    glDeleteBuffers(1, &(m_colorVBO));
    glDeleteBuffers(1, &(m_tangentVBO));
    glDeleteBuffers(1, &(m_bitangentVBO));
    glDeleteBuffers(1, &(m_uvVBO));
    glDeleteBuffers(1, &(m_indexVBO));
    glDeleteVertexArrays(1, &(m_meshVAO));
}


void DrawableMesh::createMeshVAO(TriMesh* _triMesh)
{
    // read vertices from mesh object and fill in VAO
    fillVAO(_triMesh, true);
}


void DrawableMesh::createQuadVAO(int _type, float _y, glm::vec3 _centerCoords, float _radScene)
{
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;

    if( _type == 1)
    {
        // SCREEN
        if(_radScene != 0.0f)
            std::cerr << "[WARNING] DrawableMesh::createQuadVAO(): Create screen quad: radius of the scene is not null" << std::endl;

        // generate a quad in front of the camera
        vertices = { glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(1.0f, -1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(-1.0f, 1.0f, 0.0f) };
        normals = { glm::vec3(1.0f, 1.0f,  1.0f), glm::vec3(1.0f, 1.0f,  1.0f), glm::vec3(1.0f, 1.0f,  1.0f), glm::vec3(1.0f, 1.0f,  1.0f) };
    }
    else
    {
        // FLOOR
        if(_radScene == 0.0f)
            std::cerr << "[WARNING] DrawableMesh::createQuadVAO(): Create floor quad: radius of the scene is null" << std::endl;

        // generate a quad on the XZ axis, centered on the scene center.
        // Dimension adapted to the size of the scene.
        float halfEdgeSize = _radScene * 2.0f;
        float xMin = _centerCoords.x - halfEdgeSize;
        float zMin = _centerCoords.z - halfEdgeSize;
        float xMax = _centerCoords.x + halfEdgeSize;
        float zMax = _centerCoords.z + halfEdgeSize;
        // small offset on the y coord so the floor is below the mesh
        float y = _y - _radScene*0.1f;
        vertices = { glm::vec3(xMin, y, zMax), glm::vec3(xMax, y, zMax), glm::vec3(xMax, y, zMin), glm::vec3(xMin, y, zMin) };
        normals = { glm::vec3(0.0f, 1.0f,  0.0f), glm::vec3(0.0f, 1.0f,  0.0f), glm::vec3(0.0f, 1.0f,  0.0f), glm::vec3(0.0f, 1.0f,  0.0f) };
    }

    // add UV coords so we can map textures on the creen quad
    std::vector<glm::vec2> texcoords{ glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec2(1.0f, 1.0f), glm::vec2(0.0f, 1.0f) };
    std::vector<uint32_t> indices {0, 1, 2, 2, 3, 0 };


    // Generates and populates a VBO for vertex coords
    glGenBuffers(1, &(m_vertexVBO));
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexVBO);
    size_t verticesNBytes = vertices.size() * sizeof(vertices[0]);
    glBufferData(GL_ARRAY_BUFFER, verticesNBytes, vertices.data(), GL_STATIC_DRAW);

    // Generates and populates a VBO for vertex normals
    glGenBuffers(1, &(m_normalVBO));
    glBindBuffer(GL_ARRAY_BUFFER, m_normalVBO);
    size_t normalsNBytes = normals.size() * sizeof(normals[0]);
    glBufferData(GL_ARRAY_BUFFER, normalsNBytes, normals.data(), GL_STATIC_DRAW);

    // Generates and populates a VBO for the element indices
    glGenBuffers(1, &(m_indexVBO));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexVBO);
    auto indicesNBytes = indices.size() * sizeof(indices[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesNBytes, indices.data(), GL_STATIC_DRAW);

    // Generates and populates a VBO for UV coords
    glGenBuffers(1, &(m_uvVBO));
    glBindBuffer(GL_ARRAY_BUFFER, m_uvVBO);
    size_t texcoordsNBytes = texcoords.size() * sizeof(texcoords[0]);
    glBufferData(GL_ARRAY_BUFFER, texcoordsNBytes, texcoords.data(), GL_STATIC_DRAW);


    // Creates a vertex array object (VAO) for drawing the mesh
    glGenVertexArrays(1, &(m_meshVAO));
    glBindVertexArray(m_meshVAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_vertexVBO);
    glEnableVertexAttribArray(POSITION);
    glVertexAttribPointer(POSITION, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindBuffer(GL_ARRAY_BUFFER, m_normalVBO);
    glEnableVertexAttribArray(NORMAL);
    glVertexAttribPointer(NORMAL, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindBuffer(GL_ARRAY_BUFFER, m_uvVBO);
    glEnableVertexAttribArray(UV);
    glVertexAttribPointer(UV, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexVBO);
    glBindVertexArray(m_defaultVAO); // unbinds the VAO

    // Additional information required by draw calls
    m_numVertices = (int)vertices.size();
    m_numIndices = (int)indices.size();

    // Clear temporary vectors
    vertices.clear();
    normals.clear();
    indices.clear();
}


void DrawableMesh::createCubeVAO(glm::vec3 _centerCoords, float _radScene)
{
    glm::vec3 maxPt = _centerCoords + glm::vec3(_radScene * 13.0f);
    glm::vec3 minPt = _centerCoords - glm::vec3(_radScene * 13.0f);

    std::vector<glm::vec3> vertices = { glm::vec3(-1.0f,  1.0f, -1.0f), glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3( 1.0f, -1.0f, -1.0f), glm::vec3( 1.0f,  1.0f, -1.0f), 
                                        glm::vec3(-1.0f,  1.0f,  1.0f), glm::vec3(-1.0f, -1.0f,  1.0f), glm::vec3( 1.0f, -1.0f,  1.0f), glm::vec3( 1.0f,  1.0f,  1.0f) };


    std::vector<uint32_t> indices { 0, 1, 2, 2, 3, 0, // back face
                                    7, 6, 5, 5, 4, 7, // front face
                                    4, 5, 1, 1, 0, 4, // left face
                                    3, 2, 6, 6, 7, 3, // right face
                                    4, 0, 3, 3, 7, 4, // top face
                                    1, 5, 6, 6, 2, 1  // bottom face
                                  };

    // Generates and populates a VBO for vertex coords
    glGenBuffers(1, &(m_vertexVBO));
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexVBO);
    size_t verticesNBytes = vertices.size() * sizeof(vertices[0]);
    glBufferData(GL_ARRAY_BUFFER, verticesNBytes, vertices.data(), GL_STATIC_DRAW);

    // Generates and populates a VBO for the element indices
    glGenBuffers(1, &(m_indexVBO));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexVBO);
    auto indicesNBytes = indices.size() * sizeof(indices[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesNBytes, indices.data(), GL_STATIC_DRAW);


    // Creates a vertex array object (VAO) for drawing the mesh
    glGenVertexArrays(1, &(m_meshVAO));
    glBindVertexArray(m_meshVAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_vertexVBO);
    glEnableVertexAttribArray(POSITION);
    glVertexAttribPointer(POSITION, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexVBO);
    glBindVertexArray(m_defaultVAO); // unbinds the VAO

    // Additional information required by draw calls
    m_numVertices = vertices.size();
    m_numIndices = indices.size();

    // Clear temporary vectors
    vertices.clear();
    indices.clear();
}


void DrawableMesh::updateMeshVAO(TriMesh* _triMesh)
{
    // read vertices from mesh file and fill in VAO
    fillVAO(_triMesh, false);
}


void DrawableMesh::fillVAO(TriMesh* _triMesh, bool _create)
{
    // mandatory data
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<uint32_t> indices;      // !! uint32_t !!

    // optional data
    std::vector<glm::vec3> colors;
    std::vector<glm::vec2> texcoords;   // !! vec2 !!
    std::vector<glm::vec3> tangents;
    std::vector<glm::vec3> bitangents;

    _triMesh->getVertices(vertices);
    _triMesh->getNormals(normals);
    _triMesh->getIndices(indices);

    _triMesh->getColors(colors);
    _triMesh->getTexCoords(texcoords);
    _triMesh->getTangents(tangents);
    _triMesh->getBitangents(bitangents);

    // update flags according to data provided
    vertices.size() ?  m_vertexProvided = true :  m_vertexProvided = false;
    normals.size() ?  m_normalProvided = true :  m_normalProvided = false;
    indices.size() ?  m_indexProvided = true :  m_indexProvided = false;
    colors.size() ?  m_colorProvided = true :  m_colorProvided = false;
    texcoords.size() ?  m_uvProvided = true :  m_uvProvided = false;
    tangents.size() ?  m_tangentProvided = true :  m_tangentProvided = false;
    bitangents.size() ?  m_bitangentProvided = true :  m_bitangentProvided = false;

    if(!m_vertexProvided)
        std::cerr << "[WARNING] DrawableMesh::createVAO(): No vertex provided" << std::endl;
    if(!m_normalProvided)
        std::cerr << "[WARNING] DrawableMesh::createVAO(): No normal provided" << std::endl;
    if(!m_indexProvided)
        std::cerr << "[WARNING] DrawableMesh::createVAO(): No index provided" << std::endl;

    // Generates and populates a VBO for vertex coords
    if(_create)
        glGenBuffers(1, &(m_vertexVBO));
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexVBO);
    size_t verticesNBytes = vertices.size() * sizeof(vertices[0]);
    glBufferData(GL_ARRAY_BUFFER, verticesNBytes, vertices.data(), GL_STATIC_DRAW);

    // Generates and populates a VBO for vertex normals
    if(_create)
        glGenBuffers(1, &(m_normalVBO));
    glBindBuffer(GL_ARRAY_BUFFER, m_normalVBO);
    size_t normalsNBytes = normals.size() * sizeof(normals[0]);
    glBufferData(GL_ARRAY_BUFFER, normalsNBytes, normals.data(), GL_STATIC_DRAW);

    // Generates and populates a VBO for the element indices
    if(_create)
        glGenBuffers(1, &(m_indexVBO));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexVBO);
    auto indicesNBytes = indices.size() * sizeof(indices[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesNBytes, indices.data(), GL_STATIC_DRAW);


    // Generates and populates a VBO for vertex colors
    if(_create)
        glGenBuffers(1, &(m_colorVBO));
    glBindBuffer(GL_ARRAY_BUFFER, m_colorVBO);
    if(m_colorProvided)
    {
        size_t colorsNBytes = colors.size() * sizeof(colors[0]);
        glBufferData(GL_ARRAY_BUFFER, colorsNBytes, colors.data(), GL_STATIC_DRAW);
    }
    else
    {
        size_t colorsNBytes = 1.0f * sizeof(colors[0]);
        glBufferData(GL_ARRAY_BUFFER, colorsNBytes, nullptr, GL_STATIC_DRAW);
    }


    // Generates and populates a VBO for UV coords
    if(_create)
        glGenBuffers(1, &(m_uvVBO));
    glBindBuffer(GL_ARRAY_BUFFER, m_uvVBO);
    if(m_uvProvided) 
    {
        size_t texcoordsNBytes = texcoords.size() * sizeof(texcoords[0]);
        glBufferData(GL_ARRAY_BUFFER, texcoordsNBytes, texcoords.data(), GL_STATIC_DRAW);
    }
    else
    {
        size_t texcoordsNBytes = 1.0f * sizeof(texcoords[0]);
        glBufferData(GL_ARRAY_BUFFER, texcoordsNBytes, nullptr, GL_STATIC_DRAW);
    }

    // Generates and populates a VBO for tangent vectors
    if(_create)
        glGenBuffers(1, &(m_tangentVBO));
    glBindBuffer(GL_ARRAY_BUFFER, m_tangentVBO);
    if(m_tangentProvided) 
    {
        size_t tangentsNBytes = tangents.size() * sizeof(tangents[0]);
        glBufferData(GL_ARRAY_BUFFER, tangentsNBytes, tangents.data(), GL_STATIC_DRAW);
    }
    else
    {
        size_t tangentsNBytes = 1.0f * sizeof(tangents[0]);
        glBufferData(GL_ARRAY_BUFFER, tangentsNBytes, nullptr, GL_STATIC_DRAW);
    }

    // Generates and populates a VBO for bitangent vectors
    if(_create)
        glGenBuffers(1, &(m_bitangentVBO));
    glBindBuffer(GL_ARRAY_BUFFER, m_bitangentVBO);
    if(m_bitangentProvided) 
    {
        size_t bitangentsNBytes = bitangents.size() * sizeof(bitangents[0]);
        glBufferData(GL_ARRAY_BUFFER, bitangentsNBytes, bitangents.data(), GL_STATIC_DRAW);
    }
    else
    {
        size_t bitangentsNBytes = 1.0f * sizeof(bitangents[0]);
        glBufferData(GL_ARRAY_BUFFER, bitangentsNBytes, nullptr, GL_STATIC_DRAW);
    }

    // Creates a vertex array object (VAO) for drawing the mesh
    if(_create)
        glGenVertexArrays(1, &(m_meshVAO));
    glBindVertexArray(m_meshVAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_vertexVBO);
    glEnableVertexAttribArray(POSITION);
    glVertexAttribPointer(POSITION, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindBuffer(GL_ARRAY_BUFFER, m_normalVBO);
    glEnableVertexAttribArray(NORMAL);
    glVertexAttribPointer(NORMAL, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindBuffer(GL_ARRAY_BUFFER, m_colorVBO);
    glEnableVertexAttribArray(COLOR);
    glVertexAttribPointer(COLOR, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindBuffer(GL_ARRAY_BUFFER, m_uvVBO);
    glEnableVertexAttribArray(UV);
    glVertexAttribPointer(UV, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindBuffer(GL_ARRAY_BUFFER, m_tangentVBO);
    glEnableVertexAttribArray(TANGENT);
    glVertexAttribPointer(TANGENT, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindBuffer(GL_ARRAY_BUFFER, m_bitangentVBO);
    glEnableVertexAttribArray(BITANGENT);
    glVertexAttribPointer(BITANGENT, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexVBO);
    glBindVertexArray(m_defaultVAO); // unbinds the VAO

    // Additional information required by draw calls
    m_numVertices = (int)vertices.size();
    m_numIndices = (int)indices.size();

    // Clear temporary vectors
    vertices.clear();
    normals.clear();
    indices.clear();
    colors.clear();
    texcoords.clear();
    tangents.clear();
    bitangents.clear();
}


void DrawableMesh::draw(GLuint _program, glm::mat4 _modelMat, glm::mat4 _viewMat, glm::mat4 _projMat, glm::vec3 _lightPos,  glm::vec3 _camPos,  glm::vec3 _lightCol, glm::mat4 _lightMat, float _distLightMax)
{

    if(m_shadedRenderOn)
    {
        // Activate program
        glUseProgram(_program);

        // Bind textures
        if(m_useAlbedoTex)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, m_albedoTex);
        }
        if(m_useNormalMap)
        {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, m_normalMap);
        }
        if(m_usePBR)
        {
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, m_metalMap);
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, m_glossMap);
        }
        if(m_useAmbMap)
        {
            glActiveTexture(GL_TEXTURE4);
            glBindTexture(GL_TEXTURE_2D, m_ambientMap);
        }
        if(m_useEnvMapReflec || m_useEnvMapRefrac)
        {
            glActiveTexture(GL_TEXTURE5);
            glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubeMap);
        }
        if(m_useShadowMap || m_useSimTransmit)
        {
            glActiveTexture(GL_TEXTURE6);
            glBindTexture(GL_TEXTURE_2D, m_shadowMap);
        }

        // ...


        // Pass uniforms
        glUniformMatrix4fv(glGetUniformLocation(_program, "u_matM"), 1, GL_FALSE, &_modelMat[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(_program, "u_matV"), 1, GL_FALSE, &_viewMat[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(_program, "u_matP"), 1, GL_FALSE, &_projMat[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(_program, "u_matPV_light"), 1, GL_FALSE, &_lightMat[0][0]);
        glUniform3fv(glGetUniformLocation(_program, "u_lightPos"), 1, &_lightPos[0]);
        glUniform3fv(glGetUniformLocation(_program, "u_camPos"), 1, &_camPos[0]);

        glUniform3fv(glGetUniformLocation(_program, "u_lightColor"), 1, &_lightCol[0]);

        glUniform3fv(glGetUniformLocation(_program, "u_ambientColor"), 1, &m_ambientColor[0]);
        glUniform3fv(glGetUniformLocation(_program, "u_diffuseColor"), 1, &m_diffuseColor[0]);
        glUniform3fv(glGetUniformLocation(_program, "u_specularColor"), 1, &m_specularColor[0]);
        glUniform1f(glGetUniformLocation(_program, "u_specularPower"), m_specPow);
 
        glUniform1i(glGetUniformLocation(_program, "u_albedoTex"), 0);
        glUniform1i(glGetUniformLocation(_program, "u_normalMap"), 1);
        glUniform1i(glGetUniformLocation(_program, "u_metalMap"), 2);
        glUniform1i(glGetUniformLocation(_program, "u_glossMap"), 3);
        glUniform1i(glGetUniformLocation(_program, "u_ambientMap"), 4);
        glUniform1i(glGetUniformLocation(_program, "u_cubemap"), 5);
        glUniform1i(glGetUniformLocation(_program, "u_shadowMap"), 6);
        glUniform1f(glGetUniformLocation(_program, "u_distLightMax"), _distLightMax);



    

        if(m_useAmbient)
            glUniform1i(glGetUniformLocation(_program, "u_useAmbient"), 1);
        else
            glUniform1i(glGetUniformLocation(_program, "u_useAmbient"), 0);
        if(m_useDiffuse)
            glUniform1i(glGetUniformLocation(_program, "u_useDiffuse"), 1);
        else
            glUniform1i(glGetUniformLocation(_program, "u_useDiffuse"), 0);
        if(m_useSpecular)
            glUniform1i(glGetUniformLocation(_program, "u_useSpecular"), 1);
        else
            glUniform1i(glGetUniformLocation(_program, "u_useSpecular"), 0);

        if(m_useAlbedoTex)
            glUniform1i(glGetUniformLocation(_program, "u_useAlbedoTex"), 1);
        else
            glUniform1i(glGetUniformLocation(_program, "u_useAlbedoTex"), 0);
        if(m_useNormalMap)
            glUniform1i(glGetUniformLocation(_program, "u_useNormalMap"), 1);
        else
            glUniform1i(glGetUniformLocation(_program, "u_useNormalMap"), 0);
        if(m_usePBR)
            glUniform1i(glGetUniformLocation(_program, "u_usePBR"), 1);
        else
            glUniform1i(glGetUniformLocation(_program, "u_usePBR"), 0);
        if(m_useAmbMap)
            glUniform1i(glGetUniformLocation(_program, "u_useAmbMap"), 1);
        else
            glUniform1i(glGetUniformLocation(_program, "u_useAmbMap"), 0);
        if(m_useEnvMapReflec)
            glUniform1i(glGetUniformLocation(_program, "u_useEnvMapReflec"), 1);
        else
            glUniform1i(glGetUniformLocation(_program, "u_useEnvMapReflec"), 0);
        if(m_useEnvMapRefrac)
            glUniform1i(glGetUniformLocation(_program, "u_useEnvMapRefrac"), 1);
        else
            glUniform1i(glGetUniformLocation(_program, "u_useEnvMapRefrac"), 0);
        if(m_useShadowMap)
            glUniform1i(glGetUniformLocation(_program, "u_useShadowMap"), 1);
        else
            glUniform1i(glGetUniformLocation(_program, "u_useShadowMap"), 0);

        if(m_useGammaCorrec)
            glUniform1i(glGetUniformLocation(_program, "u_useGammaCorrec"), 1);
        else
            glUniform1i(glGetUniformLocation(_program, "u_useGammaCorrec"), 0);

        if(m_isLightDir)
            glUniform1i(glGetUniformLocation(_program, "u_isLightDir"), 1);
        else
            glUniform1i(glGetUniformLocation(_program, "u_isLightDir"), 0);

        if(m_useSimTransmit)
            glUniform1i(glGetUniformLocation(_program, "u_useSimTransmit"), 1);
        else
            glUniform1i(glGetUniformLocation(_program, "u_useSimTransmit"), 0);

        if(m_useTSD)
            glUniform1i(glGetUniformLocation(_program, "u_useTSD"), 1);
        else
            glUniform1i(glGetUniformLocation(_program, "u_useTSD"), 0);
        // ...

        // Draw!
        glBindVertexArray(m_meshVAO);                       // bind the VAO
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexVBO);  // do not forget to bind the index buffer AFTER !

        glDrawElements(GL_TRIANGLES, m_numIndices, GL_UNSIGNED_INT, 0);

        glBindVertexArray(m_defaultVAO);

    }

    glUseProgram(0);
}


void DrawableMesh::drawSkyBox(GLuint _program, glm::mat4 _viewMat, glm::mat4 _projMat)
{


        // Activate program
        glUseProgram(_program);

        //glDepthMask(GL_FALSE);
        glDepthFunc(GL_LEQUAL);
       
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubeMap);
        

        // Pass uniforms
        glUniformMatrix4fv(glGetUniformLocation(_program, "u_view"), 1, GL_FALSE, &_viewMat[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(_program, "u_projection"), 1, GL_FALSE, &_projMat[0][0]);
        glUniform1i(glGetUniformLocation(_program, "u_cubemap"), 0);

        // Draw!
        glBindVertexArray(m_meshVAO);                       // bind the VAO
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexVBO);  // do not forget to bind the index buffer AFTER !

        glDrawElements(GL_TRIANGLES, m_numIndices, GL_UNSIGNED_INT, 0);

        

        glBindVertexArray(m_defaultVAO);

        //glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);

        glUseProgram(0);
}



void DrawableMesh::drawScreenQuad(GLuint _program, GLuint _tex, bool _isBlurOn, bool _isGaussH, int _filterWidth)
{

        // Activate program
        glUseProgram(_program);

        // bind texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, _tex);


        if(_isBlurOn)
            glUniform1i(glGetUniformLocation(_program, "isBlurOn"), 1);
        else
            glUniform1i(glGetUniformLocation(_program, "isBlurOn"), 0);

        if(_isGaussH)
            glUniform1i(glGetUniformLocation(_program, "isFilterH"), 1);
        else
            glUniform1i(glGetUniformLocation(_program, "isFilterH"), 0);

        glUniform1i(glGetUniformLocation(_program, "filterSize"), _filterWidth);


        GLint ShadowMapUniform = glGetUniformLocation(_program, "u_screenTex");
        if (ShadowMapUniform == -1) {
            fprintf(stderr, "[ERROR] DrawableMesh::drawScreenQuad(): Could not bind screen quad texture\n");
            exit(-1);
        }
        glUniform1i(ShadowMapUniform, 0);


        
        glBindVertexArray(m_meshVAO);                       // bind the VAO
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexVBO);  // do not forget to bind the index buffer AFTER !

        glDrawElements(GL_TRIANGLES, m_numIndices, GL_UNSIGNED_INT, 0);

        glBindVertexArray(m_defaultVAO);


        glUseProgram(0);
}



void DrawableMesh::drawScreenQuadSSAO(GLuint _program, glm::mat4 _projMat, GLuint _posTex, GLuint _normalTex, float _radius, float _screenWidth, float _screenHeight)
{
        // Activate program
        glUseProgram(_program);

        // bind textures
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_noiseTex);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, _posTex);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, _normalTex);


        // Pass uniforms

        for (unsigned int i = 0; i < 64; ++i)
        {
            std::string str = "u_samples[" + std::to_string(i) + "]";
            glm::vec3 myVec = m_ssaoKernel[i];
            glUniform3fv(glGetUniformLocation(_program, str.c_str()), 1, &myVec[0]);
        }     
 
        glUniform1i(glGetUniformLocation(_program, "u_noiseTex"), 0);
        glUniform1i(glGetUniformLocation(_program, "u_posTex"), 1);
        glUniform1i(glGetUniformLocation(_program, "u_normalTex"), 2);
        glUniform1f(glGetUniformLocation(_program, "u_radius"), _radius);
        glUniform1f(glGetUniformLocation(_program, "u_screenWidth"), _screenWidth);
        glUniform1f(glGetUniformLocation(_program, "u_screenHeight"), _screenHeight);
        glUniformMatrix4fv(glGetUniformLocation(_program, "u_matP"), 1, GL_FALSE, &_projMat[0][0]);


        // Draw!
        glBindVertexArray(m_meshVAO);                       // bind the VAO
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexVBO);  // do not forget to bind the index buffer AFTER !

        glDrawElements(GL_TRIANGLES, m_numIndices, GL_UNSIGNED_INT, 0);

        glBindVertexArray(m_defaultVAO);


        glUseProgram(0);
}



void DrawableMesh::drawScreenQuadSSLR(GLuint _program, glm::mat4 _modelMat, glm::mat4 _viewMat, glm::mat4 _projMat, GLuint _posTex, GLuint _normalTex, GLuint _screenTex, float _radius, float _screenWidth, float _screenHeight)
{
        // Activate program
        glUseProgram(_program);

        // bind textures
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_noiseTex);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, _posTex);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, _normalTex);

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubeMap);

        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, _screenTex);


        // Pass uniforms

        for (unsigned int i = 0; i < 64; ++i)
        {
            std::string str = "u_samples[" + std::to_string(i) + "]";
            glm::vec3 myVec = m_ssaoKernel[i];
            glUniform3fv(glGetUniformLocation(_program, str.c_str()), 1, &myVec[0]);
        }     
 
        glUniform1i(glGetUniformLocation(_program, "u_noiseTex"), 0);
        glUniform1i(glGetUniformLocation(_program, "u_posTex"), 1);
        glUniform1i(glGetUniformLocation(_program, "u_normalTex"), 2);
        glUniform1i(glGetUniformLocation(_program, "u_cubemap"), 3);
        glUniform1i(glGetUniformLocation(_program, "u_screenTex"), 4);

        glUniformMatrix4fv(glGetUniformLocation(_program, "u_matV"), 1, GL_FALSE, &_viewMat[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(_program, "u_matP"), 1, GL_FALSE, &_projMat[0][0]);

        glUniform1f(glGetUniformLocation(_program, "u_radius"), _radius);
        glUniform1f(glGetUniformLocation(_program, "u_screenWidth"), _screenWidth);
        glUniform1f(glGetUniformLocation(_program, "u_screenHeight"), _screenHeight);


        // Draw!
        glBindVertexArray(m_meshVAO);                       // bind the VAO
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexVBO);  // do not forget to bind the index buffer AFTER !

        glDrawElements(GL_TRIANGLES, m_numIndices, GL_UNSIGNED_INT, 0);

        glBindVertexArray(m_defaultVAO);


        glUseProgram(0);
}


void DrawableMesh::drawScreenQuadFinal(GLuint _program, glm::mat4 _modelMat, glm::mat4 _viewMat, glm::mat4 _projMat, GLuint _ssaotex, GLuint _screenTex, int _occType)
{
        // Activate program
        glUseProgram(_program);

        // bind texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, _screenTex);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, _ssaotex);



        // Pass uniforms
        glUniformMatrix4fv(glGetUniformLocation(_program, "u_matM"), 1, GL_FALSE, &_modelMat[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(_program, "u_matV"), 1, GL_FALSE, &_viewMat[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(_program, "u_matP"), 1, GL_FALSE, &_projMat[0][0]);



        for (unsigned int i = 0; i < 64; ++i)
        {
            std::string str = "u_samples[" + std::to_string(i) + "]";
            glm::vec3 myVec = m_ssaoKernel[i];
            glUniform3fv(glGetUniformLocation(_program, str.c_str()), 1, &myVec[0]);
        }

          
 
        glUniform1i(glGetUniformLocation(_program, "u_colorTex"), 0);

        glUniform1i(glGetUniformLocation(_program, "u_aoTex"), 1);

        glUniform1i(glGetUniformLocation(_program, "u_occlusion_type"), _occType); 

        // Draw!
        glBindVertexArray(m_meshVAO);                       // bind the VAO
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexVBO);  // do not forget to bind the index buffer AFTER !

        glDrawElements(GL_TRIANGLES, m_numIndices, GL_UNSIGNED_INT, 0);

        glBindVertexArray(m_defaultVAO);


        glUseProgram(0);
}


void DrawableMesh::drawTex(GLuint _program, glm::mat4 _modelMat, glm::mat4 _viewMat, glm::mat4 _projMat, GLuint _tex )
{
        // Activate program
        glUseProgram(_program);

        // bind texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, _tex);


        // Pass uniforms
        glUniformMatrix4fv(glGetUniformLocation(_program, "u_matM"), 1, GL_FALSE, &_modelMat[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(_program, "u_matV"), 1, GL_FALSE, &_viewMat[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(_program, "u_matP"), 1, GL_FALSE, &_projMat[0][0]);

 
       // glUniform1i(glGetUniformLocation(m_program, "u_albedoTex"), 0);
        GLint ShadowMapUniform = glGetUniformLocation(_program, "u_albedoTex");
        if (ShadowMapUniform == -1) {
            fprintf(stderr, "Could not bind uniform ShadowMap\n");
            exit(-1);
        }
        glUniform1i(ShadowMapUniform, 0);


        // Draw!
        glBindVertexArray(m_meshVAO);                       // bind the VAO
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexVBO);  // do not forget to bind the index buffer AFTER !

        glDrawElements(GL_TRIANGLES, m_numIndices, GL_UNSIGNED_INT, 0);

        glBindVertexArray(m_defaultVAO);


        glUseProgram(0);
}


void DrawableMesh::drawShadow(GLuint _program, glm::mat4 _lvp)
{

    // Activate program
    glUseProgram(_program);

    // Pass uniforms
    glUniformMatrix4fv(glGetUniformLocation(_program, "u_lvp"), 1, GL_FALSE, &_lvp[0][0]);

    // Draw!
    glBindVertexArray(m_meshVAO);                       // bind the VAO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexVBO);  // do not forget to bind the index buffer AFTER !

    glDrawElements(GL_TRIANGLES, m_numIndices, GL_UNSIGNED_INT, 0);

    glBindVertexArray(m_defaultVAO);

    glUseProgram(0);
}


void DrawableMesh::drawGbuffer(GLuint _program, glm::mat4 _modelMat, glm::mat4 _viewMat, glm::mat4 _projMat, bool _isFloor)
{
    // Activate program
    glUseProgram(_program);

    // Pass uniforms
    glUniformMatrix4fv(glGetUniformLocation(_program, "u_matM"), 1, GL_FALSE, &_modelMat[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(_program, "u_matV"), 1, GL_FALSE, &_viewMat[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(_program, "u_matP"), 1, GL_FALSE, &_projMat[0][0]);

    if(_isFloor)
        glUniform1i(glGetUniformLocation(_program, "isFloor"), 1);
    else
        glUniform1i(glGetUniformLocation(_program, "isFloor"), 0);

    // Draw!
    glBindVertexArray(m_meshVAO);                       // bind the VAO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexVBO);  // do not forget to bind the index buffer AFTER !

    glDrawElements(GL_TRIANGLES, m_numIndices, GL_UNSIGNED_INT, 0);

    glBindVertexArray(m_defaultVAO);

    glUseProgram(0);
}


GLuint DrawableMesh::load2DTexture(const std::string& _filename, bool _repeat)
{
    std::vector<unsigned char> data;
    unsigned width, height;
    unsigned error = lodepng::decode(data, width, height, _filename);
    if (error != 0) 
    {
        std::cerr << "[ERROR] DrawableMesh::load2DTexture(): " << lodepng_error_text(error) << std::endl;
        std::exit(EXIT_FAILURE);
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    if(!_repeat)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    else
    {
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );  
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT ); 
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &(data[0]));
    glBindTexture(GL_TEXTURE_2D, 0);

    return texture;
}


GLuint DrawableMesh::loadCubemap(const std::string& _dirname)
{
    bool isValid = true;
    const char *filenames[] = { "posx.png", "negx.png", "posy.png", "negy.png", "posz.png", "negz.png" };
    const GLenum targets[] = {
        GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
        GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
        GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
    };
    const unsigned int num_sides = 6; 

    std::vector<unsigned char> data[num_sides];
    unsigned int width;
    unsigned int height;
    for (unsigned int i = 0; i < num_sides; ++i) 
    {
        std::string filename = _dirname + "/" + filenames[i];
        unsigned int error = lodepng::decode(data[i], width, height, filename);
        if (error != 0) 
        {
            std::cerr << "[ERROR] DrawableMesh::loadCubemap(): " << lodepng_error_text(error) << std::endl;
            isValid = false;
        }
    }

    GLuint texture;
    if(isValid)
    {
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        for (unsigned int i = 0; i < num_sides; ++i) 
        {
            glTexImage2D(targets[i], 0, GL_SRGB8_ALPHA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &(data[i][0]));
        }
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    }
    return texture;

}


