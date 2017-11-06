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


static bool valid_host_name (string host)
{
	return 0 < host.size () && host.at (0) != '#';
}


static set <string> get_domains (string list_url)
{
	string domains_s = http_get (list_url);
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
	set <string> tacostea = get_domains (string {"https://raw.githubusercontent.com/TacosTea/versionbattle/master/instances.list"});
	set <string> distsn = get_domains (string {"https://raw.githubusercontent.com/distsn/follow-recommendation/master/hosts.txt"});

	set <string> all;
	all.insert (tacostea.begin (), tacostea.end ());
	all.insert (distsn.begin (), distsn.end ());

	cout << "Content-type: application/json" << endl << endl;
	cout << "[";
	
	for (auto domain: all) {
		if (domain != (* all.begin ())) {
			cout << ",";
		}
		cout << "{";
		cout << "\"domain\":\"" << escape_json (domain) <<"\",";
		cout << "\"tacostea\":" << (tacostea.find (domain) != tacostea.end () ? "true": "false") << ",";
		cout << "\"distsn\":" << (distsn.find (domain) != distsn.end () ? "true": "false");
		cout << "}";
	};
	
	cout << "]";
	return 0;
}



