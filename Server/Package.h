#pragma once

#include <string>

#include "Typedefs.h"

namespace AsyncServer
{
    struct Package
    {
        unsigned int m_dataSize;
        bool m_isScene;
        RayTracer::Scene m_sceneObject;
        unsigned int m_xPoz;
        unsigned int m_yPoz;

        template <typename Archive>
        void serialize(Archive& ar, const unsigned int version)
        {
            ar & m_dataSize;
            ar & m_sceneObject;
            ar & m_isScene;
            ar & m_xPoz;
            ar & m_yPoz;
        }
    };
}
