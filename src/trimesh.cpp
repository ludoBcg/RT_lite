/*********************************************************************************************************************
 *
 * trimesh.cpp
 * 
 * RT_lite
 * Ludovic Blache
 *
 *********************************************************************************************************************/

#include "trimesh.h"


TriMesh::TriMesh()
{
    m_TBComputed = false;

    m_bBoxMin = glm::vec3(0.0f, 0.0f, 0.0f);
    m_bBoxMax = glm::vec3(0.0f, 0.0f, 0.0f);
}


TriMesh::TriMesh(bool _normals, bool _texCoords2D, bool _col)
{
    m_TBComputed = false;

    m_bBoxMin = glm::vec3(0.0f, 0.0f, 0.0f);
    m_bBoxMax = glm::vec3(0.0f, 0.0f, 0.0f);
}


TriMesh::~TriMesh()
{
    clear();
}


void TriMesh::getVertices(std::vector<glm::vec3>& _vertices)
{
    if(_vertices.size() != 0)
        _vertices.clear();

    if(m_vertices.size() != 0)
    {
        _vertices.assign(m_vertices.begin(), m_vertices.end());
    }
    else
    {
        std::cerr << "[WARNING] TriMesh::getVertices(): Empty vertices array" << std::endl;
    }
}


void TriMesh::getNormals(std::vector<glm::vec3>& _normals)
{
    if(_normals.size() != 0)
        _normals.clear();

    if(m_normals.size() != 0)
    {
        _normals.assign(m_normals.begin(), m_normals.end());
    }
    else
    {
        std::cerr << "[WARNING] TriMesh::getNormals(): Empty normals array" << std::endl;
    }
}


void TriMesh::getIndices(std::vector<uint32_t>& _indices)
{
    if(_indices.size() != 0)
        _indices.clear();

    if(m_indices.size() != 0)
    {
        _indices.assign(m_indices.begin(), m_indices.end());
    }
    else
    {
        std::cerr << "[WARNING] TriMesh::getIndices(): Empty indices array" << std::endl;
    }
}


void TriMesh::getColors(std::vector<glm::vec3>& _colors)
{
    if(_colors.size() != 0)
        _colors.clear();

    if(m_colors.size() != 0)
    {
        _colors.assign(m_colors.begin(), m_colors.end());
    }
}


void TriMesh::getTexCoords(std::vector<glm::vec2>& _texcoords)
{
    if(_texcoords.size() != 0)
        _texcoords.clear();

    if(m_texcoords.size() != 0)
    {
        _texcoords.assign(m_texcoords.begin(), m_texcoords.end());
    }
}


void TriMesh::getTangents(std::vector<glm::vec3>& _tangents)
{
    if(_tangents.size() != 0)
        _tangents.clear();

    if(m_tangents.size() != 0)
    {
        _tangents.assign(m_tangents.begin(), m_tangents.end());
    }
}


void TriMesh::getBitangents(std::vector<glm::vec3>& _bitangents)
{
    if(_bitangents.size() != 0)
        _bitangents.clear();

    if(m_bitangents.size() != 0)
    {
        _bitangents.assign(m_bitangents.begin(), m_bitangents.end());
    }
}


bool TriMesh::readFile(std::string _filename)
{
    if(_filename.substr(_filename.find_last_of(".") + 1) == "obj")
    {
        importOBJ(_filename);
        return true;
    }
    else
    {
        std::cerr << "[ERROR] TriMesh::readFile(): Invalid file extension: only .obj are supported" << std::endl;
    }
    return false;
}



