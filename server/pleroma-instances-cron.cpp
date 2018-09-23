#include <cstdio>
#include <cstdlib>
#include <curl/curl.h>
#include <iostream>
#include <map>
#include <vector>
#include <sstream>
#include <set>

#include <socialnet-1.h>

#include "picojson.h"
#include "distsn.h"


using namespace std;


class Host {
public:
	string domain;
	string title;
	string thumbnail;
	bool registration;
	bool chat;
	bool gopher;
	bool who_to_follow;
	bool media_proxy;
	bool scope_options;
	unsigned int text_limit;
public:
	Host (string a_domain, string a_title, string a_thumbnail):
		domain (a_domain),
		title (a_title),
		thumbnail (a_thumbnail),
		registration (false),
		chat (false),
		gopher (false),
		who_to_follow (false),
		media_proxy (false),
		scope_options (false),
		text_limit (0)
	{ };
	Host ():
		registration (false),
		chat (false),
		gopher (false),
		who_to_follow (false),
		media_proxy (false),
		scope_options (false),
		text_limit (0)
	{ };
public:
	string format () const;
};


static string escape_json (string in)
{
	string out;
	for (auto c: in) {
		if (c == '\n') {
			out += string {"\\n"};
		} else if (c == '"'){
			out += string {"\\\""};
		} else if (c == '\\'){
			out += string {"\\\\"};
		} else {
			out.push_back (c);
		}
	}
	return out;
}


string Host::format () const
{
	string out;
	out += "{";
	out += "\"domain\":\"" + escape_json (domain) + "\",";
	out += "\"title\":\"" + escape_json (title) + "\",";
	out += "\"thumbnail\":\"" + escape_json (thumbnail) + "\",";
	out += "\"registration\":" + (registration? string {"true"}: string {"false"}) + ",";
	out += "\"chat\":" + (chat? string {"true"}: string {"false"}) + ",";
	out += "\"gopher\":" + (gopher? string {"true"}: string {"false"}) + ",";
	out += "\"who_to_follow\":" + (who_to_follow? string {"true"}: string {"false"}) + ",";
	out += "\"media_proxy\":" + (media_proxy? string {"true"}: string {"false"}) + ",";
	out += "\"scope_options\":" + (scope_options? string {"true"}: string {"false"}) + ",";
	stringstream text_limit_stream;
	text_limit_stream << text_limit;
	out += "\"text_limit\":" + text_limit_stream.str ();
	out += "}";
	return out;
}


bool by_domain (const Host &left, const Host &right)
{
	return left.domain < right.domain;
}


static void write_storage (FILE *out, vector <Host> hosts)
{
	fprintf (out, "[");
	for (unsigned int cn = 0; cn < hosts.size (); cn ++) {
		if (0 < cn) {
			fprintf (out, ",\n");
		}
		Host host = hosts.at (cn);
		fprintf (out, host.format ().c_str ());
	}
	fprintf (out, "]");
}


static void get_host_nodeinfo (
	string host,
	bool &a_registration,
	bool &a_chat,
	bool &a_gopher,
	bool &a_suggestions,
	bool &a_media_proxy,
	socialnet::Http &http
) {
	string reply = http.perform (string {"https://"} + host + string {"/nodeinfo/2.0.json"});

	picojson::value json_value;
	string error = picojson::parse (json_value, reply);
	if (! error.empty ()) {
		throw (socialnet::HostException {__LINE__});
	}
	if (! json_value.is <picojson::object> ()) {
		throw (socialnet::HostException {__LINE__});
	}
	auto json_object = json_value.get <picojson::object> ();
	if (json_object.find (string {"openRegistrations"}) != json_object.end ()) {
		auto open_registrations_value = json_object.at (string {"openRegistrations"});
		if (open_registrations_value.is <bool> ()) {
			a_registration = open_registrations_value.get <bool> ();
		}
	}
	if (json_object.find (string {"metadata"}) != json_object.end ()) {
		auto metadata_value = json_object.at (string {"metadata"});
		if (metadata_value.is <picojson::object> ()) {
			auto metadata_object = metadata_value.get <picojson::object> ();
			if (metadata_object.find (string {"chat"}) != metadata_object.end ()) {
				auto chat_value = metadata_object.at (string {"chat"});
				if (chat_value.is <bool> ()) {
					bool chat_bool = chat_value.get <bool> ();
					a_chat = chat_bool;
				}
			}
			if (metadata_object.find (string {"gopher"}) != metadata_object.end ()) {
				auto gopher_value = metadata_object.at (string {"gopher"});
				if (gopher_value.is <bool> ()) {
					bool gopher_bool = gopher_value.get <bool> ();
					a_gopher = gopher_bool;
				}
			}
			if (metadata_object.find (string {"mediaProxy"}) != metadata_object.end ()) {
				auto media_proxy_value = metadata_object.at (string {"mediaProxy"});
				if (media_proxy_value.is <bool> ()) {
					bool media_proxy_bool = media_proxy_value.get <bool> ();
					a_media_proxy = media_proxy_bool;
				}
			}
			if (metadata_object.find (string {"suggestions"}) != metadata_object.end ()) {
				auto suggestions_value = metadata_object.at (string {"suggestions"});
				if (suggestions_value.is <picojson::object> ()) {
					auto suggestions_object = suggestions_value.get <picojson::object> ();
					if (suggestions_object.find (string {"enabled"}) != suggestions_object.end ()) {
						auto enabled_value = suggestions_object.at (string {"enabled"});
						if (enabled_value.is <bool> ()) {
							bool enabled_bool = enabled_value.get <bool> ();
							a_suggestions = enabled_bool;
						}
					}
				}
			}
		}
	}
}


