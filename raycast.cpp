#include "raycast.hpp"

#include <cmath>
#include <glm/geometric.hpp>
#include <glm/fwd.hpp>
#include <glm/trigonometric.hpp>
#include <glm/mat2x2.hpp>

#include "collision-lib/line.hpp"

namespace pf 
{
  RaycastCamera::RaycastCamera() 
  {
    /*if (sf::Shader::isAvailable()) 
    {
      if (!fogShader.loadFromMemory("\n\
          #version 120 \n\
          \n\
          uniform sampler2D texture; \n\
          uniform float fogLevel = 0.0; \n\
          \n\
          void main() \n\
          { \n\
            vec4 texel = texture2D(texture, gl_TexCoord[0].xy); \n\
            gl_FragColor = vec4(mix(texel.rgb, gl_Color.rgb, fogLevel), texel.a); \n\
          } \n\
        ", sf::Shader::Type::Fragment)) 
      {
        std::cout << "Shader failed to load, fog will be disabled.\n";
      }
    } else 
    {
      std::cout << "Shaders not available, fog will be disabled.\n";
    }*/

    floorColor = glm::vec4(1.0f);
    ceilingColor = glm::vec4(1.0f);
    skyColor = glm::vec4(1.0f);
  }

  // Note: This function will invalidate the current contents of the world
  void RaycastCamera::resizeWorld(glm::uvec2 newSize)
  {
    wallMapSize = newSize;
    wallMap.resize(newSize.x * newSize.y);
  }

  Wall& RaycastCamera::wall(glm::uvec2 wallPos)
  {
    return wallMap[wallPos.y*wallMapSize.x + wallPos.x];
  }

  const Wall& RaycastCamera::wall(glm::uvec2 wallPos) const
  {
    return wallMap[wallPos.y*wallMapSize.x + wallPos.x];
  }

  void RaycastCamera::sky(float startSky) 
  {
    if (skyImg.data && drawTextureRect)
    {
      drawTextureRect(skyImg, glm::vec2(-1.0f), glm::vec2(1.0f), glm::vec2(std::atan2(-front.y, -front.x)/M_PI - 1.0f, 0.0f), glm::vec2(std::atan2(-front.y, -front.x)/M_PI - 0.5f, 1.0f));
    }
  }

  void RaycastCamera::floorsAndCeilings(float startCeil, float startFloor) 
  {
    Wall* playerTile = &wall(glm::uvec2(pos));

    if (!floorImg.data) 
    {
      if (drawRect)
      {
        drawRect(floorColor, glm::vec2(-1.0f, startFloor), glm::vec2(1.0f, 1.0f));
      }
    } else 
    {
      float dis;
      float step = 2.0f / res.y;
      glm::vec2 startDir = front - right;
      glm::vec2 endDir = front + right;
      for (float y = 1.0f; y > startFloor; y -= step) 
      {
        dis = (pos.z * 2.0f) / (y/* - (facing + 0.5f) * 2.0f*/);

        /*scanLine[0].color = sfColor(playerTile->fogColor);
        scanLine[1].color = scanLine[0].color;
        scanLine[2].color = scanLine[0].color;
        scanLine[3].color = scanLine[0].color;*/

        if (drawTextureLine)
        {
          drawTextureLine(floorImg, glm::vec2(-1.0f, y), glm::vec2(1.0f, y), glm::vec2(glm::vec2(pos) + startDir*dis)/floorScale, glm::vec2(glm::vec2(pos) + endDir*dis)/floorScale);
        }
      }
    }

    if (!ceilingImg.data) 
    {
      if (drawRect)
      {
        drawRect(ceilingColor, glm::vec2(-1.0f, startCeil), glm::vec2(1.0f, 1.0f));
      }
    } else 
    {
      float dis;
      float step = 2.0f / res.y;
      glm::vec2 startDir = front - right;
      glm::vec2 endDir = front + right;
      for (float y = -1.0f; y < startCeil; y += step) 
      {
        dis = ((1.0f - pos.z) * 2.0f) / (/*(facing + 0.5f) * 2.0f - */-y);

        /*scanLine[0].color = sfColor(playerTile->fogColor);
        scanLine[1].color = scanLine[0].color;
        scanLine[2].color = scanLine[0].color;
        scanLine[3].color = scanLine[0].color;*/
    
        if (drawTextureLine)
        {
          drawTextureLine(ceilingImg, glm::vec2(-1.0f, y), glm::vec2(1.0f, y), glm::vec2(glm::vec2(pos) + startDir*dis)/ceilingScale, glm::vec2(glm::vec2(pos) + endDir*dis)/ceilingScale);
        }
      }
    }
  }

