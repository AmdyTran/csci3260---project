/*
Student Information
Student ID:
Student Names:
*/

//#include<windows.h>
#include <time.h>

#include "Dependencies/glew/glew.h"
#include "Dependencies/GLFW/glfw3.h"
#include "Dependencies/glm/glm.hpp"
#include "Dependencies/glm/gtc/matrix_transform.hpp"

//#define STB_IMAGE_IMPLEMENTATION
//#include "Dependencies/stb_image/stb_image.h"

#include "Shader.h"
#include "Texture.h"
#include "camera.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <map>

// screen setting
const int SCR_WIDTH = 800;
const int SCR_HEIGHT = 600;

GLint programID;

// global vars
float ship_x = 0, ship_y = 0, ship_z = 0;
float camX, camY, camZ;
float sens = 0.05f;
float ship_rotate; //degrees
int spotOn = 0; // spotlight on/off

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

// struct for storing the obj file
struct Vertex {
    glm::vec3 position;
    glm::vec2 uv;
    glm::vec3 normal;
};

struct Model {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
};

Model loadOBJ(const char* objPath)
{
    // function to load the obj file
    // Note: this simple function cannot load all obj files.

    struct V {
        // struct for identify if a vertex has showed up
        unsigned int index_position, index_uv, index_normal;
        bool operator == (const V& v) const {
            return index_position == v.index_position && index_uv == v.index_uv && index_normal == v.index_normal;
        }
        bool operator < (const V& v) const {
            return (index_position < v.index_position) ||
                (index_position == v.index_position && index_uv < v.index_uv) ||
                (index_position == v.index_position && index_uv == v.index_uv && index_normal < v.index_normal);
        }
    };

    std::vector<glm::vec3> temp_positions;
    std::vector<glm::vec2> temp_uvs;
    std::vector<glm::vec3> temp_normals;

    std::map<V, unsigned int> temp_vertices;

    Model model;
    unsigned int num_vertices = 0;

    std::cout << "\nLoading OBJ file " << objPath << "..." << std::endl;

    std::ifstream file;
    file.open(objPath);

    // Check for Error
    if (file.fail()) {
        std::cerr << "Impossible to open the file! Do you use the right path? See Tutorial 6 for details" << std::endl;
        exit(1);
    }

    while (!file.eof()) {
        // process the object file
        char lineHeader[128];
        file >> lineHeader;

        if (strcmp(lineHeader, "v") == 0) {
            // geometric vertices
            glm::vec3 position;
            file >> position.x >> position.y >> position.z;
            temp_positions.push_back(position);
        }
        else if (strcmp(lineHeader, "vt") == 0) {
            // texture coordinates
            glm::vec2 uv;
            file >> uv.x >> uv.y;
            temp_uvs.push_back(uv);
        }
        else if (strcmp(lineHeader, "vn") == 0) {
            // vertex normals
            glm::vec3 normal;
            file >> normal.x >> normal.y >> normal.z;
            temp_normals.push_back(normal);
        }
        else if (strcmp(lineHeader, "f") == 0) {
            // Face elements
            V vertices[3];
            for (int i = 0; i < 3; i++) {
                char ch;
                file >> vertices[i].index_position >> ch >> vertices[i].index_uv >> ch >> vertices[i].index_normal;
            }

            // Check if there are more than three vertices in one face.
            std::string redundency;
            std::getline(file, redundency);
            if (redundency.length() >= 5) {
                std::cerr << "There may exist some errors while load the obj file. Error content: [" << redundency << " ]" << std::endl;
                std::cerr << "Please note that we only support the faces drawing with triangles. There are more than three vertices in one face." << std::endl;
                std::cerr << "Your obj file can't be read properly by our simple parser :-( Try exporting with other options." << std::endl;
                exit(1);
            }

            // change to opengl format
            
            for (int i = 0; i < 3; i++) {
                if (temp_vertices.find(vertices[i]) == temp_vertices.end()) {
                    // the vertex never shows before
                    Vertex vertex;
                    vertex.position = temp_positions[vertices[i].index_position - 1];
                    vertex.uv = temp_uvs[vertices[i].index_uv - 1];
                    vertex.normal = temp_normals[vertices[i].index_normal - 1];

                    model.vertices.push_back(vertex);
                    model.indices.push_back(num_vertices);
                    temp_vertices[vertices[i]] = num_vertices;
                    num_vertices += 1;
                }
                else {
                    // reuse the existing vertex
                    unsigned int index = temp_vertices[vertices[i]];
                    model.indices.push_back(index);
                }
            } // for
        } // else if
        else {
            // it's not a vertex, texture coordinate, normal or face
            char stupidBuffer[1024];
            file.getline(stupidBuffer, 1024);
        }
    }
    file.close();

    std::cout << "There are " << num_vertices << " vertices in the obj file.\n" << std::endl;
    return model;
}

