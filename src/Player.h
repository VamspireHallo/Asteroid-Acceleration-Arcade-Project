#pragma once

#include "ofMain.h"
#include "Shape.h"

class Player : public Shape {
public:
    Player() {
        pos = glm::vec3(ofGetWidth() / 2, ofGetHeight() / 2, 0);
        velocity = glm::vec3(0, 0, 0);
        acceleration = glm::vec3(0, 0, 0);
        rot = 0.0f;
        radius = 15.0f;
        invulnerable = true;  // Player starts as invulnerable
        fadeSpeed = 2.0f;  // Speed of the fading effect
        alpha = 0;  // Start invisible
        fadeDirection = 1;
    }

    void update() {
        float currentTime = ofGetElapsedTimef();

        if (isExploding) {
            updateExplosionParticles();

            if (currentTime - explosionStartTime > explosionDuration) {
                isExploding = false;
                invulnerable = true;
                invulnerableStartTime = currentTime;
                reset();
            }
            return;
        }

        // Check if invulnerable conditions are done
        if (invulnerable && (currentTime - invulnerableStartTime > invulnerableDuration) && glm::length(velocity) > 0.1f) {
            invulnerable = false;
        }

        // invulnerable semi-transparency 
        if (invulnerable) {
            alpha += fadeDirection * fadeSpeed;
            if (alpha <= 0 || alpha >= 255) {
                fadeDirection *= -1;  
            }
        }

        velocity += acceleration;   // Applying acceleration to player velocity
        pos += velocity;            // Updating position based on velocity
        velocity *= 0.99f;             // Damping velocity value
        acceleration = glm::vec3(0, 0, 0);

        // Screen wrap-around
        if (pos.x > ofGetWidth()) pos.x = 0;
        else if (pos.x < 0) pos.x = ofGetWidth();
        if (pos.y > ofGetHeight()) pos.y = 0;
        else if (pos.y < 0) pos.y = ofGetHeight();
    }

    void draw() override {
        if (isExploding) {
            drawExplosionParticles();
            return;
        }

        ofPushMatrix();
        ofTranslate(pos.x, pos.y);
        ofRotateDeg(rot);

        // Check whether invulnerable or not
        if (invulnerable) {
            ofSetColor(255, 255, 255, alpha);
        }
        else {
            ofSetColor(0, 255, 0, 255);
        }

        ofDrawLine(-10, 10, 10, 0);
        ofDrawLine(10, 0, -10, -10);
        ofDrawLine(-10, -10, -10, 10);

        ofPopMatrix();
    }

    void applyForce(glm::vec3 force) { acceleration += force; }
    void rotate(float angle) { rot += angle; }

    void thrust(float power) {
        float radians = glm::radians(rot);
        glm::vec3 thrustDir = glm::vec3(cos(radians), sin(radians), 0);
        applyForce(thrustDir * power * 0.75f);
    }

    bool checkCollision(glm::vec3 otherPos, float otherRadius) {
        return glm::distance(pos, otherPos) < (radius + otherRadius);
    }

    float getRadius() const { return radius; }

    void playerHit() {
        if (invulnerable) return;
        triggerExplosion(pos);
    }

    void reset() {
        pos = glm::vec3(ofGetWidth() / 2, ofGetHeight() / 2, 0);
        velocity = glm::vec3(0, 0, 0);
        acceleration = glm::vec3(0, 0, 0);
        rot = 0.0f;
    }

    float radius;
    glm::vec3 velocity;
    glm::vec3 acceleration;

    bool isExploding = false;
    float explosionStartTime = 0;
    float explosionDuration = 2.0f;

    bool invulnerable = false;
    float invulnerableStartTime = 0;
    float invulnerableDuration = 1.0f;

    float alpha;
    float fadeSpeed;
    int fadeDirection;

private:
    // Explosion Particle System
    struct ExplosionParticle {
        glm::vec3 pos;
        glm::vec3 vel;
        float lifespan;
        float age;
        float radius;
    };

    std::vector<ExplosionParticle> explosionParticles;

    void triggerExplosion(const glm::vec3& explosionPos) {
        isExploding = true;
        explosionStartTime = ofGetElapsedTimef();
        explosionParticles.clear();

        for (int i = 0; i < 80; i++) {
            ExplosionParticle p;
            p.pos = explosionPos;
            p.vel = glm::vec3(ofRandom(-5, 5), ofRandom(-5, 5), 0);
            p.lifespan = ofRandom(1.0f, 2.0f);
            p.age = 0;
            p.radius = ofRandom(1.5f, 3.5f);
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
            std::remove_if(explosionParticles.begin(), explosionParticles.end(),
                [](const ExplosionParticle& p) { return p.age > p.lifespan; }),
            explosionParticles.end()
        );
    }

    void drawExplosionParticles() {
        int segments = 8;
        for (auto& p : explosionParticles) {
            float alpha = ofMap(p.age, 0, p.lifespan, 255, 0);
            ofSetColor(0, 255, 0, alpha);

            std::vector<glm::vec3> circlePoints;
            float angleStep = TWO_PI / segments;

            for (int i = 0; i < segments; ++i) {
                float angle = i * angleStep;
                float x = p.pos.x + cos(angle) * p.radius;
                float y = p.pos.y + sin(angle) * p.radius;
                circlePoints.emplace_back(x, y, 0);
            }

            for (int i = 0; i < segments; ++i) {
                glm::vec3 a = circlePoints[i];
                glm::vec3 b = circlePoints[(i + 1) % segments];
                ofDrawLine(a, b);
            }
        }
    }
};