  void RaycastCamera::walls() 
  {
    float lineWidth = 2.0f/float(res.x);
    for (float ray = -1.0f; ray < 1.0f; ray += lineWidth) 
    {
      glm::vec2 rayDir = front + right * ray;

      std::vector<RayCastData> eyeCasts = castRay(glm::vec2(pos.x, pos.y), rayDir);

      for (RayCastData &eyeCast: eyeCasts)
      {
        if (eyeCast.tileHit == nullptr) 
        {
          continue;
        }

        DrawData scanLine;
        float centerY = (pos.z - 0.5f) * 2.0f / eyeCast.dis + facing;
        float halfSize = 1.0f/eyeCast.dis;

        scanLine.pos1 = glm::vec2(ray, centerY - halfSize);
        scanLine.pos2 = glm::vec2(ray+lineWidth, centerY + halfSize);

        glm::vec2 relHitPos(eyeCast.hitPos.x - (int)eyeCast.hitPos.x, eyeCast.hitPos.y - (int)eyeCast.hitPos.y);
        float dis = glm::distance(glm::vec2(pos.x, pos.y), glm::vec2(eyeCast.hitPos.x, eyeCast.hitPos.y));
        if (eyeCast.verticalHit) 
        {
          if (eyeCast.hitPos.y < pos.y) 
          {
            scanLine.color = eyeCast.tileHit->down.color;
            if (eyeCast.tileHit->down.texture.data) 
            {
              scanLine.tex = eyeCast.tileHit->down.texture;
            }
          } else 
          {
            scanLine.color = eyeCast.tileHit->up.color;
            if (eyeCast.tileHit->up.texture.data) 
            {
              scanLine.tex = eyeCast.tileHit->up.texture;
            }
          }

          scanLine.tPos1 = glm::vec2(relHitPos.x, 0.0f);
          scanLine.tPos2 = glm::vec2(relHitPos.x + 1.0f/scanLine.tex.width, 1.0f);
        } else 
        {
          if (eyeCast.hitPos.x < pos.x) 
          {
            scanLine.color = eyeCast.tileHit->right.color;
            if (eyeCast.tileHit->right.texture.data) 
            {
              scanLine.tex = eyeCast.tileHit->right.texture;
            }
          } else 
          {
            scanLine.color = eyeCast.tileHit->left.color;
            if (eyeCast.tileHit->left.texture.data) 
            {
              scanLine.tex = eyeCast.tileHit->left.texture;
            }
          }

          scanLine.tPos1 = glm::vec2(relHitPos.y, 0.0f);
          scanLine.tPos2 = glm::vec2(relHitPos.y + 1.0f/scanLine.tex.width, 1.0f);
        }
        scanLine.dis = eyeCast.dis;

        /*bool pVert = verticalHit;
        double pEndX = endX;
        double pEndY = endY;
        int pTileType = tileType;
        if (doDarkness) 
        {
          int tileEndX = tileHitX;
          int tileEndY = tileHitY;
          double pEndX = endX;
          double pEndY = endY;
          wallLight = sf::Color::Black;
          for (int l = 0; l < lights.size(); l++) 
          {
            if (doShadows)
            {
              castRay(lights[l].pos, pf::getAngle(pEndX, pEndY, lights[l].pos.x, lights[l].pos.y)); 
            } else 
            {
              dis = pf::distance(pEndX, pEndY, lights[l].pos.x, lights[l].pos.y); 
            }
            
            if (!doShadows || (floor(pEndX) == floor(endX) && floor(pEndY) == floor(endY))) 
            {
              wallLight.r = (wallLight.r+(lights[l].color.r/(dis*dis)*lights[l].intensity) > 255 ? 255 : wallLight.r+(lights[l].color.r/(dis*dis)*lights[l].intensity));
              wallLight.g = (wallLight.g+(lights[l].color.g/(dis*dis)*lights[l].intensity) > 255 ? 255 : wallLight.g+(lights[l].color.g/(dis*dis)*lights[l].intensity));
              wallLight.b = (wallLight.b+(lights[l].color.b/(dis*dis)*lights[l].intensity) > 255 ? 255 : wallLight.b+(lights[l].color.b/(dis*dis)*lights[l].intensity));
            }
            //std::cout << (lights[l].intensity*256) << '/' << dis << '\n';
            //wall.setFillColor(sf::Color(lightColor.r, lightColor.g, lightColor.b, 255-(doShadows ? (dis/lightIntensity) : 0)));
          }
        } else wallLight = sf::Color::White;
        if (pTileType == 0) 
        {
          wallLight = sf::Color::Transparent;
          rectSpr.setFillColor(wallLight);
        } else 
        {
          if (pVert) 
          {
            if (pEndY < pos.y) 
            {
              wall.setFillColor(wallLight * tile->down.color);
            } else 
            {
              wall.setFillColor(wallLight * tile->up.color);
            }
          } else 
          {
            if (pEndX < pos.x) 
            {
              wall.setFillColor(wallLight * tile->right.color);
            } else 
            {
              wall.setFillColor(wallLight * tile->left.color);
            }
          }
        }*/

        toDraw.push_back(scanLine);
        // lightIntensity += ((rand()%3)-1)*0.001;
      }
    }
  }

