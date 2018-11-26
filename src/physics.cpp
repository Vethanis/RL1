#include "physics.h"

#include "macro.h"
#include "gen_array.h"
#include "blockalloc.h"
#include "transform.h"

btDefaultCollisionConfiguration     ms_collisionConfig;
btCollisionDispatcher               ms_dispatcher(&ms_collisionConfig);
btDbvtBroadphase                    ms_broadphase;
btSequentialImpulseConstraintSolver ms_solver;
btDiscreteDynamicsWorld             ms_world(
                                        &ms_dispatcher, 
                                        &ms_broadphase, 
                                        &ms_solver, 
                                        &ms_collisionConfig);

TBlockAlloc<btBoxShape> ms_shapes;
TBlockAlloc<btRigidBody> ms_bodies;
TBlockAlloc<btDefaultMotionState> ms_motionStates;

namespace Physics
{
    void Init()
    {
        ms_world.setGravity(btVector3(0.0f, -9.81f, 0.0f));
    }
    void Update(float dt)
    {
        ms_world.stepSimulation(dt);

        for(const slot* s = Components::begin(); s != Components::end(); ++s)
        {
            PhysicsComponent* pc = Components::Get<PhysicsComponent>(*s);
            TransformComponent* tc = Components::Get<TransformComponent>(*s);
            if(!pc | !tc)
            {
                continue;
            }

            tc->m_matrix = pc->GetTransform();
        }
    }
    btRigidBody* Create(float mass, const vec3& position, const vec3& extent)
    {
        btBoxShape* shape = ms_shapes.Alloc();
        new (shape) btBoxShape(toB3(extent));

        btRigidBody* body = ms_bodies.Alloc();
        btVector3 inertia(0.0f, 0.0f, 0.0f);
        if(mass != 0.0f)
        {
            shape->calculateLocalInertia(mass, inertia);
        }
        btTransform xform;
        xform.setIdentity();
        xform.setOrigin(toB3(position));
        btDefaultMotionState* state = ms_motionStates.Alloc();
        new (state) btDefaultMotionState(xform);
        new (body) btRigidBody(mass, state, shape, inertia);
        ms_world.addRigidBody(body);
        return body;
    }
    void Destroy(btRigidBody* body)
    {
        if(body)
        {
            btCollisionShape* shape = body->getCollisionShape();
            btMotionState* state = body->getMotionState();
            ms_world.removeRigidBody(body);
            ms_shapes.Free(static_cast<btBoxShape*>(shape));
            ms_bodies.Free(body);
            ms_motionStates.Free(static_cast<btDefaultMotionState*>(state));
        }
    }
};
