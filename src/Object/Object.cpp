#include "Object.h"

namespace fs = std::filesystem;

Object::Object(const char *path)
 : model(path), modelMat(glm::mat4(1.0f)), position(glm::vec3(0.0f)), scale(glm::vec3(1.0f)), rotation(glm::vec3(0.0f)), pivot(glm::vec3(0.0f)) {

    this->path = path;

    auto folderPath = fs::path(path).parent_path();

    bool foundObj = false;
    std::string objPath;
    std::string objName;
    bool foundCnfg = false;
    std::string cnfgPath;

    for (const auto &fileEntry : fs::directory_iterator(folderPath)) {
        if (!fileEntry.is_regular_file()) { continue; }

        fs::path file = fileEntry.path();
        if (!foundObj && file.extension().string() == ".glb") {
            objPath = file.string();
            objName = file.filename().string();
            foundObj = true;
        } else if (!foundCnfg && file.filename().string() == "objconfig") {
            cnfgPath = file.string();
            foundCnfg = true;
        }
    }

    if (!foundObj) {
        std::cout << "No obj in: " << folderPath << std::endl;
        return;
    }
    if (!foundCnfg) {
        std::cout << "COULDN'T FIND CONFIG FOR: " << objPath << std::endl;
        return;
    }

    std::ifstream shaderCnfg(cnfgPath);
    if (!shaderCnfg) {
        std::cout << "FAILED TO OPEN SHADER CONFIG: " << cnfgPath << std::endl;
        return;
    }

    // parse the 1 or 0 bit at the top
    std::string line;
    int yesOrNo = 0;
    if (!std::getline(shaderCnfg, line)) {
        std::cerr << "INVALID OBJ INIT FORMAT (BEGINNING BIT) IN: " << cnfgPath << std::endl;
        return;
    }
    std::istringstream firstBit(line);
    if (!(firstBit >> yesOrNo)) {
        std::cerr << "ERROR PARSING BIT IN: " << cnfgPath << std::endl;
        return;
    }
    if (!yesOrNo) { return; }

    // parse the first 9 flaots
    glm::vec3 pos, scale, rot;
    std::vector<glm::vec3*> objInit = {&pos, &scale, &rot};
    bool parseError = false;

    for (int i = 0; i < 3; i++) {
        if (!std::getline(shaderCnfg, line)) {
            std::cerr << "INVALID OBJ INIT FORMAT AT LINE: " << i << "IN: " << cnfgPath << std::endl;
            parseError = true;
            break;
        }

        std::istringstream iss(line);

        if (!(iss >> objInit[i]->x >> objInit[i]->y >> objInit[i]->z)) {
            std::cerr << "ERROR PARSING FLOATS AT LINE: " << i << "IN: " << cnfgPath << std::endl;
            parseError = true;
            break;
        }
    }
    if (parseError) {
        shaderCnfg.close();
        return;
    }

    // parse the sahder paths
    std::string vertPath;
    std::string fragPath;
    if (!std::getline(shaderCnfg, vertPath) || !std::getline(shaderCnfg, fragPath)) {
        std::cout << "INVALID STRING FORMAT FOR CONFIG IN: " << cnfgPath << std::endl;
        return;
    }

    auto shade = std::make_shared<Shader>(vertPath.c_str(), fragPath.c_str());
    this->name = objName.c_str();
    this->shader = shade;
    this->setPosition(pos);
    this->setScale(scale);
    this->setRotation(rot);

    setOrigin(model.centerOfMeshes());
    updateShader();
}

void Object::Draw(glm::mat4 &view, glm::mat4 &proj) {
    updateShader();
    shader->setMat4("view", view);
    shader->setMat4("proj", proj);
    model.Draw(*shader);
}

void Object::updateShader() {
    shader->use();
    shader->setMat4("model", modelMat);
}

void Object::updateModelMatrix() {

    glm::mat4 translate = glm::translate(glm::mat4(1.0f), position);
    
    glm::mat4 rotationX = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 rotationY = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 rotationZ = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    
    glm::mat4 scaling = glm::scale(glm::mat4(1.0f), scale);

    glm::mat4 toPivot = glm::translate(glm::mat4(1.0f), -pivot);
    glm::mat4 fromPivot = glm::translate(glm::mat4(1.0f), pivot);

    modelMat = translate * fromPivot * rotationZ * rotationY * rotationX * scaling * toPivot;
}

std::string Object::getName() { return name; }

glm::vec3 Object::getPosition() { return position; }

glm::vec3 Object::getScale() { return scale; }

glm::vec3 Object::getRotationEuler() { return rotation; }


void Object::setName(const char *name) { this->name = name; }

void Object::setOrigin(glm::vec3 pos) {
    pivot = pos;
}

void Object::setPosition(glm::vec3 vector) {
    position = vector;
    updateModelMatrix();
    updateShader();
}

void Object::setScale(glm::vec3 vector) {
    scale = vector;
    updateModelMatrix();
    updateShader();
}

void Object::setRotation(glm::vec3 vector) {
    rotation = vector;
    rotation.x = fmod(rotation.x, 360.0f);
    rotation.y = fmod(rotation.y, 360.0f);
    rotation.z = fmod(rotation.z, 360.0f);
    if (rotation.x < 0) rotation.x += 360.0f;
    if (rotation.y < 0) rotation.y += 360.0f;
    if (rotation.z < 0) rotation.z += 360.0f;
    updateModelMatrix();
    updateShader();
}