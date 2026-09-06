#include "npm/Installation.h"

#include "Url.h"

#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
#include <fcntl.h>
#include <filesystem>
#include <ranges>
#include <algorithm>
#include <exception>


// #ifdef __WIN32__
// #include "windows.h"
// #include "winbase.h"
// #endif

#include "npm/async/SubprocessSpawner.h"
#include "npm/schedules/NpmRegistry.h"
#include "npm/schedules/Repository.h"
#include "npm/utils/SemanticVersioning.h"


namespace sylvanmats::npm{
    
    Installation::Installation(std::string& sslCertificationLocation, std::string& moduleDirectory, size_t timeout, sylvanmats::io::json::Path type, sylvanmats::npm::graphs::Relational& relationalGraph) : sslCertificationLocation (sslCertificationLocation), moduleDirectory (moduleDirectory), timeout (timeout), type (type), relationalGraph (relationalGraph),
         home ((std::getenv("HOME")!=nullptr) ?std::getenv("HOME") : "c:/Users/Roger"), cnpmHome ((std::getenv("CNPM_HOME")!=nullptr) ?std::getenv("CNPM_HOME") : ".") {
    }
    
    void Installation::operator()(std::string& packageName){
        url::Url url(packageName);
       std::cout<<"\t"<<url.has_scheme()<<" "<<url.syntax_ok()<<" "<<url.valid_host()<<" |" << url.host()<<"| "<<url.path()<<std::endl;
        bool hitVersion=false;
        if(url.syntax_ok() && !hitVersion){
            unsigned int index=url.path().rfind('/');
            std::cout<<"packageName "<<packageName<<" "<<(index!=std::string::npos)<<std::endl;
            if(index!=std::string::npos){
                std::string_view key{packageName.substr(index+1, packageName.length()-index)};
                std::string_view val{packageName};
                relationalGraph(sylvanmats::npm::graphs::project_properties{key, val, "", "", "", "", "", true, type.front().label.starts_with("dev")});
                relationalGraph(current_source, relationalGraph.getNumberOfProjects()-1);
                traverse(key, val);
            }
        }
    }
    
    void Installation::operator()(sylvanmats::io::json::Binder& jb){

        jb(type, [&](std::string_view& key, std::any& v){
            std::string_view val{std::any_cast<std::string_view>(v)};
            traverse(key, val);
        });
        std::vector<size_t>&& missing=relationalGraph.enqueueMissingDependencies();
        install(missing);
//         jb(type, [&](std::string_view& key, std::any& v){
// //            if(key.compare("axios")==0)std::cout<<key<<" : "<<std::any_cast<std::string_view>(v)<<" "<<v.type().name()<<std::endl;
//             std::string_view val{std::any_cast<std::string_view>(v)};
//             install(key, val);
//         });
    }
    
