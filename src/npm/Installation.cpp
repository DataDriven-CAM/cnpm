#include "npm/Installation.h"

#include "Url.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <filesystem>
#include <ranges>
#include <algorithm>
#include <exception>

#include "npm/SymanticVersioning.h"
#include "npm/WebGetter.h"
#include "npm/TGZDecompressor.h"


namespace sylvanmats::npm{
    
    Installation::Installation(std::string& moduleDirectory, sylvanmats::io::json::path type) : moduleDirectory (moduleDirectory), type (type), home ((std::getenv("HOME")!=NULL) ?std::getenv("HOME") : "c:/Users/Roger"), cnpmHome ((std::getenv("CNPM_HOME")!=NULL) ?std::getenv("CNPM_HOME") : ".") {
    }
    
    void Installation::operator()(std::string& packageName){
        url::Url url(packageName);
//        std::cout<<"\t"<<url.has_scheme()<<" "<<url.syntax_ok()<<" "<<url.valid_host()<<" |" << url.host()<<"| "<<url.path()<<std::endl;
        bool hitVersion=false;
        if(url.syntax_ok() && !hitVersion){
            unsigned int index=url.path().rfind('/');
            if(index!=std::string::npos){
                std::string_view key{packageName.substr(index+1, packageName.length()-index)};
                std::string_view val{packageName};
                install(key, val);
            }
        }
    }
    
    void Installation::operator()(sylvanmats::io::json::JsonBinder& jb){
        jb(type, [&](std::string_view& key, std::any& v){
//            if(key.compare("axios")==0)std::cout<<key<<" : "<<std::any_cast<std::string_view>(v)<<" "<<v.type().name()<<std::endl;
            std::string_view val{std::any_cast<std::string_view>(v)};
            install(key, val);
        });
    }
    
