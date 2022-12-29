#include "npm/Removal.h"

#include "Url.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <filesystem>
#include <regex>
#include <ranges>
#include <algorithm>
#include <iterator>

#include "npm/WebGetter.h"

namespace sylvanmats::npm{
    
    Removal::Removal() {
        sylvanmats::npm::unordered_map<std::string, std::string> holder;
    }
    
    bool Removal::operator()(sylvanmats::io::json::Binder& jb, sylvanmats::io::json::Path type, std::string packageName){
        url::Url url(packageName);
        std::cout<<"\t"<<url.has_scheme()<<" "<<url.syntax_ok()<<" "<<url.valid_host()<<" |" << url.host()<<"| "<<url.path()<<std::endl;
        bool hitVersion=false;
        if(url.syntax_ok() && !hitVersion){
            unsigned int index=url.path().rfind('/');
            if(index!=std::string::npos){
                std::string moduleName=packageName.substr(index+1, packageName.length()-index);
                bool status=jb(type, moduleName);
                if(status){
                    std::filesystem::path localLinkPath="./cpp_modules/"+moduleName;
                    if(std::filesystem::exists(localLinkPath) && std::filesystem::is_symlink(localLinkPath)){
                        std::filesystem::remove(localLinkPath);
                    }
                }
                return status;
            }
        }
        
        return false;
    }
}

