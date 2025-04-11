#ifndef RAYCAST_WALL_HPP
#define RAYCAST_WALL_HPP

#include <vector>
#include <glm/ext/vector_float2.hpp>
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

        // Separate lines
        // Two coords to a line
        Segments,

        // Line strip
        // Each coord extends the current shape
        Strip,

        // Polygon
        // Like Strip, but the first and last coords are connected
        Shape,

        // Full block
        // The simplest and fastest to calculate
        Filled
      } fillState;
  
      struct ColorData
      {
        glm::vec4 color = glm::vec4(1.0);
        Texture texture;
        float reflection = 0.0f;
      };

      std::vector<ColorData> colorData;
      std::vector<glm::vec2> positionData;
  
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
