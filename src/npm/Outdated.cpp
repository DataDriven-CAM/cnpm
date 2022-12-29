#include "npm/Outdated.h"

#include "Url.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <filesystem>
#include <regex>
#include <ranges>
#include <algorithm>

#include "npm/SymanticVersioning.h"
#include "npm/WebGetter.h"


namespace sylvanmats::npm{
    
    Outdated::Outdated() : home ((std::getenv("HOME")!=NULL) ?std::getenv("HOME") : "c:/Users/Roger"), cnpmHome ((std::getenv("CNPM_HOME")!=NULL) ?std::getenv("CNPM_HOME") : ".") {
    }
    
    void Outdated::operator()(sylvanmats::io::json::Binder& jb, sylvanmats::io::json::Path type){
        sylvanmats::io::json::Path jpName;
        jpName["name"];
        std::string_view currentPackageName;
        jb(jpName, [&currentPackageName](std::any& v){
            currentPackageName=std::any_cast<std::string_view>(v);
        });
        std::cout<<" currentPackageName "<<currentPackageName<<std::endl;
        jb(type, [&](std::string_view& key, std::any& v){
            std::cout<<key<<" : "<<std::any_cast<std::string_view>(v)<<" "<<v.type().name()<<std::endl;
            std::string_view val{std::any_cast<std::string_view>(v)};
            url::Url url(std::string{val});
//            std::cout<<"\t"<<url.has_scheme()<<" "<<url.syntax_ok()<<" "<<url.valid_host()<<" |" << url.host()<<"| "<<url.path()<<std::endl;
            bool hitVersion=false;
            if(url.host().empty()){
                std::string moduleName{key};
                SymanticVersioning symanticVersioning(true);
                symanticVersioning(val, [&moduleName, &hitVersion, &currentPackageName](std::string_view base, std::string_view wildcard){
                    if(base.compare(wildcard)==0)return;
                    sylvanmats::reading::WebGetter webGetter;
                    std::string uri = "https://registry.npmjs.org/"+moduleName+"/"+std::string(wildcard);
                    std::string fileName=moduleName+".json";
                    std::filesystem::path tmpPath=std::filesystem::temp_directory_path()/fileName;
//                    std::cout<<"t file "<<tmpPath<<std::endl;
                    webGetter(uri, [&moduleName, &base, &currentPackageName](std::istream& is){
                        sylvanmats::io::json::Binder jsonBinder;
                        jsonBinder(is);
                        sylvanmats::io::json::Path jp;
                        jp["version"];
                        jsonBinder(jp, [&](std::any& v){
                          std::cout<<moduleName<<" "<<base<<" "<<std::any_cast<std::string_view>(v)<<" "<<currentPackageName<<std::endl;  
                        });
                        //std::cout<<jsonBinder<<std::endl;
                    });
                    
                });
            }
        });
    }
}
