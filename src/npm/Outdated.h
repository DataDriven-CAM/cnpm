#pragma once

#include <string>

#include "io/json/Binder.h"

#include "git2.h"

namespace sylvanmats::npm{
    
    class Outdated{
    protected:
        std::string home;
        std::string cnpmHome;        
    public:
        Outdated();
        Outdated(const Outdated& orig) =  delete;
        virtual ~Outdated() = default;
    public:
        void operator()(sylvanmats::io::json::Binder& jb, sylvanmats::io::json::Path type);
    };
}


