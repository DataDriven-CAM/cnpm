#include "npm/Installation.h"

#include "Url.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <filesystem>
#include <regex>
#include <ranges>
#include <algorithm>

#include "npm/WebGetter.h"
#include "npm/TGZDecompressor.h"


namespace sylvanmats::npm{
    
    Installation::Installation() : home (std::getenv("HOME")), cnpmHome ((std::getenv("CNPM_HOME")!=NULL) ?std::getenv("CNPM_HOME") : ".") {
    }
    
    void Installation::operator()(std::string& packageName){
        url::Url url(packageName);
        std::cout<<"\t"<<url.has_scheme()<<" "<<url.syntax_ok()<<" "<<url.valid_host()<<" |" << url.host()<<"| "<<url.path()<<std::endl;
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
    
    void Installation::operator()(sylvanmats::io::json::JsonBinder& jb, sylvanmats::io::json::path p){
        jb(p, [&](std::string_view& key, std::any& v){
//            std::cout<<key<<" : "<<std::any_cast<std::string_view>(v)<<" "<<v.type().name()<<std::endl;
            std::string_view val{std::any_cast<std::string_view>(v)};
            install(key, val);
        });
    }
    
    void Installation::install(std::string_view& key, std::string_view& val){
            url::Url url(std::string{val});
            std::cout<<"\t"<<url.has_scheme()<<" "<<url.syntax_ok()<<" "<<url.valid_host()<<" |" << url.host()<<"| "<<url.path()<<std::endl;
            bool hitVersion=false;
            if(url.host().empty()){
                std::string moduleName=std::string{key};
                std::filesystem::path localLinkPath="./cpp_modules/"+moduleName;
                if(!std::filesystem::exists(localLinkPath)){
                    std::regex versionExpression(R"((\^)?((\d+)(\.(\d+)(\.(\d+))?)?))");
                    std::smatch versionMatch;
                    std::string version=std::string{val};
                    if (std::regex_match(version, versionMatch, versionExpression)) {
                        std::cout<<"versionMatch? "<<versionMatch.size()<<std::endl;
                        if (versionMatch.size() >= 4) {
                            std::ssub_match base_sub_match = versionMatch[3];
                            std::string base = base_sub_match.str();
                            std::cout << val << " has a major of " << base << '\n';
                        }
                        if (versionMatch.size() >= 3) {
                            std::ssub_match base_sub_match = versionMatch[2];
                            std::string base = base_sub_match.str();
                            std::cout << val << " version " << base << '\n';
                            sylvanmats::reading::WebGetter webGetter;
                            std::string uri = "https://registry.npmjs.org/"+moduleName+"/-/"+moduleName+"-"+base+".tgz";
                            std::cout<<"uri "<<uri<<std::endl;
                            std::string fileName=moduleName+"-"+base+".tgz";
                            std::filesystem::path tmpPath=std::filesystem::temp_directory_path()/fileName;
                            std::cout<<"t file "<<tmpPath<<std::endl;
                            std::ofstream tgzFile(tmpPath.c_str(), std::ios::binary);
                            webGetter(uri, tgzFile, [&moduleName, &base](std::istream& is){
                            });
                            tgzFile.close();
                            TGZDecompressor tgzDecompressor;
//                                    tgzDecompressor(is, tmpPath);
                            tgzDecompressor(tmpPath, [&](std::filesystem::path& newPath, std::ostream& content){
                                std::filesystem::path localPath=home+"/.cnpm/cpp_modules/"+moduleName;
                                if(!std::filesystem::exists(localLinkPath) && std::filesystem::exists(localPath))std::filesystem::create_directory_symlink(localPath, localLinkPath);
                                localPath/=newPath;
                                std::cout<<" "<<localPath.parent_path()<<" "<<localPath.filename()<<" "<<content.tellp()<<std::endl;
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
                    }
                }
            }
            if(url.syntax_ok() && !hitVersion){
                std::string uri=(url.host().empty()) ? "git://github.com/"+url.path()+".git" : url.as_string();
                std::string moduleName=std::string{key};
                std::filesystem::path localLinkPath="./cpp_modules/"+moduleName;
                std::filesystem::path localPath=home+"/.cnpm/cpp_modules/"+moduleName;
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
                    int err=git_clone(&gitRepository, uri.c_str(), localPath.c_str(), &opts);
                    if(err!=0)
                        std::cout<<"err "<<err<<" "<<git_error_last()->klass<<" "<<git_error_last()->message<<std::endl;
                    git_libgit2_shutdown();
                }
                if(!std::filesystem::exists(localLinkPath) && std::filesystem::exists(localPath))std::filesystem::create_directory_symlink(localPath, localLinkPath);
            }
        
    }
    
}
