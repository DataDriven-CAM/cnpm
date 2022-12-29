#pragma once

#include <string>
#include <tuple>
#include <filesystem>

#include "io/json/Binder.h"

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
        std::string moduleDirectory;
        sylvanmats::io::json::Path type;
        std::string home;
        std::string cnpmHome;
        unsigned int depth=0;
    public:
        Installation(std::string& moduleDirectory, sylvanmats::io::json::Path type);
        Installation(const Installation& orig) =  delete;
        virtual ~Installation() = default;
    public:
        void operator()(std::string& packageName);
        void operator()(sylvanmats::io::json::Binder& jb);
    protected:
        void install(std::string_view& key, std::string_view& val);
        void recurseModules(std::filesystem::path localLinkPath);
        void linkAnyBinaries(sylvanmats::io::json::Binder& jb, std::filesystem::path& localLinkPath);
        inline std::tuple<std::string, std::string> parseModuleName(std::string_view& key){
            std::string moduleName=std::string{key};
            std::string scope="";
            if(moduleName.length()>0 && moduleName.at(0)=='@'){
                unsigned int offset=moduleName.find('/');
                if(offset!=std::string::npos){
                    scope=moduleName.substr(1, offset-1);
                    moduleName=moduleName.substr(offset+1);
                }
            }
            return std::make_tuple(scope, moduleName);
        };
    };
}

