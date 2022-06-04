#include"scene.h"
#include<fstream>

void Scene::SceneSave(){
    std::ofstream ofs;
    ofs.open("../media/info.cgf",std::ios::out|std::ios::trunc);
    for(int i=0;i<_objectlist.ModelList.size();i++){
        ofs<<"OBJ"<<std::endl;
        ofs<<i<<std::endl;
        ofs<<_objectlist.objectname[i]<<std::endl;
        ofs<<_objectlist.visible[i]<<std::endl;
        ofs<<_objectlist.color_flag[i]<<std::endl;
        ofs<<_objectlist.TextureIndex[i]<<std::endl;
        ofs<<_objectlist.Color[i].x<<std::endl;
        ofs<<_objectlist.Color[i].y<<std::endl;
        ofs<<_objectlist.Color[i].z<<std::endl;
        ofs<<_objectlist.roughness[i]<<std::endl;
        ofs<<_objectlist.metallic[i]<<std::endl;
        ofs<<_objectlist.filepath[i]<<std::endl;
        ofs<<_objectlist.ModelList[i]->scale.x<<std::endl;
        ofs<<_objectlist.ModelList[i]->scale.y<<std::endl;
        ofs<<_objectlist.ModelList[i]->scale.z<<std::endl;
        ofs<<_objectlist.ModelList[i]->position.x<<std::endl;
        ofs<<_objectlist.ModelList[i]->position.y<<std::endl;
        ofs<<_objectlist.ModelList[i]->position.z<<std::endl;
        ofs<<_objectlist.ModelList[i]->rotation.w<<std::endl;
        ofs<<_objectlist.ModelList[i]->rotation.x<<std::endl;
        ofs<<_objectlist.ModelList[i]->rotation.y<<std::endl;
        ofs<<_objectlist.ModelList[i]->rotation.z<<std::endl;
    }
    for(int i=0;i<_texturelist.filepath.size();i++){
        ofs<<"TEXTURE"<<std::endl;
        ofs<<i<<std::endl;
        ofs<<_texturelist.texturename[i]<<std::endl;
        ofs<<_texturelist.filepath[i]<<std::endl;
    }
    ofs<<"SERIES"<<std::endl;
    ofs<<_serise.max<<std::endl;
    ofs<<_serise.sequence.size()<<std::endl;
    for(int i=0;i<_serise.sequence.size();i++){
        ofs<<_serise.sequence[i]<<std::endl;
    }
    ofs.close();
}

bool Scene::SceneLoad(){
    std::ifstream ifs;
    ifs.open("../media/info.cgf");
    if(!ifs) return false;
    std::string buf;
    while(getline(ifs,buf)){
        if(buf=="OBJ"){
            glm::vec3 color;
            glm::vec3 position;
            glm::vec3 scale;
            glm::quat rotation;
            getline(ifs,buf);
            int index = std::stoi(buf);getline(ifs,buf);
            _objectlist.objectname.push_back(buf);getline(ifs,buf);
            _objectlist.visible.push_back((bool) std::stoi(buf));getline(ifs,buf);
            _objectlist.color_flag.push_back((bool) std::stoi(buf));getline(ifs,buf);
            _objectlist.TextureIndex.push_back(std::stoi(buf));getline(ifs,buf);
            color.x=std::stof(buf);getline(ifs,buf);
            color.y=std::stof(buf);getline(ifs,buf);
            color.z=std::stof(buf);getline(ifs,buf);
            _objectlist.Color.push_back(color);
            _objectlist.roughness.push_back(std::stof(buf));getline(ifs,buf);
            _objectlist.metallic.push_back(std::stof(buf));getline(ifs,buf);
            _objectlist.ModelList.push_back(nullptr);
            _objectlist.filepath.push_back(buf);
            _objectlist.ModelList[index].reset(new Model(buf));getline(ifs,buf);
            scale.x=std::stof(buf);getline(ifs,buf);
            scale.y=std::stof(buf);getline(ifs,buf);
            scale.z=std::stof(buf);getline(ifs,buf);
            _objectlist.ModelList[index]->scale=scale;
            position.x=std::stof(buf);getline(ifs,buf);
            position.y=std::stof(buf);getline(ifs,buf);
            position.z=std::stof(buf);getline(ifs,buf);
            _objectlist.ModelList[index]->position=position;
            rotation.w=std::stof(buf);getline(ifs,buf);
            rotation.x=std::stof(buf);getline(ifs,buf);
            rotation.y=std::stof(buf);getline(ifs,buf);
            rotation.z=std::stof(buf);
            _objectlist.ModelList[index]->rotation=rotation;
        }
        if(buf=="TEXTURE"){
            getline(ifs,buf);
            int index = std::stoi(buf);getline(ifs,buf);
            _texturelist.texturename.push_back(buf);getline(ifs,buf);
            _texturelist.filepath.push_back(buf);
            _texturelist.texture.push_back(nullptr);
	        _texturelist.texture[index].reset(new Texture2D(_texturelist.filepath[index]));
        }
        if(buf=="SERIES"){
            getline(ifs,buf);
            _serise.max=std::stoi(buf);getline(ifs,buf);
            int size=std::stoi(buf);
            for(int i=0;i<size;i++){
                getline(ifs,buf);
                _serise.sequence.push_back(std::stoi(buf));
            }
        }
    }
    ifs.close();
    return true;
}

