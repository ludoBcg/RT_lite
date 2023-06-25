/*********************************************************************************************************************
 *
 * trimesh.h
 *
 * Triangle soup data structure
 * 
 * RT_lite
 * Ludovic Blache
 *
 *********************************************************************************************************************/

#ifndef TRIMESHSOUP_H
#define TRIMESHSOUP_H

#include <map>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>



/*!
* \class TriMesh
* \brief Triangle soup mesh (i.e. no adjacency information)
* Read and write STL and OBJ files and store data in dynamic arrays
* Duplicate vertices data to handle multiple UV coords and/or normals
*/
class TriMesh
{
    public:

        /*------------------------------------------------------------------------------------------------------------+
        |                                        CONSTRUCTORS / DESTRUCTORS                                           |
        +------------------------------------------------------------------------------------------------------------*/

        /*!
        * \fn TriMesh
        * \brief Default constructor of TriMeshSoup
        */
        TriMesh();

        /*!
        * \fn TriMesh
        * \brief Constructor of TriMesh with various vertex properties.
        *        Other properties can still be requested when needed.
        * \param _normals : true if mesh needs normals
        * \param _texCoords2D : true if mesh needs texCoords2D
        * \param _col : true if mesh needs colors
        */
        TriMesh(bool _normals, bool _texCoords2D, bool _col);

        /*!
        * \fn ~TriMesh
        * \brief Destructor of TriMesh
        */
        ~TriMesh();


        /*------------------------------------------------------------------------------------------------------------+
        |                                              GETTERS/SETTERS                                                |
        +-------------------------------------------------------------------------------------------------------------*/

        /*! \fn getVertices */
        void getVertices(std::vector<glm::vec3>& _vertices);
        /*! \fn getNormals */
        void getNormals(std::vector<glm::vec3>& _normals);
        /*! \fn getIndices */
        void getIndices(std::vector<uint32_t>& _indices);

        /*! \fn getColors */
        void getColors(std::vector<glm::vec3>& _colors);
        /*! \fn getTexCoords */
        void getTexCoords(std::vector<glm::vec2>& _texcoords);
        /*! \fn getTangents */
        void getTangents(std::vector<glm::vec3>& _tangents);
        /*! \fn getBitangents */
        void getBitangents(std::vector<glm::vec3>& _bitangents);

        /*!
        * \fn getBBoxMin
        * \brief get min point of the bounding box
        * \return 3D coords of the min point of the BBox
        */
        glm::vec3 getBBoxMin() { return m_bBoxMin; }
        /*!
        * \fn getBBoxMax
        * \brief get max point of the bounding box
        * \return 3D coords of the max point of the BBox
        */
        glm::vec3 getBBoxMax() { return m_bBoxMax; }


        /*------------------------------------------------------------------------------------------------------------+
        |                                               OTHER METHODS                                                 |
        +-------------------------------------------------------------------------------------------------------------*/

        /*!
        * \fn readFile
        * \brief read a mesh from a file
        * \param _filename : name of the file to read
        * \return false if file extension is not supported, true if it is
        */
        bool readFile(std::string _filename);

        /*!
        * \fn computeAABB
        * \brief compute Axis Oriented Bounding Box
        */
        void computeAABB();

        /*!
        * \fn computeNormals
        * \brief recompute the triangle normals and update vertex normals
        */
        void computeNormals();

        /*!
        * \fn computeTB
        * \brief Compute tangent and bitangent vectors for all vertices of the mesh
        */
        void computeTB();


    protected:

        /*------------------------------------------------------------------------------------------------------------+
        |                                                ATTRIBUTES                                                   |
        +-------------------------------------------------------------------------------------------------------------*/

        std::vector<glm::vec3> m_vertices;      /*!< vertices positions array (3D coords) */
        std::vector<glm::vec3> m_normals;       /*!< vertices normal vectors array (3D coords) */
        std::vector<uint32_t> m_indices;        /*!< vertices indices array (uint) */

        std::vector<glm::vec3> m_colors;        /*!< vertices RGB colors array (3D coords) */
        std::vector<glm::vec2> m_texcoords;     /*!< vertices uvs array (2D coords) */
        std::vector<glm::vec3> m_tangents;      /*!< vertices tangent vectors array (3D coords) */
        std::vector<glm::vec3> m_bitangents;    /*!< vertices bitangent vectors array (3D coords) */
        
        bool m_TBComputed;                      /*!< Flag that indicates if tangent and bitangents had been computed */

        glm::vec3 m_bBoxMin;                    /*!< 3D coordinates of the min corner of the bounding box */
        glm::vec3 m_bBoxMax;                    /*!< 3D coordinates of the max corner of the bounding box */


        /*------------------------------------------------------------------------------------------------------------+
        |                                               OTHER METHODS                                                 |
        +-------------------------------------------------------------------------------------------------------------*/

        /*!
        * \fn importOBJ
        * \brief read OBJ file
        * \param _filename: name of file
        */
        bool importOBJ(const std::string &_filename);

        /*!
        * \fn compTandBTt
        * \brief Compute tangent and bitangent vectors from delta uv and delta pos
        *        http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-13-normal-mapping/
        * \param _delta_pos1 : vector along first edge of triangle
        * \param _delta_pos2 : vector along second edge of triangle
        * \param _delta_uv1 : differences in uv coords along first edge of triangle
        * \param _delta_uv1 : differences in uv coords along second edge of triangle
        * \param _tangent : result tangent vector
        * \param _bitangent : result bitangent vector
        */
        void compTandBT(glm::vec3 _delta_pos1, glm::vec3 _delta_pos2, glm::vec3 _delta_uv1, glm::vec3 _delta_uv2, glm::vec3& _tangent, glm::vec3& _bitangent);

        /*!
        * \fn clear
        * \brief Clear the content of all the attribute vectors
        */
        void clear();

};
#endif // TRIMESH_H