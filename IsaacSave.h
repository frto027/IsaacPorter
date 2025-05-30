#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <exception>
#include <fstream>

namespace IsaacSave {
	bool verbose = false;

	enum GameVersion {
		VER_REP
	};
	/*

	ok, the block type means:

	//1:achievement (parsed)
	//2:counter (parsed)
	//3:Level Counter (parsed)
	//4:Collectibles (parsed)
	//5:MiniBosses
	//6:Bosses
	//7:Challenge Counters (parsed)
	//8:CutsceneCounters (parsed)
	//9:GameSettings
	//10:SpecialSeedCounters
	//11:BestiaryCounters (parsed)

	only achievement and chounter is handled
	*/


	struct BlockBase {
		bool is_already_readed = false;
		virtual int block_type() = 0;
		uint32_t block_size;
		uint32_t extra_number;
		void read(std::istream& in) {
			if (verbose) std::cerr << "before read block " << block_type() << " at " << in.tellg() << "\n";
			if (in.read((char*)&block_size, 4).eof()) {
				throw std::exception("Can't read blocksize");
			}
			if (in.read((char*)&extra_number, 4).eof()) {
				throw std::exception("Can't read extra_number");
			}
			read_body(in);
			if (verbose) std::cerr << "read block " << block_type() << " done at " << in.tellg() << "\n";
			if (verbose) std::cerr << element_count() << " element readed\n";
		}
		virtual void read_body(std::istream& in) = 0;

		virtual void cut(GameVersion version) {
			uint32_t old_count = element_count();
			if (old_count == 0) {
				if (verbose) std::cerr << "don't cut " << block_type() << " because event counter is zero\n";
				return;
			}

			if (verbose) std::cerr << block_type() << ":fix cut(" << block_size << "," << extra_number << ")->\n";
			uint32_t bsize_mul = block_size / old_count;
			uint32_t extra_mul = extra_number / old_count;
			uint32_t extra_div = old_count / extra_number;
			if (bsize_mul * old_count != block_size) {
				throw std::exception("error bsize mul");
			}
			if (extra_mul * old_count != extra_number && extra_div * extra_number != old_count) {
				throw std::exception("error extra mul");
			}

			cut_impl(version);

			uint32_t new_count = element_count();

			block_size = new_count * bsize_mul;

			if (block_type() == 11) {
				//don't fix for bestiary block, it's always 4
			}
			else {
				if (extra_mul * old_count == extra_number) {
					extra_number = extra_mul * new_count;
				}
				else {
					extra_number = new_count / extra_div;
					if (extra_div * extra_number != new_count) {
						throw std::exception("error extra solve");
					}
				}

			}
			if (verbose) std::cerr << "  ->(" << block_size << "," << extra_number << ")\n";
		};

		virtual void cut_impl(GameVersion version) {}

		virtual void write(std::ostream& out) {
			if (verbose) std::cerr << "write block " << block_type() << " at " << out.tellp() << "\n";
			out.write((char*)&block_size, 4);
			out.write((char*)&extra_number, 4);
			write_body(out);
			if (verbose) std::cerr << "write block done at " << out.tellp() << "\n";
		}
		virtual void write_body(std::ostream& out) = 0;

		virtual uint32_t element_count() { return 0; }
	};

	template<int type>
	struct Block : public BlockBase {
		static const int block_type_value = type;
		virtual int block_type() override {
			return type;
		}
	};

	struct AchievementBlock : public Block<1> {
		std::vector<int>achis;
		uint32_t element_count() override { return(uint32_t)achis.size(); }

		virtual void read_body(std::istream& in) override {
			uint32_t achi_count = extra_number;
			char r;
			for (uint32_t i = 0; i < block_size; i++) {
				if (in.read(&r, 1).eof())
					throw std::exception("can't read achi");
				if (i < achi_count)
					achis.push_back(r);
			}
		}

		virtual void cut_impl(GameVersion version) override {
			if (version == VER_REP) {
				while (achis.size() > 638) {
					achis.pop_back();
					std::cout << "drop achievement " << achis.size() << "\n";
				}
			}
		}

		virtual void write_body(std::ostream& out) override {
			for (uint32_t i = 0; i < block_size; i++) {
				char o = 0;
				if (i < achis.size()) {
					o = achis[i];
				}
				out.write(&o, 1);
			}
		}

	};
	struct CounterBlock : public Block<2> {
		std::vector<uint32_t>counters;
		uint32_t element_count() override { return (uint32_t)counters.size(); }