    void Installation::install(std::string_view& key, std::string_view& val){
            if(depth>=2)return;
            url::Url url(std::string{val});
//            std::cout<<val<<"\t"<<url.has_scheme()<<" "<<url.syntax_ok()<<" "<<url.valid_host()<<" |" << url.host()<<"| "<<url.path()<<std::endl;
            bool hitVersion=false;
            if(url.host().empty()){
                auto&& [scope, moduleName]=parseModuleName(key);
                std::filesystem::path localLinkPath=(!scope.empty())? "./"+moduleDirectory+"/"+scope+"/"+moduleName : "./"+moduleDirectory+"/"+moduleName;
                if(!std::filesystem::exists(localLinkPath)){
                    if(!std::filesystem::exists(localLinkPath.parent_path()))std::filesystem::create_directories(localLinkPath.parent_path());
                    SymanticVersioning symanticVersioning;
                    symanticVersioning(val, [&](std::string_view base, std::string_view wildcard){
                        std::cout << val << " version " << base<< " "<< wildcard << '\n';
                        std::filesystem::path localPath=(!scope.empty()) ? home+"/.cnpm/"+moduleDirectory+"/"+scope+"/"+moduleName+"-"+std::string(base) : home+"/.cnpm/"+moduleDirectory+"/"+moduleName+"-"+std::string(base);
                        if(std::filesystem::exists(localPath))
                            std::filesystem::create_directory_symlink(localPath, localLinkPath);
                        else{
                            sylvanmats::reading::WebGetter webGetter;
                            std::string uri =(!scope.empty()) ? "https://registry.npmjs.org/@"+scope+"/"+moduleName+"/-/"+moduleName+"-"+std::string(wildcard)+".tgz" : "https://registry.npmjs.org/"+moduleName+"/-/"+moduleName+"-"+std::string(wildcard)+".tgz";
                            std::cout<<"uri "<<uri<<std::endl;
                            std::string fileName=moduleName+"-"+std::string(wildcard)+".tgz";
                            std::filesystem::path tmpPath=std::filesystem::temp_directory_path()/fileName;
    //                        std::cout<<"t file "<<tmpPath<<std::endl;
                            std::ofstream tgzFile(tmpPath.c_str(), std::ios::binary);
                            webGetter(uri, tgzFile);
                            tgzFile.close();
                            TGZDecompressor tgzDecompressor;
    //                                    tgzDecompressor(is, tmpPath);
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
                        }
                        hitVersion=true;
                    });
                }
                else hitVersion=true;
                if(hitVersion)recurseModules(localLinkPath);
            }
            if(url.syntax_ok() && !hitVersion){
                std::string uri=(url.host().empty()) ? "git://github.com/"+url.path()+".git" : url.as_string();
                auto&& [scope, moduleName]=parseModuleName(key);
                std::filesystem::path localLinkPath=(!scope.empty()) ? std::filesystem::path(".")/moduleDirectory/scope/moduleName : std::filesystem::path(".")/moduleDirectory/moduleName;
                std::filesystem::path localPath= (!scope.empty())? std::filesystem::path(home)/".cnpm"/moduleDirectory/scope/moduleName : std::filesystem::path(home)/".cnpm"/moduleDirectory/moduleName;
                std::string oid="";
                if(!std::filesystem::exists(localPath)){
                    git_libgit2_init();
                    //git_libgit2_opts(GIT_OPT_SET_SSL_CERT_LOCATIONS, NULL, cnpmHome.c_str());
                    git_clone_options opts=GIT_CLONE_OPTIONS_INIT;
//                                int err=git_clone_options_init(&opts, GIT_CLONE_OPTIONS_VERSION);
                    cb_payload cbPayload;
                    cbPayload.publickey=cnpmHome+"/key.pem";
                    cbPayload.privatekey=cnpmHome+"/cert.pem";
                    cbPayload.cberr=1;
                    opts.checkout_opts=GIT_CHECKOUT_OPTIONS_INIT;
                    opts.checkout_opts.checkout_strategy = GIT_CHECKOUT_SAFE;
                    opts.fetch_opts=GIT_FETCH_OPTIONS_INIT;
                    opts.fetch_opts.callbacks.payload=&cbPayload;
                    opts.fetch_opts.callbacks.credentials=[](git_credential **out, const char *url, const char *username_from_url, unsigned int allowed_types, void *payload)->int{
                        std::cout<<"allowed_types "<<allowed_types<<std::endl;
                        cb_payload* cbPayload=(cb_payload*)(payload);
                        std::cout<<"cbPayload: "<<cbPayload->cberr<<" "<<allowed_types<<std::endl;
                        switch(allowed_types){
                            case GIT_CREDENTIAL_SSH_KEY:
                                break;
                            case GIT_CREDENTIAL_USERPASS_PLAINTEXT:
                                break;
                            case GIT_CREDENTIAL_USERNAME:
                                break;
                        }
                        out=&cbPayload->gitCredential;
                        cbPayload->cberr--;
                        std::cout<<"cbPayload: "<<cbPayload->cberr<<std::endl;
                        return cbPayload->cberr;
                    };
                    opts.fetch_opts.callbacks.sideband_progress=[](const char *str, int len, void *payload)->int{
                            (void)payload; /* unused */

                            std::printf("remote: %.*s", len, str);
                            std::cout.flush();
                            return 0;
                    };
//                                opts.fetch_opts.callbacks.transfer_progress=
//                                opts.fetch_opts.callbacks.certificate_check
                    git_repository *gitRepository;
                    std::string lpath={ std::begin(localPath.string()), std::end(localPath.string()) };
                    int err= git_clone(&gitRepository, uri.c_str(), lpath.c_str(), &opts);
                    if(err!=0)
                        std::cout<<"err "<<err<<" "<<git_error_last()->klass<<" "<<git_error_last()->message<<std::endl;
                    else{
                        const char *branch = NULL;
                        git_reference *head = NULL;

                        err = git_repository_head(&head, gitRepository);

                        if (err == GIT_EUNBORNBRANCH || err == GIT_ENOTFOUND)
                          branch = NULL;
                        else if (!err) {
                          branch = git_reference_shorthand(head);
                        }

                          printf("## %s\n", branch ? branch : "HEAD (no branch)");

                        git_reference_free(head);
                        
                        git_remote *remote = NULL;
                        err = git_remote_lookup(&remote, gitRepository, uri.c_str());
                        if (err < 0)
                            err = git_remote_create_anonymous(&remote, gitRepository, uri.c_str());
                            if (err >= 0) {
                                const git_remote_head **refs;
                                git_remote_callbacks callbacks = GIT_REMOTE_CALLBACKS_INIT;
                                err = git_remote_connect(remote, GIT_DIRECTION_FETCH, &callbacks, NULL, NULL);
                                if (err >= 0){
                                    size_t refs_len;
                                    if (git_remote_ls(&refs, &refs_len, remote) >= 0){
                                        bool hit=false;
                                        for (size_t i = 0; !hit && i < refs_len; i++) {
                                          char git_oid[GIT_OID_HEXSZ + 1] = {0};
                                          git_oid_fmt(git_oid, &refs[i]->oid);
                                          printf("%s\t%s\n", git_oid, refs[i]->name);
                                          if(std::string("HEAD").compare(refs[i]->name)==0){
                                              oid=std::string(git_oid);
                                              hit=true;
                                          }
                                        }
                                    }
                                }
                                else std::cout<<"err2 "<<err<<" "<<git_error_last()->klass<<" "<<git_error_last()->message<<std::endl;
                            }
                            else std::cout<<"err1 "<<err<<" "<<git_error_last()->klass<<" "<<git_error_last()->message<<std::endl;
                        git_remote_free(remote);
                    }
                    git_libgit2_shutdown();
                }
                if(!oid.empty())std::cout<<"oid "<<oid<<std::endl;
                if(!std::filesystem::exists(localLinkPath.parent_path()))std::filesystem::create_directories(localLinkPath.parent_path());
                if(!std::filesystem::exists(localLinkPath) && std::filesystem::exists(localPath)){
                    try{
                        std::filesystem::create_directory_symlink(localPath, localLinkPath);
                    }
                    catch(std::filesystem::filesystem_error& ex){
                        std::throw_with_nested( std::runtime_error("Couldn't create symbolic link " + localLinkPath.string()) );
                    }
                }
                recurseModules(localLinkPath);
            }
        
    }

    void Installation::recurseModules(std::filesystem::path localLinkPath){
         if(depth<2)
            for(auto& p: std::filesystem::directory_iterator(localLinkPath)){
                if(p.path().filename().compare("package.json")==0 && std::filesystem::exists(p.path())){
                    //std::cout<<depth<<" "<<p.path()<<std::endl;
                    sylvanmats::io::json::JsonBinder jsonBinder;
                    std::ifstream is(p.path());
                    jsonBinder(is);
                    depth++;
                    this->operator()(jsonBinder);
                    linkAnyBinaries(jsonBinder, localLinkPath);
                    depth--;
                    //std::cout<<"depth "<<depth<<std::endl;
                }
            }
    }
    
    void Installation::linkAnyBinaries(sylvanmats::io::json::JsonBinder& jb, std::filesystem::path& localLinkPath){
        if(std::filesystem::exists(localLinkPath)){
            std::filesystem::path binPath="./"+moduleDirectory;
            binPath/=".bin";
            if(!std::filesystem::exists(binPath))std::filesystem::create_directories(binPath);
            sylvanmats::io::json::path jp="bin";
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
    
}
