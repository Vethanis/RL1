#include "physics.h"

#include "macro.h"
#include "gen_array.h"
#include "blockalloc.h"
#include "rendercomponent.h"

static btDefaultCollisionConfiguration     ms_collisionConfig;
static btCollisionDispatcher               ms_dispatcher = btCollisionDispatcher(
                                        &ms_collisionConfig);
static btDbvtBroadphase                    ms_broadphase;
static btSequentialImpulseConstraintSolver ms_solver;
static btDiscreteDynamicsWorld             ms_world = btDiscreteDynamicsWorld(
                                        &ms_dispatcher, 
                                        &ms_broadphase, 
                                        &ms_solver, 
                                        &ms_collisionConfig);

static TBlockAlloc<btBoxShape>             ms_shapes;
static TBlockAlloc<btRigidBody>            ms_bodies;
static TBlockAlloc<btDefaultMotionState>   ms_motionStates;

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
            RenderComponent* rc = Components::Get<RenderComponent>(*s);
            if(!pc | !rc)
            {
                continue;
            }

            rc->m_matrix = pc->GetTransform();
        }
    }
    void Shutdown()
    {
        // zero out ms_world so it doesnt try to delete addresses in .bss region
        // THIS WILL LEAK, only use when you are REALLY shutting down!
        // Bullet design demands operator new, which I dislike
        memset(&ms_world, 0, sizeof(ms_world));
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
