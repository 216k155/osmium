#ifndef OSMIUM_SER_BUILDER_HPP
#define OSMIUM_SER_BUILDER_HPP

/*

Copyright 2013 Jochen Topf <jochen@topf.org> and others (see README).

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

#include <osmium/ser/buffer.hpp>
#include <osmium/ser/item.hpp>

namespace Osmium {

    namespace Ser {

        class Builder {

        public:

            Builder(Buffer& buffer, Builder* parent) :
                m_buffer(buffer),
                m_parent(parent),
                m_size(buffer.get_space_for<size_t>()) {
                *m_size = 0;
                if (m_parent) {
                    m_parent->add_size(sizeof(size_t));
                }
            }

            void add_size(size_t size) {
                *m_size += size;
                if (m_parent) {
                    m_parent->add_size(size);
                }
            }

            size_t size() const {
                return *m_size;
            }

            static const size_t pad_bytes = 8;

            /**
             * Add padding if needed.
             *
             * Adds size to parent, but not to self!
             */
            void add_padding() {
                size_t mod = size() % pad_bytes;
                if (mod != 0) {
                    m_buffer.get_space(8-mod);
                    if (m_parent) {
                        m_parent->add_size(8-mod);
                    }
                }
            }

            void add_string(const char* str) {
                size_t len = strlen(str) + 1;
                *m_buffer.get_space_for<size_t>() = len;
                m_buffer.append(str);
                add_size(sizeof(size_t) + len);

                size_t mod = len % pad_bytes;
                if (mod != 0) {
                    m_buffer.get_space(8-mod);
                    add_size(8-mod);
                }
            }

        protected:

            Buffer& m_buffer;
            Builder* m_parent;
            size_t* m_size;

        }; // Builder

        class NodeListBuilder : public Builder {

        public:

            NodeListBuilder(Buffer& buffer, Builder* parent=NULL) :
                Builder(buffer, parent) {
            }

            ~NodeListBuilder() {
                add_padding();
            }

            void add_node(osm_object_id_t ref) {
                *m_buffer.get_space_for<osm_object_id_t>() = ref;
                add_size(sizeof(osm_object_id_t));
            }

        }; // class NodeListBuilder

        class TagListBuilder : public Builder {

        public:

            TagListBuilder(Buffer& buffer, Builder* parent=NULL) :
                Builder(buffer, parent) {
            }

            ~TagListBuilder() {
                add_padding();
            }

            void add_tag(const char* key, const char* value) {
                size_t old_size = m_buffer.pos();
                m_buffer.append(key);
                m_buffer.append(value);
                add_size(m_buffer.pos() - old_size);
            }

        }; // class TagListBuilder

        class RelationMemberBuilder : public Builder {

        public:

            RelationMemberBuilder(Buffer& buffer, Builder* parent=NULL) :
                Builder(buffer, parent) {
            }

            ~RelationMemberBuilder() {
                add_padding();
            }

            void add_member(char type, osm_object_id_t ref, const char* role) {
                RelationMember* member = m_buffer.get_space_for<RelationMember>();
                member->type = type;
                member->ref = ref;
                add_size(sizeof(RelationMember));
                add_string(role);
            }

        }; // class RelationMemberBuilder

        template <class T>
        class ObjectBuilder : public Builder {

        public:

            ObjectBuilder(Buffer& buffer, Builder* parent=NULL) :
                Builder(buffer, parent),
                m_object(buffer.get_space_for<T>()) {
                add_size(sizeof(T));
                m_object->type = T::object_type();
            }

            T& object() {
                return *m_object;
            }

        private:

            T* m_object;

        }; // class ObjectBuilder

    } // namespace Ser

} // namespace Osmium

#endif // OSMIUM_SER_BUILDER_HPP