float yaw = -90.0f;
float pitch = 17.0f;
//float pitch = 0.0f;

double xpos = 400, ypos = 300;

void get_OpenGL_info()
{
    // OpenGL information
    const GLubyte* name = glGetString(GL_VENDOR);
    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* glversion = glGetString(GL_VERSION);
    std::cout << "OpenGL company: " << name << std::endl;
    std::cout << "Renderer name: " << renderer << std::endl;
    std::cout << "OpenGL version: " << glversion << std::endl;
}

bool checkStatus(
    GLuint objectID,
    PFNGLGETSHADERIVPROC objectPropertyGetterFunc,
    PFNGLGETSHADERINFOLOGPROC getInfoLogFunc,
    GLenum statusType)
{
    GLint status;
    objectPropertyGetterFunc(objectID, statusType, &status);
    if (status != GL_TRUE)
    {
        GLint infoLogLength;
        objectPropertyGetterFunc(objectID, GL_INFO_LOG_LENGTH, &infoLogLength);
        GLchar* buffer = new GLchar[infoLogLength];

        GLsizei bufferSize;
        getInfoLogFunc(objectID, infoLogLength, &bufferSize, buffer);
        std::cout << buffer << std::endl;

        delete[] buffer;
        return false;
    }
    return true;
}

Model Spacecraft;
GLuint vaoSpacecraft, vboSpacecraft, eboSpacecraft;
Texture textureSpacecraft;
Texture textureSpacecraftOn;

void loadSpacecraft() {
    Spacecraft = loadOBJ("./instances/object/spacecraft.obj");
    textureSpacecraft.setupTexture("./instances/texture/spacecraftTexture.bmp");
    textureSpacecraftOn.setupTexture("./instances/texture/spacecraftTexture2.bmp");

    // VAO
    glGenVertexArrays(1, &vaoSpacecraft);
    glBindVertexArray(vaoSpacecraft);

    // VBO
    glGenBuffers(1, &vboSpacecraft);
    glBindBuffer(GL_ARRAY_BUFFER, vboSpacecraft);
    glBufferData(GL_ARRAY_BUFFER, Spacecraft.vertices.size() * sizeof(Vertex), &Spacecraft.vertices[0], GL_STATIC_DRAW);

    // EBO
    glGenBuffers(1, &eboSpacecraft);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboSpacecraft);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, Spacecraft.indices.size() * sizeof(unsigned int), &Spacecraft.indices[0], GL_STATIC_DRAW);

    // send to shader: position, uv, normals
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
}


Model Craft;
GLuint vaoCraft, vboCraft, eboCraft;
Texture textureCraft;

