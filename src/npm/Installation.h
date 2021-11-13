#pragma once

#include <string>

#include "io/json/JsonBinder.h"

#include "git2.h"

namespace sylvanmats::npm{
    
    struct cb_payload{
        std::string publickey;
        std::string privatekey;
        git_credential* gitCredential;
        int cberr=0;
    };

    class Installation{
    protected:
        std::string home;
        std::string cnpmHome;        
    public:
        Installation();
        Installation(const Installation& orig) =  delete;
        virtual ~Installation() = default;
    public:
        void operator()(std::string& packageName);
        void operator()(sylvanmats::io::json::JsonBinder& jb, sylvanmats::io::json::path type);
    protected:
        void install(std::string_view& key, std::string_view& val);
    };
}

