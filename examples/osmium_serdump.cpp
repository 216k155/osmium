/*

  This is a small tool to dump the contents of the input file
  in serialized format to stdout.

  The code in this example file is released into the Public Domain.

*/

#include <iostream>

#define OSMIUM_WITH_PBF_INPUT
#define OSMIUM_WITH_XML_INPUT

#include <osmium.hpp>
#include <osmium/ser/buffer_manager.hpp>
#include <osmium/ser/handler.hpp>
#include <osmium/ser/index.hpp>
#include <osmium/storage/member/map_vector.hpp>

typedef Osmium::Ser::Index::VectorWithId index_t;
typedef Osmium::Storage::Member::MapVector map_t;

int main(int argc, char* argv[]) {
    std::ios_base::sync_with_stdio(false);

    assert(sizeof(Osmium::Ser::TypedItem) % 8 == 0);
    assert(sizeof(Osmium::Ser::Object)    % 8 == 0);
    assert(sizeof(Osmium::Ser::Node)      % 8 == 0);
    assert(sizeof(Osmium::Ser::Way)       % 8 == 0);
    assert(sizeof(Osmium::Ser::Relation)  % 8 == 0);

    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " OSMFILE DUMPFILE\n";
        exit(1);
    }

    Osmium::OSMFile infile(argv[1]);

    std::string dumpfile(argv[2]);
    int dump_fd = ::open(dumpfile.c_str(), O_WRONLY | O_CREAT, 0666);
    if (dump_fd < 0) {
        throw std::runtime_error(std::string("Can't open dump file ") + dumpfile);
    }

    Osmium::Ser::BufferManager::Malloc manager(1024 * 1024);
    index_t node_index;
    index_t way_index;
    index_t relation_index;
    map_t way_node_map;
    Osmium::Ser::Handler<Osmium::Ser::BufferManager::Malloc, index_t, index_t, index_t, map_t> handler(manager, node_index, way_index, relation_index, way_node_map, dump_fd);
    Osmium::Input::read(infile, handler);

    int fd = ::open("nodes.idx", O_WRONLY | O_CREAT, 0666);
    if (fd < 0) {
        throw std::runtime_error("Can't open nodes.idx");
    }
    node_index.dump(fd);
    close(fd);

    fd = ::open("ways.idx", O_WRONLY | O_CREAT, 0666);
    if (fd < 0) {
        throw std::runtime_error("Can't open ways.idx");
    }
    way_index.dump(fd);
    close(fd);

    fd = ::open("relations.idx", O_WRONLY | O_CREAT, 0666);
    if (fd < 0) {
        throw std::runtime_error("Can't open relations.idx");
    }
    relation_index.dump(fd);
    close(fd);

    way_node_map.dump();

    google::protobuf::ShutdownProtobufLibrary();
}