void Scene::exportOBJ(const std::vector<Vertex> _vertices,const std::vector<uint32_t> _indices,std::string filename){
    std::ofstream ofs;
    std::string filepath = "../media/" + filename + ".obj";
    ofs.open(filepath,std::ios::out|std::ios::trunc);
    for (int i=0;i<_vertices.size();i++) 
        ofs<<"v "<<_vertices[i].position.x<<" "<<_vertices[i].position.y<<" "<<_vertices[i].position.z<<std::endl;
    for (int i=0;i<_vertices.size();i++) 
        ofs<<"vt "<<_vertices[i].texCoord.x<<" "<<_vertices[i].texCoord.y<<std::endl;
    for (int i=0;i<_vertices.size();i++) 
        ofs<<"vn "<<_vertices[i].normal.x<<" "<<_vertices[i].normal.y<<" "<<_vertices[i].normal.z<<std::endl;
    for (int i=0;i<_indices.size();i=i+3){
        ofs<<"f "<<_indices[i]+1<<"/"<<_indices[i]+1<<"/"<<_indices[i]+1<<" "<<
        _indices[i+1]+1<<"/"<<_indices[i+1]+1<<"/"<<_indices[i+1]+1<<" "<<
        _indices[i+2]+1<<"/"<<_indices[i+2]+1<<"/"<<_indices[i+2]+1<<std::endl;
    }
    ofs.close();
}

void Scene::exportTransOBJ(const std::vector<Vertex> _vertices,const std::vector<uint32_t> _indices,std::string filename,glm::mat4 model){
    std::ofstream ofs;
    std::string filepath = "../media/" + filename + ".obj";
    ofs.open(filepath,std::ios::out|std::ios::trunc);
    for (int i=0;i<_vertices.size();i++) {
        glm::vec3 position=model*glm::vec4(_vertices[i].position,1.0f);
        ofs<<"v "<<position.x<<" "<<position.y<<" "<<position.z<<std::endl;
    }
    for (int i=0;i<_vertices.size();i++) 
        ofs<<"vt "<<_vertices[i].texCoord.x<<" "<<_vertices[i].texCoord.y<<std::endl;
    for (int i=0;i<_vertices.size();i++) {
        glm::vec3 normal=model*glm::vec4(_vertices[i].normal,0.0f);
        ofs<<"vn "<<normal.x<<" "<<normal.y<<" "<<normal.z<<std::endl;
    }
    for (int i=0;i<_indices.size();i=i+3){
        ofs<<"f "<<_indices[i]+1<<"/"<<_indices[i]+1<<"/"<<_indices[i]+1<<" "<<
        _indices[i+1]+1<<"/"<<_indices[i+1]+1<<"/"<<_indices[i+1]+1<<" "<<
        _indices[i+2]+1<<"/"<<_indices[i+2]+1<<"/"<<_indices[i+2]+1<<std::endl;
    }
    ofs.close();
}

