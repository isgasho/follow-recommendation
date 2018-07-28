#include <cstdio>
#include <cstdlib>
#include <curl/curl.h>
#include <iostream>
#include <map>
#include <vector>
#include <sstream>
#include <set>
#include "picojson.h"
#include "distsn.h"


using namespace std;


class Host {
public:
	string domain;
	string title;
	string thumbnail;
	bool media_proxy;
	bool who_to_follow;
	bool chat;
	bool registration;
	bool scope_options;
	unsigned int text_limit;
public:
	Host (string a_domain, string a_title, string a_thumbnail):
		domain (a_domain),
		title (a_title),
		thumbnail (a_thumbnail),
		media_proxy (false),
		who_to_follow (false),
		chat (false),
		registration (false),
		scope_options (false),
		text_limit (0)
	{ };
	Host ():
		media_proxy (false),
		who_to_follow (false),
		chat (false),
		registration (false),
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
	out += "\"media_proxy\":" + (media_proxy? string {"true"}: string {"false"}) + ",";
	out += "\"who_to_follow\":" + (who_to_follow? string {"true"}: string {"false"}) + ",";
	out += "\"chat\":" + (chat? string {"true"}: string {"false"}) + ",";
	out += "\"registration\":" + (registration? string {"true"}: string {"false"}) + ",";
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


static void get_host_pleroma_config (string host, bool &a_who_to_follow, bool &a_chat, bool &a_scope_options, Http &http)
{
	string reply = http.perform (string {"https://"} + host + string {"/static/config.json"});

	picojson::value json_value;
	string error = picojson::parse (json_value, reply);
	if (! error.empty ()) {
		throw (HostException {__LINE__});
	}
	if (! json_value.is <picojson::object> ()) {
		throw (HostException {__LINE__});
	}
	auto json_object = json_value.get <picojson::object> ();
	if (json_object.find (string {"showWhoToFollowPanel"}) != json_object.end ()) {
		auto who_to_follow_value = json_object.at (string {"showWhoToFollowPanel"});
		if (who_to_follow_value.is <bool> ()) {
			a_who_to_follow = who_to_follow_value.get <bool> ();
		}
	}
	if (json_object.find (string {"chatDisabled"}) != json_object.end ()) {
		auto chat_disabled_value = json_object.at (string {"chatDisabled"});
		if (chat_disabled_value.is <bool> ()) {
			a_chat = (! chat_disabled_value.get <bool> ());
		}
	}
	if (json_object.find (string {"scopeOptionsEnabled"}) != json_object.end ()) {
		auto scope_options_value = json_object.at (string {"scopeOptionsEnabled"});
		if (scope_options_value.is <bool> ()) {
			a_scope_options = scope_options_value.get <bool> ();
		}
	}
}


static void get_host_nodeinfo (string host, bool &a_registration, bool &a_media_proxy, Http &http)
{
	string reply = http.perform (string {"https://"} + host + string {"/nodeinfo/2.0.json"});

	picojson::value json_value;
	string error = picojson::parse (json_value, reply);
	if (! error.empty ()) {
		throw (HostException {__LINE__});
	}
	if (! json_value.is <picojson::object> ()) {
		throw (HostException {__LINE__});
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
			if (metadata_object.find (string {"mediaProxy"}) != metadata_object.end ()) {
				auto media_proxy_value = metadata_object.at (string {"mediaProxy"});
				if (media_proxy_value.is <bool> ()) {
					bool media_proxy_bool = media_proxy_value.get <bool> ();
					a_media_proxy = media_proxy_bool;
				}
			}
		}
	}
}


static void get_host_statusnet_config (string host, unsigned int &a_text_limit, Http &http)
{
	string reply = http.perform (string {"https://"} + host + string {"/api/statusnet/config"});

	picojson::value json_value;
	string error = picojson::parse (json_value, reply);
	if (! error.empty ()) {
		throw (HostException {__LINE__});
	}
	if (! json_value.is <picojson::object> ()) {
		throw (HostException {__LINE__});
	}
	auto json_object = json_value.get <picojson::object> ();
	if (json_object.find (string {"site"}) == json_object.end ()) {
		throw (HostException {__LINE__});
	}
	auto site_value = json_object.at (string {"site"});
	if (! site_value.is <picojson::object> ()) {
		throw (HostException {__LINE__});
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
}


static Host for_host (string domain)
{
	Http http;
	string reply_string;

	try {
		reply_string = http.perform (string {"https://"} + domain + string {"/api/pleroma/emoji"});
	} catch (HttpException e) {
		cerr << domain << " has no /api/pleroma/emoji" << endl;
		throw (HostException {__LINE__});
	}

	picojson::value reply_value;
	string error = picojson::parse (reply_value, reply_string);
	if (! error.empty ()) {
		cerr << error << endl;
		throw (HostException {__LINE__});
	}

	string title;
	try {
		title = get_host_title (domain, http);
	} catch (HostException e) {
		/* Do nothing. */
	}

	string thumbnail;
	try {
		thumbnail = get_host_thumbnail (domain, http);
	} catch (HostException e) {
		/* Do nothing. */
	}

	Host host {domain, title, thumbnail};
	
	try {
		bool who_to_follow = false;
		bool chat = false;
		bool scope_options = false;
		get_host_pleroma_config (domain, who_to_follow, chat, scope_options, http);
		host.who_to_follow = who_to_follow;
		host.chat = chat;
		host.scope_options = scope_options;
	} catch (ExceptionWithLineNumber e) {
		cerr << e.line << endl;
	}

	try {
		bool registration = false;
		bool media_proxy = false;
		get_host_nodeinfo (domain, registration, media_proxy, http);
		host.registration = registration;
		host.media_proxy = media_proxy;
	} catch (ExceptionWithLineNumber e) {
		cerr << e.line << endl;
	}

	try {
		unsigned int text_limit = 0;
		get_host_statusnet_config (domain, text_limit, http);
		host.text_limit = text_limit;
	} catch (ExceptionWithLineNumber e) {
		cerr << e.line << endl;
	}
	
	return host;
}


int main (int argc, char **argv)
{
	set <string> domains;
	domains = get_international_hosts ();

	vector <Host> hosts;
	for (auto domain: domains) {
		cerr << domain << endl;
		try {
			Host host = for_host (string {domain});
			hosts.push_back (host);
		} catch (HttpException e) {
			/* Nothing. */
			cerr << "HttpException " << e.line << endl;
		} catch (HostException e) {
			/* Nothing. */
			cerr << "HostException " << e.line << endl;
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

