#include "../IsaacSave.h"

using namespace std;
using namespace IsaacSave;

#define PROGNAME "revert.exe"

void printUsage() {
	std::cout << "command line usage:\n"
		"  " PROGNAME  " help\n"
		"  " PROGNAME  " revert <frompath> <topath>\n"
		"  " PROGNAME  " vrevert <frompath> <topath>\n"
		"     revert the repentence plus save file <frompath> to repentence save file <topath>\n"
		"     e.g. " PROGNAME " revert D:\\rep+persistentgamedata1.dat  D:\\rep_persistentgamedata1.dat\n"
		"     vrevert means verbose revert, and print more logs\n\n"
		"WARNING: This program has not been fully tested and may corrupt your saved data\n"
		"PLEASE BACK UP your save data before and AFTER you using it.\n"
		;
}

int main(int argc, char** argv) {
	if (argc == 1) {
		printUsage();
		return 1;
	}
	if (strcmp(argv[1], "help") == 0) {
		printUsage();
		return 1;
	}

	if (argc == 4 && strcmp(argv[1], "revert") || strcmp(argv[1], "vrevert")) {
		if (strcmp(argv[1], "vrevert") == 0)
			verbose = true;
		try {
			std::cout << "repentence+ savedata to repentence reverter, by frto027\n";
			std::string from = argv[2], to = argv[3];
			std::cout << "reading save data...";
			SaveData s(from);
			std::cout << "done.\nmodifying...";
			s.cut(VER_REP);
			std::cout << "done.\nsaving...";
			s.WriteTo(to);
			std::cout << "done.\nfix checksum for file...";
			FixChecksumForFile(to);
			std::cout << "done.\n";
		}
		catch (std::exception e) {
			std::cout << "Error: " << e.what() << "\n";
			return 0;
		}
		return 0;
	}
	std::cout << "Invalid command line argument\n";
	return 0;
	//read_debug();
}