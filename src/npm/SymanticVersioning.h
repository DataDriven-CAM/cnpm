#pragma once

#include <string_view>
#include <regex>
#include <functional>

namespace sylvanmats::npm{
    class SymanticVersioning{
    public:
        SymanticVersioning() = default;
        SymanticVersioning(const SymanticVersioning* orig) = delete;
        virtual ~SymanticVersioning() = default;
    public:
        void operator()(std::string_view& val, std::function<void(std::string_view base, std::string_view wildcard)> apply){
            std::regex versionExpression(R"((\^|\~)?((\d+)(\.(\d+|x)(\.(\d+|x))?)?))");
            std::smatch versionMatch;
            std::string semanticVersion=std::string{val};
            if (std::regex_match(semanticVersion, versionMatch, versionExpression)) {
                std::string version;
                std::string wildcard="latest";
                std::string major;
                std::string minor;
                std::string patch;
                std::string prefix;
//                std::cout<<"versionMatch? "<<versionMatch.size()<<std::endl;
                if (versionMatch.size() >= 4) {
                    std::ssub_match base_sub_match = versionMatch[3];
                    major = base_sub_match.str();
                }
                if (versionMatch.size() >= 6) {
                    std::ssub_match base_sub_match = versionMatch[5];
                    minor = base_sub_match.str();
                }
                if (versionMatch.size() >= 8) {
                    std::ssub_match base_sub_match = versionMatch[7];
                    patch = base_sub_match.str();
                }
                if (versionMatch.size() >= 3) {
                    std::ssub_match base_sub_match = versionMatch[2];
                    version=base_sub_match.str();
                    wildcard=version;
//                    std::cout << val << " has is " << version << '\n';
                }
                if (versionMatch.size() >= 2) {
                    std::ssub_match base_sub_match = versionMatch[1];
                    prefix = base_sub_match.str();
                }
                if(prefix.compare("^")==0)wildcard=major+"."+minor+".x";
//                std::cout << val << " becomes " << wildcard <<" "<<major<<" "<<minor<<" "<<patch<<" "<< '\n';
                apply(std::string_view{version}, std::string_view{wildcard});
            }
        }
    };

}
