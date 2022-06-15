#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <algorithm>

#include "../base/texture.h"
#include "../base/model.h"

using namespace glm;

// 物体表面材质定义
struct Material {
    vec3 emissive = vec3(0, 0, 0);
    vec3 baseColor = vec3(1, 1, 1);
    float subsurface = 0.0;
    float metallic = 0.0;
    float specular = 0.5;
    float specularTint = 0.0;
    float roughness = 0.5;
    float anisotropic = 0.0;
    float sheen = 0.0;
    float sheenTint = 0.5;
    float clearcoat = 0.0;
    float clearcoatGloss = 1.0;
    float IOR = 1.0;
    float transmission = 0.0;
};

// 三角形定义
struct Triangle {
    vec3 p1, p2, p3;    // 顶点坐标
    vec3 n1, n2, n3;    // 顶点法线
    Material material;  // 材质
};

// BVH 树节点
struct BVHNode {
    int left, right;    // 左右子树索引
    int n, index;       // 叶子节点信息               
    vec3 AA, BB;        // 碰撞盒
};

struct Triangle_encoded {
    vec3 p1, p2, p3;    // 顶点坐标
    vec3 n1, n2, n3;    // 顶点法线
    vec3 emissive;      // 自发光参数
    vec3 baseColor;     // 颜色
    vec3 param1;        // (subsurface, metallic, specular)
    vec3 param2;        // (specularTint, roughness, anisotropic)
    vec3 param3;        // (sheen, sheenTint, clearcoat)
    vec3 param4;        // (clearcoatGloss, IOR, transmission)
};

struct BVHNode_encoded {
    vec3 childs;        // (left, right, 保留)
    vec3 leafInfo;      // (n, index, 保留)
    vec3 AA, BB;        
};

class RenderPass {
public:
    GLuint FBO = 0;
    GLuint vao, vbo;
    std::vector<GLuint> colorAttachments;
    GLuint program;

    int width = 1280;
    int height = 720;
    
    void bindData(bool finalPass = false) {
        if (!finalPass) glGenFramebuffers(1, &FBO);
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);

        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        std::vector<vec3> square = { vec3(-1, -1, 0), vec3(1, -1, 0), vec3(-1, 1, 0), vec3(1, 1, 0), vec3(-1, 1, 0), vec3(1, -1, 0) };
        glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * square.size(), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec3) * square.size(), &square[0]);

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glEnableVertexAttribArray(0);   // layout (location = 0) 
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
        // 不是 finalPass 则生成帧缓冲的颜色附件
        if (!finalPass) {
            std::vector<GLuint> attachments;
            for (int i = 0; i < colorAttachments.size(); i++) {
                glBindTexture(GL_TEXTURE_2D, colorAttachments[i]);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorAttachments[i], 0);// 将颜色纹理绑定到 i 号颜色附件
                attachments.push_back(GL_COLOR_ATTACHMENT0 + i);
            }
            glDrawBuffers(attachments.size(), &attachments[0]);
        }
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    
    void draw(std::vector<GLuint> texPassArray = {}) {
        glUseProgram(program);
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
        glBindVertexArray(vao);

        // 传上一帧的帧缓冲颜色附件
        for (int i = 0; i < texPassArray.size(); i++) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, texPassArray[i]);
            std::string uName = "texPass" + std::to_string(i);
            glUniform1i(glGetUniformLocation(program, uName.c_str()), i);
        }
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glBindVertexArray(0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glUseProgram(0);
    }
};