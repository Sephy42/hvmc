#include "hvmc_physics.h"

//void RigidBody::Update( f32 dt )
//{
//    vec2 a = im * forces;
//    velocity += dt * a;
//    position += velocity * dt;
//}

bool Collide( RigidBody* a, RigidBody* b, CollisionInfo& info ) ;

void RigidBody::ApplyForce( vec2 const& f )
{
    forces += f;
}

void RigidBody::ResetForces()
{
    forces = { 0.f, 0.f };
    torque = 0.f;
}

void RigidBody::IntegrateForces( f32 dt )
{
    velocity += (forces * im) * dt;
    angularVelocity += (torque * iI) * dt;
}

void RigidBody::IntegrateVelocities( f32 dt )
{
    previousPosition = position;
    position += velocity * dt;
    rotation += angularVelocity * dt;
}

void RigidBody::ApplyImpulse( vec2 const& impulse, vec2 const& contactVector )
{
    velocity += impulse * im;
    angularVelocity += Cross(contactVector, impulse * iI);
}

void RigidBody::SetKinematic()
{
    I = iI = m = im = torque = rotation = angularVelocity = 0.f;
    forces = velocity = vec2{0.f, 0.f};
}

bool PhysicsSystem::Init()
{
    gravity = vec2{ 0.f, -9.81f };
    return true;
}

void PhysicsSystem::Cleanup()
{
    rigidBodies.clear();
}

RigidBody* PhysicsSystem::AddSphere( vec2 const& pos, f32 radius )
{
    RigidBody* body = new RigidBody;
    
    body->forces = { 0.f, 0.f };
    body->im = 1.f; // 1 kg
    body->m = 1.f;
    body->iI = 1.f;
    body->I = 1.f;
    body->position = body->previousPosition = pos;
    body->velocity = { 0.f, 0.f };

    body->collider.type = RIGID_BODY_SPHERE;
    body->collider.radius = radius;

    rigidBodies.push_back( body );
    return body;
}

RigidBody* PhysicsSystem::AddBox( vec2 const& pos, vec2 const& dims )
{
    RigidBody* body = new RigidBody; 
    
    body->forces = { 0.f, 0.f };
    body->im = 1.f; // 1 kg
    body->m = 1.f;
    body->iI = 1.f;
    body->I = 1.f;
    body->position = body->previousPosition = pos;
    body->velocity = { 0.f, 0.f };
    
    body->collider.type = RIGID_BODY_BOX;
    body->collider.dims = dims;

    rigidBodies.push_back( body );
    return body;
}

RigidBody* PhysicsSystem::AddWall( vec2 const& pos, vec2 const& dims )
{
    RigidBody* body = new RigidBody;

    body->im = 0.f;
    body->position = body->previousPosition = pos;
	body->angularVelocity = 0.f;
    body->velocity = { 0.f, 0.f };
    body->collider.type = RIGID_BODY_BOX;
    body->collider.dims = dims;

    rigidBodies.push_back( body );
    return body;
}



void PhysicsSystem::Update( f32 dt ) {

    // Add gravity
    for ( auto& rb : rigidBodies ) rb->forces += rb->m * gravity;

    std::vector<CollisionInfo> collisions;
    u32 count = rigidBodies.size();

    // Generate contact infos
    // NOTE: This double loop prevents testing
    // an object against another multiple times
    for ( u32 i = 0; i < count - 1; ++i )
    {
        for ( u32 j = i + 1; j < count; ++j )
        {
            RigidBody* a = rigidBodies[i];
            RigidBody* b = rigidBodies[j];
            CollisionInfo info;

            // Pour du débug (affichage à partir du premier élément non mural uniquement)
            info.debug = j > 3;

            // Test collisions, add to list if colliding
            if ( Collide( a, b, info ) )
            {
                collisions.push_back( info );
            }
        }
    }

    // Solve contacts
    for ( auto const& collision : collisions )
        collision.Solve();

    // Integrate forces
    for ( auto& rb : rigidBodies )
        rb->IntegrateForces( dt );

    // Integrate velocities
    for ( auto& rb : rigidBodies )
        rb->IntegrateVelocities( dt );

    // Position corrections
    for ( auto const& c : collisions )
        c.PositionCorrection();

    // Clear forces
    for ( auto& rb : rigidBodies )
    {
        rb->forces = vec2{ 0.f, 0.f };
        rb->torque = 0.f;
    }

}
