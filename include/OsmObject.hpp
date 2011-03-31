#ifndef OSMIUM_OSM_OBJECT_HPP
#define OSMIUM_OSM_OBJECT_HPP

/** @file
*   @brief Contains the Osmium::OSM::Object class.
*/

#include <stdexcept>
#include <vector>
#include <time.h>

#ifdef WITH_SHPLIB
# include <shapefil.h>
#endif

#ifdef WITH_JAVASCRIPT
# include "v8.h"
# include "JavascriptTemplate.hpp"
#endif

namespace Osmium {

    namespace OSM {

        /**
        *
        * Parent class for nodes, ways, and relations.
        *
        */
        class Object {

          public:

            static const int max_length_timestamp = 20 + 1; ///< maximum length of OSM object timestamp string (20 characters + null byte)

            static const int max_characters_username = 255;
            static const int max_utf16_length_username = 2 * (max_characters_username + 1); ///< maximum number of UTF-16 units

            static const int max_length_username = 255 * 4 + 1; ///< maximum length of OSM user name (255 UTF-8 characters + null byte)

            osm_object_id_t    id;        ///< object id
            osm_version_t      version;   ///< object version
            osm_user_id_t      uid;       ///< user id of user who last changed this object
            osm_changeset_id_t changeset; ///< id of last changeset that changed this object

            // timestamp is stored as string, no need to parse it in most cases
            char timestamp_str[max_length_timestamp];
            time_t timestamp;
            char user[max_length_username]; ///< name of user who last changed this object

          private:

            osm_object_id_t string_to_osm_object_id(const char *x) {
                return atol(x);
            }

            osm_version_t string_to_osm_version(const char *x) {
                return atoi(x);
            }

            osm_changeset_id_t string_to_osm_changeset_id(const char *x) {
                return atol(x);
            }

            osm_user_id_t string_to_osm_user_id(const char *x) {
                return atol(x);
            }

          protected:

            // how many tags are there on this object (XXX we could probably live without this and just use tags.size())
            int num_tags;

          public:

            std::vector<Tag> tags;

            void *wrapper;

            Object() : tags() {
                reset();
            }

            Object(const Object &o) {
                id        = o.id;
                version   = o.version;
                uid       = o.uid;
                changeset = o.changeset;
                timestamp = o.timestamp;
                num_tags  = o.num_tags;
                tags      = o.tags;
                strncpy(timestamp_str, o.timestamp_str, max_length_timestamp);
                strncpy(user, o.user, max_length_username);
            }

            ~Object() {
            }

            virtual osm_object_type_t get_type() const = 0;

            virtual void reset() {
                id               = 0;
                version          = 0;
                uid              = 0;
                changeset        = 0;
                timestamp_str[0] = '\0';
                timestamp        = 0;
                user[0]          = '\0';
                num_tags         = 0;
                tags.clear();
            }

            virtual void set_attribute(const char *attr, const char *value) {
                if (!strcmp(attr, "id")) {
                    id = string_to_osm_object_id(value);
                } else if (!strcmp(attr, "version")) {
                    version = string_to_osm_version(value);
                } else if (!strcmp(attr, "timestamp")) {
                    if (! memccpy(timestamp_str, value, 0, max_length_timestamp)) {
                        throw std::length_error("timestamp too long");
                    }
                } else if (!strcmp(attr, "uid")) {
                    uid = string_to_osm_user_id(value);
                } else if (!strcmp(attr, "user")) {
                    if (! memccpy(user, value, 0, max_length_username)) {
                        throw std::length_error("user name too long");
                    }
                } else if (!strcmp(attr, "changeset")) {
                    changeset = string_to_osm_changeset_id(value);
                }
            }

            virtual osm_object_id_t get_id() const {
                return id;
            }

            osm_version_t get_version() const {
                return version;
            }

            // get numerical user id, 0 if unknown
            osm_user_id_t get_uid() const {
                return uid;
            }

            const char *get_user() const {
                return user;
            }

            osm_changeset_id_t get_changeset() const {
                return changeset;
            }

            time_t get_timestamp() {
                if (timestamp == 0) {
                    if (timestamp_str[0] == '\0') {
                        return 0;
                    }
                    struct tm tm;
                    if (strptime(timestamp_str, "%Y-%m-%dT%H:%M:%SZ", &tm) == NULL) {
                        timestamp = -1;
                    } else {
                        timestamp = mktime(&tm);
                    }
                }
                return timestamp;
            }

            const char *get_timestamp_str() {
                if (timestamp_str[0] == '\0') {
                    if (timestamp == 0) {
                        return "";
                    }
                    struct tm *tm;
                    tm = gmtime(&timestamp);
                    strftime(timestamp_str, sizeof(timestamp_str), "%Y-%m-%dT%H:%M:%SZ", tm);
                }
                return timestamp_str;
            }

            void add_tag(const char *key, const char *value) {
                /* first we resize the vector... */
                tags.resize(num_tags+1);
                /* ...so that we can directly write into the memory and avoid
                a second copy */
                if (!memccpy(tags[num_tags].key, key, 0, Tag::max_length_key)) {
                    throw std::length_error("tag key too long");
                }
                if (!memccpy(tags[num_tags].value, value, 0, Tag::max_length_value)) {
                    throw std::length_error("tag value too long");
                }
                num_tags++;
            }

            int tag_count() const {
                return num_tags;
            }

            const char *get_tag_by_key(const char *key) const {
                for (int i=0; i < num_tags; i++) {
                    if (!strcmp(tags[i].key, key)) {
                        return tags[i].value;
                    }
                }
                return 0;
            }

            const char *get_tag_key(int n) const {
                if (n < num_tags) {
                    return tags[n].key;
                }
                throw std::range_error("no tag with this index");
            }

            const char *get_tag_value(int n) const {
                if (n < num_tags) {
                    return tags[n].value;
                }
                throw std::range_error("no tag with this index");
            }

#ifdef WITH_SHPLIB
            virtual SHPObject *create_shpobject(int shp_type) = 0;
#endif

#ifdef WITH_JAVASCRIPT
            v8::Local<v8::Object> js_object_instance;
            v8::Local<v8::Object> js_tags_instance;
#endif

#ifdef WITH_JAVASCRIPT
            v8::Local<v8::Object> get_instance() {
                return js_object_instance;
            }
            v8::Local<v8::Object> get_tags_instance() {
                return js_tags_instance;
            }
#endif

        }; // class Object

    } // namespace OSM

} // namespace Osmium

#endif // OSMIUM_OSM_OBJECT_HPP