		void read_body(std::istream& in) override {
			uint32_t count;
			for (uint32_t i = 0; i < block_size; i += 4) {
				if (in.read((char*)&count, 4).eof())
					throw std::exception("can't read achi");
				counters.push_back(count);
			}
		}

		virtual void cut_impl(GameVersion version) override {
			if (VER_REP == version) {
				while (counters.size() > 496) {
					uint32_t value = counters.back();
					counters.pop_back();
					std::cout << "drop counter " << counters.size() << ", the value is " << value << "\n";
				}

			}
		}

		void write_body(std::ostream& out) override {
			for (uint32_t i = 0; i < block_size; i += 4) {
				out.write((char*)&counters[i / 4], 4);
			}
		}
	};
	struct LevelBlock :public Block<3> {
		std::vector<uint32_t>counters;
		uint32_t element_count() { return (uint32_t)counters.size(); }

		void read_body(std::istream& in) override {
			uint32_t count;
			for (uint32_t i = 0; i < extra_number; i++) {
				if (in.read((char*)&count, 4).eof())
					throw std::exception("can't read achi");
				counters.push_back(count);
			}
		}
		void write_body(std::ostream& out) override {
			for (uint32_t i = 0; i < extra_number; i++) {
				out.write((char*)&counters[i], 4);
			}
		}
	};

	struct CollectibleBlock :public Block<4> {
		std::vector<char>counters;
		uint32_t element_count() override { return (uint32_t)counters.size(); }

		void read_body(std::istream& in) override {
			char count;
			for (uint32_t i = 0; i < extra_number; i++) {
				if (in.read((char*)&count, 1).eof())
					throw std::exception("can't read collectible");
				counters.push_back(count);
			}
		}
		void write_body(std::ostream& out) override {
			for (uint32_t i = 0; i < extra_number; i++) {
				out.write((char*)&counters[i], 1);
			}
		}
	};
	struct ChallengeBlock :public Block<7> {
		std::vector<char>counters;
		uint32_t element_count() override { return (uint32_t)counters.size(); }

		void read_body(std::istream& in) override {
			char count;
			for (uint32_t i = 0; i < extra_number; i++) {
				if (in.read((char*)&count, 1).eof())
					throw std::exception("can't read challenge");
				counters.push_back(count);
			}
		}
		void write_body(std::ostream& out) override {
			for (uint32_t i = 0; i < extra_number; i++) {
				out.write((char*)&counters[i], 1);
			}
		}
	};

	struct CutsceneBlock :public Block<8> {
		std::vector<uint32_t>counters;
		uint32_t element_count() override { return (uint32_t)counters.size(); }

		void read_body(std::istream& in) override {
			uint32_t count;
			for (uint32_t i = 0; i < extra_number; i++) {
				if (in.read((char*)&count, 4).eof())
					throw std::exception("can't read achi");
				counters.push_back(count);
			}
		}
		void write_body(std::ostream& out) override {
			for (uint32_t i = 0; i < extra_number; i++) {
				out.write((char*)&counters[i], 4);
			}
		}
	};

	template<int type>
	struct Bypass : public Block<type> {
		std::vector<char> datas;
		uint32_t element_count() override { return (uint32_t)datas.size(); }

		void read_body(std::istream& in) override {
			datas.resize(this->extra_number);
			if (in.read(datas.data(), this->extra_number).eof())
				throw std::exception("can't read section");
		}
		void write_body(std::ostream& out) override {
			out.write(datas.data(), this->extra_number);
		}
	};
	template<int type>
	struct BlockSizeBypassBlock : public Block<type> {
		std::vector<char> datas;
		uint32_t element_count() override { return (uint32_t)datas.size(); }

		void read_body(std::istream& in) override {
			datas.resize(this->block_size);
			if (in.read(datas.data(), this->block_size).eof())
				throw std::exception("can't read section");
		}
		void write_body(std::ostream& out) override {
			out.write(datas.data(), this->block_size);
		}
	};
	///////////////////////// begin of block type 11 //////////////
	struct BestiaryItem {
		char SubType, Variant;
		uint16_t Type;
		uint16_t value;
	};

	struct BestiarySubBlock {
		uint32_t type;//1 2 3 4
		std::vector<BestiaryItem> items;

		uint32_t size() {
			return (uint32_t)items.size() * 4;
		}

