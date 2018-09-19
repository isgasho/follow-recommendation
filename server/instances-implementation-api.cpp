#include <algorithm>

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

	Http http;

	string reply = http.perform (string {"https://distsn.org/cgi-bin/instances-raw-api.cgi"});

	picojson::value reply_value;
	string error = picojson::parse (reply_value, reply);
	if (! error.empty ()) {
		cerr << error << " " << __LINE__ << endl;
		return 1;
	}
	if (! reply_value.is <picojson::array> ()) {
		cerr << __LINE__ << endl;
	}
	auto reply_array = reply_value.get <picojson::array> ();
	
	vector <string> host_names;
	
	for (auto host_value: reply_array) {
		auto host_object = host_value.get <picojson::object> ();
		string implementation_string = host_object.at (string {"implementation"}).get <string> ();
		string host_name_string = host_object.at (string {"host"}).get <string> ();
		if (implementation_string == a_implementation) {
			host_names.push_back (host_name_string);
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