  std::vector<RayCastData> RaycastCamera::castRay(glm::vec2 startPos, glm::vec2 rayDir, float startDis, uint32_t startRenderDis) 
  {
    std::vector<RayCastData> returnValue(1);

    RayCastData *ray = &returnValue[0];

    ray->tileHitPos = glm::floor(startPos);

    glm::vec2 edgeDelta;
    glm::vec2 tileDelta = glm::abs(1.0f / rayDir);

    glm::i8vec2 stepDir;

    bool hitWall = false;

    // setup ray dirs
    if (rayDir.x < 0) 
    {
      stepDir.x = -1;
      edgeDelta.x = (startPos.x - ray->tileHitPos.x) * tileDelta.x;
    } else 
    {
      stepDir.x = 1;
      edgeDelta.x = (ray->tileHitPos.x + 1.0 - startPos.x) * tileDelta.x;
    }
    if (rayDir.y < 0) 
    {
      stepDir.y = -1;
      edgeDelta.y = (startPos.y - ray->tileHitPos.y) * tileDelta.y;
    } else 
    {
      stepDir.y = 1;
      edgeDelta.y = (ray->tileHitPos.y + 1.0 - startPos.y) * tileDelta.y;
    }

    uint32_t tile = startRenderDis;
    for (; !hitWall && tile < renderDistance; tile++) 
    {
      if (edgeDelta.x < edgeDelta.y) 
      {
        ray->hitPos = startPos + rayDir * edgeDelta.x;
        edgeDelta.x += tileDelta.x;
        ray->tileHitPos.x += stepDir.x;
        ray->verticalHit = false;
      } else 
      {
        ray->hitPos = startPos + rayDir * edgeDelta.y;
        edgeDelta.y += tileDelta.y;
        ray->tileHitPos.y += stepDir.y;
        ray->verticalHit = true;
      }

      if (ray->tileHitPos.x >= 0 && ray->tileHitPos.x < wallMapSize.x &&
          ray->tileHitPos.y >= 0 && ray->tileHitPos.y < wallMapSize.y) 
      {
        switch ((ray->tileHit = &wall(ray->tileHitPos))->fillState) 
        {
          case Wall::Filled:
            hitWall = true;
            break;
          case Wall::Plane: {
            glm::vec2 wallTestPos(ray->tileHitPos.x,
                                  ray->tileHitPos.y + ray->tileHit->planeShift);
            glm::vec2 hitPoint = pf::lineToLineCollide(
                startPos, ray->hitPos + rayDir * 100.0f,
                glm::vec2(wallTestPos.x + 1, wallTestPos.y), wallTestPos);
            if (hitPoint == hitPoint) 
            {
              hitWall = true;
              ray->hitPos = hitPoint;
              edgeDelta.y += tileDelta.y * (rayDir.y > 0.0f ? ray->tileHit->planeShift : 1 - ray->tileHit->planeShift);
              ray->verticalHit = true;
            }
            /*if (ray->verticalHit) 
            {
              if (glm::floor(ray->hitPos + rayDir*tileDelta.y*ray->tileHit->planeShift) == glm::floor(ray->hitPos)) 
              {
                hitWall = true; ray->hitPos += rayDir*tileDelta.y*ray->tileHit->planeShift; 
                edgeDelta.y += tileDelta.y*ray->tileHit->planeShift;
              } else if (glm::ceil(ray->hitPos + rayDir*tileDelta.y*ray->tileHit->planeShift) == glm::ceil(ray->hitPos)) 
              {
                hitWall = true;
                ray->hitPos += rayDir*tileDelta.y*ray->tileHit->planeShift; 
                edgeDelta.y += tileDelta.y*ray->tileHit->planeShift;
              }
            } else 
            {
              hitWall = true;
            }*/
            break;
          }
          case Wall::Mirror:
            if (ray->verticalHit) 
            {
              stepDir.y = -stepDir.y;
              ray->tileHitPos.y += stepDir.y;
              // including these lines messed up the texture coordinates in
              // mirrors
              // startPos.y = -startPos.y;
              // rayDir.y = -rayDir.y;
            } else 
            {
              stepDir.x = -stepDir.x;
              ray->tileHitPos.x += stepDir.x;
              startPos.x = -startPos.x;
              rayDir.x = -rayDir.x;
            }
            break;
          case Wall::Empty:
            break;
        }
      }

      if (!hitWall) 
        {
        if (edgeDelta.x < edgeDelta.y) 
        {
          // ray->hitPos = startPos + rayDir*edgeDelta.x;
          // edgeDelta.x += tileDelta.x;
          // ray->tileHitPos.x += stepDir.x;
          // ray->verticalHit = false;
        } else 
        {
          // ray->hitPos = startPos + rayDir*edgeDelta.y;
          // edgeDelta.y += tileDelta.y;
          // ray->tileHitPos.y += stepDir.y;
          // ray->verticalHit = true;
        }
      }
    }

    if (ray->verticalHit) 
    {
      ray->dis = (edgeDelta.y - tileDelta.y) + startDis;
      // returnValue.hitPos = startPos + rayDir*returnValue.dis;
    } else 
    {
      ray->dis = (edgeDelta.x - tileDelta.x) + startDis;
      // returnValue.hitPos = startPos + rayDir*returnValue.dis;
    }

    if (!hitWall) 
    {
      ray->tileHit = nullptr;
    } else 
    {
      bool continueCasting = false;
      if (ray->verticalHit) 
      {
        if (stepDir.y > 0) 
        {
          if (ray->tileHit->up.color.a < 1.0f) 
          {
            continueCasting = true;
          }
        } else 
        {
          if (ray->tileHit->down.color.a < 1.0f) 
          {
            continueCasting = true;
          }
        }
      } else 
      {
        if (stepDir.x > 0) 
        {
          if (ray->tileHit->left.color.a < 1.0f) 
          {
            continueCasting = true;
          }
        } else 
        {
          if (ray->tileHit->right.color.a < 1.0f) 
          {
            continueCasting = true;
          }
        }
      }

      if (continueCasting) 
      {
        std::vector<RayCastData> childCast = castRay(ray->hitPos + rayDir * 0.01f, rayDir, ray->dis, tile);
        returnValue.insert(returnValue.end(), childCast.begin(), childCast.end());
      }
    }

    return returnValue;
  }

