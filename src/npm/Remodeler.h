#pragma once

#include <string>
#include <map>
#include <unordered_map>

#include "unordered_map.h"

#include "io/json/Binder.h"

#include "git2.h"

namespace sylvanmats::npm{

    class Remodeler{
    protected:
    public:
        Remodeler();
        Remodeler(const Remodeler& orig) =  delete;
        virtual ~Remodeler() = default;
    public:
        bool operator()(sylvanmats::io::json::Binder& jb, sylvanmats::io::json::Path type, std::string packageName);
    };
}