		void read(std::istream& in) {
			if (in.read((char*)&type, 4).eof())
				throw std::exception("can't read b type");
			uint32_t items_count;
			if (in.read((char*)&items_count, 4).eof())
				throw std::exception("can't read items count");
			for (uint32_t i = 0; i < items_count; i += 4) {
				BestiaryItem item;
				if (in.read((char*)&item.SubType, 1).eof())
					throw std::exception("can't read");
				if (in.read((char*)&item.Variant, 1).eof())
					throw std::exception("can't read");
				char T1, T2;
				if (in.read((char*)&T1, 1).eof())
					throw std::exception("can't read");
				if (in.read((char*)&T2, 1).eof())
					throw std::exception("can't read");

				item.Type = (0xff & (unsigned int)T1) + (((unsigned int)T2) << 8);
				if (in.read((char*)&item.value, 2).eof())
					throw std::exception("can't read");
				uint16_t padding;
				if (in.read((char*)&padding, 2).eof())
					throw std::exception("can't read");
				items.push_back(item);
			}
		}
		void write(std::ostream& out) {
			out.write((char*)&type, 4);
			uint32_t items_count = (uint32_t)items.size() * 4;
			out.write((char*)&items_count, 4);
			for (auto it = items.begin(); it != items.end(); ++it) {
				auto& item = *it;
				out.write(&item.SubType, 1);
				out.write(&item.Variant, 1);
				char T1, T2;
				T1 = (char)item.Type;
				T2 = (char)(item.Type >> 8);
				out.write(&T1, 1);
				out.write(&T2, 1);
				out.write((char*)&item.value, 2);
				uint16_t padding = 0;
				out.write((char*)&padding, 2);
			}
		}
	};
	struct BestiaryBlock : public Block<11> {
		BestiarySubBlock b1, b2, b3, b4;

		uint32_t element_count() override {
			return b1.size() + b2.size() + b3.size() + b4.size();
		}

		void read_body(std::istream& in) override {
			b1.read(in);
			b2.read(in);
			b3.read(in);
			b4.read(in);
		}
		void write_body(std::ostream& out) override {
			b1.write(out);
			b2.write(out);
			b3.write(out);
			b4.write(out);
		}

	};
	/////////////////////// end of block type 11 ///////////////////////
	struct SaveData {
		std::vector<BlockBase*> blocks = {
			new AchievementBlock(),
			new CounterBlock(),
			new LevelBlock(),
			new CollectibleBlock(),
			new Bypass<5>(),//minibosses
			new Bypass<6>(),//bosses
			new ChallengeBlock(),
			new CutsceneBlock(),
			new BlockSizeBypassBlock<9>(),//gamesetthings
			new Bypass<10>(),//specialseedcounters
			new BestiaryBlock(),
		};

		template<typename T>
		T* GetComponent() {
			int v = T::block_type_value;
			for (auto it = blocks.begin(); it != blocks.end(); ++it) {
				if ((*it)->is_already_readed && (*it)->block_type() == v) {
					return dynamic_cast<T*>(*it);
				}
			}
			return nullptr;
		}

		int blocks_readed_count = 0;

		char preleading[16] = "";
		uint32_t checksum = 0;

		char tail_bits[8];
		void read(std::istream& in) {
			if (in.read(preleading, 16).eof())
				throw std::exception("PraseError: invalid file header");
			if (strncmp(preleading, "ISAACNGSAVE09R  ", 16) != 0)
				throw std::exception("ParseError: your save data version is not supported");
			if (in.read((char*)&checksum, 4).eof())
				throw std::exception("InvalidHeader: Can't read checksum");
			while (!in.eof()) {
				if (blocks_readed_count == blocks.size()) {
					if (in.read(tail_bits, 8).eof())
						throw std::exception("Invalid file tail");
					break;
				}
				uint32_t block_type = 0;
				if (in.read((char*)&block_type, 4).eof())
					throw std::exception("Can't read blocktype");
				bool readed = false;
				for (auto it = blocks.begin(); it != blocks.end(); ++it) {
					if ((*it)->block_type() == block_type) {
						(*it)->read(in);
						readed = true;
						(*it)->is_already_readed = true;
						blocks_readed_count++;
						break;
					}
				}
				if (!readed) {
					throw std::exception("Can't read block");
				}
			}
			char foo;
			if (!in.read(&foo, 1).eof()) {
				throw std::exception("unknown file structure");
			}
		}

