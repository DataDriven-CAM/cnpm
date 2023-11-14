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
#include <execution>
#include <algorithm>
#include <iterator>

#include "cnpm.h"
#include "npm/Initialization.h"
#include "npm/Installation.h"
#include "npm/Addition.h"
#include "npm/Remodeler.h"
#include "npm/Removal.h"
#include "npm/Outdated.h"
#include "npm/Snap.h"
#include "io/json/Binder.h"

#include <typeinfo>
#include <typeindex>

int main(int argc, char** argv, char **envp) {
    try{
        std::string home=(std::getenv("HOME")!=nullptr) ? std::getenv("HOME") : "c:/Users/Roger";
        std::string cnpmHome=(std::getenv("CNPM_HOME")!=nullptr) ?std::getenv("CNPM_HOME") : ".";
        CLI::App app{"C++ dependency manager"};

        std::string filename = "package.json";
        std::vector<std::string> positional;
        std::string packageName;
        app.add_option("-f,--file,custom", positional, "input package");
        app.require_subcommand(0,1);
        bool dev{false};
        bool global{false};
        CLI::App &install = *app.add_subcommand("install", "Install all dependencies for a project")->alias("i");
        CLI::App &init = *app.add_subcommand("init", "Initializes project");
        CLI::App &add = *app.add_subcommand("add", "Installs a package and any packages that it depends on. By default, any new package is installed as a prod dependency");
        add.add_option("name", packageName, "package name");
        add.add_flag("-D,--save-dev", dev, "Save package to your 'devDependencies'");
        CLI::App &link = *app.add_subcommand("link", "Connect the local project to another one")->alias("ln");
        link.add_option("name", packageName, "package name or directory");
        CLI::App &outdated = *app.add_subcommand("outdated", "Check for outdated packages");
        CLI::App &rebuild = *app.add_subcommand("rebuild", "Rebuild a package");
        rebuild.add_option("name", packageName, "package name");
        CLI::App &remove = *app.add_subcommand("remove", "Removes packages from cpp_modules and from the project's package.json")->alias("rm");
        remove.add_option("name", packageName, "package name");
        remove.add_flag("-D,--save-dev", dev, "Save package to your 'devDependencies'");
        CLI::App &prefix = *app.add_subcommand("prefix", "Display prefix");
        prefix.add_flag("-g,--global", global, "Display global prefix'");
        CLI::App &test = *app.add_subcommand("test", R"(Runs a package's "test" script, if one was provided)")->alias("t");
        CLI::App &install_test = *app.add_subcommand("install-test", R"(Runs a cnpm install followed immediately by a cnpm test)")->alias("it");
        app.set_config("--config", cnpmHome+"/config.toml", "Read a toml file");
        app.add_option("module-directory")->group("");
        CLI::App &security = *app.add_subcommand("security", "Security properties in config file");
        security.add_option("username", username, "git user name");
        security.add_option("passphrase", passphrase, "git pass phrase");
        security.configurable();
        CLI::App &snap = *app.add_subcommand("snap", "Output snap craft yaml");

        CLI11_PARSE(app, argc, argv);

        std::string moduleDirectory=(app.count("module-directory"))? app.get_option("module-directory")->as<std::string>(): "cpp_modules";
        username=(app.got_subcommand("security") && security.count("username"))? security.get_option("username")->as<std::string>(): "anonymous";
        passphrase=(app.got_subcommand("security") && security.count("passphrase"))? security.get_option("passphrase")->as<std::string>(): "";
        for(auto& o : positional | std::views::filter([](std::string& s){return s.ends_with(".json");}))
            filename=o;

        const auto [first, last] = std::ranges::remove_if(positional, [&filename](std::string& s){return s.compare(filename)==0;});
        positional.erase(first, last);

        sylvanmats::io::json::Binder jsonBinder;
        std::filesystem::path packagePath=filename;
        if(init && !std::filesystem::exists(packagePath)){
            sylvanmats::npm::Initialization initialization;
            initialization(packageName, jsonBinder);
            std::ofstream o(filename.c_str());
            o << std::setw(4) << jsonBinder;
            o.close();
        }
        else if(std::filesystem::exists(packagePath)){
            std::ifstream is(packagePath);
            std::string jsonContent((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());
            jsonBinder(jsonContent);
        }
 
        bool update=false;
        if(add){
            sylvanmats::npm::Addition addition;
            sylvanmats::io::json::Path jp=Root();
            (dev) ? jp["devDependencies"] : jp["dependencies"];
            if(addition(jsonBinder, jp, packageName)){
                //filename="test-"+filename;
                std::ofstream o(filename.c_str());
                o << std::setw(4) << jsonBinder;
                update=true;
            }
        }
        else if(rebuild){
            sylvanmats::npm::Remodeler remodeler;
            sylvanmats::io::json::Path jp=(dev) ? "devDependencies" :"dependencies";
            if(remodeler(jsonBinder, jp, packageName)){
                //filename="test-"+filename;
                std::ofstream o(filename.c_str());
                o << std::setw(4) << jsonBinder;
                //update=true;
            }
        }
        else if(remove){
            sylvanmats::npm::Removal removal;
            sylvanmats::io::json::Path jp=(dev) ? "devDependencies" :"dependencies";
            if(removal(jsonBinder, jp, packageName)){
                //filename="test-"+filename;
                std::ofstream o(filename.c_str());
                o << std::setw(4) << jsonBinder;
                update=true;
            }
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
            sylvanmats::io::json::Path jp;
            jp["dependencies"];
            outdated(jsonBinder, jp);
            sylvanmats::io::json::Path jp2;
            jp2["devDependencies"];
            outdated(jsonBinder, jp2);
        }
        else if(prefix && global){
#if __linux__
            std::filesystem::path selfPath="/proc/self/exe";
            std::filesystem::path appDirectory=std::filesystem::read_symlink(selfPath);
            if(appDirectory.has_parent_path())appDirectory=appDirectory.parent_path();
            std::cout<<" "<<appDirectory<<std::endl;
#endif
        }
        else if(prefix){
            std::filesystem::path currentDirectory=std::filesystem::current_path();
            currentDirectory=std::filesystem::canonical(currentDirectory);
            bool found=false;
            while(!found && currentDirectory.has_parent_path()){
            
            std::filesystem::path packagePath=currentDirectory/filename;
            std::filesystem::path modulePath=currentDirectory/moduleDirectory;
            if(std::filesystem::exists(packagePath) || std::filesystem::exists(modulePath)){
                if(std::filesystem::exists(packagePath)){
                    packagePath=std::filesystem::canonical(packagePath);
                    if(packagePath.has_parent_path()){
                        std::cout<<packagePath.parent_path().string()<<std::endl;
                        found=true;
                    }
                }
                else if(std::filesystem::exists(modulePath)){
                    modulePath=std::filesystem::canonical(modulePath);
                    if(modulePath.has_parent_path()){
                        std::cout<<modulePath.parent_path().string()<<std::endl;
                        found=true;
                    }
                }
            }
            if(!found)currentDirectory=currentDirectory.parent_path();
            }
        }
        else if(snap){
            sylvanmats::npm::Snap snap;
            if(snap(jsonBinder, [](std::string& yamlContent){
                std::string yamlName="snapcraft.yaml";
                std::ofstream o(yamlName.c_str());
                o << std::setw(4) << yamlContent;
                o.close();
            })){
            }
        }

        if(install || install_test || update){
            sylvanmats::io::json::Path jp;
            jp["dependencies"];
            sylvanmats::npm::Installation installation(moduleDirectory, jp);
            installation(jsonBinder);
            sylvanmats::io::json::Path jp2;
            jp2["devDependencies"];
            sylvanmats::npm::Installation installation2(moduleDirectory, jp2);
            installation2(jsonBinder);

        }
        if(test || install_test){
            sylvanmats::io::json::Path jp;
            jp["scripts"];
            jsonBinder(jp, [](std::string_view& key, std::any& val){
                if(key.compare("test")==0){
                    std::string command{std::any_cast<std::string_view>(val)};
                    replace(command, "\\\"", "\"");
                    std::cout<<"\ttest : " << command << std::endl;
                    int status=system(command.c_str());
                    std::cout<<"\t"<<key << " : " << status << std::endl;
                }
            });

        }
        for(auto& o : positional){
            std::cout<<"pos scriptKeys: "<<o<<std::endl;
            sylvanmats::io::json::Path jp;
            jp["scripts"];
            jsonBinder(jp, [&o](std::string_view& key, std::any& val){
                if(o.compare(key)==0){
                    std::cout<<"o "<<o<<" "<<key<<std::endl;
                    std::string command{std::any_cast<std::string_view>(val)};
                    std::cout<<"\t"<<o << " : " << command << std::endl;
                    replace(command, "\\\"", "\"");
//                    std::cout<< "command : " << command << std::endl;
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
