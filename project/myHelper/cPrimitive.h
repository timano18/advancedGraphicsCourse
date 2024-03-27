
#include <glad/glad.h>
#include <iostream>
#include <myHelper/shader_s.h>
#include <myHelper/cMaterial.h>

#include <string>
#include <vector>




class cPrimitive {
public:
    GLuint VAO, VBO, EBO;
    cgltf_size globalIndexCount;
    GLenum index_type;
    Material material;

    cPrimitive(GLuint nVAO, GLuint nVBO, GLuint nEBO, cgltf_size nGlobalIndexCount, GLenum nIndex_type, Material nMaterial)
    {
        VAO = nVAO;
        VBO = nVBO;
        EBO = nEBO;
        globalIndexCount = nGlobalIndexCount;
        index_type = nIndex_type;
        material = nMaterial;
    }

    void draw(Shader& shader)
    {
        shader.use();
        shader.setVec3("objectColor", material.baseColor.x, material.baseColor.y, material.baseColor.z);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, material.textureID);
        GLint textureUniformLocation = glGetUniformLocation(shader.ID, "textureDiffuse");
        glUniform1i(textureUniformLocation, 0);
        glBindVertexArray(VAO);
        
       
        glDrawElements(GL_TRIANGLES, // mode: specifies the kind of primitives to render
            globalIndexCount, // count: specifies the number of elements to be rendered
            index_type, // type: specifies the type of the values in indices
            0); // indices: specifies a pointer to the location where the indices are stored (NULL if EBO is bound)

        // Unbind the VAO to prevent accidental modifications
        glBindVertexArray(0);
       
    }
};