		SaveData(std::istream& in) {
			read(in);
		}

		SaveData(std::string path) {
			std::ifstream in(path, std::ios::in | std::ios::binary);
			read(in);
		}

		void cut(GameVersion version) {
			for (auto it = blocks.begin(); it != blocks.end(); ++it) {
				(*it)->cut(version);
			}
		}

		void WriteTo(std::ostream& ostream) {
			ostream.write("ISAACNGSAVE09R  ", 16);
			ostream.write((char*)&checksum, 4);
			for (auto it = blocks.begin(); it != blocks.end(); ++it) {
				uint32_t type = (*it)->block_type();
				if ((*it)->is_already_readed) {

				}
				else {
					std::cerr << "We didn't have block type " << type << ", skip it.\n";
					continue;
				}
				ostream.write((char*)&type, 4);

				(*it)->write(ostream);
			}

			ostream.write(tail_bits, 8);
		}


		void WriteTo(std::string path) {
			std::ofstream file(path, std::ios::out | std::ios::binary);
			WriteTo(file);
		}
		~SaveData() {
			for (auto it = blocks.begin(); it != blocks.end(); ++it)
				delete* it;
		}
	};

	uint32_t EvalChecksum(std::fstream& in, size_t count) {
		//https://github.com/jamesthejellyfish/isaac-save-edit-script/blob/main/script.py
		uint32_t crctable[] = {
			0x00000000, 0x09073096, 0x120E612C, 0x1B0951BA, 0xFF6DC419, 0xF66AF48F, 0xED63A535, 0xE46495A3,
			0xFEDB8832, 0xF7DCB8A4, 0xECD5E91E, 0xE5D2D988, 0x01B64C2B, 0x08B17CBD, 0x13B82D07, 0x1ABF1D91,
			0xFDB71064, 0xF4B020F2, 0xEFB97148, 0xE6BE41DE, 0x02DAD47D, 0x0BDDE4EB, 0x10D4B551, 0x19D385C7,
			0x036C9856, 0x0A6BA8C0, 0x1162F97A, 0x1865C9EC, 0xFC015C4F, 0xF5066CD9, 0xEE0F3D63, 0xE7080DF5,
			0xFB6E20C8, 0xF269105E, 0xE96041E4, 0xE0677172, 0x0403E4D1, 0x0D04D447, 0x160D85FD, 0x1F0AB56B,
			0x05B5A8FA, 0x0CB2986C, 0x17BBC9D6, 0x1EBCF940, 0xFAD86CE3, 0xF3DF5C75, 0xE8D60DCF, 0xE1D13D59,
			0x06D930AC, 0x0FDE003A, 0x14D75180, 0x1DD06116, 0xF9B4F4B5, 0xF0B3C423, 0xEBBA9599, 0xE2BDA50F,
			0xF802B89E, 0xF1058808, 0xEA0CD9B2, 0xE30BE924, 0x076F7C87, 0x0E684C11, 0x15611DAB, 0x1C662D3D,
			0xF6DC4190, 0xFFDB7106, 0xE4D220BC, 0xEDD5102A, 0x09B18589, 0x00B6B51F, 0x1BBFE4A5, 0x12B8D433,
			0x0807C9A2, 0x0100F934, 0x1A09A88E, 0x130E9818, 0xF76A0DBB, 0xFE6D3D2D, 0xE5646C97, 0xEC635C01,
			0x0B6B51F4, 0x026C6162, 0x196530D8, 0x1062004E, 0xF40695ED, 0xFD01A57B, 0xE608F4C1, 0xEF0FC457,
			0xF5B0D9C6, 0xFCB7E950, 0xE7BEB8EA, 0xEEB9887C, 0x0ADD1DDF, 0x03DA2D49, 0x18D37CF3, 0x11D44C65,
			0x0DB26158, 0x04B551CE, 0x1FBC0074, 0x16BB30E2, 0xF2DFA541, 0xFBD895D7, 0xE0D1C46D, 0xE9D6F4FB,
			0xF369E96A, 0xFA6ED9FC, 0xE1678846, 0xE860B8D0, 0x0C042D73, 0x05031DE5, 0x1E0A4C5F, 0x170D7CC9,
			0xF005713C, 0xF90241AA, 0xE20B1010, 0xEB0C2086, 0x0F68B525, 0x066F85B3, 0x1D66D409, 0x1461E49F,
			0x0EDEF90E, 0x07D9C998, 0x1CD09822, 0x15D7A8B4, 0xF1B33D17, 0xF8B40D81, 0xE3BD5C3B, 0xEABA6CAD,
			0xEDB88320, 0xE4BFB3B6, 0xFFB6E20C, 0xF6B1D29A, 0x12D54739, 0x1BD277AF, 0x00DB2615, 0x09DC1683,
			0x13630B12, 0x1A643B84, 0x016D6A3E, 0x086A5AA8, 0xEC0ECF0B, 0xE509FF9D, 0xFE00AE27, 0xF7079EB1,
			0x100F9344, 0x1908A3D2, 0x0201F268, 0x0B06C2FE, 0xEF62575D, 0xE66567CB, 0xFD6C3671, 0xF46B06E7,
			0xEED41B76, 0xE7D32BE0, 0xFCDA7A5A, 0xF5DD4ACC, 0x11B9DF6F, 0x18BEEFF9, 0x03B7BE43, 0x0AB08ED5,
			0x16D6A3E8, 0x1FD1937E, 0x04D8C2C4, 0x0DDFF252, 0xE9BB67F1, 0xE0BC5767, 0xFBB506DD, 0xF2B2364B,
			0xE80D2BDA, 0xE10A1B4C, 0xFA034AF6, 0xF3047A60, 0x1760EFC3, 0x1E67DF55, 0x056E8EEF, 0x0C69BE79,
			0xEB61B38C, 0xE266831A, 0xF96FD2A0, 0xF068E236, 0x140C7795, 0x1D0B4703, 0x060216B9, 0x0F05262F,
			0x15BA3BBE, 0x1CBD0B28, 0x07B45A92, 0x0EB36A04, 0xEAD7FFA7, 0xE3D0CF31, 0xF8D99E8B, 0xF1DEAE1D,
			0x1B64C2B0, 0x1263F226, 0x096AA39C, 0x006D930A, 0xE40906A9, 0xED0E363F, 0xF6076785, 0xFF005713,
			0xE5BF4A82, 0xECB87A14, 0xF7B12BAE, 0xFEB61B38, 0x1AD28E9B, 0x13D5BE0D, 0x08DCEFB7, 0x01DBDF21,
			0xE6D3D2D4, 0xEFD4E242, 0xF4DDB3F8, 0xFDDA836E, 0x19BE16CD, 0x10B9265B, 0x0BB077E1, 0x02B74777,
			0x18085AE6, 0x110F6A70, 0x0A063BCA, 0x03010B5C, 0xE7659EFF, 0xEE62AE69, 0xF56BFFD3, 0xFC6CCF45,
			0xE00AE278, 0xE90DD2EE, 0xF2048354, 0xFB03B3C2, 0x1F672661, 0x166016F7, 0x0D69474D, 0x046E77DB,
			0x1ED16A4A, 0x17D65ADC, 0x0CDF0B66, 0x05D83BF0, 0xE1BCAE53, 0xE8BB9EC5, 0xF3B2CF7F, 0xFAB5FFE9,
			0x1DBDF21C, 0x14BAC28A, 0x0FB39330, 0x06B4A3A6, 0xE2D03605, 0xEBD70693, 0xF0DE5729, 0xF9D967BF,
			0xE3667A2E, 0xEA614AB8, 0xF1681B02, 0xF86F2B94, 0x1C0BBE37, 0x150C8EA1, 0x0E05DF1B, 0x0702EF8D
		};


		uint32_t checksum = 0xFEDCBA76;
		checksum = ~checksum;

		uint32_t ringbuff[5] = { 0 };
		int i = 0;

		for (size_t i = 0;i < count;i++) {
			uint8_t data;
			if (in.read((char*)&data, 1).eof())
				break;
			checksum = crctable[data ^ (uint8_t)checksum] ^ (checksum >> 8);
		}
		return ~checksum;
	}

	inline void FixChecksumForFile(std::string path) {

		std::fstream in(path, std::ios::in | std::ios::out | std::ios::binary);
		in.seekg(-4, std::ios::end);
		auto count = in.tellg();
		if (count < 0x16)
			throw std::exception("file is too short, can't fix checksum");
		count -= 0x10;
		in.seekg(0x10, std::ios::beg);
		uint32_t checksum = EvalChecksum(in, count);

		in.seekp(-4, std::ios::end);
		in.write((char*)&checksum, 4);
	}
}