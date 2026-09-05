#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <execution>

#include <stdexec/execution.hpp>
#include <exec/static_thread_pool.hpp>

#include "npm/graphs/Relational.h"
#include "npm/async/SubprocessSpawner.h"

namespace sylvanmats::npm::schedules{
    class Repository{
        private:
        std::string home;
        std::string moduleDirectory;
        sylvanmats::npm::graphs::Relational& relationalGraph;
        public:
            Repository()= delete;
            Repository(std::string home, std::string moduleDirectory, sylvanmats::npm::graphs::Relational& relationalGraph) : home(home), moduleDirectory(moduleDirectory), relationalGraph(relationalGraph){}
            Repository(Repository const&)= delete;
            Repository(Repository&&)= default;
            virtual ~Repository()= default;
            Repository& operator=(Repository const&)= delete;
            Repository& operator=(Repository&&)= default;

            void operator()(std::vector<size_t>& missingIndices){
                std::cout<<"Repository "<<missingIndices.size()<<std::endl;
            exec::static_thread_pool pool(std::thread::hardware_concurrency());
            auto scheduler = pool.get_scheduler();
                auto work=stdexec::schedule(scheduler) 
                        | stdexec::bulk(stdexec::par, missingIndices.size(), [&](std::size_t bulk_idx) noexcept {
                        auto& missingIndex=missingIndices[bulk_idx];
                        sylvanmats::npm::graphs::project_properties& prop = relationalGraph.getProject(missingIndex);
                        url::Url url(prop.url);
                        std::string_view uriPath=url.path();
                        // auto&& [scope, moduleName]=parseModuleName(uriPath);
                        std::filesystem::path localLinkPath=(!prop.scope.empty())? "./"+moduleDirectory+"/"+prop.scope+"/"+prop.module_name : "./"+moduleDirectory+"/"+prop.module_name;
                        std::filesystem::path localPath= (!prop.scope.empty())? std::filesystem::path(home)/".cnpm"/moduleDirectory/prop.scope/prop.module_name : std::filesystem::path(home)/".cnpm"/moduleDirectory/prop.module_name;
                        
                        std::string command;
                        std::vector<std::string> args;
                        // std::cout<<"prop.url "<<prop.url<<" "<<prop.wildcard<<std::endl;
                // Parse URI prefixes natively without heavy regex engines
                if (prop.url.starts_with("hg+")) {
                    args.push_back("clone");
                    args.push_back(prop.url);
                    args.push_back(localPath.string());
                    command = "hg";
                }
                else {
                    args.push_back("clone");
                    args.push_back("--depth");
                    args.push_back("1");
                    args.push_back("--no-tags");
                    args.push_back(prop.url);
                    args.push_back(localPath.string());
                    command = "git";
                } 
                if (command.empty()) {
                    std::cerr << "\n[ERROR] Unsupported protocol URI: " << prop.url << "\n";
                    throw std::runtime_error("Invalid protocol");
                }

                // Launch non-blocking process hook instead of blocking std::system
                sylvanmats::npm::async::SubprocessSpawner spawner;
                int exit_code = spawner(command, args);
                if (exit_code == 0) {
                    if(!std::filesystem::exists(localLinkPath.parent_path()))std::filesystem::create_directories(localLinkPath.parent_path());
                    if(!std::filesystem::exists(localLinkPath) && std::filesystem::exists(localPath))std::filesystem::create_directory_symlink(localPath, localLinkPath);
                    prop.already_have_it=true;
                    std::cout << '.'; 
                    std::cout.flush(); // Only flush occasionally when we want the dot visible!
                }
                else{
                    prop.already_have_it=false;
                    std::cout << 'x'<<exit_code;
                    std::cout.flush(); // Only flush occasionally when we want the dot visible!
                }

                 return exit_code;
            });
            stdexec::sync_wait(std::move(work));

            };
    };
}
