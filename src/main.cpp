#include "CLI/App.hpp"
#include "CLI/Formatter.hpp"
#include "CLI/Config.hpp"

#include <stdlib.h>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <regex>
#include <filesystem>
#include <functional>
#include <regex>
#include <ranges>
#include <algorithm>
#include <iterator>

#include "cnpm.h"
#include "npm/Installation.h"
#include "npm/Addition.h"
#include "npm/Removal.h"
#include "io/json/JsonBinder.h"

int main(int argc, char** argv, char **envp) {
    std::string home=std::getenv("HOME");
    std::string cnpmHome=(std::getenv("CNPM_HOME")!=NULL) ?std::getenv("CNPM_HOME") : ".";
    CLI::App app{"C++ dependency manager"};

    std::string filename = "package.json";
    std::vector<std::string> positional;
    std::string packageName;
    app.add_option("-f,--file,custom", positional, "input package");
    CLI::App &install = *app.add_subcommand("install", "Install all dependencies for a project");
    CLI::App &init = *app.add_subcommand("init", "Initializes project");
    CLI::App &add = *app.add_subcommand("add", "Installs a package and any packages that it depends on. By default, any new package is installed as a prod dependency");
    add.add_option("name", packageName, "package name");
    CLI::App &link = *app.add_subcommand("link", "Connect the local project to another one");
    link.add_option("name", packageName, "package name or directory");
    CLI::App &outdated = *app.add_subcommand("outdated", "Check for outdated packages");
    CLI::App &remove = *app.add_subcommand("remove", "Removes packages from node_modules and from the project's package.json");
    remove.add_option("name", packageName, "package name");
    bool dev{false};
    add.add_flag("-D,--save-dev", dev, "Save package to your 'devDependencies'");
    remove.add_flag("-D,--save-dev", dev, "Save package to your 'devDependencies'");
    CLI::App &test = *app.add_subcommand("t,test", R"(Runs a package's "test" script, if one was provided)");
    
    CLI11_PARSE(app, argc, argv);
    
    for(auto& o : positional | std::views::filter([](std::string& s){return s.ends_with(".json");}))
        filename=o;
    
    const auto [first, last] = std::ranges::remove_if(positional, [&filename](std::string& s){return s.compare(filename)==0;});
    positional.erase(first, last);
    
    sylvanmats::io::json::JsonBinder jsonBinder;
    std::filesystem::path packagePath=filename;
    if(std::filesystem::exists(packagePath)){
        std::ifstream is(packagePath);
        std::string jsonContent((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());
        jsonBinder(jsonContent);
    }
    
    bool update=false;
    if(add){
        sylvanmats::npm::Addition addition;
        sylvanmats::io::json::path jp;
        std::string s=(dev) ? "devDependencies" :"dependencies";
        jp=s;
        if(addition(jsonBinder, jp, packageName)){
            //filename="test-"+filename;
            std::ofstream o(filename.c_str());
            o << std::setw(4) << jsonBinder;
            update=true;
        }
    }
    else if(remove){
        sylvanmats::npm::Removal removal;
        sylvanmats::io::json::path jp;
        std::string s=(dev) ? "devDependencies" :"dependencies";
        jp=s;
        if(removal(jsonBinder, jp, packageName)){
            //filename="test-"+filename;
            std::ofstream o(filename.c_str());
            o << std::setw(4) << jsonBinder;
            update=true;
        }
    }
    else if(init){//waiting for <format>
    }
    
    if(install || update){
        sylvanmats::npm::Installation installation;
        sylvanmats::io::json::path jp;
        std::string s("dependencies");
        jp=s;
        installation(jsonBinder, jp);
        s="devDependencies";
        sylvanmats::io::json::path jp2;
        jp2=s;
        installation(jsonBinder, jp2);
        
    }
    if(test){
        sylvanmats::io::json::path jp;
        std::string s("scripts");
        jp=s;
        jsonBinder(jp, [](std::string_view& key, std::any& val){
            if(key.compare("test")==0){
                std::string command{std::any_cast<std::string_view>(val)};
                std::cout<<"\ttest : " << command << std::endl;
                int status=system(command.c_str());
                std::cout<<"\t"<<key << " : " << status << std::endl;
            }
        });
        
    }
    for(auto& o : positional){
        std::cout<<"pos scriptKeys: "<<o<<std::endl;
        sylvanmats::io::json::path jp;
        std::string s("scripts");
        jp=s;
        jsonBinder(jp, [&o](std::string_view& key, std::any& val){
            if(o.compare(key)==0){
                std::cout<<"o "<<o<<" "<<key<<std::endl;
                std::string command{std::any_cast<std::string_view>(val)};
                std::cout<<"\t"<<o << " : " << command << std::endl;
                int status=system(command.c_str());
                std::cout<<"\t"<<o << " : " << status << std::endl;
            }
        });
    }
    
    return EXIT_SUCCESS;
}
