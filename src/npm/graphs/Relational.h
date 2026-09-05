#pragma once
#include <iostream>
#include <functional>
#include <vector>
#include <map>
#include <typeindex>
#include <string>
#include <string_view>
#include <tuple>


#include "graph/container/compressed_graph.hpp"

namespace sylvanmats::npm::graphs {

    struct project_properties {
        std::string_view name;
        std::string_view description;
        std::string url;
        std::string scope;
        std::string module_name;
        std::string wildcard;
        bool already_have_it=false;
        bool is_dev=false;
    };

    using G = graph::container::compressed_graph<std::string, project_properties>;

    class Relational {
        protected:
            G relationalGraph;
            std::vector<project_properties> vertices;
            std::vector<std::tuple<graph::vertex_id_t<G>, graph::vertex_id_t<G>, int>> edges;

        public:
            Relational() = default;
            Relational(const Relational& orig) =  delete;
            virtual ~Relational() = default;
            Relational& operator=(const Relational& other) = delete;

            void operator ()(project_properties vertex){
                auto it = std::find_if(vertices.begin(), vertices.end(), [&vertex](const project_properties& p) {
                    return p.name == vertex.name;
                });
                if (it == vertices.end())
                    vertices.push_back(vertex);
            };

            void operator ()(size_t source, size_t target){
                (*this)(getProject(source), getProject(target));
            };

            void operator ()(project_properties& source, project_properties& target){
                auto it = std::find_if(vertices.begin(), vertices.end(), [&source](const project_properties& p) {
                    return p.name == source.name;
                });
                if(it == vertices.end())return;
                size_t source_index = std::distance(vertices.begin(), it);
                it = std::find_if(vertices.begin(), vertices.end(), [&target](const project_properties& p) {
                    return p.name == target.name;
                });
                if(it == vertices.end())return;
                size_t target_index = std::distance(vertices.begin(), it);
                edges.push_back(std::make_tuple(source_index, target_index, 1));
            };

            size_t getNumberOfProjects(){
                return vertices.size();
            }
            size_t getNumberOfRelations(){
                return edges.size();
            }

            project_properties& getProject(size_t index){
                return vertices[index];
            }

            std::vector<size_t> enqueueMissingDependencies(){
                std::vector<size_t> missing;
                for(auto& v : vertices){
                    std::cout<<"v.already_have_it "<<v.already_have_it<<" "<<v.name<<std::endl;
                    if(!v.already_have_it){
                        // for(auto& e : edges){
                        //     if(vertices[std::get<0>(e)].source.compare(v.name)==0){
                        //         std::cout<<"missing dependency "<<e.target<<std::endl;
                        //     }
                        // }
                        missing.push_back(std::distance(vertices.begin(), std::find_if(vertices.begin(), vertices.end(), [&v](const project_properties& p){return p.name == v.name;})));
                    }
                }
                return missing;
            }

        public:
            friend std::ostream& operator<<(std::ostream& s, Relational& rg) {
            s<<R"({
  "graph": {
    "directed": true,
    "type": "graph type",
    "label": "graph label",
    "metadata": {
      "user-defined": "values"
    },
    "nodes": {
)";
    for (const auto& v : rg.vertices) {
        s<<R"(      ")"<<std::distance(rg.vertices.begin(), std::find_if(rg.vertices.begin(), rg.vertices.end(), [&v](const project_properties& p){return p.name == v.name;}))<<R"(": {
        "id": ")"<<v.name<<R"(",
        "label": ")"<<v.name<<R"(",
        "type": "vertex type",
        "metadata": {)";
        s<<R"(
          "develop": ")"<<(v.is_dev ? "true" : "false")<<R"(",
          "already_have_it": ")"<<(v.already_have_it ? "true" : "false")<<R"(",
          "user-defined": "values"
        }
    })" << (&v != &rg.vertices.back() ? "," : "");
    }
    s<<R"(    },
    "edges": [
      {
)";
            for (const auto& e : rg.edges) {
                s<<R"(      {
      "source": ")"<<std::get<0>(e)<<R"(",
        "relation": "depends on",
        "target": ")"<<std::get<1>(e)<<R"("    }
)"<<(&e != &rg.edges.back() ? "," : "");
            }
            s<<R"(    ]
  }
}
)";
            return s;
            }
    };

}