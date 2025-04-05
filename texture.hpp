#ifndef RAYCAST_TEXTURE_HPP
#define RAYCAST_TEXTURE_HPP

#include <cstdint>

namespace pf
{
  struct Texture
  {
    uint32_t width = 0;
    uint32_t height = 0;
    uint8_t channels = 0;
    const uint8_t* data = nullptr;
  };
}

#endif // RAYCAST_TEXTURE_HPP
