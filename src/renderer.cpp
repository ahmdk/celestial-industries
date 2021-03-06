#include "renderer.hpp"
#include <glm/ext.hpp>

// used to define directional light for use in the shader
const glm::vec3 Renderer::directionalLight = glm::vec3(0.49, 0.79, 0.49);

Renderer::Renderer(
    std::shared_ptr<Shader> initShader,
    std::vector<SubObjectSource> subObjectSources
)
{
    shader = initShader;
    for (auto source : subObjectSources) {
        subObjects.push_back(loadSubObject(source));
    }

    instanceDataAttribute = glGetUniformBlockIndex(shader->program, "InstancesData");
    glGenBuffers(1, &instancesDataBuffer);
    glBindBuffer(GL_UNIFORM_BUFFER, instancesDataBuffer);
    glBindBufferBase(GL_UNIFORM_BUFFER, 2, instancesDataBuffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(ShaderInstancesData), NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    instancesData.stride = subObjects.size();
}

SubObject Renderer::loadSubObject(SubObjectSource source)
{
    // Clearing errors
    gl_flush_errors();

    // Getting uniform locations for glUniform* calls
    viewProjectionUniform = glGetUniformLocation(shader->program, "vp");
    modelIndexUniform = glGetUniformLocation(shader->program, "modelIndex");
    materialUniformBlock = glGetUniformBlockIndex(shader->program, "MaterialInfo");
	viewMatrixUniform = glGetUniformLocation(shader->program, "viewMatrix");
	directionalLightUniform = glGetUniformLocation(shader->program, "directionalLight");	

    // Getting attribute locations
    positionAttribute = glGetAttribLocation(shader->program, "in_position");
    texcoordAttribute = glGetAttribLocation(shader->program, "in_texcoord");
    normalAttribute = glGetAttribLocation(shader->program, "in_normal");

    OBJ::Data obj;
    std::string path = pathBuilder({ "data", "models" });
    if (!OBJ::Loader::loadOBJ(path, source.filename, obj)) {
        // Failure message should already be handled by loadOBJ
        throw "Failed to load subobject";
    }
    GLuint vbo_id;
    // Vertex Buffer creation
    glGenBuffers(1, &vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
    glBufferData(GL_ARRAY_BUFFER, obj.data.size() * sizeof(OBJ::VertexData), obj.data.data(), GL_STATIC_DRAW);

    auto meshes = std::make_shared<std::vector<Mesh>>();
    for (auto group : obj.groups) {
        Mesh mesh;
        // Vertex Array (Container for Vertex + Index buffer)
        glGenVertexArrays(1, &mesh.vao);
        glBindVertexArray(mesh.vao);

        glBindBuffer(GL_ARRAY_BUFFER, vbo_id);

        mesh.vbo = vbo_id;
        // Index Buffer creation
        glGenBuffers(1, &mesh.ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, group.indices.size() * sizeof(unsigned int), group.indices.data(), GL_STATIC_DRAW);

		if (gl_has_errors()) {
			logger(LogLevel::ERR) << "Encountered GL error while making subobject " << '\n';
			throw "Encountered GL error while making subobject";
		}

        mesh.numIndices = group.indices.size();
        mesh.material = group.material;

        // Uniform block
        glGenBuffers(1, &mesh.ubo);
        glBindBuffer(GL_UNIFORM_BUFFER, mesh.ubo);

        ShaderMaterialData material = {
            glm::vec4(group.material.ambient, 1.0),
            glm::vec4(group.material.diffuse, 1.0),
            glm::vec4(group.material.specular, 1.0),
            group.material.hasDiffuseMap,
            false,
            false,
            false
        };

        glBindBufferBase(GL_UNIFORM_BUFFER, 1, mesh.ubo);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(ShaderMaterialData), &material, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        // Input data location as in the vertex buffer
        glEnableVertexAttribArray(positionAttribute);
        glVertexAttribPointer(positionAttribute, 3, GL_FLOAT, GL_FALSE, sizeof(OBJ::VertexData), (void*)0); // (void*)0 because we render from the start of the stride

        glEnableVertexAttribArray(texcoordAttribute);
        glVertexAttribPointer(texcoordAttribute, 2, GL_FLOAT, GL_FALSE, sizeof(OBJ::VertexData), (void*)sizeof(glm::vec3));

        glEnableVertexAttribArray(normalAttribute);
        glVertexAttribPointer(normalAttribute, 3, GL_FLOAT, GL_FALSE, sizeof(OBJ::VertexData), (void*)(sizeof(glm::vec3) + sizeof(glm::vec2)));

        meshes->push_back(mesh);
    }
    return {
        meshes,
        source.parentMesh
    };
}

void Renderer::deleteInstance(unsigned int id)
{
	instances[id].shouldDraw = false;
	for (size_t i = 0; i < subObjects.size(); i++) {
		instancesData.modelMatrices[subObjects.size()*id + i] = glm::mat4(0.0f); // Hacky way to make the whole object just a dimensionless point
	}
	graveyardIdStack.push_back(id);
}

unsigned int Renderer::getNextId()
{
	if (!graveyardIdStack.empty()) {
		int id = graveyardIdStack.back();
		graveyardIdStack.pop_back();
		instances[id].shouldDraw = true;
		for (size_t i = 0; i < subObjects.size(); i++) {
			instancesData.modelMatrices[subObjects.size()*id + i] = glm::mat4(1.0f); // Unhack the element
		}
		return id;
	}
    if (instances.size() + 1 > maxInstances / subObjects.size()) {
        throw "Too many instances! Increase maxInstances if you want to do this";
    }
    return (unsigned int)instances.size();
}

glm::mat4 Renderer::collapseMatrixVector(std::vector<glm::mat4> v)
{
    glm::mat4 result = glm::mat4(1.0f);
    for (auto it = v.rbegin(); it != v.rend(); ++it)
        result *= (*it);
    return result;
}

void Renderer::render(glm::mat4 &viewProjection, glm::mat4 &view)
{
    // Setting shaders
    glUseProgram(shader->program);

    glBindBuffer(GL_UNIFORM_BUFFER, instancesDataBuffer);
    glUniformBlockBinding(shader->program, instanceDataAttribute, 2); // layout hardcoded in shader
    glBindBufferBase(GL_UNIFORM_BUFFER, 2, instancesDataBuffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(ShaderInstancesData), &instancesData, GL_DYNAMIC_DRAW);

    glUniformMatrix4fv(viewProjectionUniform, 1, GL_FALSE, &viewProjection[0][0]);
	glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, &view[0][0]);
	glUniform3fv(directionalLightUniform, 1, &directionalLight[0]);

    for (size_t i = 0; i < subObjects.size(); i++) {
        for (const Mesh& mesh : subObjects[i].meshes) {
            // Setting vertices and indices
            glBindVertexArray(mesh.vao);

            glBindBuffer(GL_UNIFORM_BUFFER, mesh.ubo);
            glUniformBlockBinding(shader->program, materialUniformBlock, 1); // layout hardcoded in shader
            glBindBufferBase(GL_UNIFORM_BUFFER, 1, mesh.ubo);

            if (mesh.material.hasDiffuseMap) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, mesh.material.diffuseMap->id);
            }

            glUniform1i(modelIndexUniform, i);

            glDrawElementsInstanced(GL_TRIANGLES, mesh.numIndices, GL_UNSIGNED_INT, nullptr, instances.size());
        }
    }
}

