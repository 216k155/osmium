#ifndef OSMIUM_OUTPUT_OSM_PBF_HPP
#define OSMIUM_OUTPUT_OSM_PBF_HPP

/*

Copyright 2011 Jochen Topf <jochen@topf.org> and others (see README).

This file is part of Osmium (https://github.com/joto/osmium).

Osmium is free software: you can redistribute it and/or modify it under the
terms of the GNU Lesser General Public License or (at your option) the GNU
General Public License as published by the Free Software Foundation, either
version 3 of the Licenses, or (at your option) any later version.

Osmium is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU Lesser General Public Licanse and the GNU
General Public License for more details.

You should have received a copy of the Licenses along with Osmium. If not, see
<http://www.gnu.org/licenses/>.

*/

#include <netinet/in.h>
#include <algorithm>

namespace Osmium {

    namespace Output {

        namespace OSM {

            class PBF : public Base {

                static const int NANO = 1000 * 1000 * 1000;
                static const unsigned int max_block_contents = 8000;
                FILE *fd;

                bool use_dense_format_;
                bool use_compression_;

                bool headerWritten;

                OSMPBF::Blob pbf_blob;
                OSMPBF::BlobHeader pbf_blob_header;

                OSMPBF::HeaderBlock pbf_header_block;
                OSMPBF::PrimitiveBlock pbf_primitive_block;

                std::vector<Osmium::OSM::Node*> nodes;
                std::vector<Osmium::OSM::Way*> ways;
                std::vector<Osmium::OSM::Relation*> relations;
                std::map<std::string, int> strings;

                static const int MAX_BLOB_SIZE = 32 * 1024 * 1024;
                char pack_buffer[MAX_BLOB_SIZE];

                char lasttype;

                void store_blob(const std::string &type, const std::string &data) {
                    if(use_compression()) {
                        z_stream z;

                        z.next_in   = (unsigned char*)  data.c_str();
                        z.avail_in  = data.size();
                        z.next_out  = (unsigned char*)  pack_buffer;
                        z.avail_out = MAX_BLOB_SIZE;
                        z.zalloc    = Z_NULL;
                        z.zfree     = Z_NULL;
                        z.opaque    = Z_NULL;

                        if (deflateInit(&z, Z_DEFAULT_COMPRESSION) != Z_OK) {
                            throw std::runtime_error("failed to init zlib stream");
                        }
                        if (deflate(&z, Z_FINISH) != Z_STREAM_END) {
                            throw std::runtime_error("failed to deflate zlib stream");
                        }
                        if (deflateEnd(&z) != Z_OK) {
                            throw std::runtime_error("failed to deinit zlib stream");
                        }

                        if(Osmium::global.debug) fprintf(stderr, "pack %d bytes to %ld bytes (1:%f))\n", data.size(), z.total_out, (double)data.size() / z.total_out);
                        pbf_blob.set_zlib_data(pack_buffer, z.total_out);
                    }
                    else { // use_compression
                        pbf_blob.set_raw(data);
                    }

                    pbf_blob.set_raw_size(data.size());
                    std::string blob;

                    pbf_blob.SerializeToString(&blob);
                    pbf_blob.Clear();

                    pbf_blob_header.set_type(type);
                    pbf_blob_header.set_datasize(blob.size());

                    std::string blobhead;
                    pbf_blob_header.SerializeToString(&blobhead);
                    pbf_blob_header.Clear();

                    int32_t sz = htonl(blobhead.size());
                    fwrite(&sz, sizeof(sz), 1, fd);
                    fwrite(blobhead.c_str(), blobhead.size(), 1, fd);
                    fwrite(blob.c_str(), blob.size(), 1, fd);
                }

                void store_header_block() {
                    if(Osmium::global.debug) fprintf(stderr, "storing header block\n");
                    std::string header;
                    pbf_header_block.SerializeToString(&header);
                    pbf_header_block.Clear();

                    store_blob("OSMHeader", header);
                    headerWritten = true;
                }

                void record_string(const std::string& string) {
                    if(strings.count(string) > 0)
                        strings[string]++;
                    else
                        strings.insert( std::pair<std::string, int>(string, 0) );
                }

                unsigned int index_string(const std::string& str) {
                    const OSMPBF::StringTable st = pbf_primitive_block.stringtable();

                    for(int i = 0, l = st.s_size(); i<l; i++) {
                        if(st.s(i) == str)
                            return i;
                    }

                    throw std::runtime_error("Request for string not in stringable");
                    return 0;
                }

