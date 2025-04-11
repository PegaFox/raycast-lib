#ifndef RAYCAST_RENDERER
#define RAYCAST_RENDERER

#include <vector>
#include <list>

#include <glm/ext/vector_int2.hpp>
#include <glm/ext/vector_uint2.hpp>

#include "wall.hpp"
#include "texture.hpp"
#include "light.hpp"

namespace pf 
{
  struct RayCastData
  {
    Wall *tileHit = nullptr;
    glm::vec2 hitPos;
    uint32_t surfaceHit;
    float texCoord;
    float dis;
    bool verticalHit;
    glm::ivec2 tileHitPos;
  };

  class RaycastCamera 
  {
    public:
      void (*drawRect)(const glm::vec4& color, glm::vec2 pos1, glm::vec2 pos2) = nullptr;
      void (*drawTextureRect)(const Texture& tex, glm::vec2 pos1, glm::vec2 pos2, glm::vec2 tPos1, glm::vec2 tPos2, float alpha) = nullptr;
      void (*drawTextureQuad)(const Texture& tex, glm::vec2 pos1, glm::vec2 pos2, glm::vec2 pos3, glm::vec2 pos4, glm::vec2 tPos1, glm::vec2 tPos2, glm::vec2 tPos3, glm::vec2 tPos4, float alpha) = nullptr;

      glm::vec3 pos;

      glm::vec2 front = glm::vec2(0.0f, -1.0f);
      glm::vec2 right = glm::vec2(1.0f, 0.0f);

      float facing = 0.0f;
      glm::uvec2 res;
      uint32_t renderDistance = -1;
      bool doShadows = 1;

      std::vector<Light> lights;
      Texture floorImg;
      glm::vec4 floorColor;
      float floorScale = 1.0f;
      Texture skyImg;
      glm::vec4 skyColor;
      Texture ceilingImg;
      glm::vec4 ceilingColor;
      float ceilingScale = 1.0f;

      RaycastCamera();

      // Note: This function will invalidate the current contents of the world
      void resizeWorld(glm::uvec2 newSize);

      Wall& wall(glm::uvec2 wallPos);

      const Wall& wall(glm::uvec2 wallPos) const;

      void sky(float startSky);

      void floorsAndCeilings(float startCeil, float startFloor);

      void walls();

      std::vector<RayCastData> castRay(glm::vec2 startPos, glm::vec2 rayDir, float startDis = 0.0f, uint32_t startRenderDis = 0);

      void sprite(const Texture& spriteTex, const glm::vec3 &spritePos, glm::vec2 spriteSize, glm::vec2 spriteOrigin = glm::vec2(0.5f));

      void rotate(float ang);

      void update();

    private:
      float calculateFogStrength(Wall *tile, float dis);

      struct DrawData
      {
        float dis = 0.0f;

        Texture tex;
        glm::vec4 color = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);
        
        glm::vec2 pos1 = glm::vec2(-0.1f, -0.1f);
        glm::vec2 pos2 = glm::vec2(0.1f, 0.1f);

        glm::vec2 tPos1 = glm::vec2(0.0f);
        glm::vec2 tPos2 = glm::vec2(1.0f);
      };

      std::list<DrawData> toDraw;

      glm::uvec2 wallMapSize = glm::uvec2(0.0f);
      std::vector<Wall> wallMap;
  };
}
#endif
