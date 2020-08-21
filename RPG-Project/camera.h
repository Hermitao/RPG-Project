#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.0f;
const float SENSITIVITY = 0.1f;
const float FOV = 45.0f;

// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
    // camera Attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    // euler Angles
    float Yaw;
    float Pitch;
    // camera options
    float MovementSpeed;
    float VelocityMultiplier{ 1.0f };
    float MouseSensitivity;
    float Fov;

    GLboolean SmoothMovement{ false };
    float smoothVelocityForward{ 0.0f };
    float smoothVelocityRight{ 0.0f };
    float smoothVelocityUp{ 0.0f };
    float smoothYaw{ 0.0f };
    float smoothPitch{ 0.0f };

    float smoothXOffset{ 0.0f };
    float smoothYOffset{ 0.0f };

    // constructor with vectors
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Fov(FOV)
    {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }
    // constructor with scalar values
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Fov(FOV)
    {
        Position = glm::vec3(posX, posY, posZ);
        WorldUp = glm::vec3(upX, upY, upZ);
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    // returns the view matrix calculated using Euler Angles and the LookAt Matrix
    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(Position, Position + Front, Up);
    }

    void ResetMovement()
    {
        smoothVelocityForward = 0.0f;
        smoothVelocityRight = 0.0f;
        smoothVelocityUp = 0.0f;
        smoothXOffset = 0.0f;
        smoothYOffset = 0.0f;
    }

    // processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
    void ProcessKeyboard(Camera_Movement direction, float deltaTime)
    {
        float smooth{ 0.3f };

        float velocity{ MovementSpeed * deltaTime * VelocityMultiplier };
        if (direction == FORWARD)
        {
            smoothVelocityForward += smooth * deltaTime * VelocityMultiplier;
            if (!SmoothMovement)
            {
                Position += Front * velocity;
            }
        }
        if (direction == BACKWARD)
        {
            smoothVelocityForward -= smooth * deltaTime * VelocityMultiplier;
            if (!SmoothMovement)
            {
                Position -= Front * velocity;
            }
        }
        if (direction == LEFT)
        {
            smoothVelocityRight -= smooth * deltaTime * VelocityMultiplier;
            if (!SmoothMovement)
            {
                Position -= Right * velocity;
            }
        }
        if (direction == RIGHT)
        {
            smoothVelocityRight += smooth * deltaTime * VelocityMultiplier;
            if (!SmoothMovement)
            {
                Position += Right * velocity;
            }
        }
        if (direction == UP)
        {
            smoothVelocityUp += smooth * deltaTime * VelocityMultiplier;
            if (!SmoothMovement)
            {
                Position += Up * velocity;
            }
        }
        if (direction == DOWN)
        {
            smoothVelocityUp -= smooth * deltaTime * VelocityMultiplier;
            if (!SmoothMovement)
            {
                Position -= Up * velocity;
            }
        }
    }

    /// <summary>
    /// Proccesses smooth translation movement, should be called in the game loop.
    /// </summary>
    /// <param name="deltaTime"></param>
    void ProccessSmoothMovement(float deltaTime)
    {
        if (SmoothMovement)
        {
            Position += Front * smoothVelocityForward * deltaTime;
            Position += Right * smoothVelocityRight * deltaTime;
            Position += Up * smoothVelocityUp * deltaTime;
        }
    }

    // processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
    {
        float smooth{ 0.1f };

        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        if (!SmoothMovement)
        {
            Yaw += xoffset;
            Pitch += yoffset;
        }
        else
        {
            smoothXOffset += xoffset * smooth;
            smoothYOffset += yoffset * smooth;
        }

        // make sure that when pitch is out of bounds, screen doesn't get flipped
        if (constrainPitch)
        {
            if (Pitch > 89.0f)
                Pitch = 89.0f;
            if (Pitch < -89.0f)
                Pitch = -89.0f;
        }

        // update Front, Right and Up Vectors using the updated Euler angles
        updateCameraVectors();
    }

	/// <summary>
    /// Proccesses smooth rotation movement, should be called in the game loop.
    /// </summary>
    /// <param name="deltaTime"></param>
    void ProccessSmoothMouseMovement(float deltaTime, GLboolean constrainPitch = true)
    {
        if (SmoothMovement)
        {
            Yaw += smoothXOffset * deltaTime;
            Pitch += smoothYOffset * deltaTime;

			if (constrainPitch)
			{
			    if (Pitch > 89.0f)
			        Pitch = 89.0f;
			    if (Pitch < -89.0f)
			   	    Pitch = -89.0f;
			}

			// update Front, Right and Up Vectors using the updated Euler angles
			updateCameraVectors();   Pitch += smoothPitch * deltaTime;
		}
    }

    // processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void ProcessMouseScroll(float yoffset)
    {
        Fov -= (float)yoffset;
        if (Fov < 1.0f)
            Fov = 1.0f;
        if (Fov > 45.0f)
            Fov = 45.0f;
    }

private:
    // calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors()
    {
        // calculate the new Front vector
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        // also re-calculate the Right and Up vector
        Right = glm::normalize(glm::cross(Front, WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        Up = glm::normalize(glm::cross(Right, Front));
    }
};
#endif

