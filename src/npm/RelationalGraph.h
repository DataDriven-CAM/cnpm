#include <iostream>
#include <functional>
#include <vector>
#include <map>
#include <typeindex>
#include <string>
#include <string_view>
#include <tuple>


#include "graph/container/compressed_graph.hpp"

namespace sylvanmats::npm {

    struct project_properties {
        std::string_view name;
        std::string_view description;
        std::string url;
        std::string license;
        std::string version;
        std::string author;
        std::string email;
    };

    using G = graph::container::compressed_graph<std::string, project_properties>;

    class RelationalGraph {
        protected:
            G relationalGraph;
            std::vector<project_properties> vertices;
            std::vector<std::tuple<graph::vertex_id_t<G>, graph::vertex_id_t<G>, int>> edges;

        public:
            RelationalGraph() = default;
            RelationalGraph(const RelationalGraph& orig) =  delete;
            virtual ~RelationalGraph() = default;
            RelationalGraph& operator=(const RelationalGraph& other) = delete;

            void operator ()(project_properties vertex){
                std::cout<<"vertex: "<<vertex.name<<std::endl;
                auto it = std::find_if(vertices.begin(), vertices.end(), [&vertex](const project_properties& p) {
                    return p.name == vertex.name;
                });
                if (it == vertices.end())
                    vertices.push_back(vertex);
            };

            void operator ()(project_properties& source, project_properties& target){
                std::cout<<"edge: "<<source.name<<" -> "<<target.name<<std::endl;
                auto it = std::find_if(vertices.begin(), vertices.end(), [&source](const project_properties& p) {
                    return p.name == source.name;
                });
                size_t source_index = std::distance(vertices.begin(), it);
                it = std::find_if(vertices.begin(), vertices.end(), [&target](const project_properties& p) {
                    return p.name == target.name;
                });
                size_t target_index = std::distance(vertices.begin(), it);
                edges.push_back(std::make_tuple(source_index, target_index, 1));
            };

        public:
            friend std::ostream& operator<<(std::ostream& s, RelationalGraph& rg) {
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
                s<<R"(      ")"<<v.name<<R"(": {
        "id": ")"<<v.name<<R"(",
        "label": ")"<<v.name<<R"(",
        "type": "vertex type",
        "metadata": {
          "user-defined": "values"
        }
      },
)";
            }
            s<<R"(    },
    "edges": {
)";
            for (const auto& e : rg.edges) {
                s<<R"(      )"<<std::get<0>(e)<<R"( -> )"<<std::get<1>(e)<<R"(,
)";
            }
            s<<R"(    }
}
)";
            return s;
            }
    };

}