#ifndef OSMIUM_STORAGE_MEMBER_HYBRID_HPP
#define OSMIUM_STORAGE_MEMBER_HYBRID_HPP

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

#include <iostream>
#include <map>
#include <vector>

#include <osmium/storage/member/multimap.hpp>
#include <osmium/storage/member/map_vector.hpp>

namespace Osmium {

    namespace Storage {

        namespace Member {

            typedef Osmium::Storage::Member::Vector vector_t;
            typedef Osmium::Storage::Member::MultiMap multimap_t;

            class HybridIterator {

            public:

                typedef multimap_t::value_type value_type;

                HybridIterator(vector_t::iterator begin1, vector_t::iterator end1, multimap_t::iterator begin2, multimap_t::iterator end2) :
                     m_begin1(begin1),
                     m_end1(end1),
                     m_begin2(begin2),
                     m_end2(end2) {
                }

                HybridIterator& operator++() {
                    if (m_begin1 == m_end1) {
                        ++m_begin2;
                    } else {
                        ++m_begin1;
                        while (m_begin1 != m_end1 && m_begin1->second == 0) { // ignore removed elements
                            ++m_begin1;
                        }
                    }
                    return *this;
                }

                HybridIterator operator++(int) {
                    HybridIterator tmp(*this);
                    operator++();
                    return tmp;
                }

                bool operator==(const HybridIterator& rhs) const {
                    return m_begin1 == rhs.m_begin1 &&
                           m_end1   == rhs.m_end1 &&
                           m_begin2 == rhs.m_begin2 &&
                           m_end2   == rhs.m_end2;
                }

                bool operator!=(const HybridIterator& rhs) const {
                    return ! operator==(rhs);
                }

                const value_type& operator*() {
                    if (m_begin1 == m_end1) {
                        return *m_begin2;
                    } else {
                        return *m_begin1;
                    }
                }

                const value_type* operator->() {
                    return &operator*();
                }

            private:

                vector_t::iterator m_begin1;
                vector_t::iterator m_end1;
                multimap_t::iterator m_begin2;
                multimap_t::iterator m_end2;

            };

            class Hybrid {

            public:

                typedef HybridIterator::value_type value_type;
                typedef HybridIterator iterator;

                Hybrid() :
                    m_multimap(),
                    m_vector() {
                }

                Hybrid(size_t reserve_size) :
                    m_multimap(),
                    m_vector(reserve_size) {
                }

                void unsorted_set(const osm_object_id_t member_id, const osm_object_id_t object_id) {
                    m_vector.set(member_id, object_id);
                }

                void set(const osm_object_id_t member_id, const osm_object_id_t object_id) {
                    m_multimap.set(member_id, object_id);
                }

                std::pair<iterator, iterator> get(const osm_object_id_t id) {
                    std::pair<vector_t::iterator, vector_t::iterator> result_vector = m_vector.get(id);
                    std::pair<multimap_t::iterator, multimap_t::iterator> result_multimap = m_multimap.get(id);
                    return std::make_pair(HybridIterator(result_vector.first, result_vector.second, result_multimap.first, result_multimap.second),
                                          HybridIterator(result_vector.second, result_vector.second, result_multimap.second, result_multimap.second));
                }

                void remove(const osm_object_id_t member_id, const osm_object_id_t object_id) {
                    m_vector.remove(member_id, object_id);
                    m_multimap.remove(member_id, object_id);
                }

                void consolidate() {
                    m_vector.erase_removed();
                    m_vector.append(m_multimap.begin(), m_multimap.end());
                    m_multimap.clear();
                    m_vector.sort();
                }

                void dump(int fd) {
                    consolidate();
                    m_vector.dump(fd);
                }

                size_t size() const {
                    return m_vector.size() + m_multimap.size();
                }

            private:

                Osmium::Storage::Member::MultiMap m_multimap;
                Osmium::Storage::Member::Vector m_vector;

            }; // Hybrid

        } // namespace Member

    } // namespace Storage

} // namespace Osmium

#endif // OSMIUM_STORAGE_MEMBER_HYBRID_HPP
