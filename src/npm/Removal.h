#pragma once

#include <string>
#include <map>
#include <unordered_map>

#include "unordered_map.h"

#include "io/json/Binder.h"

#include "git2.h"

namespace sylvanmats::npm{

    class Removal{
    protected:
    public:
        Removal();
        Removal(const Removal& orig) =  delete;
        virtual ~Removal() = default;
    public:
        bool operator()(sylvanmats::io::json::Binder& jb, sylvanmats::io::json::Path type, std::string packageName);
    };
}