  void RaycastCamera::sprite(const Texture& spriteTex, const glm::vec3 &spritePos, glm::vec2 spriteSize, glm::vec2 spriteOrigin) 
  {
    glm::vec2 tempFront = glm::normalize(front);
    glm::vec2 tempRight = glm::normalize(right);
    glm::mat2 inverseCameraProjection = glm::inverse(glm::mat2(tempRight.x, tempFront.x, tempRight.y, tempFront.y));

    glm::vec2 transformedPos = inverseCameraProjection * glm::vec2(spritePos - pos);
    if (transformedPos.y <= 0.0f) 
    {
      return;
    }
    transformedPos.x *= glm::length(front) / glm::length(right);
    //transformedPos.y;

    //glm::vec2 projectedPos(screenSize.x * 0.5f * (transformedPos.x / transformedPos.y + 1.0f), screenSize.y * 0.5f * ((pos.z - spritePos.z) / transformedPos.y + 1.0f) + facing * screenSize.y);
    glm::vec2 projectedPos(transformedPos.x / transformedPos.y, ((pos.z - spritePos.z) / transformedPos.y) + facing);

    spriteSize = glm::abs(spriteSize / transformedPos.y);

    DrawData sprite;
    sprite.tex = spriteTex;
    sprite.pos1 = projectedPos - spriteSize*spriteOrigin;
    sprite.pos2 = projectedPos + spriteSize*(1.0f-spriteOrigin);
    //sprite.setFillColor(sf::Color(glm::min(color.r / transformedPos.y, 255.0f), glm::min(color.g / transformedPos.y, 255.0f), glm::min(color.b / transformedPos.y, 255.0f)));
    sprite.dis = transformedPos.y;
    toDraw.push_back(sprite);
  }

