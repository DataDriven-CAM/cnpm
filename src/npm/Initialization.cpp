#include "npm/Installation.h"

#include <format>

namespace sylvanmats::npm{
    
    Initialization::Initialization() : home ((std::getenv("HOME")!=NULL) ?std::getenv("HOME") : "c:/Users/Roger"), cnpmHome ((std::getenv("CNPM_HOME")!=NULL) ?std::getenv("CNPM_HOME") : ".") {
    }
    
    sylvanmats::io::json::Binder Initialization::operator()(std::string packageName){
        sylvanmats::io::json::Binder jsonBinder;
        std::filesystem::path currentPath=std::filesystem::current_path();
        std::cout<<"name? ("<<currentPath.filename()<<"): ";
        std::string name;
        std::getline(std::cin, name);
        if(name.empty())name=currentPath.filename();
        std::cout<<"version? (1.0.0): ";
        std::string version;
        std::getline(std::cin, version);
        if(version.empty())version="1.0.0";
        std::cout<<"description?: ";
        std::string description;
        std::getline(std::cin, description);
        std::cout<<"repository url?:";
        std::string repository;
        std::getline(std::cin, repository);
        std::cout<<"author?:";
        std::string author;
        std::getline(std::cin, author);
        std::cout<<"license? (MIT): ";
        std::string license;
        std::getline(std::cin, license);
        if(license.empty())license="MIT";
        std::cout<<"private?:";
        std::string Private;
        std::getline(std::cin, Private);
        if(!Private.empty())Private="null";
        std::string package=std::format(R"({
  "name": "{}",
  "version": "{}",
  "description": "{}",
  "main": "main",
  "repository": "https://github.com/{}",
  "author": "{}",
  "license": "{}",
  "private": null
})", name, version, description, repository, author, license, Private);
        std::cout<<package<<std::endl;
        return jsonBinder;
    }
}
