#pragma once

#include <string>
#include <map>
#include <unordered_map>

#include "io/json/Binder.h"

namespace sylvanmats::npm{

    class Initialization{
    protected:
    public:
        Initialization();
        Initialization(const Initialization& orig) =  delete;
        virtual ~Initialization() = default;
    public:
        sylvanmats::io::json::Binder operator()(std::string packageName);
    protected:

    };
}

