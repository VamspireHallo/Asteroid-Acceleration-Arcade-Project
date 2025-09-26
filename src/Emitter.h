#pragma once

#include "ofMain.h"
#include "Shape.h"

class Emitter : public Shape {
public:
    Emitter(glm::vec3 startPos, glm::vec3 startVelocity, float startRotation) {
        pos = startPos;
        velocity = startVelocity;
        rot = startRotation;
    }

    void update() {
        pos += velocity;
    }

    // Drawing the bullets as a small lines
    void draw() override {
        ofSetColor(0, 255, 0);
        float lineLength = 4.0f;

        ofPushMatrix();
        ofTranslate(pos.x, pos.y);
        ofRotateDeg(rot);

        ofDrawLine(-lineLength / 2, 0, lineLength / 2, 0);

        ofPopMatrix();
    }

    glm::vec3 velocity;
};



class ParticleSystem {
public:
    void update() {
        for (auto& particle : particles) {
            particle.update();
        }

        // Remove particles that are out of bounds
        particles.erase(std::remove_if(particles.begin(), particles.end(), [](Emitter& b) {
            return b.pos.x < 0 || b.pos.x > ofGetWidth() || b.pos.y < 0 || b.pos.y > ofGetHeight();
        }), particles.end());
    }

    void draw() {
        for (auto& particle : particles) {
            particle.draw();
        }
    }

    void emit(glm::vec3 position, float angle, float speed, glm::vec3 playerVelocity) {
        // Direction of bullet
        float radians = glm::radians(angle);
        glm::vec3 direction = glm::vec3(cos(radians), sin(radians), 0); 

        // Velocity of bullet
        glm::vec3 velocity = playerVelocity + (direction * speed);

        // Position of bullet
        glm::vec3 tipOffset = direction * 10;
        glm::vec3 particlestartPos = position + tipOffset;

        particles.emplace_back(particlestartPos, velocity, angle);
    }

    std::vector<Emitter> particles;
};
