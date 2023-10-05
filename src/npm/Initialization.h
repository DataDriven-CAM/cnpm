#pragma once

#include <string>
#include <map>
#include <unordered_map>

#include "io/json/Binder.h"

namespace sylvanmats::npm{

    class Initialization{
    protected:
        std::string home;
        std::string cnpmHome;
    public:
        Initialization();
        Initialization(const Initialization& orig) =  delete;
        virtual ~Initialization() = default;
    public:
        void operator()(std::string packageName, sylvanmats::io::json::Binder& jsonBinder);
    protected:

    };
}

