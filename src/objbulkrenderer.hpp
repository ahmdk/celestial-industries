#pragma once
#include "common.hpp"

struct SubObject {
    std::shared_ptr<std::vector<Mesh>> meshes;
    int parentMesh;
};

struct CompositeObjectData {
    bool shouldDraw;
    std::vector<glm::mat4> matrixStack; // Must be of identical length as subobject's meshes. enforced in constructor.
};

struct SubObjectSource {
    std::string filename;
    int parentMesh;
};

class CompositeObjectBulkRenderer {
    std::shared_ptr<Shader> shader;
    glm::mat4 collapseMatrixVector(std::vector<glm::mat4> v);
public:
    std::vector<SubObject> subObjects;
    std::vector<CompositeObjectData> objects;
    CompositeObjectBulkRenderer(
        std::shared_ptr<Shader> initShader,
        std::vector<SubObject> initSubobjects
    );
    void addSubOjbect(SubObject subObject);
    unsigned int getNextId();
    void render(glm::mat4 viewProjection);
};

class CompositeObjectBulkRenderable { // Note: does NOT extend Renderable. This is intentional as it is not compatible with Renderable
private:
    std::shared_ptr<CompositeObjectBulkRenderer> parent;
    unsigned int id;
public:
    CompositeObjectBulkRenderable(std::shared_ptr<CompositeObjectBulkRenderer> initParent);
    void shouldDraw(bool val);

    void translate(int modelIndex, glm::vec3 translation);
    void rotate(int modelIndex, float amount, glm::vec3 axis);
    void scale(int modelIndex, glm::vec3 scale);
    void setModelMatrix(int modelIndex, glm::mat4 mat);
    void setModelMatrix(int modelIndex, glm::vec3 translation = { 0,0,0 }, float angle = 0, glm::vec3 rotationAxis = { 0,1,0 }, glm::vec3 scale = { 1,1,1 });

    // When subobject modelIndex is not provided it is assumed you wish to apply the transformation to the whole model
    void translate(glm::vec3 translation);
    void rotate(float amount, glm::vec3 axis);
    void scale(glm::vec3 scale);

    virtual ~CompositeObjectBulkRenderable() {};
};