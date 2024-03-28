#pragma once

#include <cstddef>
#include <cstdint>

static constexpr uint32_t DTB_MAGIC = 0xd00dfeed;
static constexpr uint32_t DTB_BEGIN_NODE = 0x01;
static constexpr uint32_t DTB_END_NODE = 0x02;
static constexpr uint32_t DTB_PROP = 0x03;
static constexpr uint32_t DTB_NOP = 0x04;
static constexpr uint32_t DTB_END = 0x09;
