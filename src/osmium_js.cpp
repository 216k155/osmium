
#include "osmium.hpp"

Osmium::Handler::NLS_Sparsetable *osmium_handler_node_location_store;
Osmium::Handler::Javascript      *osmium_handler_javascript;
Osmium::Handler::Bbox            *osmium_handler_bbox;

void init_handler() {
    osmium_handler_node_location_store->callback_init();
    osmium_handler_javascript->callback_init();
}

void node_handler(Osmium::OSM::Node *node) {
    osmium_handler_node_location_store->callback_node(node);
    osmium_handler_javascript->callback_node(node);
    osmium_handler_bbox->callback_node(node);
}

void way_handler(Osmium::OSM::Way *way) {
    osmium_handler_node_location_store->callback_way(way);
    osmium_handler_javascript->callback_way(way);
}

void relation_handler(Osmium::OSM::Relation *relation) {
    osmium_handler_javascript->callback_relation(relation);
}

void final_handler() {
    osmium_handler_bbox->callback_final();
    osmium_handler_node_location_store->callback_final();
    osmium_handler_javascript->callback_final();
}

struct callbacks *setup_callbacks() {
    static struct callbacks cb;
    cb.init             = init_handler;
    cb.node             = node_handler;
    cb.way              = way_handler;
    cb.relation         = relation_handler;
    cb.final            = final_handler;
    return &cb;
}

/* ================================================== */

v8::Persistent<v8::Context> global_context;

int main(int argc, char *argv[])
{
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " JAVASCRIPTFILE OSMFILE\n";
        exit(1);
    }

    v8::HandleScope handle_scope;

    v8::Handle<v8::ObjectTemplate> global_template = v8::ObjectTemplate::New();
    global_template->Set(v8::String::New("print"), v8::FunctionTemplate::New(Osmium::Handler::Javascript::Print));
    global_template->Set(v8::String::New("include"), v8::FunctionTemplate::New(Osmium::Handler::Javascript::Include));

    global_context = v8::Persistent<v8::Context>::New(v8::Context::New(0, global_template));
    v8::Context::Scope context_scope(global_context);

    Osmium::Javascript::Template::init();

    osmium_handler_node_location_store = new Osmium::Handler::NLS_Sparsetable;
    osmium_handler_javascript          = new Osmium::Handler::Javascript(argv[1]);
    osmium_handler_bbox                = new Osmium::Handler::Bbox;

    Osmium::Javascript::Node::Wrapper     *wrap_node     = new Osmium::Javascript::Node::Wrapper;
    Osmium::Javascript::Way::Wrapper      *wrap_way      = new Osmium::Javascript::Way::Wrapper;
    Osmium::Javascript::Relation::Wrapper *wrap_relation = new Osmium::Javascript::Relation::Wrapper;

    Osmium::OSM::Node     *node     = wrap_node->object;
    Osmium::OSM::Way      *way      = wrap_way->object;
    Osmium::OSM::Relation *relation = wrap_relation->object;

    parse_osmfile(argv[2], setup_callbacks(), node, way, relation);
	
    global_context.Dispose();

    return 0;
}

