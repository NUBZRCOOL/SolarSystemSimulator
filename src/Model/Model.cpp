#include "Model.h"

Model::Model(const char *path) {

    loadModel(path);
}


void Model::Draw(Shader &shader) {
    for (unsigned int i = 0; i < meshes.size(); i++) {
        meshes[i].Draw(shader);
    }
}

void Model::loadModel(std::string path) {

    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices | aiProcess_ImproveCacheLocality | aiProcess_RemoveRedundantMaterials);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cout << "ASSIMP ERROR:" << importer.GetErrorString() << std::endl;
        return;
    }

    std::filesystem::path p(path);
    directory = p.parent_path().string();
    processNodes(scene->mRootNode, scene);
}

void Model::processNodes(aiNode *node, const aiScene *scene) {
    
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNodes(node->mChildren[i], scene);
    }
}

Mesh Model::processMesh(aiMesh *mesh, const aiScene *scene) {
    
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<TextStruct> textures;
    glm::vec3 min(-FLT_MAX);
    glm::vec3 max(FLT_MAX);

    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vert;

        glm::vec3 vector;
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vert.Position = vector;

        if (mesh->mNormals) {
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vert.Normal = vector;
        } else {
            // Calculate a default normal or set to zero
            vert.Normal = glm::vec3(0.0f, 1.0f, 0.0f);  // Default up vector
        }

        if (mesh->mTextureCoords[0]) {
            glm::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vert.TexCoords = vec;
        } else {
            vert.TexCoords = glm::vec2(0.0f);
        }

        vertices.push_back(vert);
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) 
            indices.push_back(face.mIndices[j]);
    }

    if (mesh->mMaterialIndex >= 0) {

        aiMaterial *mat = scene->mMaterials[mesh->mMaterialIndex];
        std::vector<TextStruct> baseColorMap = loadMaterialTextures(mat, aiTextureType_BASE_COLOR, "texture_diffuse", scene);
        textures.insert(textures.end(), baseColorMap.begin(), baseColorMap.end());
        std::vector<TextStruct> diffuseMaps = loadMaterialTextures(mat, aiTextureType_DIFFUSE, "texture_diffuse", scene);
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        std::vector<TextStruct> specularMaps = loadMaterialTextures(mat, aiTextureType_SPECULAR, "texture_specular", scene);
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        std::vector<TextStruct> normalMaps = loadMaterialTextures(mat, aiTextureType_NORMALS, "texture_normal", scene);
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
        std::vector<TextStruct> metallic = loadMaterialTextures(mat, aiTextureType_METALNESS, "texture_metallic", scene);
        textures.insert(textures.end(), metallic.begin(), metallic.end());
    }

    for (auto &v : vertices) {
        min = glm::min(min, v.Position);
        max = glm::max(max, v.Position);
    }

    return Mesh(vertices, indices, textures, min, max);
}

std::vector<TextStruct> Model::loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string name, const aiScene *scene) {

    std::vector<TextStruct> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
        aiString str;
        mat->GetTexture(type, i, &str);
        bool skip = false;
        for (unsigned int j = 0; j < textures_loaded.size(); j++) {
            if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0) {
                textures.push_back(textures_loaded[j]);
                skip = true;
                break;
            }
        }
        if (!skip) {
            TextStruct texture;
            texture.id = TextureFromFile(str.C_Str(), directory, scene);
            texture.type = name;
            texture.path = str.C_Str();
            textures.push_back(texture);
            textures_loaded.push_back(texture);
        }
    }
    return textures;
}

unsigned int Model::TextureFromFile(const char* path, const std::string &directory, const aiScene *scene) {

    unsigned int texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    const aiTexture *embedded = scene->GetEmbeddedTexture(path);

    int width, height, nrChannels;
    unsigned char *data = nullptr;

    if (embedded) {

        if (embedded->mHeight == 0) {
            data = stbi_load_from_memory(
                reinterpret_cast<unsigned char*>(embedded->pcData),
                embedded->mWidth,
                &width, &height, &nrChannels, 0
            );
        } else {
            width = embedded->mWidth;
            height = embedded->mHeight;
            nrChannels = 4;
            data = reinterpret_cast<unsigned char*>(embedded->pcData);
        }
    } else {
        std::string filename = directory + "/" + std::string(path);
        data = stbi_load(filename.c_str(), &width, &height, &nrChannels, 0);
    }

    if (!data) {
        std::cout << "FAILED TO LOAD TEXTURE: " << path << std::endl;
    }

    GLenum format = 
        nrChannels == 1 ? GL_RED : 
        nrChannels == 3 ? GL_RGB : 
                          GL_RGBA;

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (!embedded || embedded->mHeight == 0)
        stbi_image_free(data);

    return texID;
}

glm::vec3 Model::centerOfMeshes() {
    glm::vec3 res(0.0f);
    for (auto &m : meshes) {
        res += (m.aabbMax + m.aabbMin) / 2.0f;
    }
    return res / float(meshes.size());
}