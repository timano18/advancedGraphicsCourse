
#include <glad/glad.h>
#include <iostream>
#define CGLTF_IMPLEMENTATION
#include <cgltf/cgltf.h>
#include <myHelper/shader_s.h>
#include <stb/stb_image.h>
#include <myHelper/cMesh.h>

#include <thread>
#include <queue>
#include <mutex>

#include <string>
#include <vector>





class cModel {
public:
    std::vector<cMesh> meshes;
    std::string directory;

    cModel(const char* path)
    {
        std::string directoryPath = path;
        directory = directoryPath.substr(0, directoryPath.find_last_of("/") + 1);
        //loadGLTFMeshToGPU(path);
        loadModel(path);
    }

    void Draw(Shader& shader)
    {

        for (unsigned int i = 0; i < meshes.size(); i++) {
            shader.setMat4("model", meshes[i].transform);
            meshes[i].draw(shader);
        }
            

    }

 
    Material createMaterial(cgltf_primitive* primitive)
    {
        Material newMaterial = Material();
        glm::vec4 color(1.0f);
        if (primitive->material) {
            cgltf_material* material = primitive->material;
            cgltf_pbr_metallic_roughness* pbr = &material->pbr_metallic_roughness;
            if (pbr) {
                color = glm::vec4(pbr->base_color_factor[0], pbr->base_color_factor[1],
                    pbr->base_color_factor[2], pbr->base_color_factor[3]);
                newMaterial.baseColor = color;
            }
            if (pbr->base_color_texture.texture) {
                cgltf_texture* texture = pbr->base_color_texture.texture;

                cgltf_image* image = texture->image;

                if (image && image->uri) {

                    // Load the image data using stb_image
                    std::string fullPath = directory + '/' + image->uri;
                    int width, height, nrChannels;
                    unsigned char* data = stbi_load(fullPath.c_str(), &width, &height, &nrChannels, 0);


                    if (data) {
                        GLuint textureID;
                        GLenum format;
                        if (nrChannels == 1)
                            format = GL_RED;
                        else if (nrChannels == 3)
                            format = GL_RGB;
                        else if (nrChannels == 4)
                            format = GL_RGBA;

                        glGenTextures(1, &textureID);
                        glBindTexture(GL_TEXTURE_2D, textureID);
                        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
                        glGenerateMipmap(GL_TEXTURE_2D);

                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f);

                        stbi_image_free(data);

                        newMaterial.textureID = textureID;
                    }

                }

            }

        }
        return newMaterial;

    }
    


    void loadModel(const char* path)
    {
        // 1. Parse the GLTF file
        cgltf_options options = {};
        cgltf_data* data = NULL;
        cgltf_result result = cgltf_parse_file(&options, path, &data);

        if (result != cgltf_result_success) {
            std::cerr << "Failed to parse GLTF file" << std::endl;
            return;
        }

        result = cgltf_load_buffers(&options, data, path);
        if (result != cgltf_result_success) {
            std::cerr << "Failed to load buffers" << std::endl;
            cgltf_free(data);
            return;
        }
        // 2. Process Node TODO
        // After successfully loading buffers:



        for (cgltf_size i = 0; i < data->nodes_count; ++i) {
            if (!data->nodes[i].parent) { // Process root nodes
                processNode(&data->nodes[i]);
            }
        }





        cgltf_free(data);
    }

    void processNode(cgltf_node* node, glm::mat4 parentTransform = glm::mat4(1.0f))
    {
        glm::mat4 nodeTransform = parentTransform;

        if (node->has_matrix) {
            
            glm::mat4 matrix; 
            memcpy(&matrix, node->matrix, sizeof(node->matrix));
            nodeTransform = parentTransform * matrix;
        }
        else {
            glm::vec3 translation(0.0f), scale(1.0f);
            glm::quat rotation(1.0f, 0.0f, 0.0f, 0.0f);



            if (node->has_translation) {
                
                memcpy(&translation, node->translation, sizeof(node->translation));
            }

            if (node->has_rotation) {
                memcpy(&rotation, node->rotation, sizeof(node->rotation));
            }

            if (node->has_scale) {
                memcpy(&scale, node->scale, sizeof(node->scale));

            }

            // Construct the transformation matrix from T, R, S
            glm::mat4 T = glm::translate(glm::mat4(1.0f), translation);
            glm::mat4 R = glm::toMat4(rotation);
            glm::mat4 S = glm::scale(glm::mat4(1.0f), scale);

            nodeTransform = parentTransform * T * R * S;
        }


        if (node->mesh) {
            meshes.push_back(processMesh(node->mesh, nodeTransform));
        }

        for (cgltf_size i = 0; i < node->children_count; ++i) {
            processNode(node->children[i], nodeTransform);
        }
    }

    cMesh processMesh(cgltf_mesh* mesh, glm::mat4 transform) {
        std::vector<cPrimitive> primitives;
    GLenum index_type = GL_UNSIGNED_SHORT;
    for (cgltf_size p = 0; p < mesh->primitives_count; ++p) {
        cgltf_primitive* primitive = &mesh->primitives[p];

        cgltf_accessor* positions = NULL;
        cgltf_accessor* v_normals = NULL;
        cgltf_accessor* tex_coords = NULL;
        for (int i = 0; i < primitive->attributes_count; i++) {
            if (primitive->attributes[i].type == cgltf_attribute_type_position) {
                positions = primitive->attributes[i].data;
            } else if (primitive->attributes[i].type == cgltf_attribute_type_normal) {
                v_normals = primitive->attributes[i].data;
            }
            else if (primitive->attributes[i].type == cgltf_attribute_type_texcoord) {
                tex_coords = primitive->attributes[i].data;
            }
        }

        if (primitive->indices) {
            
            switch (primitive->indices->component_type) {
            case cgltf_component_type_r_8u:
                index_type = GL_UNSIGNED_BYTE;
                break;
            case cgltf_component_type_r_16u:
                index_type = GL_UNSIGNED_SHORT;
                break;
            case cgltf_component_type_r_32u:
                index_type = GL_UNSIGNED_INT;
                break;
            }
            
        }

        if (!positions || !v_normals) {
            std::cout << "Positions or normals not found for primitive " << p << std::endl;
            continue; // Skip this primitive if essential data is missing
        }

        cgltf_buffer_view* pos_view = positions->buffer_view;
        float* vertices = (float*)((char*)(pos_view->buffer->data) + pos_view->offset + positions->offset);

        cgltf_buffer_view* norm_view = v_normals->buffer_view;
        float* normals = (float*)((char*)(norm_view->buffer->data) + norm_view->offset + v_normals->offset);

        cgltf_buffer_view* tex_view = tex_coords->buffer_view;
        float* Coords = (float*)((char*)(tex_view->buffer->data) + tex_view->offset + tex_coords->offset);

        // Assuming indices are present and required for rendering
        cgltf_accessor* indices = primitive->indices;
        cgltf_buffer_view* ind_view = indices->buffer_view;
        unsigned int* int_data = (unsigned int*)((char*)(ind_view->buffer->data) + ind_view->offset + indices->offset);

        
       
     
        unsigned short* short_data = (unsigned short*)((char*)(ind_view->buffer->data) + ind_view->offset + indices->offset);

          
        // Material
        Material newMaterial = createMaterial(primitive);
        
        
        
        // Create and bind VAO, VBO, EBO
        GLuint VAO, VBO_positions, VBO_normals, VBO_coords, EBO;
        glGenVertexArrays(1, &VAO); 
        glBindVertexArray(VAO);

        //cout << *ind_data << endl;
        // Positions
        glGenBuffers(1, &VBO_positions);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_positions);
        glBufferData(GL_ARRAY_BUFFER, positions->count * sizeof(float) * 3, vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3 , (void*)0);
        glEnableVertexAttribArray(0);

        // Normals
        glGenBuffers(1, &VBO_normals);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_normals);
        glBufferData(GL_ARRAY_BUFFER, v_normals->count * sizeof(float) * 3, normals, GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);

        // Texture Coordinates
        glGenBuffers(1, &VBO_coords);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_coords);
        glBufferData(GL_ARRAY_BUFFER, tex_coords->count * sizeof(float) * 2, Coords, GL_STATIC_DRAW);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(2);

        // Indices
        glGenBuffers(1, &EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        if (index_type == GL_UNSIGNED_INT)
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices->count * sizeof(unsigned int), int_data, GL_STATIC_DRAW);
        else
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices->count * sizeof(unsigned short), short_data, GL_STATIC_DRAW);

        // Unbind VAO to avoid accidentally modifying it
        glBindVertexArray(0);

        // Add the new primitive to the primitives vector
        primitives.push_back(cPrimitive(VAO, VBO_positions, EBO, indices->count, index_type, newMaterial));
    }

    cMesh newMesh(primitives);
    newMesh.transform = transform;
    return newMesh;
}


};
