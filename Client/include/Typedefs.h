#pragma once

#include "Includes.h"

namespace AsyncClient
{
    typedef std::shared_ptr<std::vector<char> > MessagePtr;
    typedef std::queue<MessagePtr> MessageQueue;
}
