#include <cstdio>
#include <cstdlib>
#include <curl/curl.h>
#include <iostream>
#include <map>
#include <vector>
#include <sstream>
#include <set>
#include <limits>
#include <ctime>

#include <socialnet-1.h>

#include "picojson.h"


using namespace std;


static string get_id (const picojson::value &toot)
{
	if (! toot.is <picojson::object> ()) {
		throw (socialnet::TootException {});
	}
	auto properties = toot.get <picojson::object> ();
	if (properties.find (string {"id"}) == properties.end ()) {
		throw (socialnet::TootException {});
	}
	auto id_object = properties.at (string {"id"});
	string id_string;
	if (id_object.is <double> ()) {
		double id_double = id_object.get <double> ();
		stringstream s;
		s << static_cast <unsigned int> (id_double);
		id_string = s.str ();
	} else if (id_object.is <string> ()) {
		id_string = id_object.get <string> ();
	} else {
		throw (socialnet::TootException {});
	}
	return id_string;
}


static time_t str2time (string s)
{
	struct tm tm;
	strptime (s.c_str (), "%Y-%m-%dT%H:%M:%S", & tm);
	return timegm (& tm);
}


static time_t get_time (const picojson::value &toot)
{
	if (! toot.is <picojson::object> ()) {
		throw (socialnet::TootException {});
	}
	auto properties = toot.get <picojson::object> ();
	if (properties.find (string {"created_at"}) == properties.end ()) {
		throw (socialnet::TootException {});
	}
	auto time_object = properties.at (string {"created_at"});
	if (! time_object.is <string> ()) {
		throw (socialnet::TootException {});
	}
	auto time_s = time_object.get <string> ();
	return str2time (time_s);
}


static string get_url (const picojson::value &toot)
{
	if (! toot.is <picojson::object> ()) {
		throw (socialnet::TootException {__LINE__});
	}
	auto properties = toot.get <picojson::object> ();
	if (properties.find (string {"url"}) == properties.end ()) {
		throw (socialnet::TootException {__LINE__});
	}
	auto url_object = properties.at (string {"url"});
	if (! url_object.is <string> ()) {
		throw (socialnet::TootException {__LINE__});
	}
	return url_object.get <string> ();
}


class Host {
public:
	string domain;
	time_t first_toot_time;
	string first_toot_url;
	string title;
	string thumbnail;
public:
	Host (string a_domain, time_t a_time, string a_url, string a_title, string a_thumbnail) {
		domain = a_domain;
		first_toot_time = a_time;
		first_toot_url = a_url;
		title = a_title;
		thumbnail = a_thumbnail;
	};
};


class byFresh {
public:
	bool operator () (const Host &left, const Host &right) const {
		return right.first_toot_time < left.first_toot_time;
	};
};


