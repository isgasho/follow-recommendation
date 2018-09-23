#include <algorithm>

#include <socialnet-1.h>

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


int main (int argc, char ** argv)
{
	if (argc < 2) {
		return 1;
	}

	string a_implementation {argv [1]};
	cerr << a_implementation << endl;

	socialnet::eImplementation implementation;
	socialnet::decode (a_implementation, implementation);

	auto hosts = socialnet::get_hosts ();

	vector <string> host_names;
	
	for (auto host: hosts) {
		if (host->implementation () == implementation) {
			host_names.push_back (host->host_name);
		}
	}

	sort (host_names.begin (), host_names.end ());

	cout << "Access-Control-Allow-Origin: *" << endl;
	cout << "Content-type: application/json" << endl << endl;

	cout << "[";
	
	for (unsigned int cn = 0; cn < host_names.size (); cn ++) {
		if (0 < cn) {
			cout << "," << endl;
		}
		string host_name = host_names.at (cn);
		cout << "\"" << escape_json (host_name) << "\"";
	}
	
	cout << "]" << endl;
	
	return 0;
}

