#include <cstdio>
#include <cstdlib>
#include <curl/curl.h>
#include <iostream>
#include <map>
#include <vector>
#include <sstream>
#include <set>
#include "picojson.h"


using namespace std;


static int writer (char * data, size_t size, size_t nmemb, std::string * writerData)
{
	if (writerData == nullptr) {
		return 0;
	}
	writerData->append (data, size * nmemb);
	return size * nmemb;
}


class HttpException: public exception {
	/* Nothing */
};


static string http_get (string url)
{
	CURL *curl;
	CURLcode res;
	curl_global_init (CURL_GLOBAL_ALL);

	curl = curl_easy_init ();
	if (! curl) {
		throw (HttpException {});
	}
	curl_easy_setopt (curl, CURLOPT_URL, url.c_str ());
	string reply_1;
	curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, writer);
	curl_easy_setopt (curl, CURLOPT_WRITEDATA, & reply_1);
	res = curl_easy_perform (curl);
	curl_easy_cleanup (curl);
	if (res != CURLE_OK) {
		throw (HttpException {});
	}
	return reply_1;
}


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