void loadCraft() {
    Craft = loadOBJ("./instances/object/craft2.obj");
    textureCraft.setupTexture("./instances/texture/vehicleTexture.bmp");

    // VAO
    glGenVertexArrays(1, &vaoCraft);
    glBindVertexArray(vaoCraft);

    // VBO
    glGenBuffers(1, &vboCraft);
    glBindBuffer(GL_ARRAY_BUFFER, vboCraft);
    glBufferData(GL_ARRAY_BUFFER, Craft.vertices.size() * sizeof(Vertex), &Craft.vertices[0], GL_STATIC_DRAW);

    // EBO
    glGenBuffers(1, &eboCraft);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboCraft);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, Craft.indices.size() * sizeof(unsigned int), &Craft.indices[0], GL_STATIC_DRAW);

    // send to shader: position, uv, normals
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
}

Model Planet;
GLuint vaoPlanet, vboPlanet, eboPlanet;
Texture texturePlanet, normalPlanet;

void loadPlanet() {
    Planet = loadOBJ("./instances/object/planet.obj");
    texturePlanet.setupTexture("./instances/texture/earthTexture.bmp");
    normalPlanet.setupTexture("./instances/texture/earthNormal.bmp");

    // VAO
    glGenVertexArrays(1, &vaoPlanet);
    glBindVertexArray(vaoPlanet);

    // VBO
    glGenBuffers(1, &vboPlanet);
    glBindBuffer(GL_ARRAY_BUFFER, vboPlanet);
    glBufferData(GL_ARRAY_BUFFER, Planet.vertices.size() * sizeof(Vertex), &Planet.vertices[0], GL_STATIC_DRAW);

    // EBO
    glGenBuffers(1, &eboPlanet);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboPlanet);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, Planet.indices.size() * sizeof(unsigned int), &Planet.indices[0], GL_STATIC_DRAW);

    // send to shader: position, uv, normals
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
}

Model Rock;
GLuint vaoRock, vboRock, eboRock;
Texture textureRock;

void loadRock() {
    Rock = loadOBJ("./instances/object/rock.obj");
    textureRock.setupTexture("./instances/texture/rockTexture.bmp");

    // VAO
    glGenVertexArrays(1, &vaoRock);
    glBindVertexArray(vaoRock);

    // VBO
    glGenBuffers(1, &vboRock);
    glBindBuffer(GL_ARRAY_BUFFER, vboRock);
    glBufferData(GL_ARRAY_BUFFER, Rock.vertices.size() * sizeof(Vertex), &Rock.vertices[0], GL_STATIC_DRAW);

    // EBO
    glGenBuffers(1, &eboRock);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboRock);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, Rock.indices.size() * sizeof(unsigned int), &Rock.indices[0], GL_STATIC_DRAW);

    // send to shader: position, uv, normals
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
}


GLuint vaoSkybox, vboSkybox;
Texture textureSkybox;


// we don't need an ebo, since we are not drawing by indexing

void loadSkybox() {
    GLfloat skyboxVertices[] = {
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    glGenVertexArrays(1, &vaoSkybox);
    glBindVertexArray(vaoSkybox);

    glGenBuffers(1, &vboSkybox);
    glBindBuffer(GL_ARRAY_BUFFER, vboSkybox);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);

    // We only have the indices, no normals, or uvs
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glBindVertexArray(0);

    std::vector<std::string> earth_faces;
    earth_faces.push_back("./instances/skybox/right.bmp");
    earth_faces.push_back("./instances/skybox/left.bmp");
    earth_faces.push_back("./instances/skybox/bottom.bmp");
    earth_faces.push_back("./instances/skybox/top.bmp");
    earth_faces.push_back("./instances/skybox/back.bmp");
    earth_faces.push_back("./instances/skybox/front.bmp");
    textureSkybox.loadSkybox(earth_faces);
    
}

// Make sure to match this with the shader! Else run into issues.
unsigned int amountAsteroids = 500;
glm::mat4* modelMatrices = new glm::mat4[amountAsteroids];

float offset = 2.5f;
float radius = 10.0;
float startingTime = glfwGetTime();

