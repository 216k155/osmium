
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <string.h>

#include "osmium.hpp"
#include "XMLParser.hpp"
#include "PBFParser.hpp"

Osmium::Handler::Statistics      *osmium_handler_stats;
Osmium::Handler::TagStats        *osmium_handler_tagstats;
//Osmium::Handler::NLS_Sparsetable *osmium_handler_node_location_store;

void init_handler() {
    // osmium_handler_stats->callback_init();
    osmium_handler_tagstats->callback_init();
    // osmium_handler_node_location_store->callback_init();
}

void before_nodes_handler() {
    osmium_handler_tagstats->callback_before_nodes();
}

void after_nodes_handler() {
    osmium_handler_tagstats->callback_after_nodes();
}

void before_ways_handler() {
    osmium_handler_tagstats->callback_before_ways();
}

void after_ways_handler() {
    osmium_handler_tagstats->callback_after_ways();
}

void before_relations_handler() {
    osmium_handler_tagstats->callback_before_relations();
}

void after_relations_handler() {
    osmium_handler_tagstats->callback_after_relations();
}

void final_handler() {
    // osmium_handler_node_location_store->callback_final();
    osmium_handler_stats->callback_final();
    osmium_handler_tagstats->callback_final();
}

void node_handler(Osmium::OSM::Node *node) {
    osmium_handler_stats->callback_object(node);
    osmium_handler_tagstats->callback_object(node);
    osmium_handler_stats->callback_node(node);
//    osmium_handler_node_location_store->callback_node(node);
}

void way_handler(Osmium::OSM::Way *way) {
    osmium_handler_stats->callback_object(way);
    osmium_handler_tagstats->callback_object(way);
    osmium_handler_stats->callback_way(way);
//    osmium_handler_node_location_store->callback_way(way);
}

void relation_handler(Osmium::OSM::Relation *relation) {
    osmium_handler_stats->callback_object(relation);
    osmium_handler_tagstats->callback_object(relation);
    osmium_handler_stats->callback_relation(relation);
}


struct callbacks callbacks = {
    init_handler,

    before_nodes_handler,
    node_handler,
    after_nodes_handler,

    before_ways_handler,
    way_handler,
    after_ways_handler,

    before_relations_handler,
    relation_handler,
    after_relations_handler,

    NULL, // multipolygon

    final_handler
};

/* ================================================== */

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " OSMFILE\n";
        exit(1);
    }

    osmium_handler_stats               = new Osmium::Handler::Statistics;
    osmium_handler_tagstats            = new Osmium::Handler::TagStats;
//    osmium_handler_node_location_store = new Osmium::Handler::NLS_Sparsetable;

    Osmium::OSM::Node     *node     = new Osmium::OSM::Node;
    Osmium::OSM::Way      *way      = new Osmium::OSM::Way;
    Osmium::OSM::Relation *relation = new Osmium::OSM::Relation;

    parse_osmfile(argv[1], &callbacks, node, way, relation);

    return 0;
}