string escape_json (string in)
{
	string out;
	for (auto c: in) {
		if (c == '\n') {
			out += string {"\\n"};
		} else if (0x00 <= c && c < 0x20) {
			out += string {"�"};
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


static void write_storage (FILE *out, vector <Host> hosts)
{
	fprintf (out, "[");
	for (unsigned int cn = 0; cn < hosts.size (); cn ++) {
		if (0 < cn) {
			fprintf (out, ",");
		}
		Host host = hosts.at (cn);
		ostringstream time_s;
		time_s << host.first_toot_time;
		fprintf
			(out,
			"{\"domain\":\"%s\",\"first_toot_time\":\"%s\",\"first_toot_url\":\"%s\",\"title\":\"%s\",\"thumbnail\":\"%s\"}",
			host.domain.c_str (),
			time_s.str ().c_str (),
			host.first_toot_url.c_str (),
			escape_json (host.title).c_str (),
			escape_json (host.thumbnail).c_str ());
	}
	fprintf (out, "]");
}


class QueryException: public socialnet::ExceptionWithLineNumber {
public:
	QueryException () { };
	QueryException (unsigned int a_line): socialnet::ExceptionWithLineNumber (a_line) { };
};


static vector <picojson::value> get_toots_with_max_id (string host, uint64_t max_id, socialnet::Http &http)
{
	stringstream max_id_s;
	max_id_s << max_id;
	string query
		= string {"https://"}
		+ host
		+ string {"/api/v1/timelines/public?local=true&max_id="}
		+ max_id_s.str ();
	string reply = http.perform (query);

	picojson::value json_value;
	string error = picojson::parse (json_value, reply);
	if (! error.empty ()) {
		throw (QueryException {__LINE__});
	}
	if (! json_value.is <picojson::array> ()) {
		throw (QueryException {__LINE__});
	}

	vector <picojson::value> toots = json_value.get <picojson::array> ();
	return toots;
}


static vector <picojson::value> get_toots (string host, socialnet::Http &http)
{
	string query
		= string {"https://"}
		+ host
		+ string {"/api/v1/timelines/public?local=true"};
	string reply = http.perform (query);

	picojson::value json_value;
	string error = picojson::parse (json_value, reply);
	if (! error.empty ()) {
		throw (QueryException {__LINE__});
	}
	if (! json_value.is <picojson::array> ()) {
		throw (QueryException {__LINE__});
	}

	vector <picojson::value> toots = json_value.get <picojson::array> ();
	return toots;
}


static void get_first_toot (string host, uint64_t lower_bound, uint64_t upper_bound, time_t &bottom_time, string &bottom_url, socialnet::Http &http)
{
	if (! (lower_bound < upper_bound)) {
		throw socialnet::HostException {__LINE__};
	}
	if (upper_bound - lower_bound < 2) {
		throw socialnet::HostException {__LINE__};
	}
	uint64_t middle = ((upper_bound - lower_bound) / 2) + lower_bound;
	vector <picojson::value> toots;
	try {
		toots = get_toots_with_max_id (host, middle, http);
		if (toots.size () == 0) {
			get_first_toot (host, middle, upper_bound, bottom_time, bottom_url, http);
		} else if (20 <= toots.size ()) {
			get_first_toot (host, lower_bound, middle, bottom_time, bottom_url, http);
		} else {
			auto bottom_toot = toots.back ();
			try {
				bottom_time = get_time (bottom_toot);
				bottom_url = get_url (bottom_toot);
			} catch (socialnet::TootException e) {
				throw (socialnet::HostException {__LINE__});
			}
		}
	} catch (QueryException e) {
		get_first_toot (host, lower_bound, middle, bottom_time, bottom_url, http);
	}
}


static void get_first_toot (string host, time_t &bottom_time, string &bottom_url, socialnet::Http &http)
{
	auto toots = get_toots (host, http);
	if (toots.size () < 1) {
		throw (socialnet::HostException {__LINE__});
	}
	string id_string = get_id (toots.front ());
	uint64_t id_uint;
	stringstream {id_string} >> id_uint;
	get_first_toot (host, 0, id_uint, bottom_time, bottom_url, http);
}


static Host for_host (shared_ptr <socialnet::Host> socialnet_host)
{
	time_t bottom_time;
	string bottom_url;
	get_first_toot (socialnet_host->host_name, bottom_time, bottom_url, * socialnet_host->http);

	string title;
	string description;
	string thumbnail;
	try {
		socialnet_host->get_profile (title, description, thumbnail);
	} catch (socialnet::ExceptionWithLineNumber e) {
		cerr << "Error " << socialnet_host->host_name << " " << e.line << endl;
	}

	return Host {socialnet_host->host_name, bottom_time, bottom_url, title, thumbnail};
}


int main (int argc, char **argv)
{
	auto socialnet_hosts = socialnet::get_hosts ();

	const string storage_filename = string {"/var/lib/distsn/instance-first-toot/instance-first-toot.json"};

	vector <Host> hosts;

	for (auto socialnet_host: socialnet_hosts) {
		cerr << socialnet_host->host_name << endl;
		time_t begin_time = time (nullptr);
		try {
			if (socialnet_host->implementation () == socialnet::eImplementation::MASTODON) {
				Host host = for_host (socialnet_host);
				hosts.push_back (host);
				cerr << host.first_toot_url << endl;
			} else {
				cerr << socialnet_host->host_name << " is not Mastodon." << endl;
			}
		} catch (socialnet::ExceptionWithLineNumber e) {
			cerr << e.line << endl;
		}
		time_t end_time = time (nullptr);
		cerr << "time: " << end_time - begin_time << endl;
	}

	sort (hosts.begin (), hosts.end (), byFresh {});

	FILE * storage_file_out = fopen (storage_filename.c_str (), "w");
	if (storage_file_out != nullptr) {
		write_storage (storage_file_out, hosts);
		fclose (storage_file_out);
	}

	return 0;
}

