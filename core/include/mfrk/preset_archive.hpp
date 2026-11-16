#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <vector>
namespace mfrk::preset_archive {
struct Entry {std::string_view key,value;};
struct View {
 std::span<const std::byte> raw;
 std::string_view name,bank,author,unknown_header_text,description,creator_version;
 std::int64_t product=0,timestamp=0;
 std::array<std::int64_t,6> header_slots{};
 std::string_view unassigned_header,unassigned_state,trailing;
 std::vector<Entry> properties,parameters,blobs;
};
// Bounded observed Boost-text archive profile. All views refer to input bytes,
// which must remain alive and immutable. Blob values may contain arbitrary bytes.
// This decodes storage only: descriptor construction, migration, application,
// attachment decoding and audio behavior remain separate. Throws invalid_argument
// on unsupported or malformed inputs. Input is limited to 64 MiB.
View parse(std::span<const std::byte>,bool allow_attachment_archive=false);
// Owns immutable archive bytes for background loading and subsequent staging.
// Moving preserves all views in the destination and empties the moved-from view.
class Owned {
 std::vector<std::byte> storage_;
 View view_;
public:
 explicit Owned(std::span<const std::byte>,bool allow_attachment_archive=false);
 explicit Owned(std::vector<std::byte>,bool allow_attachment_archive=false);
 Owned(const Owned&)=delete;Owned& operator=(const Owned&)=delete;
 Owned(Owned&&) noexcept;
 Owned& operator=(Owned&&) noexcept;
 const View& view() const noexcept {return view_;}
};
}
