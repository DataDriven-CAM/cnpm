#include "npm/Outdated.h"

#include "Url.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <filesystem>
#include <regex>
#include <ranges>
#include <algorithm>

#include "npm/utils/SemanticVersioning.h"
#include "npm/WebGetter.h"


namespace sylvanmats::npm{
    
    Outdated::Outdated() : home ((std::getenv("HOME")!=nullptr) ? std::getenv("HOME") : "c:/Users/Roger"), cnpmHome ((std::getenv("CNPM_HOME")!=nullptr) ?std::getenv("CNPM_HOME") : ".") {
    }
    
    void Outdated::operator()(sylvanmats::io::json::Binder& jb, sylvanmats::io::json::Path type){
        sylvanmats::io::json::Path jpName;
        jpName["name"];
        std::string_view currentPackageName;
        jb(jpName, [&currentPackageName](const sylvanmats::io::json::JsonValue& v){
            if(auto pVal = std::get_if<std::string_view>(&v)) {
                currentPackageName=*pVal;
            }
        });
        std::cout<<" currentPackageName "<<currentPackageName<<std::endl;
        jb(type, [&](std::string_view key, const sylvanmats::io::json::JsonValue& v){
            if(auto pVal = std::get_if<std::string_view>(&v)) {
            std::string_view val=*pVal;
            std::cout<<key<<" : "<<val<<std::endl;//<<" "<<v.type().name()
            url::Url url(std::string{val});
//            std::cout<<"\t"<<url.has_scheme()<<" "<<url.syntax_ok()<<" "<<url.valid_host()<<" |" << url.host()<<"| "<<url.path()<<std::endl;
            bool hitVersion=false;
            if(url.host().empty()){
                std::string moduleName{key};
                sylvanmats::npm::utils::SemanticVersioning semanticVersioning(true);
                semanticVersioning(val, [&moduleName, &hitVersion, &currentPackageName](std::string_view base, std::string_view branch, std::string_view wildcard){
                    if(base.compare(wildcard)==0)return;
                    // sylvanmats::reading::WebGetter webGetter;
                    std::string uri = "https://registry.npmjs.org/"+moduleName+"/"+std::string(wildcard);
                    std::string fileName=moduleName+".json";
                    std::filesystem::path tmpPath=std::filesystem::temp_directory_path()/fileName;
//                    std::cout<<"t file "<<tmpPath<<std::endl;
                    // webGetter(uri, [&moduleName, &base, &currentPackageName](std::istream& is){
                    //     sylvanmats::io::json::Binder jsonBinder;
                    //     jsonBinder(is);
                    //     sylvanmats::io::json::Path jp;
                    //     jp["version"];
                    //     jsonBinder(jp, [&](const sylvanmats::io::json::JsonValue& v){
                    //       std::cout<<moduleName<<" "<<base<<" "<<std::any_cast<std::string_view>(v)<<" "<<currentPackageName<<std::endl;  
                    //     });
                    //     //std::cout<<jsonBinder<<std::endl;
                    // });
                    
                });
            }
        }
        });
    }
}