void Scene::AddCube(float l,std::string name){
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    glm::vec2 t={0.0,0.0};
    glm::vec3 normalx={1.0,0.0,0.0};
    glm::vec3 normaly={0.0,1.0,0.0};
    glm::vec3 normalz={0.0,0.0,1.0};
    glm::vec3 p1={l,l,l};
    glm::vec3 p2={-l,l,l};
    glm::vec3 p3={-l,-l,l};
    glm::vec3 p4={l,-l,l};
    glm::vec3 p5={l,l,-l};
    glm::vec3 p6={-l,l,-l};
    glm::vec3 p7={-l,-l,-l};
    glm::vec3 p8={l,-l,-l};
    // face z+
    vertices.push_back({p1,normalz,t});vertices.push_back({p2,normalz,t});
    vertices.push_back({p3,normalz,t});vertices.push_back({p4,normalz,t});
    indices.push_back(0);indices.push_back(1);indices.push_back(2);
    indices.push_back(0);indices.push_back(2);indices.push_back(3);
    // face z-
    vertices.push_back({p5,-normalz,t});vertices.push_back({p6,-normalz,t});
    vertices.push_back({p7,-normalz,t});vertices.push_back({p8,-normalz,t});
    indices.push_back(4);indices.push_back(5);indices.push_back(6);
    indices.push_back(4);indices.push_back(6);indices.push_back(7);
    // face x+
    vertices.push_back({p1,normalx,t});vertices.push_back({p5,normalx,t});
    vertices.push_back({p8,normalx,t});vertices.push_back({p4,normalx,t});
    indices.push_back(8);indices.push_back(9);indices.push_back(10);
    indices.push_back(8);indices.push_back(10);indices.push_back(11);
    // face x-
    vertices.push_back({p2,-normalx,t});vertices.push_back({p3,-normalx,t});
    vertices.push_back({p7,-normalx,t});vertices.push_back({p6,-normalx,t});
    indices.push_back(12);indices.push_back(13);indices.push_back(14);
    indices.push_back(12);indices.push_back(14);indices.push_back(15);
    // face y+
    vertices.push_back({p1,normaly,t});vertices.push_back({p2,normaly,t});
    vertices.push_back({p6,normaly,t});vertices.push_back({p5,normaly,t});
    indices.push_back(16);indices.push_back(17);indices.push_back(18);
    indices.push_back(16);indices.push_back(18);indices.push_back(19);
    // face y-
    vertices.push_back({p7,-normaly,t});vertices.push_back({p3,-normaly,t});
    vertices.push_back({p4,-normaly,t});vertices.push_back({p8,-normaly,t});
    indices.push_back(20);indices.push_back(21);indices.push_back(22);
    indices.push_back(20);indices.push_back(22);indices.push_back(23);
    
    std::string path="../media/"+name+".obj";
    _objectlist.filepath.push_back(path);
    _objectlist.ModelList.push_back(nullptr);
    _objectlist.objectname.push_back(name);
    _objectlist.ModelList[_objectlist.ModelList.size()-1].reset(new Model(vertices,indices));
    _objectlist.visible.push_back(true);
    _objectlist.Color.push_back(glm::vec3(1.0));
    _objectlist.TextureIndex.push_back(0);
    _objectlist.color_flag.push_back(true);
    _objectlist.roughness.push_back(0.5f);
    _objectlist.metallic.push_back(1.0f);
    exportOBJ(vertices,indices,name);
}

void Scene::AddCone(float r,float h,std::string name){
    const float PI = 3.14159265359;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    glm::vec3 center={0.0,h/2,0.0};
    glm::vec2 t={0.0,0.0};
    glm::vec3 n={0.0,1.0,0.0};
    const int sides=200;
    float a=2*PI/sides;
    float phi=atan(h/r);
    // 0
    vertices.push_back({-center,-n,t});
    // 1 ... sides
    for (int i=0;i<sides;i++){
        vertices.push_back({glm::vec3{r*cos(a),-h/2,r*sin(a)},-n,t});
        a+=2*PI/sides;
    }
    for (int i = 0; i < sides; i++){
        indices.push_back(0);
        indices.push_back(i + 1);
        if (i + 1 >= sides) indices.push_back(1);
        else indices.push_back(i + 2);
    }
    // sides+1 .. 2*sides
    for (int i=0;i<sides;i++){
        glm::vec3 n={sin(phi)*cos(a),cos(phi),sin(phi)*sin(a)};
        vertices.push_back({glm::vec3{r*cos(a),-h/2,r*sin(a)},n,t});
        a+=2*PI/sides;
    }
    // 2*sides+1 .. 3*sides
    for (int i=0;i<sides;i++){
        glm::vec3 n={sin(phi)*cos(a),cos(phi),sin(phi)*sin(a)};
        vertices.push_back({glm::vec3{0.0,0.0,0.0},n,t});
        a+=2*PI/sides;
    }
    vertices.push_back({center,n,t});
    for (int i = sides+1; i < 2*sides+1; i++){
        if (i+sides+1>=3*sides) indices.push_back(2*sides+1);
        else indices.push_back(i+sides+1);
        indices.push_back(i);
        if (i + 1 >= 2*sides+1) indices.push_back(sides+1);
        else indices.push_back(i + 1);
    }

    std::string path="../media/"+name+".obj";
    _objectlist.filepath.push_back(path);
    _objectlist.ModelList.push_back(nullptr);
    _objectlist.objectname.push_back(name);
    _objectlist.ModelList[_objectlist.ModelList.size()-1].reset(new Model(vertices,indices));
    _objectlist.visible.push_back(true);
    _objectlist.Color.push_back(glm::vec3(1.0));
    _objectlist.TextureIndex.push_back(0);
    _objectlist.color_flag.push_back(true);
    _objectlist.roughness.push_back(0.5f);
    _objectlist.metallic.push_back(1.0f);
    exportOBJ(vertices,indices,name);
}

