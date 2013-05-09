#ifndef OSMIUM_SER_BUFFER_MANAGER_HPP
#define OSMIUM_SER_BUFFER_MANAGER_HPP

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

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/utility.hpp>

#include <osmium/ser/buffer.hpp>

namespace Osmium {

    namespace Ser {

        namespace BufferManager {

            class Malloc : boost::noncopyable {

            public:
            
                Malloc(size_t size) : m_buffer(0) {
                    char* mem = static_cast<char*>(malloc(size));
                    if (!mem) {
                        throw std::bad_alloc();
                    }
                    m_buffer = new Osmium::Ser::Buffer(mem, size, boost::bind(&Malloc::full, this));
                }

                ~Malloc() {
                    free(m_buffer->ptr());
                    delete m_buffer;
                }

                Osmium::Ser::Buffer& buffer() {
                    return *m_buffer;
                }

                void full() {
                    throw std::range_error("buffer too small");
                }

            private:
            
                Osmium::Ser::Buffer* m_buffer;

            }; // class Malloc

        } // namespace BufferManager

    } // namespace Ser

} // namespace Osmium

#endif // OSMIUM_SER_BUFFER_MANAGER_HPP
