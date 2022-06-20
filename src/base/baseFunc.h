#include "scene.h"

bool cmpx(const Triangle& t1, const Triangle& t2) {
    vec3 center1 = (t1.p1 + t1.p2 + t1.p3) / vec3(3, 3, 3);
    vec3 center2 = (t2.p1 + t2.p2 + t2.p3) / vec3(3, 3, 3);
    return center1.x < center2.x;
}
bool cmpy(const Triangle& t1, const Triangle& t2) {
    vec3 center1 = (t1.p1 + t1.p2 + t1.p3) / vec3(3, 3, 3);
    vec3 center2 = (t2.p1 + t2.p2 + t2.p3) / vec3(3, 3, 3);
    return center1.y < center2.y;
}
bool cmpz(const Triangle& t1, const Triangle& t2) {
    vec3 center1 = (t1.p1 + t1.p2 + t1.p3) / vec3(3, 3, 3);
    vec3 center2 = (t2.p1 + t2.p2 + t2.p3) / vec3(3, 3, 3);
    return center1.z < center2.z;
}

GLuint getTextureRGB32F(int width, int height) {
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    return tex;
}

std::string readShaderFile(std::string filepath) {
    std::string res, line;
    std::ifstream fin(filepath);
    if (!fin.is_open())
    {
        std::cout << "文件 " << filepath << " 打开失败" << std::endl;
        exit(-1);
    }
    while (std::getline(fin, line))
    {
        res += line + '\n';
    }
    fin.close();
    return res;
}

GLuint getShaderProgram(std::string fshader, std::string vshader) {
    // 读取shader源文件
    std::string vSource = readShaderFile(vshader);
    std::string fSource = readShaderFile(fshader);
    const char* vpointer = vSource.c_str();
    const char* fpointer = fSource.c_str();

    // 容错
    GLint success;
    GLchar infoLog[512];

    // 创建并编译顶点着色器
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, (const GLchar**)(&vpointer), NULL);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);   // 错误检测
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "顶点着色器编译错误\n" << infoLog << std::endl;
        exit(-1);
    }

    // 创建并且编译片段着色器
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, (const GLchar**)(&fpointer), NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);   // 错误检测
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "片段着色器编译错误\n" << infoLog << std::endl;
        exit(-1);
    }

    // 链接两个着色器到program对象
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // 删除着色器对象
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}