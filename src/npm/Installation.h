#pragma once

#include <string>
#include <tuple>
#include <filesystem>

#include "npm/graphs/Relational.h"

#include "io/json/Binder.h"


namespace sylvanmats::npm{
    
    struct cb_payload{
        int cberr=0;
        int counter=0;
    };

    class Installation{
    protected:
        std::string sslCertificationLocation;
        std::string moduleDirectory;
        size_t timeout;
        sylvanmats::io::json::Path type;
        sylvanmats::npm::graphs::Relational& relationalGraph;
        std::string home;
        std::string cnpmHome;
        unsigned int depth=0;
        size_t current_source=0;
    public:
        Installation() = delete;
        Installation(std::string& sslCertificationLocation, std::string& moduleDirectory, size_t timeout, sylvanmats::io::json::Path type, sylvanmats::npm::graphs::Relational& relationalGraph);
        Installation(const Installation& orig) =  delete;
        virtual ~Installation() = default;
    public:
        void operator()(std::string& packageName);
        void operator()(sylvanmats::io::json::Binder& jb);
    protected:
        void traverse(std::string_view& key, std::string_view& val);
        void install(std::vector<size_t>& missing);
        void recurseModules(std::filesystem::path localLinkPath);
        void linkAnyBinaries(sylvanmats::io::json::Binder& jb, std::filesystem::path& localLinkPath);
        void emitUniversalHeartbeat();
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

