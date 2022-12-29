#pragma once

#include <string>
#include <map>
#include <unordered_map>

#include "unordered_map.h"

#include "io/json/Binder.h"

#include "git2.h"

namespace sylvanmats::npm{

    class Addition{
    protected:
    public:
        Addition();
        Addition(const Addition& orig) =  delete;
        virtual ~Addition() = default;
    public:
        bool operator()(sylvanmats::io::json::Binder& jb, sylvanmats::io::json::Path type, std::string packageName);
    };
}


