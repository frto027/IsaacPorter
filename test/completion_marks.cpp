#include <cstdint>
#include <cstring>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>
#include "../IsaacSave.h"

namespace {

// Rows are the 34 characters; columns are Heart, Isaac, Satan, Boss Rush,
// Blue Baby, Lamb, Mega Satan, Greed, Hush, Delirium, Mother and Beast.
// Independent reference IDs from https://repentogon.com/enums/EventCounter.html.
const uint32_t mark_ids[34][12] = {
    {27, 41, 55, 69, 83, 97, 116, 130, 144, 173, 423, 457}, // ISAAC
    {28, 42, 56, 70, 84, 98, 117, 131, 145, 174, 424, 458}, // MAGDALENE
    {29, 43, 57, 71, 85, 99, 118, 132, 146, 175, 425, 459}, // CAIN
    {30, 44, 58, 72, 86, 100, 119, 133, 147, 176, 426, 460}, // JUDAS
    {31, 45, 59, 73, 87, 101, 120, 134, 148, 177, 427, 461}, // BLUE_BABY
    {32, 46, 60, 74, 88, 102, 121, 135, 149, 178, 428, 462}, // EVE
    {33, 47, 61, 75, 89, 103, 122, 136, 150, 179, 429, 463}, // SAMSON
    {34, 48, 62, 76, 90, 104, 123, 137, 151, 180, 430, 464}, // AZAZEL
    {35, 49, 63, 77, 91, 105, 124, 138, 152, 181, 431, 465}, // LAZARUS
    {36, 50, 64, 78, 92, 106, 125, 139, 153, 182, 432, 466}, // EDEN
    {37, 51, 65, 79, 93, 107, 126, 140, 154, 183, 433, 467}, // THE_LOST
    {38, 52, 66, 80, 94, 108, 127, 141, 155, 184, 434, 468}, // LILITH
    {39, 53, 67, 81, 95, 109, 128, 142, 156, 185, 435, 469}, // KEEPER
    {40, 54, 68, 82, 96, 110, 129, 143, 157, 186, 436, 470}, // APOLLYON
    {203, 204, 205, 206, 207, 208, 209, 210, 211, 213, 437, 471}, // THE_FORGOTTEN
    {214, 233, 252, 271, 290, 309, 328, 347, 366, 404, 438, 472}, // BETHANY
    {215, 234, 253, 272, 291, 310, 329, 348, 367, 405, 439, 473}, // JACOB_AND_ESAU
    {216, 235, 254, 273, 292, 311, 330, 349, 368, 406, 440, 474}, // T_ISAAC
    {217, 236, 255, 274, 293, 312, 331, 350, 369, 407, 441, 475}, // T_MAGDALENE
    {218, 237, 256, 275, 294, 313, 332, 351, 370, 408, 442, 476}, // T_CAIN
    {219, 238, 257, 276, 295, 314, 333, 352, 371, 409, 443, 477}, // T_JUDAS
    {220, 239, 258, 277, 296, 315, 334, 353, 372, 410, 444, 478}, // T_BLUE_BABY
    {221, 240, 259, 278, 297, 316, 335, 354, 373, 411, 445, 479}, // T_EVE
    {222, 241, 260, 279, 298, 317, 336, 355, 374, 412, 446, 480}, // T_SAMSON
    {223, 242, 261, 280, 299, 318, 337, 356, 375, 413, 447, 481}, // T_AZAZEL
    {224, 243, 262, 281, 300, 319, 338, 357, 376, 414, 448, 482}, // T_LAZARUS
    {225, 244, 263, 282, 301, 320, 339, 358, 377, 415, 449, 483}, // T_EDEN
    {226, 245, 264, 283, 302, 321, 340, 359, 378, 416, 450, 484}, // T_THE_LOST
    {227, 246, 265, 284, 303, 322, 341, 360, 379, 417, 451, 485}, // T_LILITH
    {228, 247, 266, 285, 304, 323, 342, 361, 380, 418, 452, 486}, // T_KEEPER
    {229, 248, 267, 286, 305, 324, 343, 362, 381, 419, 453, 487}, // T_APOLLYON
    {230, 249, 268, 287, 306, 325, 344, 363, 382, 420, 454, 488}, // T_THE_FORGOTTEN
    {231, 250, 269, 288, 307, 326, 345, 364, 383, 421, 455, 489}, // T_BETHANY
    {232, 251, 270, 289, 308, 327, 346, 365, 384, 422, 456, 490}, // T_JACOB_AND_ESAU
};

void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

std::vector<bool> expected_mark_ids() {
    std::vector<bool> result(523, false);
    for (const auto& character : mark_ids) {
        for (uint32_t index : character) {
            require(index < 496 && !result[index], "Invalid reference mark ID");
            result[index] = true;
        }
    }
    return result;
}

IsaacSave::CounterBlock make_block(uint32_t count, uint32_t flags) {
    IsaacSave::CounterBlock block;
    block.block_size = count * 4;
    block.extra_number = count;
    for (uint32_t i = 0; i < count; ++i) block.counters.push_back(0x10000000u + i);
    for (const auto& character : mark_ids) {
        for (uint32_t index : character) {
            if (index < count) block.counters[index] = flags;
        }
    }
    return block;
}

void test_flag_combinations_and_preservation() {
    const uint32_t expected_levels[16] = {0, 1, 2, 2, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2};
    const auto marks = expected_mark_ids();
    for (uint32_t i = 0; i < marks.size(); ++i) {
        require(IsaacSave::CounterBlock::IsCompletionCounter(i) == marks[i], "Completion counter classification mismatch");
    }
    for (uint32_t flags = 0; flags < 16; ++flags) {
        auto block = make_block(523, flags);
        const auto before = block.counters;
        block.cut(IsaacSave::VER_REP);
        require(block.counters.size() == 496, "Rep+ counter trimming failed");
        require(block.block_size == 496 * 4 && block.extra_number == 496, "Counter metadata was not adjusted");
        for (uint32_t i = 0; i < 496; ++i) {
            require(block.counters[i] == (marks[i] ? expected_levels[flags] : before[i]), "Difficulty conversion or non-mark preservation failed");
        }
        const auto once = block.counters;
        block.cut(IsaacSave::VER_REP);
        require(block.counters == once, "Repeated downgrade changed the marks");

        std::ostringstream out(std::ios::out | std::ios::binary);
        block.write(out);
        require(out.str().size() == 8 + 496 * 4, "Serialized counter block length mismatch");
        std::istringstream in(out.str(), std::ios::in | std::ios::binary);
        IsaacSave::CounterBlock read_back;
        read_back.read(in);
        require(read_back.counters == block.counters, "Serialized marks did not round-trip");
    }
}

void test_existing_rep_states() {
    auto block = make_block(496, 0);
    for (const auto& character : mark_ids) {
        for (uint32_t index : character) block.counters[index] = index % 3;
    }
    const auto before = block.counters;
    block.cut(IsaacSave::VER_REP);
    require(block.counters == before, "Existing Rep normal/hard states changed");
}

void test_partial_counter_block() {
    auto block = make_block(100, 3);
    const auto before = block.counters;
    const auto marks = expected_mark_ids();
    block.cut(IsaacSave::VER_REP);
    require(block.counters.size() == 100, "Partial block was resized");
    for (uint32_t i = 0; i < 100; ++i) {
        require(block.counters[i] == (marks[i] ? 2u : before[i]), "Partial block conversion failed");
    }
}

void test_unknown_flags_do_not_mutate_block() {
    auto block = make_block(523, 15);
    block.counters[180] = 16;
    const auto before = block.counters;
    bool rejected = false;
    try { block.cut(IsaacSave::VER_REP); }
    catch (const std::runtime_error&) { rejected = true; }
    require(rejected, "Unknown completion flags were accepted");
    require(block.counters == before, "Failed conversion modified the block");
    require(block.block_size == 523 * 4 && block.extra_number == 523, "Failed conversion modified metadata");
}

struct CaptureOutput {
    std::ostringstream buffer;
    std::streambuf* previous;
    CaptureOutput() : previous(std::cout.rdbuf(buffer.rdbuf())) {}
    ~CaptureOutput() { std::cout.rdbuf(previous); }
};

} // namespace

int main() {
    try {
        {
            CaptureOutput ignored;
            test_flag_combinations_and_preservation();
            test_existing_rep_states();
            test_partial_counter_block();
            test_unknown_flags_do_not_mutate_block();
        }
        std::cout << "Completion mark regression tests passed (408 marks, 16 flag combinations).\n";
        return 0;
    }
    catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
