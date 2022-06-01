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