void Renderer::updateModelMatrixStack(unsigned int instanceIndex, bool updateHierarchically)
{
	if (instances[instanceIndex].shouldDraw) {
		for (size_t i = 0; i < subObjects.size(); i++) {
			if (updateHierarchically) {
				std::vector<glm::mat4> modelMatrices;
				int modelIndex = i;
				while (modelIndex != -1) {
					modelMatrices.push_back(instances[instanceIndex].matrixStack[modelIndex]);
					modelIndex = subObjects[modelIndex].parentMesh;
				}
				instancesData.modelMatrices[instanceIndex*subObjects.size() + i] = collapseMatrixVector(modelMatrices);
			}
			else {
				instancesData.modelMatrices[instanceIndex*subObjects.size() + i] = instances[instanceIndex].matrixStack[i];
			}
		}
	}
}

glm::mat4 Renderer::getModelMatrix(unsigned int id, unsigned int modelIndex)
{
    return instancesData.modelMatrices[id*instancesData.stride + modelIndex];
}

glm::vec3 Renderer::applyMatricesToVec(glm::vec3 v, unsigned int id, unsigned int modelIndex)
{
	return instancesData.modelMatrices[id*instancesData.stride + modelIndex]*glm::vec4(v, 1.0);
}

Renderable::Renderable() {}

