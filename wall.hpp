#ifndef RAYCAST_WALL_HPP
#define RAYCAST_WALL_HPP

#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
#include "texture.hpp"

namespace pf 
{
  class Wall 
  {
    public:
      enum FillState 
      {
        Empty,
        Plane,
        Mirror,
        Filled
      } fillState;
  
      struct ColorData
      {
        glm::vec4 color = glm::vec4(1.0);
        Texture texture;
        float reflection = 0.0f;
      };

      float planeShift = 0.5f;
  
      ColorData up;
      ColorData down;
      ColorData left;
      ColorData right;
      // val = minStr + min(dis/maxDis, 1.0f) * (maxStr - minStr)
      glm::vec3 fogColor;
      float fogMinStrength = 0.0f;
      float fogMaxStrength = 0.0f;
      float fogMaxDistance = 0.0f;
  
      Wall(FillState howFilled = Empty) : fillState{howFilled} 
      {

      }
    private:
  };
}

#endif // RAYCAST_WALL_HPP
