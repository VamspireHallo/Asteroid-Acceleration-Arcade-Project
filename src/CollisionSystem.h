#pragma once

#include "Asteroid.h"
#include "SmallAsteroid.h"
#include "Player.h"
#include "Emitter.h"
#include <vector>
#include <algorithm>

class CollisionSystem {
public:
    // Collision detection between bullet and large asteroid
    bool BulletLargeAsteroidCollision(std::vector<Emitter>& bullets, std::vector<Asteroid>& asteroids, std::vector<SmallAsteroid>& smallAsteroids) {
        bool asteroidDestroyed = false;

        for (int i = asteroids.size() - 1; i >= 0; --i) {
            Asteroid& asteroid = asteroids[i];

            if (asteroid.hasBeenHit) continue;  // Skip if already hit

            for (int j = bullets.size() - 1; j >= 0; --j) {
                // Check distance directly in the loop
                if (glm::distance(bullets[j].pos, asteroid.getPosition()) < asteroid.getRadius()) {
                    glm::vec3 hitPosition = bullets[j].pos;
                    asteroid.addHitPosition(hitPosition, smallAsteroids);
                    asteroid.hasBeenHit = true;  // Mark as hit
                    bullets.erase(bullets.begin() + j);  // Remove the bullet
                    asteroidDestroyed = true;
                    break;  // Stop checking other bullets
                }
            }
        }
        return asteroidDestroyed;
    }

    // Collision detection between bullet and small asteroid
    bool BulletSmallAsteroidCollision(std::vector<Emitter>& bullets, std::vector<SmallAsteroid>& smallAsteroids) {
        bool asteroidDestroyed = false;

        for (int i = smallAsteroids.size() - 1; i >= 0; --i) {
            SmallAsteroid& smallAsteroid = smallAsteroids[i];

            if (smallAsteroid.hasBeenHit) continue;  // Skip if already hit

            for (int j = bullets.size() - 1; j >= 0; --j) {
                // Check distance directly in the loop
                if (glm::distance(bullets[j].pos, smallAsteroid.getPosition()) < smallAsteroid.getRadius()) {
                    glm::vec3 hitPosition = bullets[j].pos;
                    smallAsteroid.asteroidDestoryed(hitPosition);
                    smallAsteroid.hasBeenHit = true;  // Mark as hit
                    bullets.erase(bullets.begin() + j);  // Remove the bullet
                    asteroidDestroyed = true;
                    break;  // Stop checking other bullets
                }
            }
        }
        return asteroidDestroyed;
    }

    // Handle asteroid-to-asteroid collisions (large vs large)
    static void LargeAsteroidCollision(std::vector<Asteroid>& asteroids) {
        for (int i = 0; i < asteroids.size(); ++i) {
            for (int j = i + 1; j < asteroids.size(); ++j) {
                handleCollision(asteroids[i], asteroids[j]);
            }
        }
    }

    // Handle asteroid-to-small asteroid collisions (large vs small)
    static void LargeSmallAsteroidCollision(std::vector<Asteroid>& asteroids, std::vector<SmallAsteroid>& smallAsteroids) {
        for (int i = 0; i < asteroids.size(); ++i) {
            for (int j = 0; j < smallAsteroids.size(); ++j) {
                handleCollision(asteroids[i], smallAsteroids[j]);
            }
        }
    }

    bool CollisionSystem::PlayerLargeAsteroidCollision(Player& player, std::vector<Asteroid>& asteroids) {
        for (Asteroid& asteroid : asteroids) {
            if (!player.isExploding && !asteroid.isExploding() && !player.invulnerable && player.checkCollision(asteroid.getPosition(), asteroid.getRadius())) {
                player.playerHit();
                return true;    // Player has been hit
            }
        }
        return false;           // Player has not been hit
    }

    bool CollisionSystem::PlayerSmallAsteroidCollision(Player& player, std::vector<SmallAsteroid>& smallAsteroids) {
        for (SmallAsteroid& smallAsteroid : smallAsteroids) {
            if (!player.isExploding && !smallAsteroid.isExploding() && !player.invulnerable && player.checkCollision(smallAsteroid.getPosition(), smallAsteroid.getRadius())) {
                player.playerHit();
                return true;    // Player has been hit
            }
        }
        return false;           // Player has not been hit
    }

private:
    // Collision detection handling between two asteroids
    template <typename T1, typename T2>
    static void handleCollision(T1& obj1, T2& obj2) {
        if (!obj1.isExploding() && !obj2.isExploding()) {
            glm::vec3 dir = obj1.getPosition() - obj2.getPosition();
            float dist = glm::length(dir);
            float minDist = (obj1.getRadius() * 0.9f) + (obj2.getRadius() * 0.9f);

            if (dist < minDist) {
                // Normalize direction
                glm::vec3 normal = glm::normalize(dir);

                // Reflect velocities
                glm::vec3 v1 = obj1.getVelocity();
                glm::vec3 v2 = obj2.getVelocity();

                glm::vec3 newV1 = v1 - 2 * glm::dot(v1 - v2, normal) * normal;
                glm::vec3 newV2 = v2 - 2 * glm::dot(v2 - v1, -normal) * -normal;

                obj1.setVelocity(newV1);
                obj2.setVelocity(newV2);

                // Push apart slightly to prevent sticking
                float overlap = minDist - dist;
                glm::vec3 separation = normal * (overlap / 2.0f);
                obj1.setPosition(obj1.getPosition() + separation);
                obj2.setPosition(obj2.getPosition() - separation);
            }
        }
    }
};
