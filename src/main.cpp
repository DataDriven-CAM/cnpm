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
#include "npm/Outdated.h"
#include "io/json/JsonBinder.h"

#include <typeinfo>
#include <typeindex>

void print_exception(const std::exception& e, int level =  0)
{
    std::cerr << std::string(level, ' ') << "exception: " << e.what() << '\n';
    try {
        std::rethrow_if_nested(e);
    } catch(const std::exception& e) {
        print_exception(e, level+1);
    } catch(...) {}
}

int main(int argc, char** argv, char **envp) {
    try{
        std::string home=(std::getenv("HOME")!=NULL) ? std::getenv("HOME") : "c:/Users/Roger";
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
        app.set_config("--config", cnpmHome+"/config.toml", "Read a toml file");
        app.add_option("module-directory")->group("");

        CLI11_PARSE(app, argc, argv);

        std::string moduleDirectory=(app.count("module-directory"))? app.get_option("module-directory")->as<std::string>(): "cpp_modules";
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
            sylvanmats::io::json::path jp=(dev) ? "devDependencies" :"dependencies";
            if(addition(jsonBinder, jp, packageName)){
                //filename="test-"+filename;
                std::ofstream o(filename.c_str());
                o << std::setw(4) << jsonBinder;
                update=true;
            }
        }
        else if(remove){
            sylvanmats::npm::Removal removal;
            sylvanmats::io::json::path jp=(dev) ? "devDependencies" :"dependencies";
            if(removal(jsonBinder, jp, packageName)){
                //filename="test-"+filename;
                std::ofstream o(filename.c_str());
                o << std::setw(4) << jsonBinder;
                update=true;
            }
        }
        else if(init){//waiting for <format>
            std::cout<<"Waiting for <format> in c++ compiler "<<std::endl;
        }
        else if(link){
            std::filesystem::path localPath=packageName;
            std::string moduleName=localPath.filename().string();
            std::filesystem::path localLinkPath="./"+moduleDirectory+"/"+moduleName;
            if(!std::filesystem::exists(localLinkPath) && std::filesystem::exists(localPath)){
                std::filesystem::create_directory_symlink(localPath, localLinkPath);
            }
        }
        else if(outdated){
            sylvanmats::npm::Outdated outdated;
            sylvanmats::io::json::path jp="dependencies";
            outdated(jsonBinder, jp);
            sylvanmats::io::json::path jp2="devDependencies";
            outdated(jsonBinder, jp2);
        }

        if(install || update){
            sylvanmats::io::json::path jp="dependencies";
            sylvanmats::npm::Installation installation(moduleDirectory, jp);
            installation(jsonBinder);
            sylvanmats::io::json::path jp2="devDependencies";
            sylvanmats::npm::Installation installation2(moduleDirectory, jp2);
            installation2(jsonBinder);

        }
        if(test){
            sylvanmats::io::json::path jp="scripts";
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
            sylvanmats::io::json::path jp="scripts";
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
    }
    catch(std::filesystem::filesystem_error &e) {
        std::cout<<"what? "<<e.what() << std::endl;
        return  EXIT_FAILURE;
    }
    catch(const std::exception& e) {
        print_exception(e);
        return  EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
