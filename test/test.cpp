#include "../IsaacSave.h"

using namespace std;
using namespace IsaacSave;
#include <cassert>

void test() {
	//good news that we pass the test, but I didn't test them in the actual game.
	std::string infile_path = "C:\\Users\\q6027\\Documents\\My Games\\Binding of Isaac Repentance\\20240723.rep_persistentgamedata1.dat";
	std::string tmpfile_path = "E:\\test.bin";
	{
		//read infile, write to tmp file
		ifstream infile(infile_path, ios::in | ios::binary);
		SaveData save(infile);

		ofstream o(tmpfile_path, ios::out | ios::binary);
		save.WriteTo(o);
	}
	{
		//read tmp file, should not crash
		ifstream test(tmpfile_path, ios::in | ios::binary);
		SaveData save2(test);
	}
	{
		//infile and tmp file should be same
		ifstream in(infile_path, ios::in | ios::binary);
		ifstream tmp(tmpfile_path, ios::in | ios::binary);

		while (!in.eof()) {
			char a;
			char b;
			in.read(&a, 1);
			tmp.read(&b, 1);
			assert(in.eof() == tmp.eof());
			if (!in.eof()) {
				assert(a == b);
			}
		}

		//FixChecksumForFile("E:\\test.bin");
	}

	{
		//test the cut
		ifstream infile(infile_path, ios::in | ios::binary);
		SaveData save(infile);
		save.cut(VER_REP);
	}
}

void read_debug() {
	//SaveData s("E:\\rep.dat");
	//std::cerr << "=============================\n";
	SaveData s2("E:\\repp.dat");
	s2.cut(VER_REP);
	std::cout << "revert the save data\n";
	s2.WriteTo("E:\\repp.revert.dat");
	std::cout << "fix checksum\n";
	FixChecksumForFile("E:\\repp.revert.dat");
	std::cout << "done\n";

	std::cout << "read back check\n";
	SaveData s3("E:\\repp.revert.dat");
	std::cout << "Done\n";
}

int main(int argc, char** argv) {
	read_debug();
	return 0;
}