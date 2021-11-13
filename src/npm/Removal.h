#pragma once

#include <string>
#include <map>
#include <unordered_map>

#include "unordered_map.h"

#include "io/json/JsonBinder.h"

#include "git2.h"

namespace sylvanmats::npm{

    class Removal{
    protected:
    public:
        Removal();
        Removal(const Removal& orig) =  delete;
        virtual ~Removal() = default;
    public:
        bool operator()(sylvanmats::io::json::JsonBinder& jb, sylvanmats::io::json::path type, std::string packageName);
    };
}