void TriMesh::computeAABB()
{
    if(m_vertices.size() != 0)
    {
        // init values
        glm::vec3 min = m_vertices[0];
        glm::vec3 max = m_vertices[0];

        for (unsigned int i = 0; i <m_vertices.size(); i++) 
        {
            if(m_vertices[i].x < min.x) { min.x = m_vertices[i].x; }
            if(m_vertices[i].y < min.y) { min.y = m_vertices[i].y; }
            if(m_vertices[i].z < min.z) { min.z = m_vertices[i].z; }
            if(m_vertices[i].x > max.x) { max.x = m_vertices[i].x; }
            if(m_vertices[i].y > max.y) { max.y = m_vertices[i].y; }
            if(m_vertices[i].z > max.z) { max.z = m_vertices[i].z; }
        }
        m_bBoxMin = min;
        m_bBoxMax = max;
    }
    else
    {
        std::cerr << "[WARNING] TriMesh::computeAABB(): Empty vertices array" << std::endl;
        m_bBoxMin = glm::vec3(0.0f, 0.0f, 0.0f);
        m_bBoxMax = glm::vec3(0.0f, 0.0f, 0.0f);
    }
}


void TriMesh::computeNormals()
{
    m_normals.clear();
    m_normals.resize(m_vertices.size(), glm::vec3(0.0f, 0.0f, 0.0f));

    // Compute per-vertex normals by averaging the unnormalized face normals
    std::int32_t vertexIndex0, vertexIndex1, vertexIndex2;
    glm::vec3 normal;
    int numIndices = m_indices.size();
    for (int i = 0; i < numIndices; i += 3) 
    {
        vertexIndex0 = m_indices[i];
        vertexIndex1 = m_indices[i + 1];
        vertexIndex2 = m_indices[i + 2];
        normal = glm::cross(m_vertices[vertexIndex1] - m_vertices[vertexIndex0],  m_vertices[vertexIndex2] - m_vertices[vertexIndex0]);
        m_normals[vertexIndex0] += normal;
        m_normals[vertexIndex1] += normal;
        m_normals[vertexIndex2] += normal;
    }

    int numNormals = (int)m_normals.size();
    for (int i = 0; i < numNormals; i++) 
    {
        m_normals[i] = glm::normalize(m_normals[i]);
    }
}


void TriMesh::computeTB()
{
    if( m_normals.size() == 0)
    {
        std::cerr << "[WARNING] TriMesh::computeTB(): normals not available" << std::endl; 
        computeNormals();
    }

    if( m_texcoords.size() != 0)
    {
        m_tangents.resize( m_normals.size() );
        m_bitangents.resize( m_normals.size() );
        //for each triangle
        for (int i = 0; i < m_indices.size(); i += 3)
        {
            int vertexIndex0 = m_indices[i];
            int vertexIndex1 = m_indices[i + 1];
            int vertexIndex2 = m_indices[i + 2];

            glm::vec3 tangent, bitangent;
            compTandBT( m_vertices[vertexIndex1] - m_vertices[vertexIndex0], 
                        m_vertices[vertexIndex2] - m_vertices[vertexIndex0], 
                        glm::vec3( m_texcoords[vertexIndex1].x,  m_texcoords[vertexIndex1].y, 0.0f ) - glm::vec3( m_texcoords[vertexIndex0].x,  m_texcoords[vertexIndex0].y, 0.0f ), 
                        glm::vec3( m_texcoords[vertexIndex2].x,  m_texcoords[vertexIndex2].y, 0.0f ) - glm::vec3( m_texcoords[vertexIndex0].x,  m_texcoords[vertexIndex0].y, 0.0f ), 
                        tangent, bitangent);
            m_tangents[vertexIndex0] = tangent;
            m_bitangents[vertexIndex0] = bitangent;
            compTandBT( m_vertices[vertexIndex2] - m_vertices[vertexIndex1], 
                        m_vertices[vertexIndex0] - m_vertices[vertexIndex1], 
                        glm::vec3( m_texcoords[vertexIndex2].x,  m_texcoords[vertexIndex2].y, 0.0f ) - glm::vec3( m_texcoords[vertexIndex1].x,  m_texcoords[vertexIndex1].y, 0.0f ), 
                        glm::vec3( m_texcoords[vertexIndex0].x,  m_texcoords[vertexIndex0].y, 0.0f ) - glm::vec3( m_texcoords[vertexIndex1].x,  m_texcoords[vertexIndex1].y, 0.0f ),
                        tangent, bitangent);
            m_tangents[vertexIndex1] = tangent;
            m_bitangents[vertexIndex1] = bitangent;
            compTandBT( m_vertices[vertexIndex0] - m_vertices[vertexIndex2], 
                        m_vertices[vertexIndex1] - m_vertices[vertexIndex2], 
                        glm::vec3( m_texcoords[vertexIndex0].x,  m_texcoords[vertexIndex0].y, 0.0f ) - glm::vec3( m_texcoords[vertexIndex2].x,  m_texcoords[vertexIndex2].y, 0.0f ), 
                        glm::vec3( m_texcoords[vertexIndex1].x,  m_texcoords[vertexIndex1].y, 0.0f ) - glm::vec3( m_texcoords[vertexIndex2].x,  m_texcoords[vertexIndex2].y, 0.0f ),
                        tangent, bitangent);
            m_tangents[vertexIndex2] = tangent;
            m_bitangents[vertexIndex2] = bitangent;
        }
    }
    else
    {
        std::cerr << "[WARNING] TriMesh::computeTB(): texcoords not available" << std::endl; 
    }
}