void asteroidGenerator() {
    srand(startingTime);
    for (unsigned int i = 0; i < amountAsteroids; i++)
    {
        glm::mat4 model = glm::mat4(1.0f);
        // 1. translation: displace along circle with 'radius' in range [-offset, offset]
        float angle = (float)i / (float)amountAsteroids * 360.0f;
        float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
        float x = sin(angle) * radius + displacement;
        displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
        float y = displacement * 0.4f; // keep height of field smaller compared to width of x and z
        displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
        float z = cos(angle) * radius + displacement;
        model = glm::translate(model, glm::vec3(x, y, z));

        // 2. scale: scale between 0.05 and 0.25f
        float scale = (rand() % 20) / 100.0f + 0.05;
        model = glm::scale(model, glm::vec3(scale));

        // 3. rotation: add random rotation around a (semi)randomly picked rotation axis vector
        float rotAngle = (rand() % 360) + (float) glfwGetTime()*-0.7;
        model = glm::rotate(model, rotAngle , glm::vec3(0.4f, 0.6f, 0.8f));

        // 4. now add to list of matrices
        modelMatrices[i] = model;
    }
}


void sendDataToOpenGL()
{
    //Load objects and textures
    loadSpacecraft();
    loadRock();
    loadPlanet();
    loadCraft();
    loadSkybox();
    asteroidGenerator();
}


Shader shader;
Shader skyboxShader;
Shader asteroidShader;
Shader shipShader;

glm::vec3 camPos;
glm::vec3 look;

void initializedGL(void) //run only once
{
    if (glewInit() != GLEW_OK) {
        std::cout << "GLEW not OK." << std::endl;
    }
    get_OpenGL_info();
    sendDataToOpenGL();

    shader.setupShader("./VertexShaderCode.glsl", "./FragmentShaderCode.glsl");
    skyboxShader.setupShader("./skyboxVS.glsl", "./skyboxFS.glsl");
    asteroidShader.setupShader("./asteroidVS.glsl", "./asteroidFS.glsl");
    shipShader.setupShader("./shipVS.glsl", "./shipFS.glsl");

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE);
}

void matrix(std::string object) {
    glm::mat4 rotation = glm::mat4(1.0f);
    glm::mat4 scaling = glm::mat4(1.0f);
    glm::mat4 transform = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);
    glm::mat4 model = glm::mat4(1.0f);

    glm::mat4 view = glm::mat4(1.0f);
    float self_rotate = (float) glfwGetTime() * 0.2;

    
    // for camera:
    glm::mat4 sTransform;
    glm::mat4 sRotation;

    if (object == "Spacecraft") {
        scaling = glm::scale(glm::mat4(1.0f), glm::vec3(0.01f, 0.01f, 0.01f));
        transform = glm::translate(glm::mat4(1.0f), glm::vec3(ship_x, ship_y, ship_z));
        rotation = glm::rotate(glm::mat4(1.0f), 0.07f * ship_rotate, glm::vec3(0.0f, 1.0f, 0.0f));
        sTransform = transform;
        sRotation = rotation;
    }
    else if (object == "Craft") {
        scaling = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.5f));
        transform = glm::translate(glm::mat4(1.0f), glm::vec3(3.0f, 0.0f, 15.0f));
        rotation = glm::rotate(glm::mat4(1.0f), self_rotate, glm::vec3(0.0f, 1.0f, 0.0f));
    }
    else if (object == "Planet") {
        scaling = glm::scale(glm::mat4(1.0f), glm::vec3(5.0f, 5.0f, 5.0f));
        transform = glm::translate(glm::mat4(1.0f), glm::vec3(3.0f, 0.0f, 100.0f));
        rotation = glm::rotate(glm::mat4(1.0f), self_rotate, glm::vec3(0.0f, 1.0f, 0.0f));
    }
    else if (object == "Skybox") {
        view = glm::mat4(glm::mat3(view)); // get rid of last row and column
        // can reuse projection matrix
    }
    else if (object == "Asteroids") {
        //transform = glm::translate(glm::mat4(1.0f), glm::vec3(3.0f, 0.0f, 100.0f));
        rotation = glm::rotate(glm::mat4(1.0f), self_rotate, glm::vec3(0.0f, 1.0f, 0.0f));
    }

    else {
        // pass: do nothing, if you want another object, make an else if statement
    }
    
    glm::mat4 tempRotate = glm::rotate(glm::mat4(1.0f), glm::radians(ship_rotate * 3.5f), glm::vec3(0, 1, 0));

    
    camPos = glm::vec3(ship_x + 20 * -sin(glm::radians(ship_rotate * 3.5f)), 10.0f, ship_z + 20 * -cos(glm::radians(ship_rotate * 3.5f)));
    
    look = glm::vec3(ship_x, 5.0f, ship_z) + glm::vec3(tempRotate * glm::vec4(0.0f, 0.0f, 0.f, 1.0f));


    // projection and view matrices
    projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 400.0f);

    view = glm::lookAt(
            camPos, //cam
            look, //look
            glm::vec3(0, 1, 0)
        );
    

    // Now send it to shader, but if it s a skybox we want to send it to skyboxShader
    if (object == "Skybox") {
        view = glm::mat4(glm::mat3(view)); // get rid of last row and column
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);
    }
    else if (object == "Asteroids") {
        asteroidShader.setMat4("view", view);
        asteroidShader.setMat4("projection", projection);
        asteroidShader.setMat4("transform", transform);
        asteroidShader.setMat4("rotation", rotation);
    }
    else if (object == "Spacecraft"){
        shipShader.setMat4("view", view);
        shipShader.setMat4("projection", projection);
        shipShader.setMat4("transform", transform);
        shipShader.setMat4("rotation", rotation);
        shipShader.setMat4("scaling", scaling);
    }
    else {
        //// Send the matrices to the shader
        shader.setMat4("rotationMatrix", rotation);
        shader.setMat4("scalingMatrix", scaling);
        shader.setMat4("transformMatrix", transform);
        shader.setMat4("projectionMatrix", projection);
        shader.setMat4("viewMatrix", view);

    }
}


