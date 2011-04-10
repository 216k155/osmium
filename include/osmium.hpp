#ifndef OSMIUM_OSMIUM_HPP
#define OSMIUM_OSMIUM_HPP

#ifdef WITH_GEOS
# include <geos/geom/GeometryFactory.h>
#endif

#ifdef WITH_JAVASCRIPT
# include <v8.h>
# include <unicode/ustring.h>
# include <osmium/utils/unicode.hpp>
#endif // WITH_JAVASCRIPT

namespace Osmium {

    class Framework;

    struct global {
        Framework *framework;
        bool debug;
#ifdef WITH_GEOS
        geos::geom::GeometryFactory *geos_geometry_factory;
#endif
    };

    extern struct global global;

} // namespace Osmium

#ifdef OSMIUM_MAIN
struct Osmium::global Osmium::global;
#endif

// check way geometry before making a shplib object from it
// normally this should be defined, otherwise you will generate invalid linestring geometries
#define CHECK_WAY_GEOMETRY

#ifdef WITH_JAVASCRIPT
# include <osmium/javascript/template.hpp>
#endif // WITH_JAVASCRIPT

#include <osmium/osm.hpp>
#include <osmium/input.hpp>
#include <osmium/framework.hpp>

#ifdef WITH_JAVASCRIPT
# include <osmium/output/csv.hpp>
# include <osmium/output/shapefile.hpp>
# include <osmium/javascript/object_templates.hpp>
# include <HandlerJavascript.hpp>
# ifdef OSMIUM_MAIN
namespace Osmium {

    namespace Javascript {

        namespace Template {

            Tags            *js_template_tags;
            NodeGeom        *js_template_nodegeom;
            Node            *js_template_node;
            Nodes           *js_template_nodes;
            WayGeom         *js_template_waygeom;
            Way             *js_template_way;
            Member          *js_template_member;
            Members         *js_template_members;
            Relation        *js_template_relation;
            Multipolygon    *js_template_multipolygon;
            OutputCSV       *js_template_output_csv;
            OutputShapefile *js_template_output_shapefile;

            v8::Local<v8::Object> create_tags_instance(void *wrapper) {
                return js_template_tags->create_instance(wrapper);
            }

            v8::Local<v8::Object> create_node_geom_instance(void *wrapper) {
                return js_template_nodegeom->create_instance(wrapper);
            }

            v8::Local<v8::Object> create_node_instance(void *wrapper) {
                return js_template_node->create_instance(wrapper);
            }

            v8::Local<v8::Object> create_way_nodes_instance(void *wrapper) {
                return js_template_nodes->create_instance(wrapper);
            }

            v8::Local<v8::Object> create_way_geom_instance(void *wrapper) {
                return js_template_waygeom->create_instance(wrapper);
            }

            v8::Local<v8::Object> create_way_instance(void *wrapper) {
                return js_template_way->create_instance(wrapper);
            }

            v8::Local<v8::Object> create_relation_member_instance(void *wrapper) {
                return js_template_member->create_instance(wrapper);
            }

            v8::Local<v8::Object> create_relation_members_instance(void *wrapper) {
                return js_template_members->create_instance(wrapper);
            }

            v8::Local<v8::Object> create_relation_instance(void *wrapper) {
                return js_template_relation->create_instance(wrapper);
            }

            v8::Local<v8::Object> create_multipolygon_instance(void *wrapper) {
                return js_template_multipolygon->create_instance(wrapper);
            }

            v8::Local<v8::Object> create_output_csv_instance(void *wrapper) {
                return js_template_output_csv->create_instance(wrapper);
            }

            v8::Local<v8::Object> create_output_shapefile_instance(void *wrapper) {
                return js_template_output_shapefile->create_instance(wrapper);
            }

            void init() {
                js_template_tags             = new Osmium::Javascript::Template::Tags;
                js_template_nodegeom         = new Osmium::Javascript::Template::NodeGeom;
                js_template_node             = new Osmium::Javascript::Template::Node;
                js_template_nodes            = new Osmium::Javascript::Template::Nodes;
                js_template_waygeom          = new Osmium::Javascript::Template::WayGeom;
                js_template_way              = new Osmium::Javascript::Template::Way;
                js_template_member           = new Osmium::Javascript::Template::Member;
                js_template_members          = new Osmium::Javascript::Template::Members;
                js_template_relation         = new Osmium::Javascript::Template::Relation;
                js_template_multipolygon     = new Osmium::Javascript::Template::Multipolygon;
                js_template_output_csv       = new Osmium::Javascript::Template::OutputCSV;
                js_template_output_shapefile = new Osmium::Javascript::Template::OutputShapefile;
            }

            void cleanup() {
                delete js_template_output_shapefile;
                delete js_template_output_csv;
                delete js_template_multipolygon;
                delete js_template_relation;
                delete js_template_members;
                delete js_template_member;
                delete js_template_way;
                delete js_template_waygeom;
                delete js_template_nodes;
                delete js_template_node;
                delete js_template_nodegeom;
                delete js_template_tags;
            }

        } // namespace Template

    } // namespace Javascript

} // namespace Osmium
# endif // OSMIUM_MAIN
#endif // WITH_JAVASCRIPT

#endif // OSMIUM_OSMIUM_HPP