void Scene::AddCylinder(float r,float h,std::string name){
    const float PI = 3.14159265359;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    glm::vec3 center={0.0,h/2,0.0};
    glm::vec2 t={0.0,0.0};
    glm::vec3 n={0.0,1.0,0.0};
    vertices.push_back({center,n,t});
    const int sides=100;
    float a=2*PI/sides;
    for (int i=0;i<sides;i++){
        vertices.push_back({glm::vec3{r*cos(a),h/2,r*sin(a)},n,t});
        a+=2*PI/sides;
    }
    vertices.push_back({-center,-n,t});
    for (int i=0;i<sides;i++){
        vertices.push_back({glm::vec3{r*cos(a),-h/2,r*sin(a)},-n,t});
        a+=2*PI/sides;
    }
    for (int i = 0; i < sides; i++){
        indices.push_back(0);
        indices.push_back(i + 1);
        if (i + 1 >= sides) indices.push_back(1);
        else indices.push_back(i + 2);
    }
    for (int i = sides + 1; i < 2 * sides + 1; i++){
        indices.push_back(sides + 1);
        if (i + 2 >= 2 * sides + 2) indices.push_back(sides + 2);
        else indices.push_back(i + 2);
        indices.push_back(i + 1);
    }
    // 2*side + 2 ... 3*side + 1
    for (int i=0;i<sides;i++){
        vertices.push_back({glm::vec3{r*cos(a),h/2,r*sin(a)},glm::vec3{cos(a),0.0,sin(a)},t});
        a+=2*PI/sides;
    }
    // 3*side + 2 ... 4*side +1
    for (int i=0;i<sides;i++){
        vertices.push_back({glm::vec3{r*cos(a),-h/2,r*sin(a)},glm::vec3{cos(a),0.0,sin(a)},t});
        a+=2*PI/sides;
    }
    for (int i=2*sides+2;i<3*sides+2;i++){
        indices.push_back(i);
        if(i+1>=3*sides+2) indices.push_back(2*sides+2);
        else indices.push_back(i+1);
        indices.push_back(i+sides);
        if(i+1>=3*sides+2) indices.push_back(2*sides+2);
        else indices.push_back(i+1);
        indices.push_back(i+sides);
        if(i+sides+1>=4*sides+2) indices.push_back(3*sides+2);
        else indices.push_back(i+sides+1);
    }

    std::string path="../media/"+name+".obj";
    _objectlist.filepath.push_back(path);
    _objectlist.ModelList.push_back(nullptr);
    _objectlist.objectname.push_back(name);
    _objectlist.ModelList[_objectlist.ModelList.size()-1].reset(new Model(vertices,indices));
    _objectlist.visible.push_back(true);
    _objectlist.Color.push_back(glm::vec3(1.0));
    _objectlist.TextureIndex.push_back(0);
    _objectlist.color_flag.push_back(true);
    _objectlist.roughness.push_back(0.5f);
    _objectlist.metallic.push_back(1.0f);
    exportOBJ(vertices,indices,name);
}

