#pragma once
#include "ECS/EntityDatabase.h"
#include "ECS/Contextual/Components/Transform.h"

namespace PK::ECS::EntityViews
{
    struct LightSphereView : public IEntityView
    {
        Components::Transform* transformMesh;
        Components::Transform* transformLight;
    };
}