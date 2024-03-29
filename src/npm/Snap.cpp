#include <filesystem>
#include <fstream>

#include "npm/Snap.h"
#include "npm/SymanticVersioning.h"

#include "Url.h"

#include <format>
#include <utility>

namespace sylvanmats::npm{
    Snap::Snap(std::string& moduleDirectory) : moduleDirectory (moduleDirectory), home ((std::getenv("HOME")!=nullptr) ?std::getenv("HOME") : "c:/Users/Roger"), cnpmHome ((std::getenv("CNPM_HOME")!=nullptr) ?std::getenv("CNPM_HOME") : ".") {
    }
    
    bool Snap::operator()(sylvanmats::io::json::Binder& jsonBinder, std::function<void(std::string& yamlContent)> apply){
        std::string templatePath=cnpmHome+"/templates/snap/snapcraft.txt";
        std::ifstream is(templatePath);
        std::string snapTemplate((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());
        sylvanmats::io::json::Path jpName;
        jpName["name"];
        std::string_view name;
        jsonBinder(jpName, [&name](std::any& v){
            name=std::any_cast<std::string_view>(v);
        });
        sylvanmats::io::json::Path jpVersion;
        jpVersion["version"];
        std::string_view version;
        jsonBinder(jpVersion, [&version](std::any& v){
            version=std::any_cast<std::string_view>(v);
        });
        sylvanmats::io::json::Path jpSummary;
        jpSummary["description"];
        std::string_view summary;
        jsonBinder(jpSummary, [&summary](std::any& v){
            summary=std::any_cast<std::string_view>(v);
        });
        sylvanmats::io::json::Path jpUrl;
        jpUrl["repository"];
        std::string_view url;
        jsonBinder(jpUrl, [&url](std::any& v){
            url=std::any_cast<std::string_view>(v);
        });
        std::string plugin="make";
        std::string sourceType="git";
        sylvanmats::io::json::Path type;        
        type["dependencies"];
        jsonBinder(type, [&](std::string_view& key, std::any& v){
            std::string_view value=std::any_cast<std::string_view>(v);
            if(value.starts_with("hg+")){
                if(value.starts_with("hg+https")){
                auto&& [scope, moduleName]=parseModuleName(key);
                std::string valStr{value};
//                    std::string command="hg clone "+valStr.substr(3)+" "+localPath.string();
                std::string dependencyPlugin{"nil"};
                std::string dependencySourceType={"hg"};
                std::string uri=valStr.substr(3);
                secondaryParts.push_back({.key=key, .plugin=dependencyPlugin, .sourceType=dependencySourceType, .source=uri});
                    
//                std::cout<<val<<" hg repo not quite yet supported"<<std::endl;
                return;
                }
            }
            url::Url depUrl(std::string{value});
            bool hitVersion=false;
            std::string dependencyPlugin{"nil"};
            std::string dependencySourceType={"nil"};
            if(depUrl.host().empty()){
                auto&& [scope, moduleName]=parseModuleName(key);
                    SymanticVersioning symanticVersioning;
                    symanticVersioning(value, [&](std::string_view base, std::string_view wildcard){
                        std::cout << value << " version " << base<< " "<< wildcard << '\n';
//                        std::filesystem::path localPath=(!scope.empty()) ? home+"/.cnpm/"+moduleDirectory+"/"+scope+"/"+moduleName+"-"+std::string(base) : home+"/.cnpm/"+moduleDirectory+"/"+moduleName+"-"+std::string(base);
                            std::string uri =(!scope.empty()) ? "https://registry.npmjs.org/@"+scope+"/"+moduleName+"/-/"+moduleName+"-"+std::string(wildcard)+".tgz" : "https://registry.npmjs.org/"+moduleName+"/-/"+moduleName+"-"+std::string(wildcard)+".tgz";
                secondaryParts.push_back({.key=key, .plugin=dependencyPlugin, .sourceType=dependencySourceType, .source=uri});
                        hitVersion=true;
                    });
            }
            if(depUrl.syntax_ok() && !hitVersion){
                std::string uri=(depUrl.host().empty()) ? "git://github.com/"+depUrl.path()+".git" : depUrl.as_string();
                auto&& [scope, moduleName]=parseModuleName(key);
                dependencyPlugin=std::string{"nil"};
                std::filesystem::path localLinkPath=(!scope.empty()) ? std::filesystem::path(".")/moduleDirectory/scope/moduleName : std::filesystem::path(".")/moduleDirectory/moduleName;
                if(std::filesystem::exists(localLinkPath)){
                    for(auto& p: std::filesystem::directory_iterator(localLinkPath)){
                        if(p.path().filename().compare("configure")==0 || p.path().filename().compare("config")==0 || p.path().filename().compare("configure.ac")==0){
                            dependencyPlugin=std::string{"autotools"};
                        }
                        else if(p.path().filename().compare("CMakeLists.txt")==0){
                            dependencyPlugin=std::string{"cmake"};
                        }
                    }
                    for(auto& p: std::filesystem::directory_iterator(localLinkPath)){
                        if(dependencyPlugin.compare("nil")==0 && p.path().filename().compare("Makefile")==0){
                            dependencyPlugin=std::string{"make"};
                        }
                    }
                }
                dependencySourceType=std::string{"git"};
                
                std::string part(key.begin(), key.end());
                std::transform(part.cbegin(), part.cend(), part.begin(), [](unsigned char c) {return std::tolower(c);});
                secondaryParts.push_back({.key=key, .part=part, .plugin=dependencyPlugin, .sourceType=dependencySourceType, .source=uri});
            }

        });
        std::string part(name.begin(), name.end());
        std::transform(part.cbegin(), part.cend(), part.begin(), [](unsigned char c) {return std::tolower(c);});
        auto nameArg=fmt::arg("name", name);
        auto versionArg=fmt::arg("version", version);
        auto summaryArg=fmt::arg("summary", summary);
        auto descriptionArg=fmt::arg("description", summary);
        auto partArg=fmt::arg("part", part);
        auto pluginArg=fmt::arg("plugin", plugin);
        auto sourceTypeArg=fmt::arg("source_type", sourceType);
        auto urlArg=fmt::arg("url", url);
        auto appArg=fmt::arg("app", part);
        auto secondaryPartsArg=fmt::arg("secondary_parts", secondaryParts);
        std::string snapContent=fmt::vformat(snapTemplate, fmt::make_format_args(nameArg, versionArg, summaryArg, descriptionArg, partArg, pluginArg, sourceTypeArg, urlArg, appArg, secondaryPartsArg));
        apply(snapContent);
        return true;
    }

}