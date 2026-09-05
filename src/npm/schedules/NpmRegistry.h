#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <execution>

#include <stdexec/execution.hpp>
#include <exec/static_thread_pool.hpp>

#include "npm/graphs/Relational.h"

#include "npm/async/SubprocessSpawner.h"
#include "npm/utils/TGZDecompressor.h"

namespace sylvanmats::npm::schedules{
    class NpmRegistry{
        private:
        std::string home;
        std::string moduleDirectory;
        sylvanmats::npm::graphs::Relational& relationalGraph;
        public:
            NpmRegistry()= delete;
            NpmRegistry(std::string home, std::string moduleDirectory, sylvanmats::npm::graphs::Relational& relationalGraph) : home(home), moduleDirectory(moduleDirectory), relationalGraph(relationalGraph){}
            NpmRegistry(NpmRegistry const&)= delete;
            NpmRegistry(NpmRegistry&&)= default;
            virtual ~NpmRegistry()= default;
            NpmRegistry& operator=(NpmRegistry const&)= delete;
            NpmRegistry& operator=(NpmRegistry&&)= default;

            void operator()(std::vector<size_t>& missingIndices){
                std::cout<<"NpmRegistry "<<missingIndices.size()<<std::endl;
                exec::static_thread_pool pool(std::thread::hardware_concurrency());
                auto scheduler = pool.get_scheduler();
                    auto work=stdexec::schedule(scheduler) 
                        | stdexec::bulk(stdexec::par, missingIndices.size(), [&](std::size_t bulk_idx) noexcept {
                        auto& missingIndex=missingIndices[bulk_idx];
                        sylvanmats::npm::graphs::project_properties& prop = relationalGraph.getProject(missingIndex);
                        if(prop.url.empty() || prop.wildcard.empty())return -1;
                        url::Url url(prop.url);
                        std::string_view uriPath=url.path();
                        // auto&& [scope, moduleName]=parseModuleName(uriPath);
                        std::filesystem::path localLinkPath=(!prop.scope.empty())? "./"+moduleDirectory+"/"+prop.scope+"/"+prop.module_name : "./"+moduleDirectory+"/"+prop.module_name;
                        std::filesystem::path localPath= (!prop.scope.empty())? std::filesystem::path(home)/".cnpm"/moduleDirectory/prop.scope/prop.module_name : std::filesystem::path(home)/".cnpm"/moduleDirectory/prop.module_name;
                            
                    std::string uri = (!prop.scope.empty()) 
                        ? "https://npmjs.org@" + prop.scope + "/" + prop.module_name + "/-/" + prop.module_name + "-" + prop.wildcard + ".tgz" 
                        : "https://npmjs.org" + prop.module_name + "/-/" + prop.module_name + "-" + prop.wildcard + ".tgz";

                        std::string fileName=prop.module_name+"-"+prop.wildcard+".tgz";
                    std::filesystem::path tmpPath=std::filesystem::temp_directory_path()/fileName;
                    std::vector<std::string> args = {
                        "-sS",                        // Suppress progress meter, show error messages
                        "-L",                         // Follow CDN redirects
                        "--fail",                     // Hard fail on HTTP errors (e.g., 404, 500)
                        "--connect-timeout", "10",    // Don't stall a worker thread indefinitely
                        uri,                          // The target download link
                        "-o", tmpPath.string()        // Target file destination
                    };

                    // Launch non-blocking process hook instead of blocking std::system
                    sylvanmats::npm::async::SubprocessSpawner spawner;
                    int exit_code = spawner("curl", args);
                    if (exit_code == 0) {
                        sylvanmats::npm::utils::TGZDecompressor tgzDecompressor;
                        tgzDecompressor(tmpPath, [&](std::filesystem::path& newPath, std::ostream& content){
                            if(!std::filesystem::exists(localPath.parent_path()))std::filesystem::create_directories(localPath.parent_path());
                            if(!std::filesystem::exists(localLinkPath) && std::filesystem::exists(localPath))std::filesystem::create_directory_symlink(localPath, localLinkPath);
                            localPath/=newPath;
//                            std::cout<<" "<<localPath.parent_path()<<" "<<localPath.filename()<<" "<<content.tellp()<<std::endl;
                            if(!std::filesystem::exists(localPath.parent_path()))std::filesystem::create_directories(localPath.parent_path());
                            if(!std::filesystem::exists(localPath)){
                                std::ofstream innerFile(localPath.c_str(), std::ios::binary);
                                std::istream is(dynamic_cast<std::stringbuf*>(content.rdbuf()));
                                innerFile<<dynamic_cast<std::stringbuf*>(content.rdbuf())->str();
                                innerFile.flush();
                            }
                        });
                        prop.already_have_it=true;
                        std::cout << '.'; 
                        std::cout.flush(); // Only flush occasionally when we want the dot visible!
                    }
                    else{
                        prop.already_have_it=false;
                        std::cout << 'x'; 
                        std::cout.flush(); // Only flush occasionally when we want the dot visible!
                    }
                    
                    return exit_code;
                });
                stdexec::sync_wait(std::move(work));
            }
        };
    }