    void Installation::traverse(std::string_view& key, std::string_view& val){
            if(depth>=2)return;
            url::Url url(std::string{val});
        //    std::cout<<val<<"\t"<<url.has_scheme()<<" "<<url.host().empty()<<" "<<url.syntax_ok()<<" "<<url.valid_host()<<" |" << url.host()<<"| "<<url.path()<<std::endl;
            bool hitVersion=false;
            if(url.host().empty()){
                auto&& [scope, moduleName]=parseModuleName(key);
                std::filesystem::path localLinkPath=(!scope.empty()) ? std::filesystem::path(".")/moduleDirectory/scope/moduleName : std::filesystem::path(".")/moduleDirectory/moduleName;
                if(!std::filesystem::exists(localLinkPath)){
                    sylvanmats::npm::utils::SemanticVersioning semanticVersioning;
                std::cout<<"localLinkPath "<<localLinkPath<<" "<<std::filesystem::exists(localLinkPath)<<" "<<val<<std::endl;
                    if(semanticVersioning(val, [&](std::string_view base, std::string_view branch, std::string_view wildcard){
                        // std::cout << val << " version " << base<< " "<< wildcard << '\n';
                        std::filesystem::path localPath=(!scope.empty()) ? home+"/.cnpm/"+moduleDirectory+"/"+scope+"/"+moduleName+"-"+std::string(base) : home+"/.cnpm/"+moduleDirectory+"/"+moduleName+"-"+std::string(base);
                            relationalGraph(sylvanmats::npm::graphs::project_properties{key, val, (!branch.empty())? "https://github.com/"+std::string(base): "", scope, std::string(moduleName), std::string(branch), std::string(wildcard), std::filesystem::exists(localPath), type.front().label.starts_with("dev")});
                            relationalGraph(current_source, relationalGraph.getNumberOfProjects()-1);
                        hitVersion=true;
                    })){}
                    else{
                        std::filesystem::path localPath= (!scope.empty())? std::filesystem::path(home)/".cnpm"/moduleDirectory/scope/moduleName : std::filesystem::path(home)/".cnpm"/moduleDirectory/moduleName;
                        relationalGraph(sylvanmats::npm::graphs::project_properties{key, val, "https://github.com/"+std::string(val)+".git", "", moduleName, "", "", std::filesystem::exists(localPath), type.front().label.starts_with("dev")});
                        relationalGraph(current_source, relationalGraph.getNumberOfProjects()-1);
                        hitVersion=std::filesystem::exists(localPath);
                    }
                }
                else{
                    relationalGraph(sylvanmats::npm::graphs::project_properties{key, val, "", "", "", "", "", true, type.front().label.starts_with("dev")});
                    relationalGraph(current_source, relationalGraph.getNumberOfProjects()-1);
                    hitVersion=true;
                }
                if(hitVersion)recurseModules(localLinkPath);
            }

    }

