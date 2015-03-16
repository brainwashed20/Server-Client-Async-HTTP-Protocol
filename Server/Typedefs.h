#pragma once

#include "Includes.h"

namespace RayTracer
{
    class Scene
    {
    public:
        Scene() : m_testString("test"), m_testInt(10)
        {
        }
        std::string m_testString;
        int m_testInt;

        template <typename Archive>
        void serialize(Archive& ar, const unsigned int version)
        {
            ar & m_testString;
            ar & m_testInt;
        }
    };
}

