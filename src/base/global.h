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
    ofs.close();
}

void Scene::SceneLoad(){
    std::ifstream ifs;
    ifs.open("../media/info.cgf");
    std::string buf;
    while(getline(ifs,buf)){
        if(buf=="OBJ"){
            glm::vec3 color;
            glm::vec3 position;
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
    }

    ifs.close();
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