/*
 * Read an Mesh from an .obj file. This function can read texture
 * coordinates and/or normals, in addition to vertex positions.
 */
bool TriMesh::importOBJ(const std::string &_filename)
{
    struct uvec3Less 
    {
        bool operator() (const glm::uvec3 &a, const glm::uvec3 &b) const
        {
            return (a.x < b.x) |
                    ((a.x == b.x) & (a.y < b.y)) |
                    ((a.x == b.x) & (a.y == b.y) & (a.z < b.z));
        }
    };

    const std::string VERTEX_LINE("v ");
    const std::string TEXCOORD_LINE("vt ");
    const std::string NORMAL_LINE("vn ");
    const std::string FACE_LINE("f ");

    std::string line;
    glm::vec3 vertex;
    glm::vec3 normal;
    glm::vec3 texcoord;
    std::uint32_t vindex[3];
    std::uint32_t tindex[3];
    std::uint32_t nindex[3];

    // Open OBJ file
    std::ifstream f(_filename.c_str());
    if(!f.is_open()) 
    {
        std::cerr << "[ERROR] TriMesh::importOBJ(): Could not open " << _filename << std::endl;
        return false;
    }

    // First pass: read vertex data into temporary mesh
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<uint32_t> indices;
    std::vector<glm::vec3> texcoords;
    while(!f.eof()) 
    {
        std::getline(f, line);
        if (line.substr(0, 2) == VERTEX_LINE) 
        {
            std::istringstream ss(line.substr(2));
            ss >> vertex.x >> vertex.y >> vertex.z;
            vertices.push_back(vertex);
        }
        else if (line.substr(0, 3) == TEXCOORD_LINE) 
        {
            std::istringstream ss(line.substr(3));
            ss >> texcoord.x >> texcoord.y;// >> texcoord.z;
            texcoords.push_back(texcoord);
        }
        else if (line.substr(0, 3) == NORMAL_LINE) 
        {
            std::istringstream ss(line.substr(3));
            ss >> normal.x >> normal.y >> normal.z;
            normals.push_back(normal);
        }
        else 
        {
            // Ignore line
        }
    }

    // Rewind file
    f.clear();
    f.seekg(0);

    // Clear old mesh and pre-allocate space for new mesh data
    m_vertices.clear();
    m_vertices.reserve(vertices.size());
    m_texcoords.clear();
    m_texcoords.reserve(texcoords.size());
    m_normals.clear();
    m_normals.reserve(normals.size());
    m_indices.clear();

    // Set up dictionary for mapping unique tuples to indices
    std::map<glm::uvec3, unsigned, uvec3Less> visited;
    unsigned next_index = 0;
    glm::uvec3 key;

    // Second pass: read faces and construct per-vertex texcoords/normals.
    // Note: OBJ-indices start at one, so we need to subtract indices by one.
    while (!f.eof()) 
    {
        std::getline(f, line);
        if (line.substr(0, 2) == FACE_LINE) 
        {
            if (std::sscanf(line.c_str(), "f %d %d %d", &vindex[0], &vindex[1], &vindex[2]) == 3) 
            {
                for (unsigned i = 0; i < 3; ++i) 
                {
                    key = glm::uvec3(vindex[i], 0, 0);
                    if (visited.count(key) == 0) 
                    {
                        visited[key] = next_index++;
                        m_vertices.push_back(vertices[vindex[i] - 1]);
                    }
                    m_indices.push_back(visited[key]);
                }
            }
            else if (std::sscanf(line.c_str(), "f %d/%d %d/%d %d/%d", &vindex[0], &tindex[0], &vindex[1], &tindex[1], &vindex[2], &tindex[2]) == 6) 
            {
                for (unsigned i = 0; i < 3; ++i) 
                {
                    key = glm::uvec3(vindex[i], tindex[i], 0);
                    if (visited.count(key) == 0) 
                    {
                        visited[key] = next_index++;
                        m_vertices.push_back(vertices[vindex[i] - 1]);
                        m_texcoords.push_back( glm::vec2(texcoords[tindex[i] - 1].x, texcoords[tindex[i] - 1].y) );
                    }
                    m_indices.push_back(visited[key]);
                }
            }
            else if (std::sscanf(line.c_str(), "f %d//%d %d//%d %d//%d", &vindex[0], &nindex[0], &vindex[1], &nindex[1], &vindex[2], &nindex[2]) == 6) 
            {
                for (unsigned i = 0; i < 3; ++i) 
                {
                    key = glm::uvec3(vindex[i], nindex[i], 0);
                    if (visited.count(key) == 0) 
                    {
                        visited[key] = next_index++;
                        m_vertices.push_back(vertices[vindex[i] - 1]);
                        m_normals.push_back(normals[nindex[i] - 1]);
                    }
                    m_indices.push_back(visited[key]);
                }
            }
            else if(std::sscanf(line.c_str(), "f %d/%d/%d %d/%d/%d %d/%d/%d", &vindex[0], &tindex[0], &nindex[0], &vindex[1], &tindex[1], &nindex[1], &vindex[2], &tindex[2], &nindex[2]) == 9) 
            {
                for(unsigned i = 0; i < 3; ++i) 
                {
                    key = glm::uvec3(vindex[i], tindex[i], nindex[i]);
                    if (visited.count(key) == 0) 
                    {
                        visited[key] = next_index++;
                        m_vertices.push_back(vertices[vindex[i] - 1]);
                        m_texcoords.push_back( glm::vec2(texcoords[tindex[i] - 1].x, texcoords[tindex[i] - 1].y) );
                        m_normals.push_back(normals[nindex[i] - 1]);
                    }
                    m_indices.push_back(visited[key]);
                }
            }
        }
    }

    // Compute normals (if OBJ-file did not contain normals)
    if(m_normals.size() == 0) 
    {
        std::cout << "[INFO] TriMesh::importOBJ(): Normals not provided, compute them " << std::endl;
        computeNormals();
    }

    if(m_texcoords.size() == 0) 
        std::cout << "[INFO] TriMesh::importOBJ(): UV coords not provided " << std::endl;

    return true;
}


/*
 * cf.  http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-13-normal-mapping/
 */
void TriMesh::compTandBT(glm::vec3 _delta_pos1, glm::vec3 _delta_pos2, glm::vec3 _delta_uv1, glm::vec3 _delta_uv2, glm::vec3& _tangent, glm::vec3& _bitangent)
{
    glm::normalize(_delta_pos1);
    glm::normalize(_delta_pos2);
    glm::normalize(_delta_uv1);
    glm::normalize(_delta_uv2);

	float r = 1.0f / (_delta_uv1.x * _delta_uv2.y - _delta_uv1.y * _delta_uv2.x);
    _tangent = (_delta_pos1 * _delta_uv2.y   - _delta_pos2 * _delta_uv1.y)*r;
    _bitangent = (_delta_pos2 * _delta_uv1.x   - _delta_pos1 * _delta_uv2.x)*r;

    glm::normalize(_tangent);
    glm::normalize(_bitangent);
}


void TriMesh::clear()
{
    m_vertices.clear();
    m_normals.clear();
    m_indices.clear();

    m_colors.clear();
    m_texcoords.clear();
    m_tangents.clear();
    m_bitangents.clear();
}
