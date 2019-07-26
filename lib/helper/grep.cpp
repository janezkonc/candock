#include "grep.hpp"

size_t Grep::count_matches(std::istream& stream, const boost::regex& regex) {
	size_t count = 0;

	boost::smatch what;
	std::string line;

	while(std::getline(stream, line)) {
		bool result = boost::regex_search(line, what, regex);
		if(result) {
			++count;
		}
	}

	return count;
}

std::vector<std::string> Grep::search_stream(std::istream& stream, const boost::regex& regex) {
	std::vector<std::string> all_matches;

	boost::match_results<std::string::const_iterator> what;

	std::string line;

	while(std::getline(stream, line)) {
		bool result = boost::regex_search(line, what, regex);
		if(result) {
			all_matches.push_back(std::string( what[1].first, what[1].second));
		}
	}

	return all_matches;
}