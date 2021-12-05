#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <span>
#include <tuple>
#include <any>
#include <iostream>
#include <functional>
#include <ranges>
#include <vector>

namespace sylvanmats::io::json{

    class path{
        friend class JsonBinder;
    protected:
        std::vector<std::string_view> p;
        std::string s;
    public:
        path() = default;
        path(const path& orig){
        for(auto sv : orig.p)
            p.push_back(sv);
//            std::cout<<orig.p[0]<<" cp constructor "<<p[0]<<std::endl;
        };
        path(const char* c){
//            std::cout<<"path(const char* c) "<<c<<std::endl;
            s=std::string(c);
            this->p.push_back(std::string_view(s));
        };
        
        virtual ~path() = default;
        
        const path& operator=(const char* c){
            //if(!this->p.empty())this->p.clear();
            //this->p.emplace_back(std::string(c));
//           std::cout<<"=(const char* c) "<<this->p.size()<<std::endl;
            return *this;
        }
        
        const path& operator=(const std::string& s){
            //if(!this->p.empty())this->p.clear();
            this->p.emplace_back(s);
            return *this;
        }
        
        const path& operator=(const std::string_view& sv){
            //if(!this->p.empty())this->p.clear();
       std::cout<<"= sv "<<sv<<std::endl;
            this->p.push_back(sv);
            return *this;
        }
        
        friend const path& operator/(const path& p, const std::string_view& sv){
            p.p.push_back(sv);
            return p;
        }

    };
    
    class JsonBinder{
    protected:
        std::string jsonContent="";
        std::vector<std::tuple<unsigned int, unsigned int, unsigned int, std::string_view, unsigned int>> objStart;
        std::vector<std::tuple<unsigned int, unsigned int, std::string_view, std::any>> vps;

    public:
        JsonBinder()=default;
        JsonBinder(const JsonBinder& orig) =  delete;
        virtual ~JsonBinder()= default;
        
