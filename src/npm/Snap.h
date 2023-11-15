#pragma once

#include <string>
#include <functional>
#include <algorithm>

#include "io/json/Binder.h"

#define FMT_HEADER_ONLY
#include "fmt/format.h"
#include "fmt/ranges.h"

namespace sylvanmats::npm{
    struct parts;
}

template <>
struct fmt::formatter<std::vector<sylvanmats::npm::parts>>{
    auto parse(format_parse_context& ctx){
        return ctx.begin();
    }
    
     template <typename FormatContext>
     auto format(const std::vector<sylvanmats::npm::parts>& v, FormatContext& ctx) -> decltype(ctx.out()){
        for (int i= 0; i < v.size(); ++i){
            fmt::vformat_to(ctx.out(), R"(  {:s}:
    plugin: {:s}
    source-type: {:s}
    source: {:s}
)", fmt::make_format_args(v[i].part, v[i].plugin, v[i].sourceType, v[i].source));
        }
        constexpr char* fmt={"\n"};
        return fmt::format_to(ctx.out(), fmt);
    }
};


namespace sylvanmats::npm{
    struct parts{
        std::string_view key;
        std::string part;
        std::string plugin;
        std::string sourceType;
        std::string source;
    };
    class Snap{
    protected:
        std::string moduleDirectory;
        std::string home;
        std::string cnpmHome;
        std::vector<parts> secondaryParts;
    public:
        Snap(std::string& moduleDirectory);
        Snap(const Snap& orig) =  delete;
        virtual ~Snap() = default;
    public:
        bool operator()(sylvanmats::io::json::Binder& jsonBinder, std::function<void(std::string& yamlContent)> apply);
    protected:
        inline std::tuple<std::string, std::string> parseModuleName(std::string_view& key){
            std::string moduleName=std::string{key};
            std::string scope="";
            if(moduleName.length()>0 && moduleName.at(0)=='@'){
                unsigned int offset=moduleName.find('/');
                if(offset!=std::string::npos){
                    scope=moduleName.substr(1, offset-1);
                    moduleName=moduleName.substr(offset+1);
                }
            }
            return std::make_tuple(scope, moduleName);
        };

    };
}