                static bool str_sorter(const std::pair<std::string, int> &a, const std::pair<std::string, int> b) {
                    if(a.second > b.second)
                        return true;
                    else if(a.second < b.second)
                        return false;
                    else
                        return a.first < b.first;
                }

                void sort_and_store_strings() {
                    std::vector<std::pair<std::string, int>> strvec;

                    std::copy(strings.begin(), strings.end(), back_inserter(strvec));
                    std::sort(strvec.begin(), strvec.end(), str_sorter);

                    OSMPBF::StringTable *st = pbf_primitive_block.mutable_stringtable();
                    for(int i = 0, l = strvec.size(); i<l; i++) {
                        if(Osmium::global.debug) fprintf(stderr, "store stringtable: %s (cnt=%d)\n", strvec[i].first.c_str(), strvec[i].second+1);
                        st->add_s(strvec[i].first);
                    }
                }

                int latlon2int(double latlon) {
                    return (latlon * (long int)1000000000 / pbf_primitive_block.granularity());
                }

                int timestamp2int(time_t timestamp) {
                    return timestamp * (1000 / pbf_primitive_block.date_granularity());
                }

                void store_nodes_block() {
                    if(Osmium::global.debug) fprintf(stderr, "storing nodes block with %u nodes\n", nodes.size());
                    sort_and_store_strings();

                    OSMPBF::PrimitiveGroup *pbf_primitive_group = pbf_primitive_block.add_primitivegroup();
                    for(int i = 0, l = nodes.size(); i<l; i++) {
                        Osmium::OSM::Node *node = nodes[i];
                        OSMPBF::Node *pbf_node = pbf_primitive_group->add_nodes();

                        pbf_node->set_id(node->id);

                        pbf_node->set_lat(latlon2int(node->get_lat()));
                        pbf_node->set_lon(latlon2int(node->get_lon()));

                        for (int i=0, l = node->tag_count(); i < l; i++) {
                            pbf_node->add_keys(index_string(node->get_tag_key(i)));
                            pbf_node->add_vals(index_string(node->get_tag_value(i)));
                        }

                        OSMPBF::Info *pbf_node_info = pbf_node->mutable_info();
                        pbf_node_info->set_version((google::protobuf::int32) node->version);
                        pbf_node_info->set_timestamp((google::protobuf::int64) timestamp2int(node->timestamp));
                        pbf_node_info->set_changeset((google::protobuf::int64) node->changeset);
                        pbf_node_info->set_uid((google::protobuf::int64) node->uid);
                        pbf_node_info->set_user_sid(index_string(node->user));
                        delete node;
                    }
                    nodes.clear();
                    store_primitive_block();
                }

                void store_ways_block() {
                    if(Osmium::global.debug) fprintf(stderr, "storing ways block with %u ways\n", ways.size());
                    sort_and_store_strings();

                    OSMPBF::PrimitiveGroup *pbf_primitive_group = pbf_primitive_block.add_primitivegroup();
                    for(int i = 0, l = ways.size(); i<l; i++) {
                        Osmium::OSM::Way *way = ways[i];
                        OSMPBF::Way *pbf_way = pbf_primitive_group->add_ways();

                        pbf_way->set_id(way->id);

                        for (int i=0, l = way->tag_count(); i < l; i++) {
                            pbf_way->add_keys(index_string(way->get_tag_key(i)));
                            pbf_way->add_vals(index_string(way->get_tag_value(i)));
                        }

                        long int delta = 0;
                        for (int i=0, l = way->node_count(); i < l; i++) {
                            pbf_way->add_refs(way->get_node_id(i) - delta);
                            delta = way->get_node_id(i);
                        }

                        OSMPBF::Info *pbf_way_info = pbf_way->mutable_info();
                        pbf_way_info->set_version((google::protobuf::int32) way->version);
                        pbf_way_info->set_timestamp((google::protobuf::int64) timestamp2int(way->timestamp));
                        pbf_way_info->set_changeset((google::protobuf::int64) way->changeset);
                        pbf_way_info->set_uid((google::protobuf::int64) way->uid);
                        pbf_way_info->set_user_sid(index_string(way->user));
                        delete way;
                    }
                    ways.clear();
                    store_primitive_block();
                }

                void store_relations_block() {
                    if(Osmium::global.debug) fprintf(stderr, "storing relations block with %u relations\n", relations.size());
                }

                void store_primitive_block() {
                    if(Osmium::global.debug) fprintf(stderr, "storing primitive block\n");
                    std::string block;
                    pbf_primitive_block.SerializeToString(&block);
                    pbf_primitive_block.Clear();

                    // add empty string-table entry at index 0
                    strings.clear();
                    pbf_primitive_block.mutable_stringtable()->add_s("");

                    store_blob("OSMData", block);
                }

