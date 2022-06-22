#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <algorithm>

#include "../base/texture.h"
#include "../base/model.h"

using namespace glm;

struct BVHNode {
    int left, right;    // 左右子树索引
    int n, index;       // 叶子节点信息               
    vec3 AA, BB;        // 碰撞盒
};

struct BVHNode_encoded {
    vec3 childs;        // (left, right, 保留)
    vec3 leafInfo;      // (n, index, 保留)
    vec3 AA, BB;        
};

class BVH {
public:
    std::vector<BVHNode> nodes;
    std::vector<BVHNode_encoded> nodes_encoded;
    BVH() {}
};