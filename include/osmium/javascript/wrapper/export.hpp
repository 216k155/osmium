#ifndef OSMIUM_JAVASCRIPT_WRAPPER_EXPORT_HPP
#define OSMIUM_JAVASCRIPT_WRAPPER_EXPORT_HPP

/*

Copyright 2012 Jochen Topf <jochen@topf.org> and others (see README).

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

#include <v8.h>

#include <osmium/javascript/unicode.hpp>
#include <osmium/javascript/template.hpp>

#include <osmium/export/csv.hpp>
#ifdef OSMIUM_WITH_SHPLIB
# include <osmium/export/shapefile.hpp>
#endif

namespace Osmium {

    namespace Javascript {

        namespace Wrapper {

            struct ExportCSV : public Osmium::Javascript::Template {

                static v8::Handle<v8::Value> print(const v8::Arguments& args, Osmium::Export::CSV* csv) {
                    for (int i = 0; i < args.Length(); i++) {
                        if (i != 0) {
                            csv->out << '\t';
                        }
                        Osmium::v8_String_to_ostream(args[i]->ToString(), csv->out);
                    }
                    csv->out << std::endl;
                    return v8::Integer::New(1);
                }

                static v8::Handle<v8::Value> close(const v8::Arguments& /*args*/, Osmium::Export::CSV* csv) {
                    csv->out.flush();
                    csv->out.close();
                    return v8::Undefined();
                }

                ExportCSV() : Osmium::Javascript::Template() {
                    js_template->Set("print", v8::FunctionTemplate::New(function_template<Osmium::Export::CSV, print>));
                    js_template->Set("close", v8::FunctionTemplate::New(function_template<Osmium::Export::CSV, close>));
                }

            };

#ifdef OSMIUM_WITH_SHPLIB
            struct ExportShapefile : public Osmium::Javascript::Template {

                static void add_string_attribute(Osmium::Export::Shapefile* shapefile, int n, v8::Local<v8::Value> value) {
                    uint16_t source[(Osmium::Export::Shapefile::max_dbf_field_length+2)*2];
                    char dest[(Osmium::Export::Shapefile::max_dbf_field_length+1)*4];
                    memset(source, 0, (Osmium::Export::Shapefile::max_dbf_field_length+2)*4);
                    memset(dest, 0, (Osmium::Export::Shapefile::max_dbf_field_length+1)*4);
                    int32_t dest_length;
                    UErrorCode error_code = U_ZERO_ERROR;
                    value->ToString()->Write(source, 0, Osmium::Export::Shapefile::max_dbf_field_length+1);
                    u_strToUTF8(dest, shapefile->field(n).width(), &dest_length, source, std::min(Osmium::Export::Shapefile::max_dbf_field_length+1, value->ToString()->Length()), &error_code);
                    if (error_code == U_BUFFER_OVERFLOW_ERROR) {
                        // thats ok, it just means we clip the text at that point
                    } else if (U_FAILURE(error_code)) {
                        throw std::runtime_error("UTF-16 to UTF-8 conversion failed");
                    }
                    shapefile->add_attribute(n, dest);
                }

                static void add_logical_attribute(Osmium::Export::Shapefile* shapefile, int n, v8::Local<v8::Value> value) {
                    v8::String::Utf8Value str(value);

                    if (atoi(*str) == 1 || !strncasecmp(*str, "T", 1) || !strncasecmp(*str, "Y", 1)) {
                        shapefile->add_attribute(n, true);
                    } else if ((!strcmp(*str, "0")) || !strncasecmp(*str, "F", 1) || !strncasecmp(*str, "N", 1)) {
                        shapefile->add_attribute(n, false);
                    } else {
                        shapefile->add_attribute(n);
                    }
                }

                /**
                * Add a geometry to the shapefile.
                */
                static bool add(Osmium::Export::Shapefile* shapefile,
                                Osmium::Geometry::Geometry* geometry, ///< the geometry
                                v8::Local<v8::Object> attributes) {   ///< a %Javascript object (hash) with the attributes

                    try {
                        shapefile->add_geometry(Osmium::Geometry::create_shp_object(*geometry));
                    } catch (Osmium::Exception::IllegalGeometry) {
                        return false;
                    }

                    for (size_t n=0; n < shapefile->fields().size(); n++) {
                        v8::Local<v8::String> key = v8::String::New(shapefile->field(n).name().c_str());
                        if (attributes->HasRealNamedProperty(key)) {
                            v8::Local<v8::Value> value = attributes->GetRealNamedProperty(key);
                            if (value->IsUndefined() || value->IsNull()) {
                                shapefile->add_attribute(n);
                            } else {
                                switch (shapefile->field(n).type()) {
                                    case FTString:
                                        add_string_attribute(shapefile, n, value);
                                        break;
                                    case FTInteger:
                                        shapefile->add_attribute(n, value->Int32Value());
                                        break;
                                    case FTDouble:
                                        throw std::runtime_error("fields of type double not implemented");
                                        break;
                                    case FTLogical:
                                        add_logical_attribute(shapefile, n, value);
                                        break;
                                    default:
                                        // should never be here
                                        break;
                                }
                            }
                        } else {
                            shapefile->add_attribute(n);
                        }
                    }
                    return true;
                }

                static v8::Handle<v8::Value> add_field(const v8::Arguments& args, Osmium::Export::Shapefile* shapefile) {
                    if (args.Length() < 3 || args.Length() > 4) {
                        throw std::runtime_error("Wrong number of arguments to add_field method.");
                    }

                    v8::String::Utf8Value name(args[0]);
                    std::string sname(*name);

                    v8::String::Utf8Value type(args[1]);
                    std::string stype(*type);

                    int width = args[2]->Int32Value();
                    int decimals = (args.Length() == 4) ? args[3]->Int32Value() : 0;

                    shapefile->add_field(sname, stype, width, decimals);

                    return v8::Integer::New(1);
                }

                static v8::Handle<v8::Value> add(const v8::Arguments& args, Osmium::Export::Shapefile* shapefile) {
                    if (args.Length() != 2) {
                        throw std::runtime_error("Wrong number of arguments to add method.");
                    }

                    v8::Local<v8::Object> xxx = v8::Local<v8::Object>::Cast(args[0]);
                    Osmium::Geometry::Geometry* geometry = (Osmium::Geometry::Geometry*) v8::Local<v8::External>::Cast(xxx->GetInternalField(0))->Value();

                    try {
                        add(shapefile, geometry, v8::Local<v8::Object>::Cast(args[1]));
                    } catch (Osmium::Exception::IllegalGeometry) {
                        std::cerr << "Ignoring object with illegal geometry." << std::endl;
                        return v8::Integer::New(0);
                    }

                    return v8::Integer::New(1);
                }

                static v8::Handle<v8::Value> close(const v8::Arguments& /*args*/, Osmium::Export::Shapefile* shapefile) {
                    shapefile->close();
                    return v8::Undefined();
                }

                ExportShapefile() : Osmium::Javascript::Template() {
                    js_template->Set("add_field", v8::FunctionTemplate::New(function_template<Osmium::Export::Shapefile, add_field>));
                    js_template->Set("add",       v8::FunctionTemplate::New(function_template<Osmium::Export::Shapefile, add>));
                    js_template->Set("close",     v8::FunctionTemplate::New(function_template<Osmium::Export::Shapefile, close>));
                }

            };
#endif // OSMIUM_WITH_SHPLIB

        } // namespace Wrapper

    } // namespace Javascript

} // namespace Osmium

#endif // OSMIUM_JAVASCRIPT_WRAPPER_EXPORT_HPP