  void RaycastCamera::rotate(float ang) 
  {
    glm::mat2 rotation(glm::cos(ang), glm::sin(ang), -glm::sin(ang), glm::cos(ang));

    front = rotation * front;
    right = rotation * right;
  }

  void RaycastCamera::update() 
  {
    walls();

    toDraw.sort([](const DrawData& a, const DrawData& b) 
    {
      return a.dis > b.dis;
    });

    float top = toDraw.front().pos1.y;
    sky(top);

    floorsAndCeilings(top, toDraw.front().pos2.y);

    Wall *playerTile = &wall(glm::uvec2(pos));
    while (!toDraw.empty()) 
    {
      /*fogShader.setUniform("fogLevel", calculateFogStrength(playerTile, toDraw.front().getScale().x));

      fogShader.setUniform("texture", *toDraw.front().getTexture());*/

      if (drawTextureRect && toDraw.front().tex.data)
      {
        drawTextureRect(toDraw.front().tex, toDraw.front().pos1, toDraw.front().pos2, toDraw.front().tPos1, toDraw.front().tPos2);
      } else if (drawRect)
      {
        drawRect(toDraw.front().color, toDraw.front().pos1, toDraw.front().pos2);
      }
      toDraw.pop_front();
    }
  }

  // private members
  float RaycastCamera::calculateFogStrength(Wall *tile, float dis) 
  {
    if (tile->fogMaxDistance > 0.0f || tile->fogMaxStrength > 0.0f || tile->fogMinStrength > 0.0f) 
    {
      return tile->fogMinStrength + glm::min(dis / tile->fogMaxDistance, 1.0f) * (tile->fogMaxStrength - tile->fogMinStrength);
    } else 
    {
      return 0.0f;
    }
  }
}