void paintGL(void)
{
    // always run
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);  //specify the background color
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    unsigned int slot = 0;

    
    // diffuse lights location
    glm::vec3 lightPosition = glm::vec3(20.0f, 20.0f, 50.0f);
    // flashlight position same as eyePosition
    // flashlight spotlight direction:
    glm::vec3 spotDir = glm::vec3(0, 0, 1);
    // adjust angles
    float cutoff1 = glm::cos(glm::radians(1.5f));
    float cutoff2 =glm::cos(glm::radians(3.0f));
    
    // spacecraft shader

    shipShader.use();
    matrix("Spacecraft");
    glBindVertexArray(vaoSpacecraft);
    if (spotOn == 0) {
        textureSpacecraft.bind(0);
    }
    else {
        textureSpacecraftOn.bind(0);
    }
    shipShader.setInt("textureSpacecraft", 0);
    shipShader.setInt("myTextureSampler0", 0);
    shipShader.setVec3("LightPosition", lightPosition);
    shipShader.setVec3("eyePosition", camPos);
    glDrawElements(GL_TRIANGLES, Spacecraft.indices.size(), GL_UNSIGNED_INT, 0);

    // main shader
    shader.use();
    
    matrix("Planet");
    glBindVertexArray(vaoPlanet);
    texturePlanet.bind(0);
    shader.setInt("myTextureSampler0", 0);
    normalPlanet.bind(1);
    shader.setInt("myTextureSampler1", 1);
    shader.setInt("normalMap", 1);
    shader.setVec3("LightPositionWorld", lightPosition);
    shader.setVec3("eyePosition", camPos);
    shader.setVec3("spotDirection", spotDir);
    shader.setInt("spotOn", spotOn);
    shader.setFloat("innerCutoff", cutoff1);
    shader.setFloat("outerCutoff", cutoff2);
    glDrawElements(GL_TRIANGLES, Planet.indices.size(), GL_UNSIGNED_INT, 0);


    matrix("Craft");
    glBindVertexArray(vaoCraft);
    textureCraft.bind(0);
    shader.setInt("myTextureSampler0", 0);
    shader.setInt("normalMap", 0);
    shader.setVec3("LightPositionWorld", lightPosition);
    shader.setVec3("eyePosition", camPos);
    shader.setVec3("spotDirection", spotDir);
    shader.setInt("spotOn", spotOn);
    shader.setFloat("innerCutoff", cutoff1);
    shader.setFloat("outerCutoff", cutoff2);
    glDrawElements(GL_TRIANGLES, Craft.indices.size(), GL_UNSIGNED_INT, 0);


    // Now: draw asteroids, we use the asteroid shader for it to keep code seperate.
    asteroidShader.use();
    glBindVertexArray(vaoRock);
    
    matrix("Asteroids");
    asteroidGenerator();
    for (int i = 0; i < amountAsteroids; i++) {
        asteroidShader.setMat4(("model[" + std::to_string(i) + "]"), modelMatrices[i]);
    }
    
    textureRock.bind(0);
    asteroidShader.setInt("textureAsteroid", 0);
    asteroidShader.setVec3("LightPosition", lightPosition);
    asteroidShader.setVec3("eyePosition", camPos);
    glDrawElementsInstanced(GL_TRIANGLES, Rock.indices.size(), GL_UNSIGNED_INT, 0, amountAsteroids);


    // Now: draw skybox, make sure to keep it last
    glDepthFunc(GL_LEQUAL);
    skyboxShader.use();
    glBindVertexArray(vaoSkybox);
    matrix("Skybox");
    textureSkybox.bindSkybox(2);
    skyboxShader.setInt("skybox", 2);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glDepthFunc(GL_LESS);
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{

}

