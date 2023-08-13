#pragma once
#include "node.h"
#include <stdio.h>

// this header file is only used to simulate mass-spring system

class Spring
{
public:
    Node* Node1;
    Node* Node2;
    double RestLength;
    double HookPara;
    double DampPara;

    Spring(Node* node1, Node* node2, double k)
    {
        Node1 = node1;
        Node2 = node2;

        glm::vec<3, double> deltaPosition = node2->Position - node1->Position;
        RestLength = glm::length(deltaPosition);
        HookPara = k;
        DampPara = 5.0;
    }

    // Compute spring internal force
    void applyInternalForce(double timeStep) 
    {
        double currLength = glm::length(Node1->Position - Node2->Position);
        glm::vec<3, double> force1 = (Node2->Position - Node1->Position) / currLength;
        glm::vec<3, double> diffV1 = Node2->Velocity - Node1->Velocity;
        glm::vec<3, double> f1 = force1 * ((currLength - RestLength) * HookPara + glm::dot(diffV1, force1) * DampPara);
        Node1->addForce(f1);
        Node2->addForce(-f1);
    }
};
