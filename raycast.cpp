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
      drawTextureRect(skyImg, glm::vec2(-1.0f), glm::vec2(1.0f), glm::vec2(std::atan2(-front.y, -front.x)/M_PI - 1.0f, 0.0f), glm::vec2(std::atan2(-front.y, -front.x)/M_PI - 0.5f, 1.0f), 1.0f);
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
      float step = 2.0f / res.y;
      float lastDis = (pos.z * 2.0f) / (1.0f+step - facing);

      glm::vec2 startDir = front - right;
      glm::vec2 endDir = front + right;
      for (float y = 1.0f; y > startFloor; y -= step) 
      {
        float dis = (pos.z * 2.0f) / (y - facing);

        /*scanLine[0].color = sfColor(playerTile->fogColor);
        scanLine[1].color = scanLine[0].color;
        scanLine[2].color = scanLine[0].color;
        scanLine[3].color = scanLine[0].color;*/

        if (drawTextureQuad)
        {
          drawTextureQuad(floorImg, glm::vec2(-1.0f, y), glm::vec2(1.0f, y), glm::vec2(1.0f, y-step), glm::vec2(-1.0f, y-step), glm::vec2(glm::vec2(pos) + startDir*lastDis)/floorScale, glm::vec2(glm::vec2(pos) + endDir*lastDis)/floorScale, glm::vec2(glm::vec2(pos) + endDir*dis)/floorScale, glm::vec2(glm::vec2(pos) + startDir*dis)/floorScale, floorColor.a);
        }
        lastDis = dis;
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
      float step = 2.0f / res.y;
      float lastDis = ((1.0f - pos.z) * 2.0f) / (facing + (1.0f-step));
      glm::vec2 startDir = front - right;
      glm::vec2 endDir = front + right;
      for (float y = -1.0f; y < startCeil; y += step) 
      {
        float dis = ((1.0f - pos.z) * 2.0f) / (facing - y);

        /*scanLine[0].color = sfColor(playerTile->fogColor);
        scanLine[1].color = scanLine[0].color;
        scanLine[2].color = scanLine[0].color;
        scanLine[3].color = scanLine[0].color;*/
    
        if (drawTextureQuad)
        {
          drawTextureQuad(ceilingImg, glm::vec2(-1.0f, y), glm::vec2(1.0f, y), glm::vec2(1.0f, y+step), glm::vec2(-1.0f, y+step), glm::vec2(glm::vec2(pos) + startDir*lastDis)/ceilingScale, glm::vec2(glm::vec2(pos) + endDir*lastDis)/ceilingScale, glm::vec2(glm::vec2(pos) + endDir*dis)/ceilingScale, glm::vec2(glm::vec2(pos) + startDir*dis)/ceilingScale, ceilingColor.a);
        }
        lastDis = dis;
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

        scanLine.tPos1 = glm::vec2(eyeCast.texCoord, 0.0f);
        scanLine.tPos2 = glm::vec2(eyeCast.texCoord, 1.0f);

        glm::vec2 relHitPos(eyeCast.hitPos.x - (int)eyeCast.hitPos.x, eyeCast.hitPos.y - (int)eyeCast.hitPos.y);
        float dis = glm::distance(glm::vec2(pos.x, pos.y), glm::vec2(eyeCast.hitPos.x, eyeCast.hitPos.y));

        Wall::ColorData* surfaceHit = &eyeCast.tileHit->colorData[eyeCast.surfaceHit];
        scanLine.color = surfaceHit->color;
        scanLine.color.a = glm::min(scanLine.color.a, 1.0f - surfaceHit->reflection);
        scanLine.tex = surfaceHit->texture;
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

    bool continueCasting = false;

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
      edgeDelta.x = (ray->tileHitPos.x + 1.0f - startPos.x) * tileDelta.x;
    }
    if (rayDir.y < 0) 
    {
      stepDir.y = -1;
      edgeDelta.y = (startPos.y - ray->tileHitPos.y) * tileDelta.y;
    } else 
    {
      stepDir.y = 1;
      edgeDelta.y = (ray->tileHitPos.y + 1.0f - startPos.y) * tileDelta.y;
    }

    uint32_t tile = startRenderDis;
    for (; !hitWall && tile < renderDistance; tile++) 
    {
      bool changedX = false;
      if (edgeDelta.x < edgeDelta.y) 
      {
        ray->hitPos = startPos + rayDir * edgeDelta.x;
        edgeDelta.x += tileDelta.x;
        ray->tileHitPos.x += stepDir.x;
        ray->verticalHit = false;
        changedX = true;
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
            ray->texCoord = ray->hitPos[!ray->verticalHit] - ray->tileHitPos[!ray->verticalHit];
            if (ray->tileHit->colorData.size() == 1)
            {
              ray->surfaceHit = 0;
            } else
            {
              if (ray->verticalHit)
              {
                if (rayDir.y > 0.0f)
                {
                  ray->surfaceHit = 0;
                } else
                {
                  ray->surfaceHit = 1;
                }
              } else
              {
                if (rayDir.x > 0.0f)
                {
                  ray->surfaceHit = 2;
                } else
                {
                  ray->surfaceHit = 3;
                }
              }
            }
            break;
          case Wall::Segments:
          case Wall::Strip:
          case Wall::Shape: {
            // orthogonal plane
            /*glm::vec2 wallTestPos(ray->tileHitPos.x, ray->tileHitPos.y + ray->tileHit->planeShift);
            glm::vec2 hitPoint = pf::lineToLineCollide(
                startPos, ray->hitPos + rayDir * 100.0f,
                glm::vec2(wallTestPos.x + 1, wallTestPos.y), wallTestPos);
            if (hitPoint == hitPoint) 
            {
              hitWall = true;
              ray->hitPos = hitPoint;
              edgeDelta.y += tileDelta.y * (rayDir.y > 0.0f ? ray->tileHit->planeShift : 1.0f - ray->tileHit->planeShift);
              ray->verticalHit = true;
            }*/

            glm::vec2 closeHitPoint;
            float closeTexCoord;
            float closeDis = INFINITY;
            for (uint32_t p = ray->tileHit->fillState != Wall::Shape ? 1 : 0; p < ray->tileHit->positionData.size(); p += ray->tileHit->fillState != Wall::Segments ? 1 : 2)
            {
              glm::vec2 pos1;
              glm::vec2 pos2;

              if (ray->tileHit->fillState != Wall::Shape)
              {
                pos1 = ray->tileHit->positionData[p-1];
                pos2 = ray->tileHit->positionData[p];
              } else
              {
                pos1 = ray->tileHit->positionData[p];
                pos2 = ray->tileHit->positionData[(p+1) % ray->tileHit->positionData.size()];
              }

              glm::vec2 hitPoint = pf::lineToLineCollide(
                  startPos, ray->hitPos + rayDir * 100.0f,
                  glm::vec2(ray->tileHitPos) + pos1, glm::vec2(ray->tileHitPos) + pos2);
              if (hitPoint == hitPoint)
              {
                float dis;
                if (ray->verticalHit)
                {
                  dis = tileDelta.y * (rayDir.y > 0.0f ? (hitPoint.y - ray->tileHitPos.y) : (1.0f - (hitPoint.y - ray->tileHitPos.y)));
                } else
                {
                  dis = tileDelta.x * (rayDir.x > 0.0f ? (hitPoint.x - ray->tileHitPos.x) : (1.0f - (hitPoint.x - ray->tileHitPos.x)));
                }

                float texCoord;
                if (glm::abs(pos2.x-pos1.x) > glm::abs(pos2.y-pos1.y))
                {
                  texCoord = hitPoint.x - ray->tileHitPos.x;
                } else
                {
                  texCoord = hitPoint.y - ray->tileHitPos.y;
                }

                if (dis < closeDis)
                {
                  closeDis = dis;
                  closeTexCoord = texCoord;
                  closeHitPoint = hitPoint;
                }
              }
            }

            if (closeDis != INFINITY) 
            {
              hitWall = true;

              if (ray->verticalHit)
              {
                edgeDelta.y += closeDis;
              } else
              {
                edgeDelta.x += closeDis;
              }

              ray->hitPos = closeHitPoint;

              ray->texCoord = closeTexCoord;
              
              //ray->verticalHit = true;
            }
            break;
          } /*case Wall::Mirror:
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
            break;*/
          case Wall::Empty:
            break;
        }
      }
    }

    if (hitWall && ray->tileHit->colorData[ray->surfaceHit].reflection > 0.0f)
    {
      continueCasting = true;
      rayDir[ray->verticalHit] = -rayDir[ray->verticalHit];
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
      continueCasting |= ray->tileHit->colorData[ray->surfaceHit].color.a < 1.0f;
        
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
        drawTextureRect(toDraw.front().tex, toDraw.front().pos1, toDraw.front().pos2, toDraw.front().tPos1, toDraw.front().tPos2, toDraw.front().color.a);
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