                void check_block_contents_counter(char type) {
                    if(lasttype != '\0' && lasttype != type) {
                        if(lasttype == 'n' && nodes.size() > 0)
                            store_nodes_block();

                        else if(lasttype == 'w' && ways.size())
                            store_ways_block();

                        else if(lasttype == 'r' && relations.size())
                            store_relations_block();

                    }
                    else if(nodes.size() >= max_block_contents) {
                        store_nodes_block();
                    }
                    else if(ways.size() >= max_block_contents) {
                        store_ways_block();
                    }
                    else if(relations.size() >= max_block_contents) {
                        store_relations_block();
                    }

                    lasttype = type;
                }

            public:

                PBF() : Base(),
                    fd(stdout),
                    use_dense_format_(true),
                    use_compression_(true),
                    headerWritten(false),
                    lasttype('\0') {

                    GOOGLE_PROTOBUF_VERIFY_VERSION;
                }

                PBF(std::string &filename) : Base(),
                    use_dense_format_(true),
                    use_compression_(true),
                    headerWritten(false),
                    lasttype('\0') {

                    GOOGLE_PROTOBUF_VERIFY_VERSION;

                    fd = fopen(filename.c_str(), "w");
                    if(!fd)
                        perror("unable to open outfile");
                }

                bool use_dense_format() const {
                    return use_dense_format_;
                }

                PBF& use_dense_format(bool d) {
                    use_dense_format_ = d;
                    return *this;
                }

                bool use_compression() const {
                    return use_compression_;
                }

                PBF& use_compression(bool d) {
                    use_compression_ = d;
                    return *this;
                }

                void write_init() {
                    pbf_header_block.add_required_features("OsmSchema-V0.6");

                    if(use_dense_format())
                        pbf_header_block.add_required_features("DenseNodes");

                    if(is_history_file())
                        pbf_header_block.add_required_features("HistoricalInformation");

                    // add empty string-table entry at index 0
                    pbf_primitive_block.mutable_stringtable()->add_s("");
                    pbf_header_block.set_writingprogram("Osmium (http://wiki.openstreetmap.org/wiki/Osmium)");
                }

                void write_bounds(double minlon, double minlat, double maxlon, double maxlat) {
                    if(!headerWritten) {
                        // TODO: encode lat/lon
                        //pbf_header_block.bbox().set_left(minlon);
                        //pbf_header_block.bbox().set_top(minlat);
                        //pbf_header_block.bbox().set_right(maxlon);
                        //pbf_header_block.bbox().set_bottom(maxlat);
                    }
                }

                // TODO: use dense format if enabled
                void write(Osmium::OSM::Node *node) {
                    if(!headerWritten)
                        store_header_block();

                    check_block_contents_counter('n');
                    if(Osmium::global.debug) fprintf(stderr, "node %d v%d\n", node->id, node->version);

                    for (int i=0, l = node->tag_count(); i < l; i++) {
                        record_string(node->get_tag_key(i));
                        record_string(node->get_tag_value(i));
                    }
                    record_string(node->user);

                    nodes.push_back(new Osmium::OSM::Node(*node));
                }

                void write(Osmium::OSM::Way *way) {
                    if(!headerWritten)
                        store_header_block();

                    check_block_contents_counter('w');
                    if(Osmium::global.debug) fprintf(stderr, "way %d v%d\n", way->id, way->version);

                    for (int i=0, l = way->tag_count(); i < l; i++) {
                        record_string(way->get_tag_key(i));
                        record_string(way->get_tag_value(i));
                    }
                    record_string(way->user);

                    ways.push_back(new Osmium::OSM::Way(*way));
                }

                void write(Osmium::OSM::Relation *relation) {
                    if(!headerWritten)
                        store_header_block();

                    check_block_contents_counter('r');
                    if(Osmium::global.debug) fprintf(stderr, "relation %d v%d\n", relation->id, relation->version);
                }

                void write_final() {
                    if(Osmium::global.debug) fprintf(stderr, "finishing\n");
                    if(nodes.size() > 0)
                        store_nodes_block();

                    if(ways.size() > 0)
                        store_ways_block();

                    if(relations.size() > 0)
                        store_relations_block();

                    if(fd)
                        fclose(fd);
                }

            }; // class PBF

        } // namespace OSM

    } // namespace Output

} // namespace Osmium

#endif // OSMIUM_OUTPUT_OSM_PBF_HPP
