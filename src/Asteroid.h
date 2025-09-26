#pragma once

#include "ofMain.h"
#include "Shape.h"
#include <vector>
#include <random>
#include "SmallAsteroid.h"

class Asteroid : public Shape {
public:
    // Constructor that generates the asteroid with random line segments
    Asteroid(glm::vec3 position, int numSides = 10, float minRadius = 40.0f, float maxRadius = 60.0f) {
        this->pos = position;
        this->numSides = numSides;
        this->minRadius = minRadius;
        this->maxRadius = maxRadius;
        velocity = glm::vec3(ofRandom(-1, 1), ofRandom(-1, 1), 0);
        acceleration = glm::vec3(0, 0, 0);
        rotationSpeed = ofRandom(-1.0f, 1.0f);
        rot = 0;
        collisionPoint = glm::vec3(0, 0, 0);
        generateLines();
    }

    void draw() override {
        if (!exploded) {
            ofPushMatrix();
            ofMultMatrix(getTransform());
            ofRotateDeg(rot);
            ofSetColor(255);
            for (size_t i = 0; i < lines.size(); ++i) {
                const auto& line = lines[i];
                ofDrawLine(line.first, line.second);
            }
            ofPopMatrix();
        }

        drawExplosionParticles();
    }

    void update() {
        if (!exploded) {
            glm::vec3 steering = glm::vec3(ofRandom(-0.1, 0.1), ofRandom(-0.1, 0.1), 0);
            velocity += steering;
            float fixedSpeed = 2.0f;
            velocity = glm::normalize(velocity) * fixedSpeed;
            pos += velocity;
            rot += rotationSpeed;

            if (pos.x > ofGetWidth()) pos.x = 0;
            else if (pos.x < 0) pos.x = ofGetWidth();
            if (pos.y > ofGetHeight()) pos.y = 0;
            else if (pos.y < 0) pos.y = ofGetHeight();
        }

        updateExplosionParticles();
    }

    void applyForce(glm::vec3 force) {
        acceleration += force;
    }

    glm::vec3 getPosition() const {
        return pos;
    }

    float getRadius() const {
        return maxRadius;
    }

    // Mark where player shot asteroid
    void addHitPosition(const glm::vec3& hitPos, std::vector<SmallAsteroid>& smallAsteroids) {
        if (!exploded) {
            triggerExplosion(hitPos);
            for (int i = 0; i < 3; i++) {
                SmallAsteroid newAsteroid(hitPos);
                smallAsteroids.push_back(newAsteroid);
            }
        }
    }

    // Check collision with emitter particle
    bool checkCollisionWithParticle(const glm::vec3& particlePos) {
        if (glm::distance(particlePos, pos) < maxRadius) {
            collisionPoint = particlePos;
            return true;
        }
        return false;
    }

    bool isExplosionFinished() const {
        return exploded && explosionParticles.empty();
    }

    void updateExplosion() {
        updateExplosionParticles();
    }

    void drawExplosion() {
        drawExplosionParticles();
    }

    bool isExploding() const {
        return exploded;
    }

    glm::vec3 getVelocity() const { return velocity; }
    void setVelocity(const glm::vec3& v) { velocity = v; }
    void setPosition(const glm::vec3& p) { pos = p; }

    bool hasBeenHit = false;

private:
    int numSides;
    float minRadius, maxRadius;
    std::vector<std::pair<glm::vec3, glm::vec3>> lines;
    glm::vec3 velocity, acceleration;
    float rotationSpeed;
    float rot;
    std::vector<glm::vec3> hitPositions;
    glm::vec3 collisionPoint;

    void generateLines() {
        float angleStep = TWO_PI / numSides;

        lines.clear();
        glm::vec3 lastPoint = glm::vec3(cos(0) * ofRandom(minRadius, maxRadius), sin(0) * ofRandom(minRadius, maxRadius), 0);

        for (int i = 1; i < numSides; ++i) {
            float angle = i * angleStep;
            float length = ofRandom(minRadius, maxRadius);
            glm::vec3 newPoint = glm::vec3(cos(angle) * length, sin(angle) * length, 0);
            lines.push_back(std::make_pair(lastPoint, newPoint));
            lastPoint = newPoint;
        }

        lines.push_back(std::make_pair(lastPoint, lines.front().first));
    }

    // Explosion Particle System
    struct ExplosionParticle {
        glm::vec3 pos;
        glm::vec3 vel;
        float lifespan;
        float age;
        float radius;
    };

    bool exploded = false;
    std::vector<ExplosionParticle> explosionParticles;

    void triggerExplosion(const glm::vec3& hitPos) {
        exploded = true;
        explosionParticles.clear();

        for (int i = 0; i < 100; i++) {
            ExplosionParticle p;
            p.pos = hitPos;
            p.vel = glm::vec3(ofRandom(-5, 5), ofRandom(-5, 5), 0);
            p.lifespan = ofRandom(1.0f, 2.0f);
            p.age = 0;
            p.radius = ofRandom(1.0f, 3.0f);
            explosionParticles.push_back(p);
        }
    }

    void updateExplosionParticles() {
        for (auto& p : explosionParticles) {
            p.pos += p.vel;
            p.vel *= 0.95f;
            p.age += 1.0f / 60.0f;
        }

        explosionParticles.erase(
            std::remove_if(
                explosionParticles.begin(), explosionParticles.end(),
                [](const ExplosionParticle& p) { return p.age > p.lifespan; }),
            explosionParticles.end()
        );
    }

    void drawExplosionParticles() {
        int segments = 8;  // Number of lines per explosion particle
        for (auto& p : explosionParticles) {
            float alpha = ofMap(p.age, 0, p.lifespan, 255, 0);
            ofSetColor(255, alpha);

            float angleStep = TWO_PI / segments;
            std::vector<glm::vec3> circlePoints;

            for (int i = 0; i < segments; ++i) {
                float angle = i * angleStep;
                float x = p.pos.x + cos(angle) * p.radius;
                float y = p.pos.y + sin(angle) * p.radius;
                circlePoints.emplace_back(x, y, 0);
            }

            // Connect the points into a loop
            for (int i = 0; i < segments; ++i) {
                glm::vec3 a = circlePoints[i];
                glm::vec3 b = circlePoints[(i + 1) % segments];
                ofDrawLine(a, b);
            }
        }
    }
};
