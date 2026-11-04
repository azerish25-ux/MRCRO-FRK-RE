#pragma once
#include "mfrk/notepath.hpp"
#include <cstdint>
#include <array>
namespace mfrk::notepath {
struct TestAccess {
  using Fields = std::array<std::uint64_t, 810>;
  static Fields fields(const Engine& n) noexcept {
    const auto& s = n.state_;
    return {
      s.poly.enabled,
      s.poly.priority,
      s.poly.legato,
      s.poly.age[0],
      s.poly.age[1],
      s.poly.age[2],
      s.poly.age[3],
      s.poly.age[4],
      s.poly.age[5],
      s.poly.age[6],
      s.poly.age[7],
      s.poly.age[8],
      s.poly.age[9],
      s.poly.age[10],
      s.poly.age[11],
      s.poly.age[12],
      s.poly.age[13],
      s.poly.age[14],
      s.poly.age[15],
      s.poly.age[16],
      s.poly.age[17],
      s.poly.age[18],
      s.poly.age[19],
      s.poly.age[20],
      s.poly.age[21],
      s.poly.age[22],
      s.poly.age[23],
      s.poly.age[24],
      s.poly.age[25],
      s.poly.age[26],
      s.poly.age[27],
      s.poly.age[28],
      s.poly.age[29],
      s.poly.age[30],
      s.poly.age[31],
      s.poly.age[32],
      s.poly.age[33],
      s.poly.age[34],
      s.poly.age[35],
      s.poly.age[36],
      s.poly.age[37],
      s.poly.age[38],
      s.poly.age[39],
      s.poly.age[40],
      s.poly.age[41],
      s.poly.age[42],
      s.poly.age[43],
      s.poly.age[44],
      s.poly.age[45],
      s.poly.age[46],
      s.poly.age[47],
      s.poly.age[48],
      s.poly.age[49],
      s.poly.age[50],
      s.poly.age[51],
      s.poly.age[52],
      s.poly.age[53],
      s.poly.age[54],
      s.poly.age[55],
      s.poly.age[56],
      s.poly.age[57],
      s.poly.age[58],
      s.poly.age[59],
      s.poly.age[60],
      s.poly.age[61],
      s.poly.age[62],
      s.poly.age[63],
      s.poly.age[64],
      s.poly.age[65],
      s.poly.age[66],
      s.poly.age[67],
      s.poly.age[68],
      s.poly.age[69],
      s.poly.age[70],
      s.poly.age[71],
      s.poly.age[72],
      s.poly.age[73],
      s.poly.age[74],
      s.poly.age[75],
      s.poly.age[76],
      s.poly.age[77],
      s.poly.age[78],
      s.poly.age[79],
      s.poly.age[80],
      s.poly.age[81],
      s.poly.age[82],
      s.poly.age[83],
      s.poly.age[84],
      s.poly.age[85],
      s.poly.age[86],
      s.poly.age[87],
      s.poly.age[88],
      s.poly.age[89],
      s.poly.age[90],
      s.poly.age[91],
      s.poly.age[92],
      s.poly.age[93],
      s.poly.age[94],
      s.poly.age[95],
      s.poly.age[96],
      s.poly.age[97],
      s.poly.age[98],
      s.poly.age[99],
      s.poly.age[100],
      s.poly.age[101],
      s.poly.age[102],
      s.poly.age[103],
      s.poly.age[104],
      s.poly.age[105],
      s.poly.age[106],
      s.poly.age[107],
      s.poly.age[108],
      s.poly.age[109],
      s.poly.age[110],
      s.poly.age[111],
      s.poly.age[112],
      s.poly.age[113],
      s.poly.age[114],
      s.poly.age[115],
      s.poly.age[116],
      s.poly.age[117],
      s.poly.age[118],
      s.poly.age[119],
      s.poly.age[120],
      s.poly.age[121],
      s.poly.age[122],
      s.poly.age[123],
      s.poly.age[124],
      s.poly.age[125],
      s.poly.age[126],
      s.poly.age[127],
      s.poly.velocity[0],
      s.poly.velocity[1],
      s.poly.velocity[2],
      s.poly.velocity[3],
      s.poly.velocity[4],
      s.poly.velocity[5],
      s.poly.velocity[6],
      s.poly.velocity[7],
      s.poly.velocity[8],
      s.poly.velocity[9],
      s.poly.velocity[10],
      s.poly.velocity[11],
      s.poly.velocity[12],
      s.poly.velocity[13],
      s.poly.velocity[14],
      s.poly.velocity[15],
      s.poly.velocity[16],
      s.poly.velocity[17],
      s.poly.velocity[18],
      s.poly.velocity[19],
      s.poly.velocity[20],
      s.poly.velocity[21],
      s.poly.velocity[22],
      s.poly.velocity[23],
      s.poly.velocity[24],
      s.poly.velocity[25],
      s.poly.velocity[26],
      s.poly.velocity[27],
      s.poly.velocity[28],
      s.poly.velocity[29],
      s.poly.velocity[30],
      s.poly.velocity[31],
      s.poly.velocity[32],
      s.poly.velocity[33],
      s.poly.velocity[34],
      s.poly.velocity[35],
      s.poly.velocity[36],
      s.poly.velocity[37],
      s.poly.velocity[38],
      s.poly.velocity[39],
      s.poly.velocity[40],
      s.poly.velocity[41],
      s.poly.velocity[42],
      s.poly.velocity[43],
      s.poly.velocity[44],
      s.poly.velocity[45],
      s.poly.velocity[46],
      s.poly.velocity[47],
      s.poly.velocity[48],
      s.poly.velocity[49],
      s.poly.velocity[50],
      s.poly.velocity[51],
      s.poly.velocity[52],
      s.poly.velocity[53],
      s.poly.velocity[54],
      s.poly.velocity[55],
      s.poly.velocity[56],
      s.poly.velocity[57],
      s.poly.velocity[58],
      s.poly.velocity[59],
      s.poly.velocity[60],
      s.poly.velocity[61],
      s.poly.velocity[62],
      s.poly.velocity[63],
      s.poly.velocity[64],
      s.poly.velocity[65],
      s.poly.velocity[66],
      s.poly.velocity[67],
      s.poly.velocity[68],
      s.poly.velocity[69],
      s.poly.velocity[70],
      s.poly.velocity[71],
      s.poly.velocity[72],
      s.poly.velocity[73],
      s.poly.velocity[74],
      s.poly.velocity[75],
      s.poly.velocity[76],
      s.poly.velocity[77],
      s.poly.velocity[78],
      s.poly.velocity[79],
      s.poly.velocity[80],
      s.poly.velocity[81],
      s.poly.velocity[82],
      s.poly.velocity[83],
      s.poly.velocity[84],
      s.poly.velocity[85],
      s.poly.velocity[86],
      s.poly.velocity[87],
      s.poly.velocity[88],
      s.poly.velocity[89],
      s.poly.velocity[90],
      s.poly.velocity[91],
      s.poly.velocity[92],
      s.poly.velocity[93],
      s.poly.velocity[94],
      s.poly.velocity[95],
      s.poly.velocity[96],
      s.poly.velocity[97],
      s.poly.velocity[98],
      s.poly.velocity[99],
      s.poly.velocity[100],
      s.poly.velocity[101],
      s.poly.velocity[102],
      s.poly.velocity[103],
      s.poly.velocity[104],
      s.poly.velocity[105],
      s.poly.velocity[106],
      s.poly.velocity[107],
      s.poly.velocity[108],
      s.poly.velocity[109],
      s.poly.velocity[110],
      s.poly.velocity[111],
      s.poly.velocity[112],
      s.poly.velocity[113],
      s.poly.velocity[114],
      s.poly.velocity[115],
      s.poly.velocity[116],
      s.poly.velocity[117],
      s.poly.velocity[118],
      s.poly.velocity[119],
      s.poly.velocity[120],
      s.poly.velocity[121],
      s.poly.velocity[122],
      s.poly.velocity[123],
      s.poly.velocity[124],
      s.poly.velocity[125],
      s.poly.velocity[126],
      s.poly.velocity[127],
      s.poly.counter,
      s.poly.selected,
      s.poly.source,
      s.poly.pending.bytes[0],
      s.poly.pending.bytes[1],
      s.poly.pending.bytes[2],
      s.poly.pending.bytes[3],
      s.poly.pending.bytes[4],
      s.poly.pending.bytes[5],
      s.poly.pending.bytes[6],
      s.poly.pending.bytes[7],
      s.poly.pending.bytes[8],
      s.poly.pending.bytes[9],
      s.poly.pending.bytes[10],
      s.poly.pending.bytes[11],
      s.poly.pending.bytes[12],
      s.poly.pending.bytes[13],
      s.poly.pending.bytes[14],
      s.poly.pending.bytes[15],
      s.poly.pending.bytes[16],
      s.poly.pending.bytes[17],
      s.poly.pending.bytes[18],
      s.poly.pending.bytes[19],
      s.poly.pending.bytes[20],
      s.poly.pending.bytes[21],
      s.poly.pending.bytes[22],
      s.poly.pending.bytes[23],
      s.poly.pending.bytes[24],
      s.poly.pending.bytes[25],
      s.poly.pending.bytes[26],
      s.poly.pending.bytes[27],
      s.poly.pending.bytes[28],
      s.poly.pending.bytes[29],
      s.poly.pending.bytes[30],
      s.poly.pending.bytes[31],
      s.poly.pending.bytes[32],
      s.poly.pending.bytes[33],
      s.poly.pending.bytes[34],
      s.poly.pending.bytes[35],
      s.poly.pending.bytes[36],
      s.poly.pending.bytes[37],
      s.poly.pending.bytes[38],
      s.poly.pending.bytes[39],
      s.poly.pending.bytes[40],
      s.poly.pending.bytes[41],
      s.poly.pending.bytes[42],
      s.poly.pending.bytes[43],
      s.poly.pending.bytes[44],
      s.poly.pending.bytes[45],
      s.poly.pending.bytes[46],
      s.poly.pending.bytes[47],
      s.poly.pending.bytes[48],
      s.poly.pending.bytes[49],
      s.poly.pending.bytes[50],
      s.poly.pending.bytes[51],
      s.poly.pending.bytes[52],
      s.poly.pending.bytes[53],
      s.poly.pending.bytes[54],
      s.poly.pending.bytes[55],
      s.poly.pending.bytes[56],
      s.chord.mode,
      s.chord.root,
      s.chord.learned,
      s.chord.count,
      s.chord.intervals[0],
      s.chord.intervals[1],
      s.chord.intervals[2],
      s.chord.intervals[3],
      s.chord.intervals[4],
      s.chord.intervals[5],
      s.chord.intervals[6],
      s.chord.intervals[7],
      s.unison.enabled,
      s.unison.mode,
      s.unison.count,
      s.unison.held,
      s.scale.mode,
      s.scale.type,
      s.scale.root,
      s.scale.mask[0],
      s.scale.mask[1],
      s.scale.mask[2],
      s.scale.mask[3],
      s.scale.mask[4],
      s.scale.mask[5],
      s.scale.mask[6],
      s.scale.mask[7],
      s.scale.mask[8],
      s.scale.mask[9],
      s.scale.mask[10],
      s.scale.mask[11],
      s.scale.map[0],
      s.scale.map[1],
      s.scale.map[2],
      s.scale.map[3],
      s.scale.map[4],
      s.scale.map[5],
      s.scale.map[6],
      s.scale.map[7],
      s.scale.map[8],
      s.scale.map[9],
      s.scale.map[10],
      s.scale.map[11],
      s.allocator.rows[0].bytes[0],
      s.allocator.rows[0].bytes[1],
      s.allocator.rows[0].bytes[2],
      s.allocator.rows[0].bytes[3],
      s.allocator.rows[0].bytes[4],
      s.allocator.rows[0].bytes[5],
      s.allocator.rows[0].bytes[6],
      s.allocator.rows[0].bytes[7],
      s.allocator.rows[0].bytes[8],
      s.allocator.rows[0].bytes[9],
      s.allocator.rows[0].bytes[10],
      s.allocator.rows[0].bytes[11],
      s.allocator.rows[0].bytes[12],
      s.allocator.rows[0].bytes[13],
      s.allocator.rows[0].bytes[14],
      s.allocator.rows[0].bytes[15],
      s.allocator.rows[0].bytes[16],
      s.allocator.rows[0].bytes[17],
      s.allocator.rows[0].bytes[18],
      s.allocator.rows[0].bytes[19],
      s.allocator.rows[1].bytes[0],
      s.allocator.rows[1].bytes[1],
      s.allocator.rows[1].bytes[2],
      s.allocator.rows[1].bytes[3],
      s.allocator.rows[1].bytes[4],
      s.allocator.rows[1].bytes[5],
      s.allocator.rows[1].bytes[6],
      s.allocator.rows[1].bytes[7],
      s.allocator.rows[1].bytes[8],
      s.allocator.rows[1].bytes[9],
      s.allocator.rows[1].bytes[10],
      s.allocator.rows[1].bytes[11],
      s.allocator.rows[1].bytes[12],
      s.allocator.rows[1].bytes[13],
      s.allocator.rows[1].bytes[14],
      s.allocator.rows[1].bytes[15],
      s.allocator.rows[1].bytes[16],
      s.allocator.rows[1].bytes[17],
      s.allocator.rows[1].bytes[18],
      s.allocator.rows[1].bytes[19],
      s.allocator.rows[2].bytes[0],
      s.allocator.rows[2].bytes[1],
      s.allocator.rows[2].bytes[2],
      s.allocator.rows[2].bytes[3],
      s.allocator.rows[2].bytes[4],
      s.allocator.rows[2].bytes[5],
      s.allocator.rows[2].bytes[6],
      s.allocator.rows[2].bytes[7],
      s.allocator.rows[2].bytes[8],
      s.allocator.rows[2].bytes[9],
      s.allocator.rows[2].bytes[10],
      s.allocator.rows[2].bytes[11],
      s.allocator.rows[2].bytes[12],
      s.allocator.rows[2].bytes[13],
      s.allocator.rows[2].bytes[14],
      s.allocator.rows[2].bytes[15],
      s.allocator.rows[2].bytes[16],
      s.allocator.rows[2].bytes[17],
      s.allocator.rows[2].bytes[18],
      s.allocator.rows[2].bytes[19],
      s.allocator.rows[3].bytes[0],
      s.allocator.rows[3].bytes[1],
      s.allocator.rows[3].bytes[2],
      s.allocator.rows[3].bytes[3],
      s.allocator.rows[3].bytes[4],
      s.allocator.rows[3].bytes[5],
      s.allocator.rows[3].bytes[6],
      s.allocator.rows[3].bytes[7],
      s.allocator.rows[3].bytes[8],
      s.allocator.rows[3].bytes[9],
      s.allocator.rows[3].bytes[10],
      s.allocator.rows[3].bytes[11],
      s.allocator.rows[3].bytes[12],
      s.allocator.rows[3].bytes[13],
      s.allocator.rows[3].bytes[14],
      s.allocator.rows[3].bytes[15],
      s.allocator.rows[3].bytes[16],
      s.allocator.rows[3].bytes[17],
      s.allocator.rows[3].bytes[18],
      s.allocator.rows[3].bytes[19],
      s.allocator.rows[4].bytes[0],
      s.allocator.rows[4].bytes[1],
      s.allocator.rows[4].bytes[2],
      s.allocator.rows[4].bytes[3],
      s.allocator.rows[4].bytes[4],
      s.allocator.rows[4].bytes[5],
      s.allocator.rows[4].bytes[6],
      s.allocator.rows[4].bytes[7],
      s.allocator.rows[4].bytes[8],
      s.allocator.rows[4].bytes[9],
      s.allocator.rows[4].bytes[10],
      s.allocator.rows[4].bytes[11],
      s.allocator.rows[4].bytes[12],
      s.allocator.rows[4].bytes[13],
      s.allocator.rows[4].bytes[14],
      s.allocator.rows[4].bytes[15],
      s.allocator.rows[4].bytes[16],
      s.allocator.rows[4].bytes[17],
      s.allocator.rows[4].bytes[18],
      s.allocator.rows[4].bytes[19],
      s.allocator.rows[5].bytes[0],
      s.allocator.rows[5].bytes[1],
      s.allocator.rows[5].bytes[2],
      s.allocator.rows[5].bytes[3],
      s.allocator.rows[5].bytes[4],
      s.allocator.rows[5].bytes[5],
      s.allocator.rows[5].bytes[6],
      s.allocator.rows[5].bytes[7],
      s.allocator.rows[5].bytes[8],
      s.allocator.rows[5].bytes[9],
      s.allocator.rows[5].bytes[10],
      s.allocator.rows[5].bytes[11],
      s.allocator.rows[5].bytes[12],
      s.allocator.rows[5].bytes[13],
      s.allocator.rows[5].bytes[14],
      s.allocator.rows[5].bytes[15],
      s.allocator.rows[5].bytes[16],
      s.allocator.rows[5].bytes[17],
      s.allocator.rows[5].bytes[18],
      s.allocator.rows[5].bytes[19],
      s.allocator.rows[6].bytes[0],
      s.allocator.rows[6].bytes[1],
      s.allocator.rows[6].bytes[2],
      s.allocator.rows[6].bytes[3],
      s.allocator.rows[6].bytes[4],
      s.allocator.rows[6].bytes[5],
      s.allocator.rows[6].bytes[6],
      s.allocator.rows[6].bytes[7],
      s.allocator.rows[6].bytes[8],
      s.allocator.rows[6].bytes[9],
      s.allocator.rows[6].bytes[10],
      s.allocator.rows[6].bytes[11],
      s.allocator.rows[6].bytes[12],
      s.allocator.rows[6].bytes[13],
      s.allocator.rows[6].bytes[14],
      s.allocator.rows[6].bytes[15],
      s.allocator.rows[6].bytes[16],
      s.allocator.rows[6].bytes[17],
      s.allocator.rows[6].bytes[18],
      s.allocator.rows[6].bytes[19],
      s.allocator.rows[7].bytes[0],
      s.allocator.rows[7].bytes[1],
      s.allocator.rows[7].bytes[2],
      s.allocator.rows[7].bytes[3],
      s.allocator.rows[7].bytes[4],
      s.allocator.rows[7].bytes[5],
      s.allocator.rows[7].bytes[6],
      s.allocator.rows[7].bytes[7],
      s.allocator.rows[7].bytes[8],
      s.allocator.rows[7].bytes[9],
      s.allocator.rows[7].bytes[10],
      s.allocator.rows[7].bytes[11],
      s.allocator.rows[7].bytes[12],
      s.allocator.rows[7].bytes[13],
      s.allocator.rows[7].bytes[14],
      s.allocator.rows[7].bytes[15],
      s.allocator.rows[7].bytes[16],
      s.allocator.rows[7].bytes[17],
      s.allocator.rows[7].bytes[18],
      s.allocator.rows[7].bytes[19],
      s.allocator.rows[8].bytes[0],
      s.allocator.rows[8].bytes[1],
      s.allocator.rows[8].bytes[2],
      s.allocator.rows[8].bytes[3],
      s.allocator.rows[8].bytes[4],
      s.allocator.rows[8].bytes[5],
      s.allocator.rows[8].bytes[6],
      s.allocator.rows[8].bytes[7],
      s.allocator.rows[8].bytes[8],
      s.allocator.rows[8].bytes[9],
      s.allocator.rows[8].bytes[10],
      s.allocator.rows[8].bytes[11],
      s.allocator.rows[8].bytes[12],
      s.allocator.rows[8].bytes[13],
      s.allocator.rows[8].bytes[14],
      s.allocator.rows[8].bytes[15],
      s.allocator.rows[8].bytes[16],
      s.allocator.rows[8].bytes[17],
      s.allocator.rows[8].bytes[18],
      s.allocator.rows[8].bytes[19],
      s.allocator.rows[9].bytes[0],
      s.allocator.rows[9].bytes[1],
      s.allocator.rows[9].bytes[2],
      s.allocator.rows[9].bytes[3],
      s.allocator.rows[9].bytes[4],
      s.allocator.rows[9].bytes[5],
      s.allocator.rows[9].bytes[6],
      s.allocator.rows[9].bytes[7],
      s.allocator.rows[9].bytes[8],
      s.allocator.rows[9].bytes[9],
      s.allocator.rows[9].bytes[10],
      s.allocator.rows[9].bytes[11],
      s.allocator.rows[9].bytes[12],
      s.allocator.rows[9].bytes[13],
      s.allocator.rows[9].bytes[14],
      s.allocator.rows[9].bytes[15],
      s.allocator.rows[9].bytes[16],
      s.allocator.rows[9].bytes[17],
      s.allocator.rows[9].bytes[18],
      s.allocator.rows[9].bytes[19],
      s.allocator.rows[10].bytes[0],
      s.allocator.rows[10].bytes[1],
      s.allocator.rows[10].bytes[2],
      s.allocator.rows[10].bytes[3],
      s.allocator.rows[10].bytes[4],
      s.allocator.rows[10].bytes[5],
      s.allocator.rows[10].bytes[6],
      s.allocator.rows[10].bytes[7],
      s.allocator.rows[10].bytes[8],
      s.allocator.rows[10].bytes[9],
      s.allocator.rows[10].bytes[10],
      s.allocator.rows[10].bytes[11],
      s.allocator.rows[10].bytes[12],
      s.allocator.rows[10].bytes[13],
      s.allocator.rows[10].bytes[14],
      s.allocator.rows[10].bytes[15],
      s.allocator.rows[10].bytes[16],
      s.allocator.rows[10].bytes[17],
      s.allocator.rows[10].bytes[18],
      s.allocator.rows[10].bytes[19],
      s.allocator.rows[11].bytes[0],
      s.allocator.rows[11].bytes[1],
      s.allocator.rows[11].bytes[2],
      s.allocator.rows[11].bytes[3],
      s.allocator.rows[11].bytes[4],
      s.allocator.rows[11].bytes[5],
      s.allocator.rows[11].bytes[6],
      s.allocator.rows[11].bytes[7],
      s.allocator.rows[11].bytes[8],
      s.allocator.rows[11].bytes[9],
      s.allocator.rows[11].bytes[10],
      s.allocator.rows[11].bytes[11],
      s.allocator.rows[11].bytes[12],
      s.allocator.rows[11].bytes[13],
      s.allocator.rows[11].bytes[14],
      s.allocator.rows[11].bytes[15],
      s.allocator.rows[11].bytes[16],
      s.allocator.rows[11].bytes[17],
      s.allocator.rows[11].bytes[18],
      s.allocator.rows[11].bytes[19],
      s.allocator.capacity,
      s.allocator.policy,
      s.allocator.allocation,
      s.allocator.steal,
      s.allocator.selected,
      s.allocator.legato,
      s.allocator.parity,
      s.allocator.unison_table,
      s.allocator.duplicates,
      s.allocator.cursor,
      s.allocator.age_counter,
      s.allocator.active_bits,
      s.allocator.sequence_index,
      s.allocator.pitch_table[0],
      s.allocator.pitch_table[1],
      s.allocator.pitch_table[2],
      s.allocator.pitch_table[3],
      s.allocator.pitch_table[4],
      s.allocator.pitch_table[5],
      s.allocator.pitch_table[6],
      s.allocator.pitch_table[7],
      s.allocator.pitch_table[8],
      s.allocator.pitch_table[9],
      s.allocator.pitch_table[10],
      s.allocator.pitch_table[11],
      s.allocator.pitch_table[12],
      s.allocator.pitch_table[13],
      s.allocator.pitch_table[14],
      s.allocator.pitch_table[15],
      s.allocator.pitch_table[16],
      s.allocator.pitch_table[17],
      s.allocator.pitch_table[18],
      s.allocator.pitch_table[19],
      s.allocator.pitch_table[20],
      s.allocator.pitch_table[21],
      s.allocator.pitch_table[22],
      s.allocator.pitch_table[23],
      s.allocator.pitch_table[24],
      s.allocator.pitch_table[25],
      s.allocator.pitch_table[26],
      s.allocator.pitch_table[27],
      s.allocator.pitch_table[28],
      s.allocator.pitch_table[29],
      s.allocator.pitch_table[30],
      s.allocator.pitch_table[31],
      s.allocator.pitch_table[32],
      s.allocator.pitch_table[33],
      s.allocator.pitch_table[34],
      s.allocator.pitch_table[35],
      s.allocator.pitch_table[36],
      s.allocator.pitch_table[37],
      s.allocator.pitch_table[38],
      s.allocator.pitch_table[39],
      s.allocator.pitch_table[40],
      s.allocator.pitch_table[41],
      s.allocator.pitch_table[42],
      s.allocator.pitch_table[43],
      s.allocator.pitch_table[44],
      s.allocator.pitch_table[45],
      s.allocator.pitch_table[46],
      s.allocator.pitch_table[47],
      s.allocator.pitch_table[48],
      s.allocator.pitch_table[49],
      s.allocator.pitch_table[50],
      s.allocator.pitch_table[51],
      s.allocator.pitch_table[52],
      s.allocator.pitch_table[53],
      s.allocator.pitch_table[54],
      s.allocator.pitch_table[55],
      s.allocator.pitch_table[56],
      s.allocator.pitch_table[57],
      s.allocator.pitch_table[58],
      s.allocator.pitch_table[59],
      s.allocator.pitch_table[60],
      s.allocator.pitch_table[61],
      s.allocator.pitch_table[62],
      s.allocator.pitch_table[63],
      s.allocator.pitch_table[64],
      s.allocator.pitch_table[65],
      s.allocator.pitch_table[66],
      s.allocator.pitch_table[67],
      s.allocator.pitch_table[68],
      s.allocator.pitch_table[69],
      s.allocator.pitch_table[70],
      s.allocator.pitch_table[71],
      s.allocator.pitch_table[72],
      s.allocator.pitch_table[73],
      s.allocator.pitch_table[74],
      s.allocator.pitch_table[75],
      s.allocator.pitch_table[76],
      s.allocator.pitch_table[77],
      s.allocator.pitch_table[78],
      s.allocator.pitch_table[79],
      s.allocator.pitch_table[80],
      s.allocator.pitch_table[81],
      s.allocator.pitch_table[82],
      s.allocator.pitch_table[83],
      s.allocator.pitch_table[84],
      s.allocator.pitch_table[85],
      s.allocator.pitch_table[86],
      s.allocator.pitch_table[87],
      s.allocator.pitch_table[88],
      s.allocator.pitch_table[89],
      s.allocator.pitch_table[90],
      s.allocator.pitch_table[91],
      s.allocator.pitch_table[92],
      s.allocator.pitch_table[93],
      s.allocator.pitch_table[94],
      s.allocator.pitch_table[95],
      s.allocator.pitch_table[96],
      s.allocator.pitch_table[97],
      s.allocator.pitch_table[98],
      s.allocator.pitch_table[99],
      s.allocator.pitch_table[100],
      s.allocator.pitch_table[101],
      s.allocator.pitch_table[102],
      s.allocator.pitch_table[103],
      s.allocator.pitch_table[104],
      s.allocator.pitch_table[105],
      s.allocator.pitch_table[106],
      s.allocator.pitch_table[107],
      s.allocator.pitch_table[108],
      s.allocator.pitch_table[109],
      s.allocator.pitch_table[110],
      s.allocator.pitch_table[111],
      s.allocator.pitch_table[112],
      s.allocator.pitch_table[113],
      s.allocator.pitch_table[114],
      s.allocator.pitch_table[115],
      s.allocator.pitch_table[116],
      s.allocator.pitch_table[117],
      s.allocator.pitch_table[118],
      s.allocator.pitch_table[119],
      s.allocator.pitch_table[120],
      s.allocator.pitch_table[121],
      s.allocator.pitch_table[122],
      s.allocator.pitch_table[123],
      s.allocator.pitch_table[124],
      s.allocator.pitch_table[125],
      s.allocator.pitch_table[126],
      s.allocator.pitch_table[127],
      s.last_input.bytes[0],
      s.last_input.bytes[1],
      s.last_input.bytes[2],
      s.last_input.bytes[3],
      s.last_input.bytes[4],
      s.last_input.bytes[5],
      s.last_input.bytes[6],
      s.last_input.bytes[7],
      s.last_input.bytes[8],
      s.last_input.bytes[9],
      s.last_input.bytes[10],
      s.last_input.bytes[11],
      s.last_input.bytes[12],
      s.last_input.bytes[13],
      s.last_input.bytes[14],
      s.last_input.bytes[15],
      s.last_input.bytes[16],
      s.last_input.bytes[17],
      s.last_input.bytes[18],
      s.last_input.bytes[19],
      s.last_input.bytes[20],
      s.last_input.bytes[21],
      s.last_input.bytes[22],
      s.last_input.bytes[23],
      s.last_input.bytes[24],
      s.last_input.bytes[25],
      s.last_input.bytes[26],
      s.last_input.bytes[27],
      s.last_input.bytes[28],
      s.last_input.bytes[29],
      s.last_input.bytes[30],
      s.last_input.bytes[31],
      s.last_input.bytes[32],
      s.last_input.bytes[33],
      s.last_input.bytes[34],
      s.last_input.bytes[35],
      s.last_input.bytes[36],
      s.last_input.bytes[37],
      s.last_input.bytes[38],
      s.last_input.bytes[39],
      s.last_input.bytes[40],
      s.last_input.bytes[41],
      s.last_input.bytes[42],
      s.last_input.bytes[43],
      s.last_input.bytes[44],
      s.last_input.bytes[45],
      s.last_input.bytes[46],
      s.last_input.bytes[47],
      s.last_input.bytes[48],
      s.last_input.bytes[49],
      s.last_input.bytes[50],
      s.last_input.bytes[51],
      s.last_input.bytes[52],
      s.last_input.bytes[53],
      s.last_input.bytes[54],
      s.last_input.bytes[55],
      s.last_input.bytes[56],
      s.held,
      s.last_active_row,
      s.keyboard_scale_mask,
      s.global_scale_mask,
      s.global_scale,
      s.global_root,
      s.status_pending,
      s.suppress_status_clear,
      s.on_count,
      s.off_count,
    };
  }
  static void load(Engine& n, const Fields& f) noexcept {
    auto& s = n.state_;
    s.poly.enabled = static_cast<std::uint8_t>(f[0]);
    s.poly.priority = static_cast<std::uint8_t>(f[1]);
    s.poly.legato = static_cast<std::uint8_t>(f[2]);
    s.poly.age[0] = static_cast<std::uint16_t>(f[3]);
    s.poly.age[1] = static_cast<std::uint16_t>(f[4]);
    s.poly.age[2] = static_cast<std::uint16_t>(f[5]);
    s.poly.age[3] = static_cast<std::uint16_t>(f[6]);
    s.poly.age[4] = static_cast<std::uint16_t>(f[7]);
    s.poly.age[5] = static_cast<std::uint16_t>(f[8]);
    s.poly.age[6] = static_cast<std::uint16_t>(f[9]);
    s.poly.age[7] = static_cast<std::uint16_t>(f[10]);
    s.poly.age[8] = static_cast<std::uint16_t>(f[11]);
    s.poly.age[9] = static_cast<std::uint16_t>(f[12]);
    s.poly.age[10] = static_cast<std::uint16_t>(f[13]);
    s.poly.age[11] = static_cast<std::uint16_t>(f[14]);
    s.poly.age[12] = static_cast<std::uint16_t>(f[15]);
    s.poly.age[13] = static_cast<std::uint16_t>(f[16]);
    s.poly.age[14] = static_cast<std::uint16_t>(f[17]);
    s.poly.age[15] = static_cast<std::uint16_t>(f[18]);
    s.poly.age[16] = static_cast<std::uint16_t>(f[19]);
    s.poly.age[17] = static_cast<std::uint16_t>(f[20]);
    s.poly.age[18] = static_cast<std::uint16_t>(f[21]);
    s.poly.age[19] = static_cast<std::uint16_t>(f[22]);
    s.poly.age[20] = static_cast<std::uint16_t>(f[23]);
    s.poly.age[21] = static_cast<std::uint16_t>(f[24]);
    s.poly.age[22] = static_cast<std::uint16_t>(f[25]);
    s.poly.age[23] = static_cast<std::uint16_t>(f[26]);
    s.poly.age[24] = static_cast<std::uint16_t>(f[27]);
    s.poly.age[25] = static_cast<std::uint16_t>(f[28]);
    s.poly.age[26] = static_cast<std::uint16_t>(f[29]);
    s.poly.age[27] = static_cast<std::uint16_t>(f[30]);
    s.poly.age[28] = static_cast<std::uint16_t>(f[31]);
    s.poly.age[29] = static_cast<std::uint16_t>(f[32]);
    s.poly.age[30] = static_cast<std::uint16_t>(f[33]);
    s.poly.age[31] = static_cast<std::uint16_t>(f[34]);
    s.poly.age[32] = static_cast<std::uint16_t>(f[35]);
    s.poly.age[33] = static_cast<std::uint16_t>(f[36]);
    s.poly.age[34] = static_cast<std::uint16_t>(f[37]);
    s.poly.age[35] = static_cast<std::uint16_t>(f[38]);
    s.poly.age[36] = static_cast<std::uint16_t>(f[39]);
    s.poly.age[37] = static_cast<std::uint16_t>(f[40]);
    s.poly.age[38] = static_cast<std::uint16_t>(f[41]);
    s.poly.age[39] = static_cast<std::uint16_t>(f[42]);
    s.poly.age[40] = static_cast<std::uint16_t>(f[43]);
    s.poly.age[41] = static_cast<std::uint16_t>(f[44]);
    s.poly.age[42] = static_cast<std::uint16_t>(f[45]);
    s.poly.age[43] = static_cast<std::uint16_t>(f[46]);
    s.poly.age[44] = static_cast<std::uint16_t>(f[47]);
    s.poly.age[45] = static_cast<std::uint16_t>(f[48]);
    s.poly.age[46] = static_cast<std::uint16_t>(f[49]);
    s.poly.age[47] = static_cast<std::uint16_t>(f[50]);
    s.poly.age[48] = static_cast<std::uint16_t>(f[51]);
    s.poly.age[49] = static_cast<std::uint16_t>(f[52]);
    s.poly.age[50] = static_cast<std::uint16_t>(f[53]);
    s.poly.age[51] = static_cast<std::uint16_t>(f[54]);
    s.poly.age[52] = static_cast<std::uint16_t>(f[55]);
    s.poly.age[53] = static_cast<std::uint16_t>(f[56]);
    s.poly.age[54] = static_cast<std::uint16_t>(f[57]);
    s.poly.age[55] = static_cast<std::uint16_t>(f[58]);
    s.poly.age[56] = static_cast<std::uint16_t>(f[59]);
    s.poly.age[57] = static_cast<std::uint16_t>(f[60]);
    s.poly.age[58] = static_cast<std::uint16_t>(f[61]);
    s.poly.age[59] = static_cast<std::uint16_t>(f[62]);
    s.poly.age[60] = static_cast<std::uint16_t>(f[63]);
    s.poly.age[61] = static_cast<std::uint16_t>(f[64]);
    s.poly.age[62] = static_cast<std::uint16_t>(f[65]);
    s.poly.age[63] = static_cast<std::uint16_t>(f[66]);
    s.poly.age[64] = static_cast<std::uint16_t>(f[67]);
    s.poly.age[65] = static_cast<std::uint16_t>(f[68]);
    s.poly.age[66] = static_cast<std::uint16_t>(f[69]);
    s.poly.age[67] = static_cast<std::uint16_t>(f[70]);
    s.poly.age[68] = static_cast<std::uint16_t>(f[71]);
    s.poly.age[69] = static_cast<std::uint16_t>(f[72]);
    s.poly.age[70] = static_cast<std::uint16_t>(f[73]);
    s.poly.age[71] = static_cast<std::uint16_t>(f[74]);
    s.poly.age[72] = static_cast<std::uint16_t>(f[75]);
    s.poly.age[73] = static_cast<std::uint16_t>(f[76]);
    s.poly.age[74] = static_cast<std::uint16_t>(f[77]);
    s.poly.age[75] = static_cast<std::uint16_t>(f[78]);
    s.poly.age[76] = static_cast<std::uint16_t>(f[79]);
    s.poly.age[77] = static_cast<std::uint16_t>(f[80]);
    s.poly.age[78] = static_cast<std::uint16_t>(f[81]);
    s.poly.age[79] = static_cast<std::uint16_t>(f[82]);
    s.poly.age[80] = static_cast<std::uint16_t>(f[83]);
    s.poly.age[81] = static_cast<std::uint16_t>(f[84]);
    s.poly.age[82] = static_cast<std::uint16_t>(f[85]);
    s.poly.age[83] = static_cast<std::uint16_t>(f[86]);
    s.poly.age[84] = static_cast<std::uint16_t>(f[87]);
    s.poly.age[85] = static_cast<std::uint16_t>(f[88]);
    s.poly.age[86] = static_cast<std::uint16_t>(f[89]);
    s.poly.age[87] = static_cast<std::uint16_t>(f[90]);
    s.poly.age[88] = static_cast<std::uint16_t>(f[91]);
    s.poly.age[89] = static_cast<std::uint16_t>(f[92]);
    s.poly.age[90] = static_cast<std::uint16_t>(f[93]);
    s.poly.age[91] = static_cast<std::uint16_t>(f[94]);
    s.poly.age[92] = static_cast<std::uint16_t>(f[95]);
    s.poly.age[93] = static_cast<std::uint16_t>(f[96]);
    s.poly.age[94] = static_cast<std::uint16_t>(f[97]);
    s.poly.age[95] = static_cast<std::uint16_t>(f[98]);
    s.poly.age[96] = static_cast<std::uint16_t>(f[99]);
    s.poly.age[97] = static_cast<std::uint16_t>(f[100]);
    s.poly.age[98] = static_cast<std::uint16_t>(f[101]);
    s.poly.age[99] = static_cast<std::uint16_t>(f[102]);
    s.poly.age[100] = static_cast<std::uint16_t>(f[103]);
    s.poly.age[101] = static_cast<std::uint16_t>(f[104]);
    s.poly.age[102] = static_cast<std::uint16_t>(f[105]);
    s.poly.age[103] = static_cast<std::uint16_t>(f[106]);
    s.poly.age[104] = static_cast<std::uint16_t>(f[107]);
    s.poly.age[105] = static_cast<std::uint16_t>(f[108]);
    s.poly.age[106] = static_cast<std::uint16_t>(f[109]);
    s.poly.age[107] = static_cast<std::uint16_t>(f[110]);
    s.poly.age[108] = static_cast<std::uint16_t>(f[111]);
    s.poly.age[109] = static_cast<std::uint16_t>(f[112]);
    s.poly.age[110] = static_cast<std::uint16_t>(f[113]);
    s.poly.age[111] = static_cast<std::uint16_t>(f[114]);
    s.poly.age[112] = static_cast<std::uint16_t>(f[115]);
    s.poly.age[113] = static_cast<std::uint16_t>(f[116]);
    s.poly.age[114] = static_cast<std::uint16_t>(f[117]);
    s.poly.age[115] = static_cast<std::uint16_t>(f[118]);
    s.poly.age[116] = static_cast<std::uint16_t>(f[119]);
    s.poly.age[117] = static_cast<std::uint16_t>(f[120]);
    s.poly.age[118] = static_cast<std::uint16_t>(f[121]);
    s.poly.age[119] = static_cast<std::uint16_t>(f[122]);
    s.poly.age[120] = static_cast<std::uint16_t>(f[123]);
    s.poly.age[121] = static_cast<std::uint16_t>(f[124]);
    s.poly.age[122] = static_cast<std::uint16_t>(f[125]);
    s.poly.age[123] = static_cast<std::uint16_t>(f[126]);
    s.poly.age[124] = static_cast<std::uint16_t>(f[127]);
    s.poly.age[125] = static_cast<std::uint16_t>(f[128]);
    s.poly.age[126] = static_cast<std::uint16_t>(f[129]);
    s.poly.age[127] = static_cast<std::uint16_t>(f[130]);
    s.poly.velocity[0] = static_cast<std::uint8_t>(f[131]);
    s.poly.velocity[1] = static_cast<std::uint8_t>(f[132]);
    s.poly.velocity[2] = static_cast<std::uint8_t>(f[133]);
    s.poly.velocity[3] = static_cast<std::uint8_t>(f[134]);
    s.poly.velocity[4] = static_cast<std::uint8_t>(f[135]);
    s.poly.velocity[5] = static_cast<std::uint8_t>(f[136]);
    s.poly.velocity[6] = static_cast<std::uint8_t>(f[137]);
    s.poly.velocity[7] = static_cast<std::uint8_t>(f[138]);
    s.poly.velocity[8] = static_cast<std::uint8_t>(f[139]);
    s.poly.velocity[9] = static_cast<std::uint8_t>(f[140]);
    s.poly.velocity[10] = static_cast<std::uint8_t>(f[141]);
    s.poly.velocity[11] = static_cast<std::uint8_t>(f[142]);
    s.poly.velocity[12] = static_cast<std::uint8_t>(f[143]);
    s.poly.velocity[13] = static_cast<std::uint8_t>(f[144]);
    s.poly.velocity[14] = static_cast<std::uint8_t>(f[145]);
    s.poly.velocity[15] = static_cast<std::uint8_t>(f[146]);
    s.poly.velocity[16] = static_cast<std::uint8_t>(f[147]);
    s.poly.velocity[17] = static_cast<std::uint8_t>(f[148]);
    s.poly.velocity[18] = static_cast<std::uint8_t>(f[149]);
    s.poly.velocity[19] = static_cast<std::uint8_t>(f[150]);
    s.poly.velocity[20] = static_cast<std::uint8_t>(f[151]);
    s.poly.velocity[21] = static_cast<std::uint8_t>(f[152]);
    s.poly.velocity[22] = static_cast<std::uint8_t>(f[153]);
    s.poly.velocity[23] = static_cast<std::uint8_t>(f[154]);
    s.poly.velocity[24] = static_cast<std::uint8_t>(f[155]);
    s.poly.velocity[25] = static_cast<std::uint8_t>(f[156]);
    s.poly.velocity[26] = static_cast<std::uint8_t>(f[157]);
    s.poly.velocity[27] = static_cast<std::uint8_t>(f[158]);
    s.poly.velocity[28] = static_cast<std::uint8_t>(f[159]);
    s.poly.velocity[29] = static_cast<std::uint8_t>(f[160]);
    s.poly.velocity[30] = static_cast<std::uint8_t>(f[161]);
    s.poly.velocity[31] = static_cast<std::uint8_t>(f[162]);
    s.poly.velocity[32] = static_cast<std::uint8_t>(f[163]);
    s.poly.velocity[33] = static_cast<std::uint8_t>(f[164]);
    s.poly.velocity[34] = static_cast<std::uint8_t>(f[165]);
    s.poly.velocity[35] = static_cast<std::uint8_t>(f[166]);
    s.poly.velocity[36] = static_cast<std::uint8_t>(f[167]);
    s.poly.velocity[37] = static_cast<std::uint8_t>(f[168]);
    s.poly.velocity[38] = static_cast<std::uint8_t>(f[169]);
    s.poly.velocity[39] = static_cast<std::uint8_t>(f[170]);
    s.poly.velocity[40] = static_cast<std::uint8_t>(f[171]);
    s.poly.velocity[41] = static_cast<std::uint8_t>(f[172]);
    s.poly.velocity[42] = static_cast<std::uint8_t>(f[173]);
    s.poly.velocity[43] = static_cast<std::uint8_t>(f[174]);
    s.poly.velocity[44] = static_cast<std::uint8_t>(f[175]);
    s.poly.velocity[45] = static_cast<std::uint8_t>(f[176]);
    s.poly.velocity[46] = static_cast<std::uint8_t>(f[177]);
    s.poly.velocity[47] = static_cast<std::uint8_t>(f[178]);
    s.poly.velocity[48] = static_cast<std::uint8_t>(f[179]);
    s.poly.velocity[49] = static_cast<std::uint8_t>(f[180]);
    s.poly.velocity[50] = static_cast<std::uint8_t>(f[181]);
    s.poly.velocity[51] = static_cast<std::uint8_t>(f[182]);
    s.poly.velocity[52] = static_cast<std::uint8_t>(f[183]);
    s.poly.velocity[53] = static_cast<std::uint8_t>(f[184]);
    s.poly.velocity[54] = static_cast<std::uint8_t>(f[185]);
    s.poly.velocity[55] = static_cast<std::uint8_t>(f[186]);
    s.poly.velocity[56] = static_cast<std::uint8_t>(f[187]);
    s.poly.velocity[57] = static_cast<std::uint8_t>(f[188]);
    s.poly.velocity[58] = static_cast<std::uint8_t>(f[189]);
    s.poly.velocity[59] = static_cast<std::uint8_t>(f[190]);
    s.poly.velocity[60] = static_cast<std::uint8_t>(f[191]);
    s.poly.velocity[61] = static_cast<std::uint8_t>(f[192]);
    s.poly.velocity[62] = static_cast<std::uint8_t>(f[193]);
    s.poly.velocity[63] = static_cast<std::uint8_t>(f[194]);
    s.poly.velocity[64] = static_cast<std::uint8_t>(f[195]);
    s.poly.velocity[65] = static_cast<std::uint8_t>(f[196]);
    s.poly.velocity[66] = static_cast<std::uint8_t>(f[197]);
    s.poly.velocity[67] = static_cast<std::uint8_t>(f[198]);
    s.poly.velocity[68] = static_cast<std::uint8_t>(f[199]);
    s.poly.velocity[69] = static_cast<std::uint8_t>(f[200]);
    s.poly.velocity[70] = static_cast<std::uint8_t>(f[201]);
    s.poly.velocity[71] = static_cast<std::uint8_t>(f[202]);
    s.poly.velocity[72] = static_cast<std::uint8_t>(f[203]);
    s.poly.velocity[73] = static_cast<std::uint8_t>(f[204]);
    s.poly.velocity[74] = static_cast<std::uint8_t>(f[205]);
    s.poly.velocity[75] = static_cast<std::uint8_t>(f[206]);
    s.poly.velocity[76] = static_cast<std::uint8_t>(f[207]);
    s.poly.velocity[77] = static_cast<std::uint8_t>(f[208]);
    s.poly.velocity[78] = static_cast<std::uint8_t>(f[209]);
    s.poly.velocity[79] = static_cast<std::uint8_t>(f[210]);
    s.poly.velocity[80] = static_cast<std::uint8_t>(f[211]);
    s.poly.velocity[81] = static_cast<std::uint8_t>(f[212]);
    s.poly.velocity[82] = static_cast<std::uint8_t>(f[213]);
    s.poly.velocity[83] = static_cast<std::uint8_t>(f[214]);
    s.poly.velocity[84] = static_cast<std::uint8_t>(f[215]);
    s.poly.velocity[85] = static_cast<std::uint8_t>(f[216]);
    s.poly.velocity[86] = static_cast<std::uint8_t>(f[217]);
    s.poly.velocity[87] = static_cast<std::uint8_t>(f[218]);
    s.poly.velocity[88] = static_cast<std::uint8_t>(f[219]);
    s.poly.velocity[89] = static_cast<std::uint8_t>(f[220]);
    s.poly.velocity[90] = static_cast<std::uint8_t>(f[221]);
    s.poly.velocity[91] = static_cast<std::uint8_t>(f[222]);
    s.poly.velocity[92] = static_cast<std::uint8_t>(f[223]);
    s.poly.velocity[93] = static_cast<std::uint8_t>(f[224]);
    s.poly.velocity[94] = static_cast<std::uint8_t>(f[225]);
    s.poly.velocity[95] = static_cast<std::uint8_t>(f[226]);
    s.poly.velocity[96] = static_cast<std::uint8_t>(f[227]);
    s.poly.velocity[97] = static_cast<std::uint8_t>(f[228]);
    s.poly.velocity[98] = static_cast<std::uint8_t>(f[229]);
    s.poly.velocity[99] = static_cast<std::uint8_t>(f[230]);
    s.poly.velocity[100] = static_cast<std::uint8_t>(f[231]);
    s.poly.velocity[101] = static_cast<std::uint8_t>(f[232]);
    s.poly.velocity[102] = static_cast<std::uint8_t>(f[233]);
    s.poly.velocity[103] = static_cast<std::uint8_t>(f[234]);
    s.poly.velocity[104] = static_cast<std::uint8_t>(f[235]);
    s.poly.velocity[105] = static_cast<std::uint8_t>(f[236]);
    s.poly.velocity[106] = static_cast<std::uint8_t>(f[237]);
    s.poly.velocity[107] = static_cast<std::uint8_t>(f[238]);
    s.poly.velocity[108] = static_cast<std::uint8_t>(f[239]);
    s.poly.velocity[109] = static_cast<std::uint8_t>(f[240]);
    s.poly.velocity[110] = static_cast<std::uint8_t>(f[241]);
    s.poly.velocity[111] = static_cast<std::uint8_t>(f[242]);
    s.poly.velocity[112] = static_cast<std::uint8_t>(f[243]);
    s.poly.velocity[113] = static_cast<std::uint8_t>(f[244]);
    s.poly.velocity[114] = static_cast<std::uint8_t>(f[245]);
    s.poly.velocity[115] = static_cast<std::uint8_t>(f[246]);
    s.poly.velocity[116] = static_cast<std::uint8_t>(f[247]);
    s.poly.velocity[117] = static_cast<std::uint8_t>(f[248]);
    s.poly.velocity[118] = static_cast<std::uint8_t>(f[249]);
    s.poly.velocity[119] = static_cast<std::uint8_t>(f[250]);
    s.poly.velocity[120] = static_cast<std::uint8_t>(f[251]);
    s.poly.velocity[121] = static_cast<std::uint8_t>(f[252]);
    s.poly.velocity[122] = static_cast<std::uint8_t>(f[253]);
    s.poly.velocity[123] = static_cast<std::uint8_t>(f[254]);
    s.poly.velocity[124] = static_cast<std::uint8_t>(f[255]);
    s.poly.velocity[125] = static_cast<std::uint8_t>(f[256]);
    s.poly.velocity[126] = static_cast<std::uint8_t>(f[257]);
    s.poly.velocity[127] = static_cast<std::uint8_t>(f[258]);
    s.poly.counter = static_cast<std::uint16_t>(f[259]);
    s.poly.selected = static_cast<std::uint8_t>(f[260]);
    s.poly.source = static_cast<std::uint8_t>(f[261]);
    s.poly.pending.bytes[0] = static_cast<std::uint8_t>(f[262]);
    s.poly.pending.bytes[1] = static_cast<std::uint8_t>(f[263]);
    s.poly.pending.bytes[2] = static_cast<std::uint8_t>(f[264]);
    s.poly.pending.bytes[3] = static_cast<std::uint8_t>(f[265]);
    s.poly.pending.bytes[4] = static_cast<std::uint8_t>(f[266]);
    s.poly.pending.bytes[5] = static_cast<std::uint8_t>(f[267]);
    s.poly.pending.bytes[6] = static_cast<std::uint8_t>(f[268]);
    s.poly.pending.bytes[7] = static_cast<std::uint8_t>(f[269]);
    s.poly.pending.bytes[8] = static_cast<std::uint8_t>(f[270]);
    s.poly.pending.bytes[9] = static_cast<std::uint8_t>(f[271]);
    s.poly.pending.bytes[10] = static_cast<std::uint8_t>(f[272]);
    s.poly.pending.bytes[11] = static_cast<std::uint8_t>(f[273]);
    s.poly.pending.bytes[12] = static_cast<std::uint8_t>(f[274]);
    s.poly.pending.bytes[13] = static_cast<std::uint8_t>(f[275]);
    s.poly.pending.bytes[14] = static_cast<std::uint8_t>(f[276]);
    s.poly.pending.bytes[15] = static_cast<std::uint8_t>(f[277]);
    s.poly.pending.bytes[16] = static_cast<std::uint8_t>(f[278]);
    s.poly.pending.bytes[17] = static_cast<std::uint8_t>(f[279]);
    s.poly.pending.bytes[18] = static_cast<std::uint8_t>(f[280]);
    s.poly.pending.bytes[19] = static_cast<std::uint8_t>(f[281]);
    s.poly.pending.bytes[20] = static_cast<std::uint8_t>(f[282]);
    s.poly.pending.bytes[21] = static_cast<std::uint8_t>(f[283]);
    s.poly.pending.bytes[22] = static_cast<std::uint8_t>(f[284]);
    s.poly.pending.bytes[23] = static_cast<std::uint8_t>(f[285]);
    s.poly.pending.bytes[24] = static_cast<std::uint8_t>(f[286]);
    s.poly.pending.bytes[25] = static_cast<std::uint8_t>(f[287]);
    s.poly.pending.bytes[26] = static_cast<std::uint8_t>(f[288]);
    s.poly.pending.bytes[27] = static_cast<std::uint8_t>(f[289]);
    s.poly.pending.bytes[28] = static_cast<std::uint8_t>(f[290]);
    s.poly.pending.bytes[29] = static_cast<std::uint8_t>(f[291]);
    s.poly.pending.bytes[30] = static_cast<std::uint8_t>(f[292]);
    s.poly.pending.bytes[31] = static_cast<std::uint8_t>(f[293]);
    s.poly.pending.bytes[32] = static_cast<std::uint8_t>(f[294]);
    s.poly.pending.bytes[33] = static_cast<std::uint8_t>(f[295]);
    s.poly.pending.bytes[34] = static_cast<std::uint8_t>(f[296]);
    s.poly.pending.bytes[35] = static_cast<std::uint8_t>(f[297]);
    s.poly.pending.bytes[36] = static_cast<std::uint8_t>(f[298]);
    s.poly.pending.bytes[37] = static_cast<std::uint8_t>(f[299]);
    s.poly.pending.bytes[38] = static_cast<std::uint8_t>(f[300]);
    s.poly.pending.bytes[39] = static_cast<std::uint8_t>(f[301]);
    s.poly.pending.bytes[40] = static_cast<std::uint8_t>(f[302]);
    s.poly.pending.bytes[41] = static_cast<std::uint8_t>(f[303]);
    s.poly.pending.bytes[42] = static_cast<std::uint8_t>(f[304]);
    s.poly.pending.bytes[43] = static_cast<std::uint8_t>(f[305]);
    s.poly.pending.bytes[44] = static_cast<std::uint8_t>(f[306]);
    s.poly.pending.bytes[45] = static_cast<std::uint8_t>(f[307]);
    s.poly.pending.bytes[46] = static_cast<std::uint8_t>(f[308]);
    s.poly.pending.bytes[47] = static_cast<std::uint8_t>(f[309]);
    s.poly.pending.bytes[48] = static_cast<std::uint8_t>(f[310]);
    s.poly.pending.bytes[49] = static_cast<std::uint8_t>(f[311]);
    s.poly.pending.bytes[50] = static_cast<std::uint8_t>(f[312]);
    s.poly.pending.bytes[51] = static_cast<std::uint8_t>(f[313]);
    s.poly.pending.bytes[52] = static_cast<std::uint8_t>(f[314]);
    s.poly.pending.bytes[53] = static_cast<std::uint8_t>(f[315]);
    s.poly.pending.bytes[54] = static_cast<std::uint8_t>(f[316]);
    s.poly.pending.bytes[55] = static_cast<std::uint8_t>(f[317]);
    s.poly.pending.bytes[56] = static_cast<std::uint8_t>(f[318]);
    s.chord.mode = static_cast<std::uint8_t>(f[319]);
    s.chord.root = static_cast<std::uint8_t>(f[320]);
    s.chord.learned = static_cast<std::uint8_t>(f[321]);
    s.chord.count = static_cast<std::uint8_t>(f[322]);
    s.chord.intervals[0] = static_cast<std::uint8_t>(f[323]);
    s.chord.intervals[1] = static_cast<std::uint8_t>(f[324]);
    s.chord.intervals[2] = static_cast<std::uint8_t>(f[325]);
    s.chord.intervals[3] = static_cast<std::uint8_t>(f[326]);
    s.chord.intervals[4] = static_cast<std::uint8_t>(f[327]);
    s.chord.intervals[5] = static_cast<std::uint8_t>(f[328]);
    s.chord.intervals[6] = static_cast<std::uint8_t>(f[329]);
    s.chord.intervals[7] = static_cast<std::uint8_t>(f[330]);
    s.unison.enabled = static_cast<std::uint8_t>(f[331]);
    s.unison.mode = static_cast<std::uint8_t>(f[332]);
    s.unison.count = static_cast<std::uint8_t>(f[333]);
    s.unison.held = static_cast<std::uint8_t>(f[334]);
    s.scale.mode = static_cast<std::uint8_t>(f[335]);
    s.scale.type = static_cast<std::uint8_t>(f[336]);
    s.scale.root = static_cast<std::uint8_t>(f[337]);
    s.scale.mask[0] = static_cast<std::uint8_t>(f[338]);
    s.scale.mask[1] = static_cast<std::uint8_t>(f[339]);
    s.scale.mask[2] = static_cast<std::uint8_t>(f[340]);
    s.scale.mask[3] = static_cast<std::uint8_t>(f[341]);
    s.scale.mask[4] = static_cast<std::uint8_t>(f[342]);
    s.scale.mask[5] = static_cast<std::uint8_t>(f[343]);
    s.scale.mask[6] = static_cast<std::uint8_t>(f[344]);
    s.scale.mask[7] = static_cast<std::uint8_t>(f[345]);
    s.scale.mask[8] = static_cast<std::uint8_t>(f[346]);
    s.scale.mask[9] = static_cast<std::uint8_t>(f[347]);
    s.scale.mask[10] = static_cast<std::uint8_t>(f[348]);
    s.scale.mask[11] = static_cast<std::uint8_t>(f[349]);
    s.scale.map[0] = static_cast<std::uint8_t>(f[350]);
    s.scale.map[1] = static_cast<std::uint8_t>(f[351]);
    s.scale.map[2] = static_cast<std::uint8_t>(f[352]);
    s.scale.map[3] = static_cast<std::uint8_t>(f[353]);
    s.scale.map[4] = static_cast<std::uint8_t>(f[354]);
    s.scale.map[5] = static_cast<std::uint8_t>(f[355]);
    s.scale.map[6] = static_cast<std::uint8_t>(f[356]);
    s.scale.map[7] = static_cast<std::uint8_t>(f[357]);
    s.scale.map[8] = static_cast<std::uint8_t>(f[358]);
    s.scale.map[9] = static_cast<std::uint8_t>(f[359]);
    s.scale.map[10] = static_cast<std::uint8_t>(f[360]);
    s.scale.map[11] = static_cast<std::uint8_t>(f[361]);
    s.allocator.rows[0].bytes[0] = static_cast<std::uint8_t>(f[362]);
    s.allocator.rows[0].bytes[1] = static_cast<std::uint8_t>(f[363]);
    s.allocator.rows[0].bytes[2] = static_cast<std::uint8_t>(f[364]);
    s.allocator.rows[0].bytes[3] = static_cast<std::uint8_t>(f[365]);
    s.allocator.rows[0].bytes[4] = static_cast<std::uint8_t>(f[366]);
    s.allocator.rows[0].bytes[5] = static_cast<std::uint8_t>(f[367]);
    s.allocator.rows[0].bytes[6] = static_cast<std::uint8_t>(f[368]);
    s.allocator.rows[0].bytes[7] = static_cast<std::uint8_t>(f[369]);
    s.allocator.rows[0].bytes[8] = static_cast<std::uint8_t>(f[370]);
    s.allocator.rows[0].bytes[9] = static_cast<std::uint8_t>(f[371]);
    s.allocator.rows[0].bytes[10] = static_cast<std::uint8_t>(f[372]);
    s.allocator.rows[0].bytes[11] = static_cast<std::uint8_t>(f[373]);
    s.allocator.rows[0].bytes[12] = static_cast<std::uint8_t>(f[374]);
    s.allocator.rows[0].bytes[13] = static_cast<std::uint8_t>(f[375]);
    s.allocator.rows[0].bytes[14] = static_cast<std::uint8_t>(f[376]);
    s.allocator.rows[0].bytes[15] = static_cast<std::uint8_t>(f[377]);
    s.allocator.rows[0].bytes[16] = static_cast<std::uint8_t>(f[378]);
    s.allocator.rows[0].bytes[17] = static_cast<std::uint8_t>(f[379]);
    s.allocator.rows[0].bytes[18] = static_cast<std::uint8_t>(f[380]);
    s.allocator.rows[0].bytes[19] = static_cast<std::uint8_t>(f[381]);
    s.allocator.rows[1].bytes[0] = static_cast<std::uint8_t>(f[382]);
    s.allocator.rows[1].bytes[1] = static_cast<std::uint8_t>(f[383]);
    s.allocator.rows[1].bytes[2] = static_cast<std::uint8_t>(f[384]);
    s.allocator.rows[1].bytes[3] = static_cast<std::uint8_t>(f[385]);
    s.allocator.rows[1].bytes[4] = static_cast<std::uint8_t>(f[386]);
    s.allocator.rows[1].bytes[5] = static_cast<std::uint8_t>(f[387]);
    s.allocator.rows[1].bytes[6] = static_cast<std::uint8_t>(f[388]);
    s.allocator.rows[1].bytes[7] = static_cast<std::uint8_t>(f[389]);
    s.allocator.rows[1].bytes[8] = static_cast<std::uint8_t>(f[390]);
    s.allocator.rows[1].bytes[9] = static_cast<std::uint8_t>(f[391]);
    s.allocator.rows[1].bytes[10] = static_cast<std::uint8_t>(f[392]);
    s.allocator.rows[1].bytes[11] = static_cast<std::uint8_t>(f[393]);
    s.allocator.rows[1].bytes[12] = static_cast<std::uint8_t>(f[394]);
    s.allocator.rows[1].bytes[13] = static_cast<std::uint8_t>(f[395]);
    s.allocator.rows[1].bytes[14] = static_cast<std::uint8_t>(f[396]);
    s.allocator.rows[1].bytes[15] = static_cast<std::uint8_t>(f[397]);
    s.allocator.rows[1].bytes[16] = static_cast<std::uint8_t>(f[398]);
    s.allocator.rows[1].bytes[17] = static_cast<std::uint8_t>(f[399]);
    s.allocator.rows[1].bytes[18] = static_cast<std::uint8_t>(f[400]);
    s.allocator.rows[1].bytes[19] = static_cast<std::uint8_t>(f[401]);
    s.allocator.rows[2].bytes[0] = static_cast<std::uint8_t>(f[402]);
    s.allocator.rows[2].bytes[1] = static_cast<std::uint8_t>(f[403]);
    s.allocator.rows[2].bytes[2] = static_cast<std::uint8_t>(f[404]);
    s.allocator.rows[2].bytes[3] = static_cast<std::uint8_t>(f[405]);
    s.allocator.rows[2].bytes[4] = static_cast<std::uint8_t>(f[406]);
    s.allocator.rows[2].bytes[5] = static_cast<std::uint8_t>(f[407]);
    s.allocator.rows[2].bytes[6] = static_cast<std::uint8_t>(f[408]);
    s.allocator.rows[2].bytes[7] = static_cast<std::uint8_t>(f[409]);
    s.allocator.rows[2].bytes[8] = static_cast<std::uint8_t>(f[410]);
    s.allocator.rows[2].bytes[9] = static_cast<std::uint8_t>(f[411]);
    s.allocator.rows[2].bytes[10] = static_cast<std::uint8_t>(f[412]);
    s.allocator.rows[2].bytes[11] = static_cast<std::uint8_t>(f[413]);
    s.allocator.rows[2].bytes[12] = static_cast<std::uint8_t>(f[414]);
    s.allocator.rows[2].bytes[13] = static_cast<std::uint8_t>(f[415]);
    s.allocator.rows[2].bytes[14] = static_cast<std::uint8_t>(f[416]);
    s.allocator.rows[2].bytes[15] = static_cast<std::uint8_t>(f[417]);
    s.allocator.rows[2].bytes[16] = static_cast<std::uint8_t>(f[418]);
    s.allocator.rows[2].bytes[17] = static_cast<std::uint8_t>(f[419]);
    s.allocator.rows[2].bytes[18] = static_cast<std::uint8_t>(f[420]);
    s.allocator.rows[2].bytes[19] = static_cast<std::uint8_t>(f[421]);
    s.allocator.rows[3].bytes[0] = static_cast<std::uint8_t>(f[422]);
    s.allocator.rows[3].bytes[1] = static_cast<std::uint8_t>(f[423]);
    s.allocator.rows[3].bytes[2] = static_cast<std::uint8_t>(f[424]);
    s.allocator.rows[3].bytes[3] = static_cast<std::uint8_t>(f[425]);
    s.allocator.rows[3].bytes[4] = static_cast<std::uint8_t>(f[426]);
    s.allocator.rows[3].bytes[5] = static_cast<std::uint8_t>(f[427]);
    s.allocator.rows[3].bytes[6] = static_cast<std::uint8_t>(f[428]);
    s.allocator.rows[3].bytes[7] = static_cast<std::uint8_t>(f[429]);
    s.allocator.rows[3].bytes[8] = static_cast<std::uint8_t>(f[430]);
    s.allocator.rows[3].bytes[9] = static_cast<std::uint8_t>(f[431]);
    s.allocator.rows[3].bytes[10] = static_cast<std::uint8_t>(f[432]);
    s.allocator.rows[3].bytes[11] = static_cast<std::uint8_t>(f[433]);
    s.allocator.rows[3].bytes[12] = static_cast<std::uint8_t>(f[434]);
    s.allocator.rows[3].bytes[13] = static_cast<std::uint8_t>(f[435]);
    s.allocator.rows[3].bytes[14] = static_cast<std::uint8_t>(f[436]);
    s.allocator.rows[3].bytes[15] = static_cast<std::uint8_t>(f[437]);
    s.allocator.rows[3].bytes[16] = static_cast<std::uint8_t>(f[438]);
    s.allocator.rows[3].bytes[17] = static_cast<std::uint8_t>(f[439]);
    s.allocator.rows[3].bytes[18] = static_cast<std::uint8_t>(f[440]);
    s.allocator.rows[3].bytes[19] = static_cast<std::uint8_t>(f[441]);
    s.allocator.rows[4].bytes[0] = static_cast<std::uint8_t>(f[442]);
    s.allocator.rows[4].bytes[1] = static_cast<std::uint8_t>(f[443]);
    s.allocator.rows[4].bytes[2] = static_cast<std::uint8_t>(f[444]);
    s.allocator.rows[4].bytes[3] = static_cast<std::uint8_t>(f[445]);
    s.allocator.rows[4].bytes[4] = static_cast<std::uint8_t>(f[446]);
    s.allocator.rows[4].bytes[5] = static_cast<std::uint8_t>(f[447]);
    s.allocator.rows[4].bytes[6] = static_cast<std::uint8_t>(f[448]);
    s.allocator.rows[4].bytes[7] = static_cast<std::uint8_t>(f[449]);
    s.allocator.rows[4].bytes[8] = static_cast<std::uint8_t>(f[450]);
    s.allocator.rows[4].bytes[9] = static_cast<std::uint8_t>(f[451]);
    s.allocator.rows[4].bytes[10] = static_cast<std::uint8_t>(f[452]);
    s.allocator.rows[4].bytes[11] = static_cast<std::uint8_t>(f[453]);
    s.allocator.rows[4].bytes[12] = static_cast<std::uint8_t>(f[454]);
    s.allocator.rows[4].bytes[13] = static_cast<std::uint8_t>(f[455]);
    s.allocator.rows[4].bytes[14] = static_cast<std::uint8_t>(f[456]);
    s.allocator.rows[4].bytes[15] = static_cast<std::uint8_t>(f[457]);
    s.allocator.rows[4].bytes[16] = static_cast<std::uint8_t>(f[458]);
    s.allocator.rows[4].bytes[17] = static_cast<std::uint8_t>(f[459]);
    s.allocator.rows[4].bytes[18] = static_cast<std::uint8_t>(f[460]);
    s.allocator.rows[4].bytes[19] = static_cast<std::uint8_t>(f[461]);
    s.allocator.rows[5].bytes[0] = static_cast<std::uint8_t>(f[462]);
    s.allocator.rows[5].bytes[1] = static_cast<std::uint8_t>(f[463]);
    s.allocator.rows[5].bytes[2] = static_cast<std::uint8_t>(f[464]);
    s.allocator.rows[5].bytes[3] = static_cast<std::uint8_t>(f[465]);
    s.allocator.rows[5].bytes[4] = static_cast<std::uint8_t>(f[466]);
    s.allocator.rows[5].bytes[5] = static_cast<std::uint8_t>(f[467]);
    s.allocator.rows[5].bytes[6] = static_cast<std::uint8_t>(f[468]);
    s.allocator.rows[5].bytes[7] = static_cast<std::uint8_t>(f[469]);
    s.allocator.rows[5].bytes[8] = static_cast<std::uint8_t>(f[470]);
    s.allocator.rows[5].bytes[9] = static_cast<std::uint8_t>(f[471]);
    s.allocator.rows[5].bytes[10] = static_cast<std::uint8_t>(f[472]);
    s.allocator.rows[5].bytes[11] = static_cast<std::uint8_t>(f[473]);
    s.allocator.rows[5].bytes[12] = static_cast<std::uint8_t>(f[474]);
    s.allocator.rows[5].bytes[13] = static_cast<std::uint8_t>(f[475]);
    s.allocator.rows[5].bytes[14] = static_cast<std::uint8_t>(f[476]);
    s.allocator.rows[5].bytes[15] = static_cast<std::uint8_t>(f[477]);
    s.allocator.rows[5].bytes[16] = static_cast<std::uint8_t>(f[478]);
    s.allocator.rows[5].bytes[17] = static_cast<std::uint8_t>(f[479]);
    s.allocator.rows[5].bytes[18] = static_cast<std::uint8_t>(f[480]);
    s.allocator.rows[5].bytes[19] = static_cast<std::uint8_t>(f[481]);
    s.allocator.rows[6].bytes[0] = static_cast<std::uint8_t>(f[482]);
    s.allocator.rows[6].bytes[1] = static_cast<std::uint8_t>(f[483]);
    s.allocator.rows[6].bytes[2] = static_cast<std::uint8_t>(f[484]);
    s.allocator.rows[6].bytes[3] = static_cast<std::uint8_t>(f[485]);
    s.allocator.rows[6].bytes[4] = static_cast<std::uint8_t>(f[486]);
    s.allocator.rows[6].bytes[5] = static_cast<std::uint8_t>(f[487]);
    s.allocator.rows[6].bytes[6] = static_cast<std::uint8_t>(f[488]);
    s.allocator.rows[6].bytes[7] = static_cast<std::uint8_t>(f[489]);
    s.allocator.rows[6].bytes[8] = static_cast<std::uint8_t>(f[490]);
    s.allocator.rows[6].bytes[9] = static_cast<std::uint8_t>(f[491]);
    s.allocator.rows[6].bytes[10] = static_cast<std::uint8_t>(f[492]);
    s.allocator.rows[6].bytes[11] = static_cast<std::uint8_t>(f[493]);
    s.allocator.rows[6].bytes[12] = static_cast<std::uint8_t>(f[494]);
    s.allocator.rows[6].bytes[13] = static_cast<std::uint8_t>(f[495]);
    s.allocator.rows[6].bytes[14] = static_cast<std::uint8_t>(f[496]);
    s.allocator.rows[6].bytes[15] = static_cast<std::uint8_t>(f[497]);
    s.allocator.rows[6].bytes[16] = static_cast<std::uint8_t>(f[498]);
    s.allocator.rows[6].bytes[17] = static_cast<std::uint8_t>(f[499]);
    s.allocator.rows[6].bytes[18] = static_cast<std::uint8_t>(f[500]);
    s.allocator.rows[6].bytes[19] = static_cast<std::uint8_t>(f[501]);
    s.allocator.rows[7].bytes[0] = static_cast<std::uint8_t>(f[502]);
    s.allocator.rows[7].bytes[1] = static_cast<std::uint8_t>(f[503]);
    s.allocator.rows[7].bytes[2] = static_cast<std::uint8_t>(f[504]);
    s.allocator.rows[7].bytes[3] = static_cast<std::uint8_t>(f[505]);
    s.allocator.rows[7].bytes[4] = static_cast<std::uint8_t>(f[506]);
    s.allocator.rows[7].bytes[5] = static_cast<std::uint8_t>(f[507]);
    s.allocator.rows[7].bytes[6] = static_cast<std::uint8_t>(f[508]);
    s.allocator.rows[7].bytes[7] = static_cast<std::uint8_t>(f[509]);
    s.allocator.rows[7].bytes[8] = static_cast<std::uint8_t>(f[510]);
    s.allocator.rows[7].bytes[9] = static_cast<std::uint8_t>(f[511]);
    s.allocator.rows[7].bytes[10] = static_cast<std::uint8_t>(f[512]);
    s.allocator.rows[7].bytes[11] = static_cast<std::uint8_t>(f[513]);
    s.allocator.rows[7].bytes[12] = static_cast<std::uint8_t>(f[514]);
    s.allocator.rows[7].bytes[13] = static_cast<std::uint8_t>(f[515]);
    s.allocator.rows[7].bytes[14] = static_cast<std::uint8_t>(f[516]);
    s.allocator.rows[7].bytes[15] = static_cast<std::uint8_t>(f[517]);
    s.allocator.rows[7].bytes[16] = static_cast<std::uint8_t>(f[518]);
    s.allocator.rows[7].bytes[17] = static_cast<std::uint8_t>(f[519]);
    s.allocator.rows[7].bytes[18] = static_cast<std::uint8_t>(f[520]);
    s.allocator.rows[7].bytes[19] = static_cast<std::uint8_t>(f[521]);
    s.allocator.rows[8].bytes[0] = static_cast<std::uint8_t>(f[522]);
    s.allocator.rows[8].bytes[1] = static_cast<std::uint8_t>(f[523]);
    s.allocator.rows[8].bytes[2] = static_cast<std::uint8_t>(f[524]);
    s.allocator.rows[8].bytes[3] = static_cast<std::uint8_t>(f[525]);
    s.allocator.rows[8].bytes[4] = static_cast<std::uint8_t>(f[526]);
    s.allocator.rows[8].bytes[5] = static_cast<std::uint8_t>(f[527]);
    s.allocator.rows[8].bytes[6] = static_cast<std::uint8_t>(f[528]);
    s.allocator.rows[8].bytes[7] = static_cast<std::uint8_t>(f[529]);
    s.allocator.rows[8].bytes[8] = static_cast<std::uint8_t>(f[530]);
    s.allocator.rows[8].bytes[9] = static_cast<std::uint8_t>(f[531]);
    s.allocator.rows[8].bytes[10] = static_cast<std::uint8_t>(f[532]);
    s.allocator.rows[8].bytes[11] = static_cast<std::uint8_t>(f[533]);
    s.allocator.rows[8].bytes[12] = static_cast<std::uint8_t>(f[534]);
    s.allocator.rows[8].bytes[13] = static_cast<std::uint8_t>(f[535]);
    s.allocator.rows[8].bytes[14] = static_cast<std::uint8_t>(f[536]);
    s.allocator.rows[8].bytes[15] = static_cast<std::uint8_t>(f[537]);
    s.allocator.rows[8].bytes[16] = static_cast<std::uint8_t>(f[538]);
    s.allocator.rows[8].bytes[17] = static_cast<std::uint8_t>(f[539]);
    s.allocator.rows[8].bytes[18] = static_cast<std::uint8_t>(f[540]);
    s.allocator.rows[8].bytes[19] = static_cast<std::uint8_t>(f[541]);
    s.allocator.rows[9].bytes[0] = static_cast<std::uint8_t>(f[542]);
    s.allocator.rows[9].bytes[1] = static_cast<std::uint8_t>(f[543]);
    s.allocator.rows[9].bytes[2] = static_cast<std::uint8_t>(f[544]);
    s.allocator.rows[9].bytes[3] = static_cast<std::uint8_t>(f[545]);
    s.allocator.rows[9].bytes[4] = static_cast<std::uint8_t>(f[546]);
    s.allocator.rows[9].bytes[5] = static_cast<std::uint8_t>(f[547]);
    s.allocator.rows[9].bytes[6] = static_cast<std::uint8_t>(f[548]);
    s.allocator.rows[9].bytes[7] = static_cast<std::uint8_t>(f[549]);
    s.allocator.rows[9].bytes[8] = static_cast<std::uint8_t>(f[550]);
    s.allocator.rows[9].bytes[9] = static_cast<std::uint8_t>(f[551]);
    s.allocator.rows[9].bytes[10] = static_cast<std::uint8_t>(f[552]);
    s.allocator.rows[9].bytes[11] = static_cast<std::uint8_t>(f[553]);
    s.allocator.rows[9].bytes[12] = static_cast<std::uint8_t>(f[554]);
    s.allocator.rows[9].bytes[13] = static_cast<std::uint8_t>(f[555]);
    s.allocator.rows[9].bytes[14] = static_cast<std::uint8_t>(f[556]);
    s.allocator.rows[9].bytes[15] = static_cast<std::uint8_t>(f[557]);
    s.allocator.rows[9].bytes[16] = static_cast<std::uint8_t>(f[558]);
    s.allocator.rows[9].bytes[17] = static_cast<std::uint8_t>(f[559]);
    s.allocator.rows[9].bytes[18] = static_cast<std::uint8_t>(f[560]);
    s.allocator.rows[9].bytes[19] = static_cast<std::uint8_t>(f[561]);
    s.allocator.rows[10].bytes[0] = static_cast<std::uint8_t>(f[562]);
    s.allocator.rows[10].bytes[1] = static_cast<std::uint8_t>(f[563]);
    s.allocator.rows[10].bytes[2] = static_cast<std::uint8_t>(f[564]);
    s.allocator.rows[10].bytes[3] = static_cast<std::uint8_t>(f[565]);
    s.allocator.rows[10].bytes[4] = static_cast<std::uint8_t>(f[566]);
    s.allocator.rows[10].bytes[5] = static_cast<std::uint8_t>(f[567]);
    s.allocator.rows[10].bytes[6] = static_cast<std::uint8_t>(f[568]);
    s.allocator.rows[10].bytes[7] = static_cast<std::uint8_t>(f[569]);
    s.allocator.rows[10].bytes[8] = static_cast<std::uint8_t>(f[570]);
    s.allocator.rows[10].bytes[9] = static_cast<std::uint8_t>(f[571]);
    s.allocator.rows[10].bytes[10] = static_cast<std::uint8_t>(f[572]);
    s.allocator.rows[10].bytes[11] = static_cast<std::uint8_t>(f[573]);
    s.allocator.rows[10].bytes[12] = static_cast<std::uint8_t>(f[574]);
    s.allocator.rows[10].bytes[13] = static_cast<std::uint8_t>(f[575]);
    s.allocator.rows[10].bytes[14] = static_cast<std::uint8_t>(f[576]);
    s.allocator.rows[10].bytes[15] = static_cast<std::uint8_t>(f[577]);
    s.allocator.rows[10].bytes[16] = static_cast<std::uint8_t>(f[578]);
    s.allocator.rows[10].bytes[17] = static_cast<std::uint8_t>(f[579]);
    s.allocator.rows[10].bytes[18] = static_cast<std::uint8_t>(f[580]);
    s.allocator.rows[10].bytes[19] = static_cast<std::uint8_t>(f[581]);
    s.allocator.rows[11].bytes[0] = static_cast<std::uint8_t>(f[582]);
    s.allocator.rows[11].bytes[1] = static_cast<std::uint8_t>(f[583]);
    s.allocator.rows[11].bytes[2] = static_cast<std::uint8_t>(f[584]);
    s.allocator.rows[11].bytes[3] = static_cast<std::uint8_t>(f[585]);
    s.allocator.rows[11].bytes[4] = static_cast<std::uint8_t>(f[586]);
    s.allocator.rows[11].bytes[5] = static_cast<std::uint8_t>(f[587]);
    s.allocator.rows[11].bytes[6] = static_cast<std::uint8_t>(f[588]);
    s.allocator.rows[11].bytes[7] = static_cast<std::uint8_t>(f[589]);
    s.allocator.rows[11].bytes[8] = static_cast<std::uint8_t>(f[590]);
    s.allocator.rows[11].bytes[9] = static_cast<std::uint8_t>(f[591]);
    s.allocator.rows[11].bytes[10] = static_cast<std::uint8_t>(f[592]);
    s.allocator.rows[11].bytes[11] = static_cast<std::uint8_t>(f[593]);
    s.allocator.rows[11].bytes[12] = static_cast<std::uint8_t>(f[594]);
    s.allocator.rows[11].bytes[13] = static_cast<std::uint8_t>(f[595]);
    s.allocator.rows[11].bytes[14] = static_cast<std::uint8_t>(f[596]);
    s.allocator.rows[11].bytes[15] = static_cast<std::uint8_t>(f[597]);
    s.allocator.rows[11].bytes[16] = static_cast<std::uint8_t>(f[598]);
    s.allocator.rows[11].bytes[17] = static_cast<std::uint8_t>(f[599]);
    s.allocator.rows[11].bytes[18] = static_cast<std::uint8_t>(f[600]);
    s.allocator.rows[11].bytes[19] = static_cast<std::uint8_t>(f[601]);
    s.allocator.capacity = static_cast<std::uint8_t>(f[602]);
    s.allocator.policy = static_cast<std::uint8_t>(f[603]);
    s.allocator.allocation = static_cast<std::uint8_t>(f[604]);
    s.allocator.steal = static_cast<std::uint8_t>(f[605]);
    s.allocator.selected = static_cast<std::uint8_t>(f[606]);
    s.allocator.legato = static_cast<std::uint8_t>(f[607]);
    s.allocator.parity = static_cast<std::uint8_t>(f[608]);
    s.allocator.unison_table = static_cast<std::uint8_t>(f[609]);
    s.allocator.duplicates = static_cast<std::uint8_t>(f[610]);
    s.allocator.cursor = static_cast<std::uint8_t>(f[611]);
    s.allocator.age_counter = static_cast<std::uint32_t>(f[612]);
    s.allocator.active_bits = static_cast<std::uint16_t>(f[613]);
    s.allocator.sequence_index = static_cast<std::uint8_t>(f[614]);
    s.allocator.pitch_table[0] = static_cast<std::uint16_t>(f[615]);
    s.allocator.pitch_table[1] = static_cast<std::uint16_t>(f[616]);
    s.allocator.pitch_table[2] = static_cast<std::uint16_t>(f[617]);
    s.allocator.pitch_table[3] = static_cast<std::uint16_t>(f[618]);
    s.allocator.pitch_table[4] = static_cast<std::uint16_t>(f[619]);
    s.allocator.pitch_table[5] = static_cast<std::uint16_t>(f[620]);
    s.allocator.pitch_table[6] = static_cast<std::uint16_t>(f[621]);
    s.allocator.pitch_table[7] = static_cast<std::uint16_t>(f[622]);
    s.allocator.pitch_table[8] = static_cast<std::uint16_t>(f[623]);
    s.allocator.pitch_table[9] = static_cast<std::uint16_t>(f[624]);
    s.allocator.pitch_table[10] = static_cast<std::uint16_t>(f[625]);
    s.allocator.pitch_table[11] = static_cast<std::uint16_t>(f[626]);
    s.allocator.pitch_table[12] = static_cast<std::uint16_t>(f[627]);
    s.allocator.pitch_table[13] = static_cast<std::uint16_t>(f[628]);
    s.allocator.pitch_table[14] = static_cast<std::uint16_t>(f[629]);
    s.allocator.pitch_table[15] = static_cast<std::uint16_t>(f[630]);
    s.allocator.pitch_table[16] = static_cast<std::uint16_t>(f[631]);
    s.allocator.pitch_table[17] = static_cast<std::uint16_t>(f[632]);
    s.allocator.pitch_table[18] = static_cast<std::uint16_t>(f[633]);
    s.allocator.pitch_table[19] = static_cast<std::uint16_t>(f[634]);
    s.allocator.pitch_table[20] = static_cast<std::uint16_t>(f[635]);
    s.allocator.pitch_table[21] = static_cast<std::uint16_t>(f[636]);
    s.allocator.pitch_table[22] = static_cast<std::uint16_t>(f[637]);
    s.allocator.pitch_table[23] = static_cast<std::uint16_t>(f[638]);
    s.allocator.pitch_table[24] = static_cast<std::uint16_t>(f[639]);
    s.allocator.pitch_table[25] = static_cast<std::uint16_t>(f[640]);
    s.allocator.pitch_table[26] = static_cast<std::uint16_t>(f[641]);
    s.allocator.pitch_table[27] = static_cast<std::uint16_t>(f[642]);
    s.allocator.pitch_table[28] = static_cast<std::uint16_t>(f[643]);
    s.allocator.pitch_table[29] = static_cast<std::uint16_t>(f[644]);
    s.allocator.pitch_table[30] = static_cast<std::uint16_t>(f[645]);
    s.allocator.pitch_table[31] = static_cast<std::uint16_t>(f[646]);
    s.allocator.pitch_table[32] = static_cast<std::uint16_t>(f[647]);
    s.allocator.pitch_table[33] = static_cast<std::uint16_t>(f[648]);
    s.allocator.pitch_table[34] = static_cast<std::uint16_t>(f[649]);
    s.allocator.pitch_table[35] = static_cast<std::uint16_t>(f[650]);
    s.allocator.pitch_table[36] = static_cast<std::uint16_t>(f[651]);
    s.allocator.pitch_table[37] = static_cast<std::uint16_t>(f[652]);
    s.allocator.pitch_table[38] = static_cast<std::uint16_t>(f[653]);
    s.allocator.pitch_table[39] = static_cast<std::uint16_t>(f[654]);
    s.allocator.pitch_table[40] = static_cast<std::uint16_t>(f[655]);
    s.allocator.pitch_table[41] = static_cast<std::uint16_t>(f[656]);
    s.allocator.pitch_table[42] = static_cast<std::uint16_t>(f[657]);
    s.allocator.pitch_table[43] = static_cast<std::uint16_t>(f[658]);
    s.allocator.pitch_table[44] = static_cast<std::uint16_t>(f[659]);
    s.allocator.pitch_table[45] = static_cast<std::uint16_t>(f[660]);
    s.allocator.pitch_table[46] = static_cast<std::uint16_t>(f[661]);
    s.allocator.pitch_table[47] = static_cast<std::uint16_t>(f[662]);
    s.allocator.pitch_table[48] = static_cast<std::uint16_t>(f[663]);
    s.allocator.pitch_table[49] = static_cast<std::uint16_t>(f[664]);
    s.allocator.pitch_table[50] = static_cast<std::uint16_t>(f[665]);
    s.allocator.pitch_table[51] = static_cast<std::uint16_t>(f[666]);
    s.allocator.pitch_table[52] = static_cast<std::uint16_t>(f[667]);
    s.allocator.pitch_table[53] = static_cast<std::uint16_t>(f[668]);
    s.allocator.pitch_table[54] = static_cast<std::uint16_t>(f[669]);
    s.allocator.pitch_table[55] = static_cast<std::uint16_t>(f[670]);
    s.allocator.pitch_table[56] = static_cast<std::uint16_t>(f[671]);
    s.allocator.pitch_table[57] = static_cast<std::uint16_t>(f[672]);
    s.allocator.pitch_table[58] = static_cast<std::uint16_t>(f[673]);
    s.allocator.pitch_table[59] = static_cast<std::uint16_t>(f[674]);
    s.allocator.pitch_table[60] = static_cast<std::uint16_t>(f[675]);
    s.allocator.pitch_table[61] = static_cast<std::uint16_t>(f[676]);
    s.allocator.pitch_table[62] = static_cast<std::uint16_t>(f[677]);
    s.allocator.pitch_table[63] = static_cast<std::uint16_t>(f[678]);
    s.allocator.pitch_table[64] = static_cast<std::uint16_t>(f[679]);
    s.allocator.pitch_table[65] = static_cast<std::uint16_t>(f[680]);
    s.allocator.pitch_table[66] = static_cast<std::uint16_t>(f[681]);
    s.allocator.pitch_table[67] = static_cast<std::uint16_t>(f[682]);
    s.allocator.pitch_table[68] = static_cast<std::uint16_t>(f[683]);
    s.allocator.pitch_table[69] = static_cast<std::uint16_t>(f[684]);
    s.allocator.pitch_table[70] = static_cast<std::uint16_t>(f[685]);
    s.allocator.pitch_table[71] = static_cast<std::uint16_t>(f[686]);
    s.allocator.pitch_table[72] = static_cast<std::uint16_t>(f[687]);
    s.allocator.pitch_table[73] = static_cast<std::uint16_t>(f[688]);
    s.allocator.pitch_table[74] = static_cast<std::uint16_t>(f[689]);
    s.allocator.pitch_table[75] = static_cast<std::uint16_t>(f[690]);
    s.allocator.pitch_table[76] = static_cast<std::uint16_t>(f[691]);
    s.allocator.pitch_table[77] = static_cast<std::uint16_t>(f[692]);
    s.allocator.pitch_table[78] = static_cast<std::uint16_t>(f[693]);
    s.allocator.pitch_table[79] = static_cast<std::uint16_t>(f[694]);
    s.allocator.pitch_table[80] = static_cast<std::uint16_t>(f[695]);
    s.allocator.pitch_table[81] = static_cast<std::uint16_t>(f[696]);
    s.allocator.pitch_table[82] = static_cast<std::uint16_t>(f[697]);
    s.allocator.pitch_table[83] = static_cast<std::uint16_t>(f[698]);
    s.allocator.pitch_table[84] = static_cast<std::uint16_t>(f[699]);
    s.allocator.pitch_table[85] = static_cast<std::uint16_t>(f[700]);
    s.allocator.pitch_table[86] = static_cast<std::uint16_t>(f[701]);
    s.allocator.pitch_table[87] = static_cast<std::uint16_t>(f[702]);
    s.allocator.pitch_table[88] = static_cast<std::uint16_t>(f[703]);
    s.allocator.pitch_table[89] = static_cast<std::uint16_t>(f[704]);
    s.allocator.pitch_table[90] = static_cast<std::uint16_t>(f[705]);
    s.allocator.pitch_table[91] = static_cast<std::uint16_t>(f[706]);
    s.allocator.pitch_table[92] = static_cast<std::uint16_t>(f[707]);
    s.allocator.pitch_table[93] = static_cast<std::uint16_t>(f[708]);
    s.allocator.pitch_table[94] = static_cast<std::uint16_t>(f[709]);
    s.allocator.pitch_table[95] = static_cast<std::uint16_t>(f[710]);
    s.allocator.pitch_table[96] = static_cast<std::uint16_t>(f[711]);
    s.allocator.pitch_table[97] = static_cast<std::uint16_t>(f[712]);
    s.allocator.pitch_table[98] = static_cast<std::uint16_t>(f[713]);
    s.allocator.pitch_table[99] = static_cast<std::uint16_t>(f[714]);
    s.allocator.pitch_table[100] = static_cast<std::uint16_t>(f[715]);
    s.allocator.pitch_table[101] = static_cast<std::uint16_t>(f[716]);
    s.allocator.pitch_table[102] = static_cast<std::uint16_t>(f[717]);
    s.allocator.pitch_table[103] = static_cast<std::uint16_t>(f[718]);
    s.allocator.pitch_table[104] = static_cast<std::uint16_t>(f[719]);
    s.allocator.pitch_table[105] = static_cast<std::uint16_t>(f[720]);
    s.allocator.pitch_table[106] = static_cast<std::uint16_t>(f[721]);
    s.allocator.pitch_table[107] = static_cast<std::uint16_t>(f[722]);
    s.allocator.pitch_table[108] = static_cast<std::uint16_t>(f[723]);
    s.allocator.pitch_table[109] = static_cast<std::uint16_t>(f[724]);
    s.allocator.pitch_table[110] = static_cast<std::uint16_t>(f[725]);
    s.allocator.pitch_table[111] = static_cast<std::uint16_t>(f[726]);
    s.allocator.pitch_table[112] = static_cast<std::uint16_t>(f[727]);
    s.allocator.pitch_table[113] = static_cast<std::uint16_t>(f[728]);
    s.allocator.pitch_table[114] = static_cast<std::uint16_t>(f[729]);
    s.allocator.pitch_table[115] = static_cast<std::uint16_t>(f[730]);
    s.allocator.pitch_table[116] = static_cast<std::uint16_t>(f[731]);
    s.allocator.pitch_table[117] = static_cast<std::uint16_t>(f[732]);
    s.allocator.pitch_table[118] = static_cast<std::uint16_t>(f[733]);
    s.allocator.pitch_table[119] = static_cast<std::uint16_t>(f[734]);
    s.allocator.pitch_table[120] = static_cast<std::uint16_t>(f[735]);
    s.allocator.pitch_table[121] = static_cast<std::uint16_t>(f[736]);
    s.allocator.pitch_table[122] = static_cast<std::uint16_t>(f[737]);
    s.allocator.pitch_table[123] = static_cast<std::uint16_t>(f[738]);
    s.allocator.pitch_table[124] = static_cast<std::uint16_t>(f[739]);
    s.allocator.pitch_table[125] = static_cast<std::uint16_t>(f[740]);
    s.allocator.pitch_table[126] = static_cast<std::uint16_t>(f[741]);
    s.allocator.pitch_table[127] = static_cast<std::uint16_t>(f[742]);
    s.last_input.bytes[0] = static_cast<std::uint8_t>(f[743]);
    s.last_input.bytes[1] = static_cast<std::uint8_t>(f[744]);
    s.last_input.bytes[2] = static_cast<std::uint8_t>(f[745]);
    s.last_input.bytes[3] = static_cast<std::uint8_t>(f[746]);
    s.last_input.bytes[4] = static_cast<std::uint8_t>(f[747]);
    s.last_input.bytes[5] = static_cast<std::uint8_t>(f[748]);
    s.last_input.bytes[6] = static_cast<std::uint8_t>(f[749]);
    s.last_input.bytes[7] = static_cast<std::uint8_t>(f[750]);
    s.last_input.bytes[8] = static_cast<std::uint8_t>(f[751]);
    s.last_input.bytes[9] = static_cast<std::uint8_t>(f[752]);
    s.last_input.bytes[10] = static_cast<std::uint8_t>(f[753]);
    s.last_input.bytes[11] = static_cast<std::uint8_t>(f[754]);
    s.last_input.bytes[12] = static_cast<std::uint8_t>(f[755]);
    s.last_input.bytes[13] = static_cast<std::uint8_t>(f[756]);
    s.last_input.bytes[14] = static_cast<std::uint8_t>(f[757]);
    s.last_input.bytes[15] = static_cast<std::uint8_t>(f[758]);
    s.last_input.bytes[16] = static_cast<std::uint8_t>(f[759]);
    s.last_input.bytes[17] = static_cast<std::uint8_t>(f[760]);
    s.last_input.bytes[18] = static_cast<std::uint8_t>(f[761]);
    s.last_input.bytes[19] = static_cast<std::uint8_t>(f[762]);
    s.last_input.bytes[20] = static_cast<std::uint8_t>(f[763]);
    s.last_input.bytes[21] = static_cast<std::uint8_t>(f[764]);
    s.last_input.bytes[22] = static_cast<std::uint8_t>(f[765]);
    s.last_input.bytes[23] = static_cast<std::uint8_t>(f[766]);
    s.last_input.bytes[24] = static_cast<std::uint8_t>(f[767]);
    s.last_input.bytes[25] = static_cast<std::uint8_t>(f[768]);
    s.last_input.bytes[26] = static_cast<std::uint8_t>(f[769]);
    s.last_input.bytes[27] = static_cast<std::uint8_t>(f[770]);
    s.last_input.bytes[28] = static_cast<std::uint8_t>(f[771]);
    s.last_input.bytes[29] = static_cast<std::uint8_t>(f[772]);
    s.last_input.bytes[30] = static_cast<std::uint8_t>(f[773]);
    s.last_input.bytes[31] = static_cast<std::uint8_t>(f[774]);
    s.last_input.bytes[32] = static_cast<std::uint8_t>(f[775]);
    s.last_input.bytes[33] = static_cast<std::uint8_t>(f[776]);
    s.last_input.bytes[34] = static_cast<std::uint8_t>(f[777]);
    s.last_input.bytes[35] = static_cast<std::uint8_t>(f[778]);
    s.last_input.bytes[36] = static_cast<std::uint8_t>(f[779]);
    s.last_input.bytes[37] = static_cast<std::uint8_t>(f[780]);
    s.last_input.bytes[38] = static_cast<std::uint8_t>(f[781]);
    s.last_input.bytes[39] = static_cast<std::uint8_t>(f[782]);
    s.last_input.bytes[40] = static_cast<std::uint8_t>(f[783]);
    s.last_input.bytes[41] = static_cast<std::uint8_t>(f[784]);
    s.last_input.bytes[42] = static_cast<std::uint8_t>(f[785]);
    s.last_input.bytes[43] = static_cast<std::uint8_t>(f[786]);
    s.last_input.bytes[44] = static_cast<std::uint8_t>(f[787]);
    s.last_input.bytes[45] = static_cast<std::uint8_t>(f[788]);
    s.last_input.bytes[46] = static_cast<std::uint8_t>(f[789]);
    s.last_input.bytes[47] = static_cast<std::uint8_t>(f[790]);
    s.last_input.bytes[48] = static_cast<std::uint8_t>(f[791]);
    s.last_input.bytes[49] = static_cast<std::uint8_t>(f[792]);
    s.last_input.bytes[50] = static_cast<std::uint8_t>(f[793]);
    s.last_input.bytes[51] = static_cast<std::uint8_t>(f[794]);
    s.last_input.bytes[52] = static_cast<std::uint8_t>(f[795]);
    s.last_input.bytes[53] = static_cast<std::uint8_t>(f[796]);
    s.last_input.bytes[54] = static_cast<std::uint8_t>(f[797]);
    s.last_input.bytes[55] = static_cast<std::uint8_t>(f[798]);
    s.last_input.bytes[56] = static_cast<std::uint8_t>(f[799]);
    s.held = static_cast<std::uint8_t>(f[800]);
    s.last_active_row = static_cast<std::uint8_t>(f[801]);
    s.keyboard_scale_mask = static_cast<std::uint16_t>(f[802]);
    s.global_scale_mask = static_cast<std::uint16_t>(f[803]);
    s.global_scale = static_cast<std::uint8_t>(f[804]);
    s.global_root = static_cast<std::uint8_t>(f[805]);
    s.status_pending = static_cast<std::uint8_t>(f[806]);
    s.suppress_status_clear = static_cast<std::uint8_t>(f[807]);
    s.on_count = static_cast<std::uint8_t>(f[808]);
    s.off_count = static_cast<std::uint8_t>(f[809]);
    n.events_.count = 0; n.published_valid_ = 0;
  }
  static unsigned direct(Engine&, unsigned, const std::uint8_t*, NotePacket&, bool&, Publications&) noexcept;
};
}