void Scene::AddFrustum(float r1,float r2,float h,std::string name,int sides){
    const float PI = 3.14159265359;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    glm::vec3 center={0.0,h/2,0.0};
    glm::vec2 t={0.0,0.0};
    glm::vec3 n={0.0,1.0,0.0};
    vertices.push_back({center,n,t});
    float a=2*PI/sides;
    for (int i=0;i<sides;i++){
        vertices.push_back({glm::vec3{r1*cos(a),h/2,r1*sin(a)},n,t});
        a+=2*PI/sides;
    }
    vertices.push_back({-center,-n,t});
    for (int i=0;i<sides;i++){
        vertices.push_back({glm::vec3{r2*cos(a),-h/2,r2*sin(a)},-n,t});
        a+=2*PI/sides;
    }
    for (int i = 0; i < sides; i++){
        indices.push_back(0);
        indices.push_back(i + 1);
        if (i + 1 >= sides) indices.push_back(1);
        else indices.push_back(i + 2);
    }
    for (int i = sides + 1; i < 2 * sides + 1; i++){
        indices.push_back(sides + 1);
        if (i + 2 >= 2 * sides + 2) indices.push_back(sides + 2);
        else indices.push_back(i + 2);
        indices.push_back(i + 1);
    }
    // 2*side + 2 ... 4*side + 1
    for (int i=0;i<sides;i++){
        vertices.push_back({glm::vec3{r1*cos(a),h/2,r1*sin(a)},glm::vec3{cos(a+PI/sides),0.0,sin(a+PI/sides)},t});
        vertices.push_back({glm::vec3{r1*cos(a+2*PI/sides),h/2,r1*sin(a+2*PI/sides)},glm::vec3{cos(a+PI/sides),0.0,sin(a+PI/sides)},t});
        a+=2*PI/sides;
    }
    // 5*side + 2 ... 6*side +1
    for (int i=0;i<sides;i++){
        vertices.push_back({glm::vec3{r2*cos(a),-h/2,r2*sin(a)},glm::vec3{cos(a+PI/sides),0.0,sin(a+PI/sides)},t});
        vertices.push_back({glm::vec3{r2*cos(a+2*PI/sides),-h/2,r2*sin(a+2*PI/sides)},glm::vec3{cos(a+PI/sides),0.0,sin(a+PI/sides)},t});
        a+=2*PI/sides;
    }
    for (int i=2*sides+2;i<4*sides+2;i=i+2){
        indices.push_back(i);
        if(i+1>=4*sides+2) indices.push_back(2*sides+2);
        else indices.push_back(i+1);
        indices.push_back(i+2*sides);
        if(i+1>=4*sides+2) indices.push_back(2*sides+2);
        else indices.push_back(i+1);
        indices.push_back(i+2*sides);
        if(i+2*sides+1>=6*sides+2) indices.push_back(4*sides+2);
        else indices.push_back(i+2*sides+1);
    }

    std::string path="../media/"+name+".obj";
    _objectlist.filepath.push_back(path);
    _objectlist.ModelList.push_back(nullptr);
    _objectlist.objectname.push_back(name);
    _objectlist.ModelList[_objectlist.ModelList.size()-1].reset(new Model(vertices,indices));
    _objectlist.visible.push_back(true);
    _objectlist.Color.push_back(glm::vec3(1.0));
    _objectlist.TextureIndex.push_back(0);
    _objectlist.color_flag.push_back(true);
    _objectlist.roughness.push_back(0.5f);
    _objectlist.metallic.push_back(1.0f);
    exportOBJ(vertices,indices,name);
}

