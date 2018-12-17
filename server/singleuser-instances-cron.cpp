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


using namespace std;


class Host {
public:
	string host_name;
	string title;
	string description;
	string thumbnail;
public:
	string format () const;
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


string Host::format () const
{
	string out;
	out += "{";

	out += "\"hostName\":\"" + escape_json (host_name) + "\",";
	out += "\"title\":\"" + escape_json (title) + "\",";
	out += "\"description\":\"" + escape_json (description) + "\",";
	out += "\"thumbnail\":\"" + escape_json (thumbnail) + "\"";

	out += "}";
	return out;
}


bool by_host_name (const Host &left, const Host &right)
{
	return left.host_name < right.host_name;
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


static Host for_host (shared_ptr <socialnet::Host> socialnet_host)
{
	string title;
	string description;
	string thumbnail;
	try {
		socialnet_host->get_profile (title, description, thumbnail);
	} catch (socialnet::ExceptionWithLineNumber e) {
		/* Do nothing. */
	}

	Host host;
	host.host_name = socialnet_host->host_name;
	host.title = title;
	host.description = description;
	host.thumbnail = thumbnail;

	return host;
}


int main (int argc, char **argv)
{
	auto socialnet_hosts = socialnet::get_hosts ();

	vector <Host> hosts;
	for (auto socialnet_host: socialnet_hosts) {
		if (socialnet_host->implementation () == socialnet::eImplementation::SINGLEUSER) {
			cerr << socialnet_host->host_name << endl;
			try {
				Host host = for_host (socialnet_host);
				hosts.push_back (host);
			} catch (socialnet::HostException e) {
				cerr << "socialnet::HostException " << e.line << endl;
			}
		}
	}
	
	sort (hosts.begin (), hosts.end (), by_host_name);

	const string storage_filename = string {"/var/lib/distsn/singleuser-instances.json"};
	FILE * storage_file_out = fopen (storage_filename.c_str (), "w");
	if (storage_file_out != nullptr) {
		write_storage (storage_file_out, hosts);
		fclose (storage_file_out);
	}

	return 0;
}

