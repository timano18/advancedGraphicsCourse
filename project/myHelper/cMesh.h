
#include <glad/glad.h>
#include <iostream>
#include <myHelper/shader_s.h>
#include <myHelper/cPrimitive.h>

#include <string>
#include <vector>




class cMesh {
public:
    std::vector<cPrimitive> primitives;
    glm::mat4 transform;

    cMesh(const std::vector<cPrimitive>& primitives)
        : primitives(primitives) { }

    void draw(Shader& shader)
    {
        for (auto& primitive : primitives) {
            primitive.draw(shader);
        }
       
    }
};