static void get_host_statusnet_config (string host, bool &a_scope_options, unsigned int &a_text_limit, socialnet::Http &http)
{
	string reply = http.perform (string {"https://"} + host + string {"/api/statusnet/config"});

	picojson::value json_value;
	string error = picojson::parse (json_value, reply);
	if (! error.empty ()) {
		throw (socialnet::HostException {__LINE__});
	}
	if (! json_value.is <picojson::object> ()) {
		throw (socialnet::HostException {__LINE__});
	}
	auto json_object = json_value.get <picojson::object> ();
	if (json_object.find (string {"site"}) == json_object.end ()) {
		throw (socialnet::HostException {__LINE__});
	}
	auto site_value = json_object.at (string {"site"});
	if (! site_value.is <picojson::object> ()) {
		throw (socialnet::HostException {__LINE__});
	}
	auto site_object = site_value.get <picojson::object> ();

	if (site_object.find (string {"textlimit"}) != site_object.end ()) {
		auto textlimit_value = site_object.at (string {"textlimit"});
		if (textlimit_value.is <string> ()) {
			string textlimit_string = textlimit_value.get <string> ();
			unsigned int textlimit_int = 0;
			stringstream textlimit_stream {textlimit_string};
			textlimit_stream >> textlimit_int;
			a_text_limit = textlimit_int;
		}
	}

	if (site_object.find (string {"pleromafe"}) != site_object.end ()) {
		auto pleromafe_value = site_object.at (string {"pleromafe"});
		if (pleromafe_value.is <picojson::object> ()) {
			auto pleromafe_object = pleromafe_value.get <picojson::object> ();
			if (pleromafe_object.find (string {"scopeOptionsEnabled"}) != pleromafe_object.end ()) {
				auto scope_value = pleromafe_object.at (string {"scopeOptionsEnabled"});
				if (scope_value.is <bool> ()) {
					bool scope_bool = scope_value.get <bool> ();
					a_scope_options = scope_bool;
				}
			}
		}
	}
}


static Host for_host (shared_ptr <socialnet::Host> socialnet_host)
{
	socialnet::Http &http {* socialnet_host->http};
	string domain = socialnet_host->host_name;

	string title;
	string description;
	string thumbnail;
	try {
		socialnet_host->get_profile (title, description, thumbnail);
	} catch (socialnet::HostException e) {
		/* Do nothing. */
	}

	Host host {domain, title, thumbnail};
	
	try {
		bool registration = false;
		bool chat = false;
		bool gopher = false;
		bool suggestions = false;
		bool media_proxy = false;
		get_host_nodeinfo (domain, registration, chat, gopher, suggestions, media_proxy, http);
		host.registration = registration;
		host.chat = chat;
		host.gopher = gopher;
		host.who_to_follow = suggestions;
		host.media_proxy = media_proxy;
	} catch (socialnet::ExceptionWithLineNumber e) {
		cerr << e.line << endl;
	}

	try {
		bool scope_options = false;
		unsigned int text_limit = 0;
		get_host_statusnet_config (domain, scope_options, text_limit, http);
		host.scope_options = scope_options;
		host.text_limit = text_limit;
	} catch (socialnet::ExceptionWithLineNumber e) {
		cerr << e.line << endl;
	}
	
	return host;
}


int main (int argc, char **argv)
{
	auto socialnet_hosts = socialnet::get_hosts ();

	vector <Host> hosts;
	for (auto socialnet_host: socialnet_hosts) {
		cerr << socialnet_host->host_name << endl;
		try {
			Host host = for_host (socialnet_host);
			hosts.push_back (host);
		} catch (socialnet::HostException e) {
			/* Nothing. */
			cerr << "socialnet::HostException " << e.line << endl;
		}
	}
	
	sort (hosts.begin (), hosts.end (), by_domain);

	const string storage_filename = string {"/var/lib/distsn/pleroma-instances.json"};
	FILE * storage_file_out = fopen (storage_filename.c_str (), "w");
	if (storage_file_out != nullptr) {
		write_storage (storage_file_out, hosts);
		fclose (storage_file_out);
	}

	return 0;
}