void Scene::AddPrism(float r,float h,std::string name,int sides){
    const float PI = 3.14159265359;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    glm::vec3 center={0.0,h/2,0.0};
    glm::vec2 t={0.0,0.0};
    glm::vec3 n={0.0,1.0,0.0};
    vertices.push_back({center,n,t});
    float a=2*PI/sides;
    for (int i=0;i<sides;i++){
        vertices.push_back({glm::vec3{r*cos(a),h/2,r*sin(a)},n,t});
        a+=2*PI/sides;
    }
    vertices.push_back({-center,-n,t});
    for (int i=0;i<sides;i++){
        vertices.push_back({glm::vec3{r*cos(a),-h/2,r*sin(a)},-n,t});
        a+=2*PI/sides;
    }
    for (int i = 0; i < sides; i++){
        indices.push_back(0);
        indices.push_back(i + 1);
        if (i + 1 >= sides) indices.push_back(1);
        else indices.push_back(i + 2);
    }
    for (int i = sides + 1; i < 2 * sides + 1; i++){
        indices.push_back(sides + 1);
        if (i + 2 >= 2 * sides + 2) indices.push_back(sides + 2);
        else indices.push_back(i + 2);
        indices.push_back(i + 1);
    }
    // 2*side + 2 ... 4*side + 1
    for (int i=0;i<sides;i++){
        vertices.push_back({glm::vec3{r*cos(a),h/2,r*sin(a)},glm::vec3{cos(a+PI/sides),0.0,sin(a+PI/sides)},t});
        vertices.push_back({glm::vec3{r*cos(a+2*PI/sides),h/2,r*sin(a+2*PI/sides)},glm::vec3{cos(a+PI/sides),0.0,sin(a+PI/sides)},t});
        a+=2*PI/sides;
    }
    // 5*side + 2 ... 6*side +1
    for (int i=0;i<sides;i++){
        vertices.push_back({glm::vec3{r*cos(a),-h/2,r*sin(a)},glm::vec3{cos(a+PI/sides),0.0,sin(a+PI/sides)},t});
        vertices.push_back({glm::vec3{r*cos(a+2*PI/sides),-h/2,r*sin(a+2*PI/sides)},glm::vec3{cos(a+PI/sides),0.0,sin(a+PI/sides)},t});
        a+=2*PI/sides;
    }
    for (int i=2*sides+2;i<4*sides+2;i=i+2){
        indices.push_back(i);
        if(i+1>=4*sides+2) indices.push_back(2*sides+2);
        else indices.push_back(i+1);
        indices.push_back(i+2*sides);
        if(i+1>=4*sides+2) indices.push_back(2*sides+2);
        else indices.push_back(i+1);
        indices.push_back(i+2*sides);
        if(i+2*sides+1>=6*sides+2) indices.push_back(4*sides+2);
        else indices.push_back(i+2*sides+1);
    }

    std::string path="../media/"+name+".obj";
    _objectlist.filepath.push_back(path);
    _objectlist.ModelList.push_back(nullptr);
    _objectlist.objectname.push_back(name);
    _objectlist.ModelList[_objectlist.ModelList.size()-1].reset(new Model(vertices,indices));
    _objectlist.visible.push_back(true);
    _objectlist.Color.push_back(glm::vec3(1.0));
    _objectlist.TextureIndex.push_back(0);
    _objectlist.color_flag.push_back(true);
    _objectlist.roughness.push_back(0.5f);
    _objectlist.metallic.push_back(1.0f);
    exportOBJ(vertices,indices,name);
}

void Scene::AddSphere(float r,std::string name){
    const float PI = 3.14159265359;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    const int num=50;
    const float dtheta=PI/num;
    const float dphi=2*PI/num;
    float theta=dtheta;
    float phi=dphi;
    glm::vec2 t{0.0,0.0};
    vertices.push_back({glm::vec3{0.0,r,0.0},glm::vec3{0.0,1.0,0.0},t});
    for(int i=0;i<num;i++){
        for(int j=0;j<num;j++){
            glm::vec3 p={sin(theta)*cos(phi),cos(theta),sin(theta)*sin(phi)};
            glm::vec3 n=normalize(p);
            // t={atan(p.y/p.x)/(2*PI),asin(p.z/r)/PI+0.5};
            vertices.push_back({p,n,t});
            phi+=dphi;
        }
        // first
        if(i==0){
            for(int j=0;j<num;j++){
                indices.push_back(0);
                indices.push_back(j+1);
                indices.push_back((j+2)%num);
            }
        }
        // (i-1)*num+1 .. i*num
        // i*num+1 .. (i+1)*num 
        else{
            for (int j=0;j<num;j++){
                indices.push_back((i-1)*num+j+1);
                indices.push_back((i-1)*num+(j+2)%num);
                indices.push_back(i*num+j+1);
                indices.push_back(i*num+j+1);
                indices.push_back((i-1)*num+(j+2)%num);
                indices.push_back(i*num+(j+2)%num);
            }
        }
        theta+=dtheta;
    }
    std::string path="../media/"+name+".obj";
    _objectlist.filepath.push_back(path);
    _objectlist.ModelList.push_back(nullptr);
    _objectlist.objectname.push_back(name);
    _objectlist.ModelList[_objectlist.ModelList.size()-1].reset(new Model(vertices,indices));
    _objectlist.visible.push_back(true);
    _objectlist.Color.push_back(glm::vec3(1.0));
    _objectlist.TextureIndex.push_back(0);
    _objectlist.color_flag.push_back(true);
    _objectlist.roughness.push_back(0.5f);
    _objectlist.metallic.push_back(1.0f);
    exportOBJ(vertices,indices,name);
}