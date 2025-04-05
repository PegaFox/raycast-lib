#ifndef RAYCAST_LIGHT_HPP
#define RAYCAST_LIGHT_HPP

#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float4.hpp>

namespace pf 
{
  class Light
  {
    public:
      glm::vec4 color;
      glm::vec2 pos;
      float intensity;
      Light(const glm::vec4& lightColor, glm::vec2 lightPos, float lightIntensity) : 
      color{lightColor}, pos{lightPos}, intensity{lightIntensity} 
      {
      
      }
  };
}

#endif // RAYCAST_LIGHT_HPP