void cursor_position_callback(GLFWwindow* window, double x, double y)
{
    // new: always responsive to mouse position
    
    ship_rotate += sens * (xpos - x);
    // keep values between -90 and 90
    ship_rotate = glm::clamp(ship_rotate, -179.0f/3.5f, 179.0f/3.5f);
    xpos = x;
    

}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    float s = 0.5f;
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    
    if (key == GLFW_KEY_UP && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
        // move ship forwards
        // div 57.3 to convert to radians
        ship_z += s * cos(ship_rotate * 3.5f/57.3f);
        ship_x += s * sin(ship_rotate * 3.5f/57.3f);
    }
    if (key == GLFW_KEY_DOWN && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
        // move ship backwards
        ship_x -= s * sin(ship_rotate * 3.5f/57.3f);
        ship_z -= s * cos(ship_rotate * 3.5f/57.3f);
    }
    if (key == GLFW_KEY_RIGHT && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
        //
        ship_x -= s * cos(ship_rotate * 3.5f/57.3f);
    }
    if (key == GLFW_KEY_LEFT && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
        ship_x += s * cos(ship_rotate * 3.5f/57.3f);
    }
    
    if (key == GLFW_KEY_Q && action == GLFW_PRESS) {
        spotOn = (spotOn + 1) % 2;
    }
        
}

int main(int argc, char* argv[])
{
    GLFWwindow* window;

    /* Initialize the glfw */
    if (!glfwInit()) {
        std::cout << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    /* glfw: configure; necessary for MAC */
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Assignment 2", NULL, NULL);
    if (!window) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    
    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    /*register callback functions*/
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);                                                                  //
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    initializedGL();
    
    // load textures once


    // Variables to create periodic event for FPS displaying
    double prevTime = 0.0;
    double crntTime = 0.0;
    double timeDiff;
    // Keeps track of the amount of frames in timeDiff
    unsigned int counter = 0;

    while (!glfwWindowShouldClose(window)) {

        clock_t tStart = clock();
        /* Render here */
        paintGL();

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();


//        printf("Time taken: %.2fs\n", (double)(clock() - tStart) / CLOCKS_PER_SEC);
    }

    glfwTerminate();

    return 0;
}
