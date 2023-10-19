#include "npm/Addition.h"

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
    
    Addition::Addition() {
        sylvanmats::npm::unordered_map<std::string, std::string> holder;
    }
    
    bool Addition::operator()(sylvanmats::io::json::Binder& jb, sylvanmats::io::json::Path type, std::string packageName){
        url::Url url(packageName);
//        std::cout<<"\t"<<url.has_scheme()<<" "<<url.syntax_ok()<<" "<<url.valid_host()<<" |" << url.host()<<"| "<<url.path()<<std::endl;
        bool hitVersion=false;
        if(url.syntax_ok() && !hitVersion){
            unsigned int index=url.path().rfind('/');
            if(index!=std::string::npos){
                bool tagExists=false;
                jb(type, [&tagExists](std::any& v){tagExists=true;});
                if(!tagExists){
                    std::cout<<"not "<<type<<std::endl;
                }
                return jb(type, packageName.substr(index+1, packageName.length()-index), packageName);
            }
        }
        return false;
    }
}
