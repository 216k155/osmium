#ifndef OSMIUM_JAVASCRIPT_OBJECT_TEMPLATES_HPP
#define OSMIUM_JAVASCRIPT_OBJECT_TEMPLATES_HPP

/*

Copyright 2011 Jochen Topf <jochen@topf.org> and others (see README).

This file is part of Osmium (https://github.com/joto/osmium).

Osmium is free software: you can redistribute it and/or modify it under the
terms of the GNU Lesser General Public License or (at your option) the GNU
General Public License as published by the Free Software Foundation, either
version 3 of the Licenses, or (at your option) any later version.

Osmium is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU Lesser General Public License and the GNU
General Public License for more details.

You should have received a copy of the Licenses along with Osmium. If not, see
<http://www.gnu.org/licenses/>.

*/

namespace Osmium {

    namespace Javascript {

        namespace Template {

            class Tags : public Base {

            public:

                Tags() : Base(1) {
                    js_template->SetNamedPropertyHandler(
                        named_property_getter<Osmium::OSM::Object, &Osmium::OSM::Object::js_get_tag_value_by_key>,
                        0,
                        0,
                        0,
                        property_enumerator<Osmium::OSM::Object, &Osmium::OSM::Object::js_enumerate_tag_keys>
                    );
                }

            }; // class Tags

            class NodeGeom : public Base {

            public:

                NodeGeom() : Base(1) {
                    js_template->SetNamedPropertyHandler(named_property_getter<Osmium::OSM::Node, &Osmium::OSM::Node::js_get_geom_property>);
                }

            }; // class NodeGeom

            class WayGeom : public Base {

            public:

                WayGeom() : Base(1) {
                    js_template->SetNamedPropertyHandler(named_property_getter<Osmium::OSM::Way, &Osmium::OSM::Way::js_get_geom_property>);
                }

            }; // class WayGeom

            class Nodes : public Base {

            public:

                Nodes() : Base(1) {
                    js_template->SetAccessor(v8::String::New("length"), accessor_getter<Osmium::OSM::Way, &Osmium::OSM::Way::js_get_nodes_length>);
                    js_template->SetIndexedPropertyHandler(
                        indexed_property_getter<Osmium::OSM::Way, &Osmium::OSM::Way::js_get_node_id>,
                        0,
                        0,
                        0,
                        property_enumerator<Osmium::OSM::Way, &Osmium::OSM::Way::js_enumerate_nodes>
                    );
                }

            }; // class Nodes

            class Members : public Base {

            public:

                Members() : Base(1) {
                    js_template->SetAccessor(v8::String::New("length"), accessor_getter<Osmium::OSM::Relation, &Osmium::OSM::Relation::js_get_members_length>);
                    js_template->SetIndexedPropertyHandler(
                        indexed_property_getter<Osmium::OSM::Relation, &Osmium::OSM::Relation::js_get_member>,
                        0,
                        0,
                        0,
                        property_enumerator<Osmium::OSM::Relation, &Osmium::OSM::Relation::js_enumerate_members>
                    );
                }

            }; // class Members

            class Multipolygon : public Osmium::OSM::Object::JavascriptTemplate {

            public:

                Multipolygon() : Osmium::OSM::Object::JavascriptTemplate() {
                    js_template->SetAccessor(v8::String::New("from"), accessor_getter<Osmium::OSM::Multipolygon, &Osmium::OSM::Multipolygon::js_get_from>);
                    js_template->SetAccessor(v8::String::New("geom"), accessor_getter<Osmium::OSM::Multipolygon, &Osmium::OSM::Multipolygon::js_get_geom>);
                }

            }; // class Multipolygon

            class MultipolygonGeom : public Base {

            public:

                MultipolygonGeom() : Base(1) {
                    js_template->SetNamedPropertyHandler(named_property_getter<Osmium::OSM::Multipolygon, &Osmium::OSM::Multipolygon::js_get_geom_property>);
                }

            }; // class MultipolygonGeom

            class OutputCSV : public Base {

            public:

                OutputCSV() : Base(1) {
                    js_template->Set("print", v8::FunctionTemplate::New(function_template<Osmium::Output::CSV, &Osmium::Output::CSV::js_print>));
                    js_template->Set("close", v8::FunctionTemplate::New(function_template<Osmium::Output::CSV, &Osmium::Output::CSV::js_close>));
                }

            }; // class OutputCSV

            class OutputShapefile : public Base {

            public:

                OutputShapefile() : Base(1) {
                    js_template->Set("add_field", v8::FunctionTemplate::New(function_template<Osmium::Output::Shapefile, &Osmium::Output::Shapefile::js_add_field>));
                    js_template->Set("add",       v8::FunctionTemplate::New(function_template<Osmium::Output::Shapefile, &Osmium::Output::Shapefile::js_add>));
                    js_template->Set("close",     v8::FunctionTemplate::New(function_template<Osmium::Output::Shapefile, &Osmium::Output::Shapefile::js_close>));
                }

            }; // class OutputShapefile

        } // namespace Template

    } // namespace Javascript

} // namespace Osmium

#endif // OSMIUM_JAVASCRIPT_OBJECT_TEMPLATES_HPP