    void Installation::install(std::vector<size_t>& missingIndices){
        
        std::cout<<"install "<<missingIndices.size()<<std::endl;
        sylvanmats::npm::schedules::NpmRegistry npmRegistry(home, moduleDirectory, relationalGraph);
        npmRegistry(missingIndices);
        std::vector<size_t> stillMissingIndices;
        stillMissingIndices.reserve(missingIndices.size()); 
        
        for (size_t idx : missingIndices) {
            if (!relationalGraph.getProject(idx).already_have_it) {
                stillMissingIndices.push_back(idx);
            }
        }

    // Pass the filtered list to the Repository scheduler stage
        if (!stillMissingIndices.empty()) {

            sylvanmats::npm::schedules::Repository repository(home, moduleDirectory, relationalGraph);
            repository(stillMissingIndices);
        }

        std::vector<size_t> stillStillMissingIndices;
        stillStillMissingIndices.reserve(stillMissingIndices.size()); 
        
        for (size_t idx : stillMissingIndices) {
            if (!relationalGraph.getProject(idx).already_have_it) {
                stillStillMissingIndices.push_back(idx);
            }
        }
        std::cout<<"stillStillMissingIndices "<<stillStillMissingIndices.size()<<std::endl;
//             if(val.starts_with("hg+")){
//                 if(val.starts_with("hg+https")){
//                 auto&& [scope, moduleName]=parseModuleName(key);
//                 std::filesystem::path localLinkPath=(!scope.empty()) ? std::filesystem::path(".")/moduleDirectory/scope/moduleName : std::filesystem::path(".")/moduleDirectory/moduleName;
//                 std::filesystem::path localPath= (!scope.empty())? std::filesystem::path(home)/".cnpm"/moduleDirectory/scope/moduleName : std::filesystem::path(home)/".cnpm"/moduleDirectory/moduleName;
//                 std::string oid="";
//                 if(!std::filesystem::exists(localPath)){
//                     std::string valStr{val};
//                     std::string command="hg clone "+valStr.substr(3)+" "+localPath.string();
                    
//                     if(std::system(command.c_str())==0){
//                         if(!std::filesystem::exists(localLinkPath.parent_path()))std::filesystem::create_directories(localLinkPath.parent_path());
//                         if(!std::filesystem::exists(localLinkPath) && std::filesystem::exists(localPath)){
// #ifdef __WIN32__
//                             CreateSymbolicLinkW(localLinkPath.c_str(), localPath.c_str(), SYMBOLIC_LINK_FLAG_DIRECTORY|SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE);
// #else
//                             try{
//                                 std::filesystem::create_directory_symlink(localPath, localLinkPath);
//                             }
//                             catch(std::filesystem::filesystem_error& ex){
//                                 std::throw_with_nested( std::runtime_error("Couldn't create symbolic link " + localLinkPath.string()) );
//                             }
// #endif
//                         }
//                         recurseModules(localLinkPath);
//                     }
//                     else std::cout<<"mercurial '"<<command<<"' failed"<<std::endl;
//                 }
//                     return;
//                 }
//                 std::cout<<val<<" hg repo not quite yet supported"<<std::endl;
//                 return;
//             }
        //     url::Url url(std::string{val});
        //    std::cout<<val<<"\t"<<url.has_scheme()<<" "<<url.host().empty()<<" "<<url.syntax_ok()<<" "<<url.valid_host()<<" |" << url.host()<<"| "<<url.path()<<std::endl;
            bool hitVersion=false;
    //         if(url.host().empty()){
    //             auto&& [scope, moduleName]=parseModuleName(key);
    //             std::filesystem::path localLinkPath=(!scope.empty())? "./"+moduleDirectory+"/"+scope+"/"+moduleName : "./"+moduleDirectory+"/"+moduleName;
    //             if(!std::filesystem::exists(localLinkPath)){
    //                 if(!std::filesystem::exists(localLinkPath.parent_path()))std::filesystem::create_directories(localLinkPath.parent_path());
    //                 SemanticVersioning semanticVersioning;
    //                 semanticVersioning(val, [&](std::string_view base,std::string_view base, std::string_view wildcard){
    //                     std::cout << val << " version " << base<< " "<< wildcard << '\n';
    //                     std::filesystem::path localPath=(!scope.empty()) ? home+"/.cnpm/"+moduleDirectory+"/"+scope+"/"+moduleName+"-"+std::string(base) : home+"/.cnpm/"+moduleDirectory+"/"+moduleName+"-"+std::string(base);
    //                     if(std::filesystem::exists(localPath)){
    //                         std::filesystem::create_directory_symlink(localPath, localLinkPath);
    //                         relationalGraph(sylvanmats::npm::graphs::project_properties{key, val, "", "", "", "", "", true, type.front().label.starts_with("dev")});
    //                         relationalGraph(current_source, relationalGraph.getNumberOfProjects()-1);
    //                     }
    //                     else{
    //                         relationalGraph(sylvanmats::npm::graphs::project_properties{key, val, "", "", "", "", "", false, type.front().label.starts_with("dev")});
    //                         relationalGraph(current_source, relationalGraph.getNumberOfProjects()-1);
    //                         sylvanmats::reading::WebGetter webGetter;
    //                         std::string uri =(!scope.empty()) ? "https://registry.npmjs.org/@"+scope+"/"+moduleName+"/-/"+moduleName+"-"+std::string(wildcard)+".tgz" : "https://registry.npmjs.org/"+moduleName+"/-/"+moduleName+"-"+std::string(wildcard)+".tgz";
    //                         std::cout<<"uri "<<uri<<std::endl;
    //                         std::string fileName=moduleName+"-"+std::string(wildcard)+".tgz";
    //                         std::filesystem::path tmpPath=std::filesystem::temp_directory_path()/fileName;
    // //                        std::cout<<"t file "<<tmpPath<<std::endl;
    //                         std::ofstream tgzFile(tmpPath.c_str(), std::ios::binary);
    //                         webGetter(uri, tgzFile);
    //                         tgzFile.close();
    //                         TGZDecompressor tgzDecompressor;
    // //                                    tgzDecompressor(is, tmpPath);
    //                         tgzDecompressor(tmpPath, [&](std::filesystem::path& newPath, std::ostream& content){
    //                             if(!std::filesystem::exists(localPath.parent_path()))std::filesystem::create_directories(localPath.parent_path());
    //                             if(!std::filesystem::exists(localLinkPath) && std::filesystem::exists(localPath))std::filesystem::create_directory_symlink(localPath, localLinkPath);
    //                             localPath/=newPath;
    // //                            std::cout<<" "<<localPath.parent_path()<<" "<<localPath.filename()<<" "<<content.tellp()<<std::endl;
    //                             if(!std::filesystem::exists(localPath.parent_path()))std::filesystem::create_directories(localPath.parent_path());
    //                             if(!std::filesystem::exists(localPath)){
    //                                 std::ofstream innerFile(localPath.c_str(), std::ios::binary);
    //                                 std::istream is(dynamic_cast<std::stringbuf*>(content.rdbuf()));
    //                                 innerFile<<dynamic_cast<std::stringbuf*>(content.rdbuf())->str();
    //                                 innerFile.flush();
    //                             }
    //                         });
    //                     }
    //                     hitVersion=true;
    //                 });
    //             }
    //             else{
    //                 relationalGraph(sylvanmats::npm::graphs::project_properties{key, val, "", "", "", "", "", true, type.front().label.starts_with("dev")});
    //                 relationalGraph(current_source, relationalGraph.getNumberOfProjects()-1);
    //                 hitVersion=true;
    //             }
    //             if(hitVersion)recurseModules(localLinkPath);
    //         }
//             if(url.syntax_ok() && !hitVersion){
//                 std::string uri=(url.host().empty()) ? "https://github.com/"+url.path()+".git" : url.as_string();
//                 auto&& [scope, moduleName]=parseModuleName(key);
//                 std::filesystem::path localLinkPath=(!scope.empty()) ? std::filesystem::path(".")/moduleDirectory/scope/moduleName : std::filesystem::path(".")/moduleDirectory/moduleName;
//                 std::filesystem::path localPath= (!scope.empty())? std::filesystem::path(home)/".cnpm"/moduleDirectory/scope/moduleName : std::filesystem::path(home)/".cnpm"/moduleDirectory/moduleName;
//                 std::string oid="";
//                 if(!std::filesystem::exists(localPath)){
//                     relationalGraph(sylvanmats::npm::graphs::project_properties{key, val, "", "", "", "", "", false, type.front().label.starts_with("dev")});
//                     relationalGraph(current_source, relationalGraph.getNumberOfProjects()-1);
// //                      git_libgit2_init();
// //                     git_libgit2_opts(GIT_OPT_SET_SSL_CERT_LOCATIONS, nullptr, sslCertificationLocation.c_str());
// //                     git_clone_options opts=GIT_CLONE_OPTIONS_INIT;
// // //                                int err=git_clone_options_init(&opts, GIT_CLONE_OPTIONS_VERSION);
// //                     cb_payload cbPayload;
// //                     cbPayload.publickey=cnpmHome+"/key.pem";
// //                     cbPayload.privatekey=cnpmHome+"/cert.pem";
// //                     //cbPayload.cberr=1;
// //                     opts.checkout_opts=GIT_CHECKOUT_OPTIONS_INIT;
// //                     opts.checkout_opts.checkout_strategy = GIT_CHECKOUT_SAFE;
// //                     opts.fetch_opts=GIT_FETCH_OPTIONS_INIT;
// //                     opts.fetch_opts.depth=1;
// //                     opts.fetch_opts.callbacks.payload=&cbPayload;
// //                     opts.fetch_opts.callbacks.credentials=[](git_credential **out, const char *url, const char *username_from_url, unsigned int allowed_types, void *payload)->int{
// //                         std::cout<<"allowed_types "<<allowed_types<<std::endl;
// //                         cb_payload* cbPayload=(cb_payload*)(payload);
// //                         std::cout<<"cbPayload: "<<cbPayload->cberr<<" "<<allowed_types<<std::endl;
// //                         switch(allowed_types){
// //                             case GIT_CREDENTIAL_SSH_KEY:
// //                                 break;
// //                             case GIT_CREDENTIAL_USERPASS_PLAINTEXT:
// //                                 break;
// //                             case GIT_CREDENTIAL_USERNAME:
// //                                 break;
// //                         }
// //                         out=&cbPayload->gitCredential;
// //                         cbPayload->cberr--;
// //                         std::cout<<"cbPayload: "<<cbPayload->cberr<<std::endl;
// //                         return cbPayload->cberr;
// //                     };
// //                     opts.fetch_opts.callbacks.sideband_progress=[](const char *str, int len, void *payload)->int{
// //                         cb_payload* cbPayload=(cb_payload*)(payload);

// //                             std::printf("remote: %.*s", len, str);
// //                             std::cout.flush();
// //                             return cbPayload->cberr;
// //                     };
// //                     opts.fetch_opts.callbacks.transfer_progress=[](const git_indexer_progress *stats, void *payload)->int{
// //                         cb_payload* cbPayload=(cb_payload*)(payload);
// //                         if((stats->received_objects % 100 ==0) || stats->received_objects==stats->total_objects)
// //                             std::cout<<"\r"<<stats->received_objects<<" of "<<stats->total_objects<<std::flush;
// //                         else
// //                             cbPayload->counter++;
// //                         return cbPayload->cberr;
// //                     };
// // //                                opts.fetch_opts.callbacks.certificate_check
// //                     git_repository *gitRepository;
// //                     std::string lpath=localPath.string();
// //                     git_libgit2_opts(GIT_OPT_SET_SERVER_CONNECT_TIMEOUT, timeout*1000);
// //                     git_libgit2_opts(GIT_OPT_SET_SERVER_TIMEOUT, timeout*1000);
// //                     int err= git_clone(&gitRepository, uri.c_str(), lpath.c_str(), &opts);
// //                     if(err!=0)
// //                         std::cout<<"err "<<err<<" "<<git_error_last()->klass<<" "<<git_error_last()->message<<std::endl;
// //                     else{
// //                         const char *branch = nullptr;
// //                         git_reference *head = nullptr;

// //                         err = git_repository_head(&head, gitRepository);

// //                         if (err == GIT_EUNBORNBRANCH || err == GIT_ENOTFOUND)
// //                           branch = nullptr;
// //                         else if (!err) {
// //                           branch = git_reference_shorthand(head);
// //                         }

// //                           printf("## %s\n", branch ? branch : "HEAD (no branch)");

// //                         git_reference_free(head);
                        
// //                         git_remote *remote = nullptr;
// //                         err = git_remote_lookup(&remote, gitRepository, uri.c_str());
// //                         if (err < 0)
// //                             err = git_remote_create_anonymous(&remote, gitRepository, uri.c_str());
// //                             if (err >= 0) {
// //                                 const git_remote_head **refs;
// //                                 git_remote_callbacks callbacks = GIT_REMOTE_CALLBACKS_INIT;
// //                                 err = git_remote_connect(remote, GIT_DIRECTION_FETCH, &callbacks, nullptr, nullptr);
// //                                 if (err >= 0){
// //                                     size_t refs_len;
// //                                     if (git_remote_ls(&refs, &refs_len, remote) >= 0){
// //                                         bool hit=false;
// //                                         for (size_t i = 0; !hit && i < refs_len; i++) {
// //                                           char git_oid[GIT_OID_HEXSZ + 1] = {0};
// //                                           git_oid_fmt(git_oid, &refs[i]->oid);
// //                                           printf("%s\t%s\n", git_oid, refs[i]->name);
// //                                           if(std::string("HEAD").compare(refs[i]->name)==0){
// //                                               oid=std::string(git_oid);
// //                                               hit=true;
// //                                           }
// //                                         }
// //                                     }
// //                                 }
// //                                 else std::cout<<"err2 "<<err<<" "<<git_error_last()->klass<<" "<<git_error_last()->message<<std::endl;
// //                             }
// //                             else std::cout<<"err1 "<<err<<" "<<git_error_last()->klass<<" "<<git_error_last()->message<<std::endl;
// //                         git_remote_free(remote);
// //                     }
// //                     git_libgit2_shutdown();
//                 }
//                 else{
//                     relationalGraph(sylvanmats::npm::graphs::project_properties{key, val, "", "", "", "", "", true, type.front().label.starts_with("dev")});
//                     relationalGraph(current_source, relationalGraph.getNumberOfProjects()-1);
                     
//                 }
//                 if(!oid.empty())std::cout<<"oid "<<oid<<std::endl;
//                 if(!std::filesystem::exists(localLinkPath.parent_path()))std::filesystem::create_directories(localLinkPath.parent_path());
//                 if(!std::filesystem::exists(localLinkPath) && std::filesystem::exists(localPath)){
// #ifdef __WIN32__
//                     CreateSymbolicLinkW(localLinkPath.c_str(), localPath.c_str(), SYMBOLIC_LINK_FLAG_DIRECTORY|SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE);
// #else
//                     try{
//                         std::filesystem::create_directory_symlink(localPath, localLinkPath);
//                     }
//                     catch(std::filesystem::filesystem_error& ex){
//                         std::throw_with_nested( std::runtime_error("Couldn't create symbolic link " + localLinkPath.string()) );
//                     }
// #endif
//                 }
//                 recurseModules(localLinkPath);
//             }
        
    }

