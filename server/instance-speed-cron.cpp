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
	double speed;
	string title;
	string thumbnail;
public:
	Host (string a_domain, double a_speed, string a_title, string a_thumbnail) {
		domain = a_domain;
		speed = a_speed;
		title = a_title;
		thumbnail = a_thumbnail;
	};
};


class bySpeed {
public:
	bool operator () (const Host &left, const Host &right) const {
		return right.speed < left.speed;
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
		fprintf
			(out,
			"{\"domain\":\"%s\",\"speed\":%e,\"title\":\"%s\",\"thumbnail\":\"%s\"}",
			host.domain.c_str (),
			host.speed,
			escape_json (host.title).c_str (),
			escape_json (host.thumbnail).c_str ());
	}
	fprintf (out, "]");
}


static Host for_host (string domain)
{
	/* Start time */
	time_t start_time;
	time (& start_time);

	/* Get timeline. */
	vector <picojson::value> toots = get_timeline (domain);
	if (toots.size () < 40) {
		throw (HostException {});
	}

	const picojson::value &top_toot = toots.at (0);
	const picojson::value &bottom_toot = toots.at (toots.size () - 1);
	time_t top_time;
	time_t bottom_time;
	try {
		top_time = get_time (top_toot);
		bottom_time = get_time (bottom_toot);
	} catch (TootException e) {
		throw (HostException {});
	}

	double duration = max (start_time, top_time) - bottom_time;
	if (! (1.0 < duration && duration < 60 * 60 * 24 * 365)) {
		throw (HostException {});
	}

	double speed = static_cast <double> (toots.size ()) / duration;

	/* Save history */
	stringstream now_s;
	now_s << start_time;
	string filename = string {"/var/lib/distsn/instance-speed-cron/"} + domain + string {".csv"};
	FILE *out = fopen (filename.c_str (), "a");
	if (out != nullptr) {
		fprintf (out, "\"%s\",\"%e\"\n", now_s.str ().c_str (), speed);
		fclose (out);
	}
	
	string title;
	try {
		title = get_host_title (domain);
	} catch (HostException e) {
		/* Do nothing. */
	}

	string thumbnail;
	try {
		thumbnail = get_host_thumbnail (domain);
	} catch (HostException e) {
		/* Do nothing. */
	}

	return Host {domain, speed, title, thumbnail};
}


int main (int argc, char **argv)
{
	set <string> domains = get_international_hosts ();
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

	sort (hosts.begin (), hosts.end (), bySpeed {});

	const string storage_filename = string {"/var/lib/distsn/instance-speed/instance-speed.json"};
	FILE * storage_file_out = fopen (storage_filename.c_str (), "w");
	if (storage_file_out != nullptr) {
		write_storage (storage_file_out, hosts);
		fclose (storage_file_out);
	}

	return 0;
}

