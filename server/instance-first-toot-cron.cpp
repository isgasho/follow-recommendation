#include <cstdio>
#include <cstdlib>
#include <curl/curl.h>
#include <iostream>
#include <map>
#include <vector>
#include <sstream>
#include <set>
#include <limits>
#include "picojson.h"
#include "distsn.h"


using namespace std;


static string get_url (const picojson::value &toot)
{
	if (! toot.is <picojson::object> ()) {
		throw (TootException {});
	}
	auto properties = toot.get <picojson::object> ();
	if (properties.find (string {"url"}) == properties.end ()) {
		throw (TootException {});
	}
	auto url_object = properties.at (string {"url"});
	if (! url_object.is <string> ()) {
		throw (TootException {});
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


class QueryException: public ExceptionWithLineNumber {
public:
	QueryException () { };
	QueryException (unsigned int a_line): ExceptionWithLineNumber (a_line) { };
};


static vector <picojson::value> get_toots_with_max_id (string host, uint64_t max_id)
{
	stringstream max_id_s;
	max_id_s << max_id;
	string query
		= string {"https://"}
		+ host
		+ string {"/api/v1/timelines/public?local=true&max_id="}
		+ max_id_s.str ();
	string reply = http_get (query);

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


static void get_first_toot (string host, uint64_t lower_bound, uint64_t upper_bound, time_t &bottom_time, string &bottom_url)
{
	if (! (lower_bound < upper_bound)) {
		throw HostException {__LINE__};
	}
	if (upper_bound - lower_bound < 2) {
		throw HostException {__LINE__};
	}
	uint64_t middle = ((upper_bound - lower_bound) / 2) + lower_bound;
	vector <picojson::value> toots;
	try {
		toots = get_toots_with_max_id (host, middle);
		if (toots.size () == 0) {
			get_first_toot (host, middle, upper_bound, bottom_time, bottom_url);
		} else if (20 <= toots.size ()) {
			get_first_toot (host, lower_bound, middle, bottom_time, bottom_url);
		} else {
			auto bottom_toot = toots.back ();
			try {
				bottom_time = get_time (bottom_toot);
				bottom_url = get_url (bottom_toot);
			} catch (TootException e) {
				throw (HostException {});
			}
		}
	} catch (QueryException e) {
		get_first_toot (host, lower_bound, middle, bottom_time, bottom_url);
	}
}


static void get_first_toot (string host, time_t &bottom_time, string &bottom_url)
{
	get_first_toot (host, 0, numeric_limits <uint64_t> ().max (), bottom_time, bottom_url);
}


static Host for_host (string domain)
{
	time_t bottom_time;
	string bottom_url;
	get_first_toot (domain, bottom_time, bottom_url);

	string title;
	try {
		title = get_host_title (domain);
	} catch (ExceptionWithLineNumber e) {
		cerr << "Error" << domain << " " << e.line << endl;
	}

	string thumbnail;
	try {
		thumbnail = get_host_thumbnail (domain);
	} catch (ExceptionWithLineNumber e) {
		cerr << "Error" << domain << " " << e.line << endl;
	}

	return Host {domain, bottom_time, bottom_url, title, thumbnail};
}


static bool valid_host_name (string host)
{
	return 0 < host.size () && host.at (0) != '#';
}


static set <string> get_domains ()
{
	string domains_s = http_get (string {"https://raw.githubusercontent.com/distsn/follow-recommendation/master/hosts.txt"});
	set <string> domains;
	string domain;
	for (char c: domains_s) {
		if (c == '\n') {
			if (valid_host_name (domain)) {
				domains.insert (domain);
			}
			domain.clear ();
		} else {
			domain.push_back (c);
		}
	}
	if (valid_host_name (domain)) {
		domains.insert (domain);
	}
	return domains;
}


int main (int argc, char **argv)
{
	set <string> domains = get_domains ();

	const string storage_filename = string {"/var/lib/distsn/instance-first-toot/instance-first-toot.json"};

	vector <Host> hosts;

	for (auto domain: domains) {
		cerr << domain << endl;
		try {
			Host host = for_host (string {domain});
			hosts.push_back (host);
			cerr << host.first_toot_url << endl;
		} catch (HttpException e) {
			cerr << e.line << endl;
		} catch (HostException e) {
			cerr << e.line << endl;
		}
	}

	sort (hosts.begin (), hosts.end (), byFresh {});

	FILE * storage_file_out = fopen (storage_filename.c_str (), "w");
	if (storage_file_out != nullptr) {
		write_storage (storage_file_out, hosts);
		fclose (storage_file_out);
	}

	return 0;
}

