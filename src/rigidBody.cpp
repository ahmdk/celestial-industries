#include "rigidBody.hpp"

void RigidBody::setVelocity(glm::vec3 _velocity)
{
    this->velocity = _velocity;
}

void RigidBody::setGravity(glm::vec3 _gravity)
{
    this->gravity = _gravity;
}

void RigidBody::setForce(glm::vec3 _force)
{
    this->appliedForce = _force;
}

void RigidBody::setGeometryId(int _id)
{
    this->geometryId = _id;
}

void RigidBody::setRotation(float angle, glm::vec3 axis)
{
    // if rotation angle is about x axis
    if (axis.x != 0.0f && axis.y == 0.0f && axis.z == 0.0f) {
        this->rotation.x += angle;
        return;
    }
    // if rotation angle is about y axis
    if (axis.x == 0.0f && axis.y != 0.0f && axis.z == 0.0f) {
        this->rotation.y += angle;
        return;
    }
    // if rotation angle is about z axis
    if (axis.x == 0.0f && axis.y == 0.0f && axis.z != 0.0f) {
        this->rotation.z += angle;
        return;
    }
}

void RigidBody::setPosition(glm::vec3 _pos)
{
    this->position = _pos;
}

void RigidBody::setInverseMass(float invMass)
{
    this->inverseMass = invMass;
}

void RigidBody::setMass(float _mass)
{
    if (_mass != 0) {
        this->inverseMass = 1.0 / _mass;
    }
}

void RigidBody::setDampingFactor(float damping)
{
    this->dampingFactor = damping;
}

glm::vec3 RigidBody::getPosition()
{
    return position;
}

float RigidBody::getRotation(glm::vec3 _axis)
{
    // if rotation angle is about x axis
    if (_axis.x != 0.0f && _axis.y == 0.0f && _axis.z == 0.0f) {
        return rotation.x;
    }
    // if rotation angle is about y axis
    if (_axis.x == 0.0f && _axis.y != 0.0f && _axis.z == 0.0f) {
        return rotation.y;
    }
    // if rotation angle is about z axis
    if (_axis.x == 0.0f && _axis.y == 0.0f && _axis.z != 0.0f) {
        return rotation.z;
    }
}

float RigidBody::getInverseMass()
{
    return this->inverseMass;
}

float RigidBody::getMass()
{
    if (this->inverseMass == 0) {

    }
    
}

float RigidBody::getDampingFactor()
{
    return this->dampingFactor;
}

glm::vec3 RigidBody::getForce()
{
    return this->appliedForce;
}

void RigidBody::setCollisionGeometryType(CollisionGeomType _cg)
{
    this->cgType = _cg;
}

CollisionGeomType RigidBody::getCollisionGeometryType()
{
    return this->cgType;
}

glm::vec3 RigidBody::getVelocity()
{
    return this->velocity;
}

long RigidBody::getGeometryId()
{
    return this->geometryId;
}