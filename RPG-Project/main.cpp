#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "camera.h"
#include "model.h"
#include "mesh.h"
#include "marchingcubes.h"

// forward declaration 
void processInput(GLFWwindow* window);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
unsigned int loadTexture(char const* path);

// settings
unsigned int SCR_WIDTH{ 800 };
unsigned int SCR_HEIGHT{ 600 };

bool vsync{ false };

GLboolean smoothMovement{ false };

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX{ SCR_WIDTH / 2.0f };
float lastY{ SCR_HEIGHT / 2.0f };
bool firstMouse{ true };

// timing
float deltaTime{ 0.0f };
float lastFrame{ 0.0f };

bool flyMode{};     // enables camera movement
bool flashlight{};

glm::mat4 projection = glm::mat4{ 1.0f };

int main()
{
	glfwInit();
    const char* glsl_version = "#version 330 core";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Unity de Cu eh Rola", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
    glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers

    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }    

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    //glfwSetKeyCallback(window, key_callback); // handle key press or release only once (not holding key)
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetKeyCallback(window, key_callback);

    glfwSwapInterval(0);
    glEnable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // ---------------------------------

    float vertices[] = {
		// positions          // normals           // texture coords
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
    };

    glm::vec3 pointLightPositions[] = {
         glm::vec3(0.7f,  0.2f,  2.0f)
    };

    unsigned int cubeVBO, cubeVAO;
	glGenBuffers(1, &cubeVBO);
    glGenVertexArrays(1, &cubeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindVertexArray(cubeVAO);

    unsigned int stride{ 8 };
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    Shader shaderDiffuse("shaders/diffuse.vert", "shaders/diffuse.frag");
    Shader shaderUnlit("shaders/unlit.vert", "shaders/unlit.frag");

    glm::vec3 ambientColor = glm::vec3(0.1f, 0.1f, 0.1f);
    glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);

    Model placeholderModel("resources/models/knight/Angel Knight.obj");

    stbi_set_flip_vertically_on_load(true);
    unsigned int diffuseMap      = loadTexture("resources/textures/RTScrate.png");
    unsigned int specularMap     = loadTexture("resources/textures/RTScrate_specular.png");

    shaderDiffuse.use();
    shaderDiffuse.setInt("material.diffuse", 0);
    shaderDiffuse.setInt("material.specular", 1);

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        camera.ProccessSmoothMovement(deltaTime);
        camera.ProccessSmoothMouseMovement(deltaTime);

        // shader activation
        shaderDiffuse.use();
		shaderDiffuse.setVec3("viewPos", camera.Position);
        shaderDiffuse.setFloat("material.shininess", 32.0f);
        shaderDiffuse.setFloat("time", currentFrame);
        
        // directional light
        shaderDiffuse.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
        shaderDiffuse.setVec3("dirLight.ambient", ambientColor);
        shaderDiffuse.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
        shaderDiffuse.setVec3("dirLight.specular", 1.0f, 1.0f, 1.0f);
        // point light 1
        shaderDiffuse.setVec3("pointLights[0].position", 0.7f,  0.2f,  2.0f);
        shaderDiffuse.setVec3("pointLights[0].ambient", ambientColor);
        shaderDiffuse.setVec3("pointLights[0].diffuse", lightColor);
        shaderDiffuse.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
        shaderDiffuse.setFloat("pointLights[0].constant", 1.0f);
        shaderDiffuse.setFloat("pointLights[0].linear", 0.09f);
        shaderDiffuse.setFloat("pointLights[0].quadratic", 0.032f);
        // spotLight
        shaderDiffuse.setVec3("spotLight.position", camera.Position);
        shaderDiffuse.setVec3("spotLight.direction", camera.Front);
        shaderDiffuse.setVec3("spotLight.ambient", ambientColor);
        shaderDiffuse.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
        shaderDiffuse.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
        shaderDiffuse.setFloat("spotLight.constant", 1.0f);
        shaderDiffuse.setFloat("spotLight.linear", 0.09f);
        shaderDiffuse.setFloat("spotLight.quadratic", 0.032f);
        shaderDiffuse.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
        shaderDiffuse.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));
        shaderDiffuse.setFloat("spotLight.on", flashlight);

        // view/projection transformations
        projection = glm::perspective(glm::radians(camera.Fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
        glm::mat4 view = camera.GetViewMatrix();
        shaderDiffuse.setMat4("projection", projection);
        shaderDiffuse.setMat4("view", view);

        // render placeholder model
        glm::mat4 model = glm::mat4(1.0f);      // identity matrix
        model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.2f));
        shaderDiffuse.setMat4("model", model);
        //placeholderModel.Draw(shaderDiffuse);
        
        model = glm::mat4(1.0f);      // identity matrix
        model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.0f));
        shaderDiffuse.setMat4("model", model);

        vSetTime(currentFrame * 0.25f);
        vMarchingCubes();

        glBindVertexArray(cubeVAO);
        shaderUnlit.use();
        shaderUnlit.setMat4("projection", projection);
        shaderUnlit.setMat4("view", view);
        for (int i{ 0 }; i < sizeof(pointLightPositions); i++)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(0.7f,  0.2f,  2.0f));
            model = glm::scale(model, glm::vec3(0.2f));
            shaderUnlit.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }


        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteBuffers(1, &cubeVBO);

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {  
        glfwSetWindowShouldClose(window, true); 
    }

    const float cameraSpeed = 5.0f * deltaTime; // adjust accordingly
    float cameraSpeedMultiplier = 1.0f;

    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    {
        camera.VelocityMultiplier = 2.5f;
    }
    else
    {
        camera.VelocityMultiplier = 1.0f;
    }

    if (flyMode)
    {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera.ProcessKeyboard(FORWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera.ProcessKeyboard(BACKWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera.ProcessKeyboard(LEFT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera.ProcessKeyboard(RIGHT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
            camera.ProcessKeyboard(DOWN, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
            camera.ProcessKeyboard(UP, deltaTime);
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_F && action == GLFW_PRESS)
    {
        flashlight = !flashlight;
    }

    if (key == GLFW_KEY_F8 && action == GLFW_PRESS)
    {
        smoothMovement = !smoothMovement;
        camera.SmoothMovement = smoothMovement;
        camera.ResetMovement();
    }

    if (key == GLFW_KEY_F7 && action == GLFW_PRESS)
    {
		if(fSample == fSample1)
		{
				fSample = fSample2;
		}
		else if(fSample == fSample2)
		{
				fSample = fSample3;
		}
		else
		{
				fSample = fSample1;
		}
    }

    // placeholder -----
    if (key == GLFW_KEY_R && action == GLFW_PRESS)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    if (key == GLFW_KEY_T && action == GLFW_PRESS)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    if (flyMode)
    {
        camera.ProcessMouseMovement(xoffset, yoffset);
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    {
        flyMode = true;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
    {
        flyMode = false;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (flyMode)
    {
        camera.ProcessMouseScroll(yoffset);
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    int tempWidth, tempHeight;
    glfwGetWindowSize(window, &tempWidth, &tempHeight);
    // check if window is not minimized first to avoid a crash
    if (tempWidth > 0 && tempHeight > 0)
    {
        SCR_WIDTH = tempWidth;
        SCR_HEIGHT = tempHeight;
    }
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
    projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
}

unsigned int loadTexture(char const* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format{};
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}