        void operator ()(std::istream& is){
            this->jsonContent=std::string((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());
            bind(0);
        }
        
        void operator ()(std::string& jsonContent){
            this->jsonContent=jsonContent;
            bind(0);
        }
        
        //add
        bool operator ()(path& p, std::string key, std::string value){
            for(auto& d : p.p){
                for(auto s : objStart | std::views::filter([&d](std::tuple<unsigned int, unsigned int, unsigned int, std::string_view, unsigned int>& s){return std::get<3>(s).compare(d)==0;})){
                    for(auto& vp : vps | std::views::filter([&key, &s](std::tuple<unsigned int, unsigned int, std::string_view, std::any>& vp){return std::get<0>(vp)==std::get<1>(s) && std::get<1>(vp)==std::get<2>(s)+1 && std::get<2>(vp).compare(key)==0;})){
                        return false;
                    }
                    std::ranges::reverse_view rv {vps};
                    for(auto& p : rv | std::views::filter([&s](std::tuple<unsigned int, unsigned int, std::string_view, std::any>& p){return std::get<0>(p)==std::get<1>(s);})){
                        std::string::size_type offset=std::to_address(std::get<2>(p).begin())-std::to_address(jsonContent.begin());
                        std::string::size_type nl=jsonContent.find_first_of(",\n}{", offset);
                        if(nl==std::string::npos)nl=offset;
                        std::string::size_type originalSize=jsonContent.length();
                        jsonContent.insert(nl, ",\n    \""+key+"\": \""+value+"\"");
                        std::string::size_type currentSize=jsonContent.length();
                        auto it=std::find_if(vps.begin(),vps.end(), [&p](std::tuple<unsigned int, unsigned int, std::string_view, std::any>& vp){return std::get<0>(vp)==std::get<0>(p) && std::get<1>(vp)==std::get<1>(p) && std::get<2>(vp).compare(std::get<2>(p))==0;});
                        //if(it!=vps.end())it++;
                        if(it!=vps.end())vps.resize(std::distance(vps.begin(), it));
                        auto itS=std::find_if(objStart.begin(),objStart.end(), [&s](std::tuple<unsigned int, unsigned int, unsigned int, std::string_view, unsigned int>& s2){return std::get<0>(s2)==std::get<0>(s) && std::get<1>(s2)==std::get<1>(s) && std::get<2>(s2)==std::get<2>(s) && std::get<3>(s2).compare(std::get<3>(s))==0;});
                        if(itS!=objStart.end())itS++;
                        if(itS!=objStart.end())objStart.resize(std::distance(objStart.begin(), itS));
                        bind(nl, std::get<1>((p)));
                        return true;
                    }
                }
            }
            return false;
        }
        
        //remove
        bool operator ()(path& p, std::string key){
            for(auto& d : p.p){
                for(auto s : objStart | std::views::filter([&d](std::tuple<unsigned int, unsigned int, unsigned int, std::string_view, unsigned int>& s){return std::get<3>(s).compare(d)==0;})){
                    for(auto& p : vps | std::views::filter([&key,&s](std::tuple<unsigned int, unsigned int, std::string_view, std::any>& p){return std::get<0>(p)==std::get<1>(s) && std::get<2>(p).compare(key)==0;})){
                        auto it=std::find_if(vps.begin(),vps.end(), [&p](std::tuple<unsigned int, unsigned int, std::string_view, std::any>& vp){std::cout<<std::get<2>(vp)<<" "<<std::get<2>(p)<<std::endl;return std::get<0>(vp)==std::get<0>(p) && std::get<1>(vp)==std::get<1>(p) && std::get<2>(vp).compare(std::get<2>(p))==0;});
                        auto itS=std::find_if(objStart.begin(),objStart.end(), [&s](std::tuple<unsigned int, unsigned int, unsigned int, std::string_view, unsigned int>& s2){return std::get<0>(s2)==std::get<0>(s) && std::get<1>(s2)==std::get<1>(s) && std::get<2>(s2)==std::get<2>(s) && std::get<3>(s2).compare(std::get<3>(s))==0;});
                        if(itS!=objStart.end())itS++;
                        std::string::size_type distance=(it!=vps.end()) ? std::distance(vps.begin(), it): vps.size();
                        std::string::size_type distance2=(itS!=objStart.end()) ? std::distance(objStart.begin(), itS): objStart.size();
                        std::string::size_type start=std::to_address(jsonContent.data());
                        std::string::size_type offset=std::to_address(std::get<2>(p).data())-start;
                        std::string::size_type previousnl=jsonContent.find_last_of(",\n{", offset);
                        std::string::size_type nl=jsonContent.find_first_of(",\n}", offset);
                        if(nl==std::string::npos)nl=previousnl;
                        jsonContent.erase(previousnl, nl-previousnl);
                        if(distance<vps.size())vps.resize(distance);
                        if(distance2<objStart.size())objStart.resize(distance2);
                        bind(previousnl, std::get<1>((p)));
                        return true;
                    }
                }
            }
            return false;
        }
        
        //get
        void operator ()(path& p, std::function<void(std::any& v)> apply){
            for(auto d : p.p){
                for(auto p : vps | std::views::filter([&d](std::tuple<unsigned int, unsigned int, std::string_view, std::any>& p){return std::get<0>(p)==0 && std::get<2>(p).compare(d)==0;})){
                    //std::cout<<"\t"<<std::get<1>(p)<<" "<<std::get<2>(p)<<std::endl;
                    apply(std::get<3>(p));
                }
            }
        }
        
        //traverse
        void operator ()(path& p, std::function<void(std::string_view& key, std::any& v)> apply){
            for(auto d : p.p){
//                std::cout<<"d: "<<d<<std::endl;
                for(auto s : objStart | std::views::filter([&d](std::tuple<unsigned int, unsigned int, unsigned int, std::string_view, unsigned int>& s){return std::get<3>(s).compare(d)==0;})){
                    //std::cout<<std::get<1>(s)<<" here "<<std::get<2>(s)<<" "<<std::get<3>(s)<<std::endl;
                    for(auto p : vps | std::views::filter([&s](std::tuple<unsigned int, unsigned int, std::string_view, std::any>& p){return std::get<0>(p)==std::get<1>(s);})){
                        //std::cout<<"\t"<<std::get<1>(p)<<" "<<std::get<2>(p)<<std::endl;
                        apply(std::get<2>(p), std::get<3>(p));
                    }
                }
            }
        }
        
    protected:
        void bind(std::string::size_type offset, unsigned int objDepth=0){
            std::span s={jsonContent};
            std::span<char>::iterator it=s.begin();
            if(offset>0)std::advance(it, offset);
            std::string_view key;
            std::any value;
            bool hitColon=false;
            while(it!=s.end()){
                switch(*it){
                    case '{':
                        objStart.push_back(std::make_tuple(1, objStart.size(), objDepth++, key, (vps.size()>0) ? vps.size()-1 : 0));
                            hitColon=false;
                    it++;
                        break;
                    case '}':
                        objStart.push_back(std::make_tuple(0, objStart.size(), --objDepth, key, (vps.size()>0) ? vps.size()-1 : 0));
                            hitColon=false;
                    it++;
                        break;
                    case '"':
                    {
                    it++;
                        std::span<char>::iterator itStart=it;
                        int c=0;
                        while((*it)!='"'){it++;c++;};
                        if(hitColon){
                            bool once=true;
                            for(unsigned int objIndex=objStart.size()-1;once && objIndex>=0;objIndex--){
                                if(std::get<0>(objStart[objIndex])==1 && std::get<2>(objStart[objIndex])==objDepth-1){
                                    value=std::string_view(itStart, it);
                                    vps.push_back(std::make_tuple(objIndex, objDepth, key, value));
                                    once=false;
                                }
                            }
                            hitColon=false;
                        }
                        else key=std::string_view(itStart, it);
                    it++;
                    }
                        break;
                    case ':':
                        hitColon=true;
                    it++;
                        break;
                    case ',':
                        hitColon=false;
                    it++;
                        break;
                    case '[':
                    it++;
                        break;
                    case ']':
                    it++;
                        break;
                    default:
                    it++;
                        break;
                }
            }
        }
        
    public:
        friend std::ostream& operator<<(std::ostream& s, JsonBinder& jb) {
          s << jb.jsonContent;
          return s;
        }
        
    };
    
}
