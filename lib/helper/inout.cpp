#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/join.hpp>
#include "error.hpp"
#include "inout.hpp"
#include "debug.hpp"

namespace inout {

	void Inout::__mkdir(const string &dir_path) {
		// makes a path "janez/aska/mia from e.g. "janez/aska/mia/test.txt"
		boost::filesystem::path dir(dir_path);
		dir.remove_filename();
		if(!dir.string().empty()) {
			if(!boost::filesystem::exists(dir)) {
				if(!boost::filesystem::create_directories(dir)) {
					throw Error("die : Cannot create directory " + dir.string());
				}
			}
		}
	}

	int Inout::__lock(const string &name) {
		int fd = open(name.c_str(),O_RDWR|O_CREAT,S_IRUSR|S_IWUSR);
		if (flock(fd,LOCK_EX) == -1) {
		} 
		return fd;
	}
	void Inout::__unlock(int fd) {
	    if (flock(fd,LOCK_UN)==-1) {
	    }
	    if (close(fd)==-1) {
	    }
	}
	int  Inout::file_size(const string &name) {
		if ( boost::filesystem::exists(name) &&
			boost::filesystem::is_regular_file(name) )
		{
			return boost::filesystem::file_size(name);
		} else {
			return 0;
		}
	}
	void Inout::read_file(const string &name, string &s, f_not_found w, 
		const int num_occur, const string &pattern) {
		streampos pos_in_file = 0;
		read_file(name, s, pos_in_file, w, num_occur, pattern);
	}
	void Inout::read_file(const string &name, vector<string> &s,
		f_not_found w, const int num_occur, const string &pattern) {
		streampos pos_in_file = 0;
		read_file(name, s, pos_in_file, w, num_occur, pattern);
	}
	void Inout::read_file(const string &name, string &s, streampos &pos_in_file, 
		f_not_found w, const int num_occur, const string &pattern) {
		vector<string> vs;
		read_file(name, vs, pos_in_file, w, num_occur, pattern);
		s = boost::algorithm::join(vs, "\n");
	}
	void Inout::read_file(const string &name, vector<string> &s, streampos &pos_in_file, 
		f_not_found w, const int num_occur, const string &pattern) {
		dbgmsg(pos_in_file);
		int fd = __lock(name);
		ifstream in(name);
		if (!in.is_open() && w == panic) {
			__unlock(fd);
			throw Error("Cannot read " + name);
		}
		in.seekg(pos_in_file);
		string line;
		int i = 0;
		streampos pos = in.tellg();
		while (getline(in, line)) {
			if (num_occur != -1 && (line.find(pattern) != string::npos && i++ == num_occur)) {
				dbgmsg("breaking on line = " << line);
				break;
			} else
				pos = in.tellg();
			s.push_back(line);
		}
		pos_in_file = pos;
		in.close();
		__unlock(fd);
	}	

	void Inout::file_open_put_contents(const string &name, 
		const vector<string> &v, ios_base::openmode mode) {
		stringstream ss;
		for (auto &s : v) ss << s << endl;
		file_open_put_stream(name, ss, mode);
	}
	void Inout::file_open_put_stream(const string &name, 
		const stringstream &ss, ios_base::openmode mode) {
		__mkdir(name);
		int fd = __lock(name);
		ofstream output_file(name, mode);
		if (!output_file.is_open()) {
			__unlock(fd);
			throw Error("Cannot open output file: " + name);  // how to emulate $! ?
		}
		output_file << ss.str();
		output_file.close();
		__unlock(fd);
	}
	vector<string> Inout::files_matching_pattern(const string &file_or_dir, 
		const string &pattern) {

		vector<string> fn;
		namespace fs = boost::filesystem;
		fs::path p(file_or_dir);
		if (p.extension().string() == ".lst") {
			read_file(file_or_dir, fn);
		} 
		else if (fs::is_directory(p)) {
			vector<fs::path> files;
			copy(fs::directory_iterator(p), 
				fs::directory_iterator(), back_inserter(files));  // get all files in directoy...
			for (auto &p : files) { 
				fn.push_back(p.string()); 
			}
		}
		else {
			throw Error("die : Cannot open directory \"" + p.string() + '"');
		}
		vector<string> filenames;
		for(string &s : fn) {
			if (boost::regex_search(s, boost::regex(pattern))) {
				filenames.push_back(s);
			}
		}
		if (filenames.empty())
			throw Error("die : should provide either a .lst file with each line having one pdb filename, or a directory in which pdb files are...");
		return filenames;
	}
};