Renderable::Renderable(std::shared_ptr<Renderer> initParent)
{
    parent = initParent;
    id = parent->getNextId();
    std::vector<glm::mat4> matrixStack;
    for (size_t i = 0; i < parent->subObjects.size(); i++) {
        matrixStack.push_back(glm::mat4(1.0f));
    }
	if (id == parent->instances.size()) {
		parent->instances.push_back({
			true,
			matrixStack,
			});
	}
	else {
		parent->instances[id].matrixStack = matrixStack;
	}
}

void Renderable::shouldUpdate(bool val)
{
    parent->instances[id].shouldDraw = val;
}

void Renderable::translate(glm::vec3 translation, bool updateHierarchically) {
    translate(0, translation);
    parent->updateModelMatrixStack(id, updateHierarchically);
}

void Renderable::rotate(float amount, glm::vec3 axis, bool updateHierarchically)
{
    rotate(0, amount, axis);
    parent->updateModelMatrixStack(id, updateHierarchically);
}

void Renderable::scale(glm::vec3 s, bool updateHierarchically)
{
    scale(0, s);
    parent->updateModelMatrixStack(id, updateHierarchically);
}

void Renderable::removeSelf()
{
	parent->deleteInstance(id);
}

/*
    Sets the individual model matrices for the 
*/
void Renderable::setModelMatricesFromComputed()
{
    for (size_t modelIndex = 0; modelIndex < parent->subObjects.size(); modelIndex++)
        parent->instances[id].matrixStack[modelIndex] = parent->getModelMatrix(id, modelIndex);
}

void Renderable::translate(int modelIndex, glm::vec3 translation, bool updateHierarchically)
{
    parent->instances[id].matrixStack[modelIndex] = glm::translate(parent->instances[id].matrixStack[modelIndex], translation);
    parent->updateModelMatrixStack(id, updateHierarchically);
}

void Renderable::rotate(int modelIndex, float amount, glm::vec3 axis, bool updateHierarchically)
{
    parent->instances[id].matrixStack[modelIndex] = glm::rotate(parent->instances[id].matrixStack[modelIndex], amount, axis);
    parent->updateModelMatrixStack(id, updateHierarchically);
}

void Renderable::scale(int modelIndex, glm::vec3 scale, bool updateHierarchically)
{

    parent->instances[id].matrixStack[modelIndex] = glm::scale(parent->instances[id].matrixStack[modelIndex], scale);
    parent->updateModelMatrixStack(id, updateHierarchically);
}

void Renderable::setModelMatrix(int modelIndex, glm::mat4 mat, bool updateHierarchically)
{
    parent->instances[id].matrixStack[modelIndex] = mat;
    parent->updateModelMatrixStack(id, updateHierarchically);
}

void Renderable::setModelMatrix(int modelIndex, glm::vec3 translation, float angle, glm::vec3 rotationAxis, glm::vec3 scale, bool updateHierarchically)
{
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::scale(model, scale);
    model = glm::rotate(model, angle, rotationAxis);
    model = glm::translate(model, translation);
    parent->instances[id].matrixStack[modelIndex] = model;
    parent->updateModelMatrixStack(id, updateHierarchically);
}

bool Renderable::operator==(const Renderable& rhs) const {
	return parent == rhs.parent &&
		   id == rhs.id;
}