    void Installation::recurseModules(std::filesystem::path localLinkPath){
         if(depth<3)
            for(auto& p: std::filesystem::directory_iterator(localLinkPath)){
                if(p.path().filename().compare("package.json")==0 && std::filesystem::exists(p.path())){
                    //std::cout<<depth<<" "<<p.path()<<std::endl;
                    sylvanmats::io::json::Binder jsonBinder;
                    std::ifstream is(p.path());
                    jsonBinder(is);
                    depth++;
                    jsonBinder(type, [&](std::string_view& key, std::any& v){
                        std::string_view val{std::any_cast<std::string_view>(v)};
                        traverse(key, val);
                    });
                    // this->operator()(jsonBinder);
                    // linkAnyBinaries(jsonBinder, localLinkPath);
                    depth--;
                    //std::cout<<"depth "<<depth<<std::endl;
                }
            }
    }
    
    void Installation::linkAnyBinaries(sylvanmats::io::json::Binder& jb, std::filesystem::path& localLinkPath){
        if(std::filesystem::exists(localLinkPath)){
            std::filesystem::path binPath="./"+moduleDirectory;
            binPath/=".bin";
            if(!std::filesystem::exists(binPath))std::filesystem::create_directories(binPath);
            sylvanmats::io::json::Path jp;
            jp["bin"];
            jb(jp, [&](std::string_view& key, std::any& v){
                std::filesystem::path execLinkPath=binPath;
                execLinkPath/=key;
                std::filesystem::path execPath=localLinkPath;
                execPath/=std::any_cast<std::string_view>(v);
                std::filesystem::path relExecPath=localLinkPath.lexically_relative(binPath);
                relExecPath/=std::any_cast<std::string_view>(v);
                if(!std::filesystem::exists(execLinkPath) && std::filesystem::exists(execPath))std::filesystem::create_directory_symlink(relExecPath, execLinkPath);
            });
        }
    }

    void Installation::emitUniversalHeartbeat() {
        // Note: Writing a single char is atomic on most platforms, 
        // but no \n means it sits in the buffer until we want it.
        std::cout << '.'; 
        std::cout.flush(); // Only flush occasionally when we want the dot visible!
    }
}
